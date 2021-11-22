#include "ooo_cpu.h"

#define TAGE_BIMODAL_TABLE_SIZE 16384
#define TAGE_PREDICTOR_TABLE_SIZE 1024
#define TAGE_NUM_COMPONENTS 4
#define TAGE_BASE_COUNTER_BITS 2
#define TAGE_COUNTER_BITS 3  
#define TAGE_USEFUL_BITS 2
#define TAGE_TAG_BITS 9
#define TAGE_HISTORY_BUFFER_LENGTH 1024
#define TAGE_MIN_HISTORY_LENGTH 4
#define TAGE_HISTORY_ALPHA 2

struct tage_predictor_table_entry{
    uint8_t ctr;  // range 0-7
    uint16_t tag; 
    uint8_t useful;  // range 0-3 
};

class Tage
{
private:
    /* data */
    int bimodal_table[NUM_CPUS][TAGE_BIMODAL_TABLE_SIZE];  // range 0-3
    struct tage_predictor_table_entry predictor_table[NUM_CPUS][TAGE_NUM_COMPONENTS][TAGE_PREDICTOR_TABLE_SIZE];
    uint8_t global_history[NUM_CPUS][TAGE_HISTORY_BUFFER_LENGTH];
    int component_history_lengths[TAGE_NUM_COMPONENTS]; 

public:
    void init(uint32_t cpu);
    uint8_t predict(uint32_t cpu, uint64_t ip);
    void update(uint32_t cpu, uint64_t ip, uint8_t taken);
    
    uint16_t get_bimodal_index(uint32_t cpu, uint64_t ip);
    uint16_t get_predictor_index(uint32_t cpu, uint64_t ip, int component);
    uint16_t get_tag(uint32_t cpu, uint64_t ip, int component);
    int get_match_below_n(uint32_t cpu, uint64_t ip, int component);

    Tage(/* args */);
    ~Tage();
};

void Tage::init(uint32_t cpu){
    for (int i=0; i < TAGE_BIMODAL_TABLE_SIZE; i++){
        bimodal_table[cpu][i] = 2;  // weakly taken
    }
    for (int i=0; i < TAGE_NUM_COMPONENTS; i++){
        for (int j=0; j < TAGE_PREDICTOR_TABLE_SIZE; j++){
            predictor_table[cpu][i][j].ctr = 4;  // weakly taken
            predictor_table[cpu][i][j].useful = 0;  // not useful
            predictor_table[cpu][i][j].tag = 0;
        }
    }

    component_history_lengths[0] = TAGE_MIN_HISTORY_LENGTH;
    int power = 1;
    for(int i=1;i<TAGE_NUM_COMPONENTS;i++){
        power *= TAGE_HISTORY_ALPHA;
        component_history_lengths[i] = int(component_history_lengths[i-1]*power + 0.5);
    }
}


uint8_t Tage::predict(uint32_t cpu, uint64_t ip){

    int provider = get_match_below_n(cpu,ip,TAGE_NUM_COMPONENTS + 1);
    //int alternate = get_match_below_n(cpu, ip, provider);
    if(provider == 0)
    {
        uint16_t index = get_bimodal_index(cpu,ip);
        return bimodal_table[cpu][index] >= (1 << (TAGE_BASE_COUNTER_BITS - 1));
    }
    else
    {
        uint16_t index = get_predictor_index(cpu,ip,provider);
        return predictor_table[cpu][provider-1][index].ctr >= (1 << (TAGE_COUNTER_BITS - 1));
    }
}

void Tage::update(uint32_t cpu, uint64_t ip, uint8_t taken){

    int pred_component = get_match_below_n(cpu, ip, TAGE_NUM_COMPONENTS+1);
    int altpred_component = get_match_below_n(cpu, ip, pred_component);

    uint8_t pred, alt_pred;

    if (pred_component > 0){
        struct tage_predictor_table_entry* entry = &predictor_table[cpu][pred_component-1][get_predictor_index(cpu, ip, pred_component)];
        pred = (entry->ctr >= 4) ? 1 : 0;
        uint8_t useful = entry->useful;

        if (alt_pred > 0){
            entry = &predictor_table[cpu][altpred_component-1][get_predictor_index(cpu, ip, altpred_component)];
            alt_pred = (entry->ctr >= 4) ? 1 : 0;

            // update ctr for alternate predictor
            if (useful == 0){
                if (taken == 1){
                    if (entry->ctr < 7) entry->ctr++;
                }
                else{
                    if (entry->ctr > 0) entry->ctr--;
                }
            }
        }

        else{
            uint8_t index = get_bimodal_index(cpu, ip);
            int ctr = bimodal_table[cpu][index];
            alt_pred = (ctr >= 2) ? 1 : 0;

            // update ctr for alternate predictor
            if (useful == 0){
                if (bimodal_table[cpu][index] < 3) bimodal_table[cpu][index]++;
            }
            else{
                if (bimodal_table[cpu][index] > 0) bimodal_table[cpu][index]--;
            }
        }

        // update u
        if (pred != alt_pred){
            if (pred == taken) {
                if (entry->useful < 3) entry->useful++;
            }
            else{
                if (entry->useful > 0) entry->useful--;
            }
        }

        // update ctr for predictor
        if (taken == 1){
            if (entry->ctr < 7) entry->ctr++;
        }
        else{
            if (entry->ctr > 0) entry->ctr--;
        }

        // allocate tagged entries on misprediction

        if (pred != taken){
            uint8_t rand = random();
            int Y = rand & ((1 << (TAGE_NUM_COMPONENTS - pred_component)) - 1);
            int X = pred_component + 1;

            if (Y & 1){
                X++;
                if (Y & 2){
                    X++;
                }
            }

            for (int i = X; i <= TAGE_NUM_COMPONENTS; i++){
                entry = &predictor_table[cpu][X-1][get_predictor_index(cpu, ip, X)];
                if (entry->useful == 0){
                    entry->tag = get_tag(cpu, ip, X);
                    entry->ctr = (entry->ctr >= 4) ? 1 : 0;
                }
            }

        }        
    }

    else{
        // update ctr for predictor
        uint8_t index = get_bimodal_index(cpu, ip);
        if (taken == 1){
            if (bimodal_table[cpu][index] < 3) bimodal_table[cpu][index]++;
        }
        else{
            if (bimodal_table[cpu][index] > 0) bimodal_table[cpu][index]--;
        }


        // allocate tagged entries on misprediction
        
        uint8_t pred = (bimodal_table[cpu][index] >= 2) ? 1 : 0;
        if (pred != taken){
            uint8_t rand = random();
            int Y = rand & ((1 << (TAGE_NUM_COMPONENTS - pred_component)) - 1);
            int X = pred_component + 1;

            if (Y & 1){
                X++;
                if (Y & 2){
                    X++;
                }
            }

            for (int i = X; i <= TAGE_NUM_COMPONENTS; i++){
                struct tage_predictor_table_entry* entry = &predictor_table[cpu][X-1][get_predictor_index(cpu, ip, X)];
                if (entry->useful == 0){
                    entry->tag = get_tag(cpu, ip, X);
                    entry->ctr = (entry->ctr >= 4) ? 1 : 0;
                }
            }
        }
    }

    // update global history
    for (int i=TAGE_HISTORY_BUFFER_LENGTH-1; i>1; i--){
        global_history[cpu][i] = global_history[cpu][i-1];
    }
    global_history[cpu][0] = taken;
    

}

uint16_t Tage::get_bimodal_index(uint32_t cpu, uint64_t ip){
    return ip & (TAGE_BIMODAL_TABLE_SIZE - 1);
}

uint16_t Tage::get_predictor_index(uint32_t cpu, uint64_t ip, int component){
    component--;

    uint16_t compressed_history = 0;
    int compressed_history_length = lg2(TAGE_PREDICTOR_TABLE_SIZE);
    for(int i=0;i<component_history_lengths[component];i++){
        uint16_t temporary_history = 0;
        for(int j=0;j<compressed_history_length;j++){
            temporary_history = (temporary_history << 1) | global_history[cpu][i];
        }
        compressed_history ^= temporary_history; 
    }

    return (compressed_history^ip) & (TAGE_PREDICTOR_TABLE_SIZE - 1); 
}

uint16_t Tage::get_tag(uint32_t cpu, uint64_t ip, int component){
    component--;

    uint16_t compressed_history = 0;
    int compressed_history_length = TAGE_TAG_BITS;
    for(int i=0;i<component_history_lengths[component];i++){
        uint16_t temporary_history = 0;
        for(int j=0;j<compressed_history_length;j++){
            temporary_history = (temporary_history << 1) | global_history[cpu][i];
        }
        compressed_history ^= temporary_history; 
    }

    return (compressed_history^ip) & ((1 << TAGE_TAG_BITS) - 1); 
}

int Tage::get_match_below_n(uint32_t cpu, uint64_t ip, int component)
{
    for(int i = component -1;i>=1;i--){
        uint16_t index = get_predictor_index(cpu,ip,i);
        uint16_t tag = get_tag(cpu,ip,i);

        if(predictor_table[cpu][i-1][index].tag == tag){
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
