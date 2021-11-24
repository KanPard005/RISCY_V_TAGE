#include "ooo_cpu.h"

#define TAGE_BIMODAL_TABLE_SIZE 16384
#define TAGE_PREDICTOR_TABLE_SIZE 1024
#define TAGE_NUM_COMPONENTS 12
#define TAGE_BASE_COUNTER_BITS 2
#define TAGE_COUNTER_BITS 3
#define TAGE_USEFUL_BITS 2
#define TAGE_TAG_BITS 9
#define TAGE_GLOBAL_HISTORY_BUFFER_LENGTH 1024
#define TAGE_PATH_HISTORY_BUFFER_LENGTH 32
#define TAGE_MIN_HISTORY_LENGTH 4
#define TAGE_HISTORY_ALPHA 1.6
#define TAGE_RESET_USEFUL_INTERVAL 512000

struct tage_predictor_table_entry
{
    uint8_t ctr; // range 0-7
    uint16_t tag;
    uint8_t useful; // range 0-3
};

class Tage
{
private:
    /* data */
    int num_branches;
    int bimodal_table[TAGE_BIMODAL_TABLE_SIZE]; // range 0-3
    struct tage_predictor_table_entry predictor_table[TAGE_NUM_COMPONENTS][TAGE_PREDICTOR_TABLE_SIZE];
    uint8_t global_history[TAGE_GLOBAL_HISTORY_BUFFER_LENGTH];
    uint8_t path_history[TAGE_PATH_HISTORY_BUFFER_LENGTH];
    uint8_t use_alt_on_na;  // 4 bit counter
    int component_history_lengths[TAGE_NUM_COMPONENTS];

public:
    void init();
    uint8_t predict(uint64_t ip);
    void update(uint64_t ip, uint8_t taken);

    uint16_t get_bimodal_index(uint64_t ip);
    uint16_t get_predictor_index(uint64_t ip, int component);
    uint16_t get_tag(uint64_t ip, int component);
    int get_match_below_n(uint64_t ip, int component);

    Tage(/* args */);
    ~Tage();
};

void Tage::init()
{
    use_alt_on_na = 8;
    for (int i = 0; i < TAGE_BIMODAL_TABLE_SIZE; i++)
    {
        bimodal_table[i] = (1 << (TAGE_BASE_COUNTER_BITS - 1)); // weakly taken
    }
    for (int i = 0; i < TAGE_NUM_COMPONENTS; i++)
    {
        for (int j = 0; j < TAGE_PREDICTOR_TABLE_SIZE; j++)
        {
            predictor_table[i][j].ctr = (1 << (TAGE_COUNTER_BITS - 1)); // weakly taken
            predictor_table[i][j].useful = 0;                           // not useful
            predictor_table[i][j].tag = 0;
        }
    }

    double power = 1;
    for (int i = 0; i < TAGE_NUM_COMPONENTS; i++)
    {
        component_history_lengths[i] = int(TAGE_MIN_HISTORY_LENGTH * power + 0.5);
        power *= TAGE_HISTORY_ALPHA;
    }

    num_branches = 0;
}

uint8_t Tage::predict(uint64_t ip)
{

    int provider = get_match_below_n(ip, TAGE_NUM_COMPONENTS + 1);
    int alternate = get_match_below_n(ip, provider);
    if (provider == 0)
    {
        uint16_t index = get_bimodal_index(ip);
        return bimodal_table[index] >= (1 << (TAGE_BASE_COUNTER_BITS - 1));
    }
    else
    {
        uint16_t index = get_predictor_index(ip, provider);
        if (use_alt_on_na < 8 || abs(2*predictor_table[provider - 1][index].ctr - ((1 << (TAGE_COUNTER_BITS))-1)) > 1){
            return predictor_table[provider - 1][index].ctr >= (1 << (TAGE_COUNTER_BITS - 1));
        }
        else{
            return predictor_table[alternate - 1][get_predictor_index(ip, alternate)].ctr >= (1 << (TAGE_COUNTER_BITS - 1));
        }
        
    }
}

void Tage::update(uint64_t ip, uint8_t taken)
{

    int pred_component = get_match_below_n(ip, TAGE_NUM_COMPONENTS + 1);
    int altpred_component = get_match_below_n(ip, pred_component);

    uint8_t pred, alt_pred;

    if (pred_component > 0)
    {
        struct tage_predictor_table_entry *entry = &predictor_table[pred_component - 1][get_predictor_index(ip, pred_component)];
        pred = (entry->ctr >= (1 << (TAGE_COUNTER_BITS - 1))) ? 1 : 0;
        uint8_t useful = entry->useful;

        if (altpred_component > 0)
        {
            struct tage_predictor_table_entry *alt_entry = &predictor_table[altpred_component - 1][get_predictor_index(ip, altpred_component)];
            alt_pred = (alt_entry->ctr >= (1 << (TAGE_COUNTER_BITS - 1))) ? 1 : 0;

            // update ctr for alternate predictor
            if (useful == 0)
            {
                if (taken == 1)
                {
                    if (alt_entry->ctr < ((1 << TAGE_COUNTER_BITS) - 1))
                        alt_entry->ctr++;
                }
                else
                {
                    if (alt_entry->ctr > 0)
                        alt_entry->ctr--;
                }
            }
        }

        else
        {
            uint16_t index = get_bimodal_index(ip);
            int ctr = bimodal_table[index];
            alt_pred = (ctr >= (1 << (TAGE_BASE_COUNTER_BITS - 1))) ? 1 : 0;

            // update ctr for alternate predictor
            if (useful == 0)
            {
                if (taken == 1)
                {
                    if (bimodal_table[index] < ((1 << TAGE_BASE_COUNTER_BITS) - 1))
                        bimodal_table[index]++;
                }
                else
                {
                    if (bimodal_table[index] > 0)
                        bimodal_table[index]--;
                }
            }
        }

        // update u
        if (pred != alt_pred)
        {
            if (pred == taken)
            {
                if (entry->useful < ((1 << TAGE_USEFUL_BITS) - 1))
                    entry->useful++;
                if (use_alt_on_na > 0)
                    use_alt_on_na--;
            }
            else
            {
                if (entry->useful > 0)
                    entry->useful--;
                if (use_alt_on_na < 15)
                    use_alt_on_na++;
            }
        }

        // update ctr for predictor
        if (taken == 1)
        {
            if (entry->ctr < ((1 << TAGE_COUNTER_BITS) - 1))
                entry->ctr++;
        }
        else
        {
            if (entry->ctr > 0)
                entry->ctr--;
        }

        // allocate tagged entries on misprediction

        if (pred != taken)
        {
            uint8_t rand = random();
            int Y = rand & ((1 << (TAGE_NUM_COMPONENTS - pred_component - 1)) - 1);
            int X = pred_component + 1;

            if (Y & 1)
            {
                X++;
                if (Y & 2)
                {
                    X++;
                }
            }

            for (int i = X; i <= TAGE_NUM_COMPONENTS; i++)
            {
                struct tage_predictor_table_entry *entry_new = &predictor_table[X - 1][get_predictor_index(ip, X)];
                if (entry_new->useful == 0)
                {
                    entry_new->tag = get_tag(ip, X);
                    entry_new->ctr = (1 << (TAGE_COUNTER_BITS - 1));
                }
            }
        }
    }

    else
    {
        // update ctr for predictor
        uint16_t index = get_bimodal_index(ip);
        if (taken == 1)
        {
            if (bimodal_table[index] < ((1 << TAGE_BASE_COUNTER_BITS) - 1))
                bimodal_table[index]++;
        }
        else
        {
            if (bimodal_table[index] > 0)
                bimodal_table[index]--;
        }

        // allocate tagged entries on misprediction

        uint8_t pred = (bimodal_table[index] >= (1 << (TAGE_BASE_COUNTER_BITS - 1))) ? 1 : 0;
        if (pred != taken)
        {
            uint8_t rand = random();
            int Y = rand & ((1 << (TAGE_NUM_COMPONENTS - pred_component - 1)) - 1);
            int X = pred_component + 1;

            if (Y & 1)
            {
                X++;
                if (Y & 2)
                {
                    X++;
                }
            }

            for (int i = X; i <= TAGE_NUM_COMPONENTS; i++)
            {
                struct tage_predictor_table_entry *entry_new = &predictor_table[X - 1][get_predictor_index(ip, X)];
                if (entry_new->useful == 0)
                {
                    entry_new->tag = get_tag(ip, X);
                    entry_new->ctr = (1 << (TAGE_COUNTER_BITS - 1));
                }
            }
        }
    }

    // update global history
    for (int i = TAGE_GLOBAL_HISTORY_BUFFER_LENGTH - 1; i > 0; i--)
    {
        global_history[i] = global_history[i - 1];
    }
    global_history[0] = taken;

    // update path history
    for (int i = TAGE_PATH_HISTORY_BUFFER_LENGTH - 1; i > 0; i--)
    {
        path_history[i] = path_history[i - 1];
    }
    path_history[0] = ip & 1;

    num_branches++;
    if (num_branches % TAGE_RESET_USEFUL_INTERVAL == 0)
    {
        num_branches = 0;
        for (int i = 0; i < TAGE_NUM_COMPONENTS; i++)
        {
            for (int j = 0; j < TAGE_PREDICTOR_TABLE_SIZE; j++)
            {
                predictor_table[i][j].useful >>= 1;
            }
        }
    }
}

uint16_t Tage::get_bimodal_index(uint64_t ip)
{
    return ip & (TAGE_BIMODAL_TABLE_SIZE - 1);
}

uint16_t Tage::get_predictor_index(uint64_t ip, int component)
{
    component--;

    uint16_t compressed_history = 0;
    uint16_t temporary_history = 0;
    int compressed_history_length = lg2(TAGE_PREDICTOR_TABLE_SIZE);
    for (int i = 0; i < component_history_lengths[component]; i++)
    {
        if (i % compressed_history_length == 0)
        {
            compressed_history ^= temporary_history;
            temporary_history = 0;
        }
        temporary_history = (temporary_history << 1) | global_history[i];
    }
    compressed_history ^= temporary_history;

    return (compressed_history ^ ip ^ (ip >> compressed_history_length)) & (TAGE_PREDICTOR_TABLE_SIZE - 1);
}

uint16_t Tage::get_tag(uint64_t ip, int component)
{
    component--;

    uint16_t compressed_history = 0;
    uint16_t temporary_history = 0;
    int compressed_history_length = TAGE_TAG_BITS;
    for (int i = 0; i < component_history_lengths[component]; i++)
    {
        if (i % compressed_history_length == 0)
        {
            compressed_history ^= temporary_history;
            temporary_history = 0;
        }
        temporary_history = (temporary_history << 1) | global_history[i];
    }
    compressed_history ^= temporary_history;

    return (compressed_history ^ ip) & ((1 << TAGE_TAG_BITS) - 1);
}

int Tage::get_match_below_n(uint64_t ip, int component)
{
    for (int i = component - 1; i >= 1; i--)
    {
        uint16_t index = get_predictor_index(ip, i);
        uint16_t tag = get_tag(ip, i);

        if (predictor_table[i - 1][index].tag == tag)
        {
            return i;
        }
    }

    return 0;
}

Tage::Tage()
{
}

Tage::~Tage()
{
}
