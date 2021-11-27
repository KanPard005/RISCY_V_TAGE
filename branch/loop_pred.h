#include <inttypes.h>

#define ENTRIES 256 // Number of entries in the predictor table
#define WAY 4       // Associativity of the predictor table
#define LOGIND 6    // Number of bits required to index into the table
#define LOGWAY 2    // Number of bits required to determine the way at a particular index
#define TAGSIZE 14  // Number of bits to represent tag in the table
#define ITERSIZE 14 // Max size of the loop that the predictor can predict properly
#define AGE 31      // Intiial age of the the entry

struct Entry
{
    uint16_t tag;          // Stores the 14-bit tag for the entry
    uint16_t past_iter;    // Stores the 14-bit count for the number of iterations seen in past
    uint16_t current_iter; // Stores the 14-bit count for the number of iterations seen currently
    uint8_t age;           // 8-bit counter signifying age of entry
    uint8_t confidence;    // 2-bit counter signifying confidence in prediction
};

class LoopPred
{
    Entry table[ENTRIES]; // Predictor table
    int ind;              // Index in loop
    int hit;              // The way in the loop where we get a hit else -1
    int ptag;             // The tag calculated
    uint8_t seed;

public:
    bool is_valid;     // Validity of prediction
    uint8_t loop_pred; // The prediction returned for current PC

    void init();
    uint8_t get_pred(uint64_t pc);
    void update_entry(uint8_t taken, uint8_t tage_pred);
};

// Intialises the predictor
void LoopPred::init()
{
    seed = 0;
    for (int i = 0; i < ENTRIES; i++)
    {
        table[i].tag = 0;
        table[i].past_iter = 0;
        table[i].current_iter = 0;
        table[i].age = 0;
        table[i].confidence = 0;
    }
}

// Predicts based on the current pc
uint8_t LoopPred::get_pred(uint64_t pc)
{
    hit = -1;
    ind = (pc & ((1 << LOGIND) - 1)) << LOGWAY;         // Calculate index
    ptag = (pc >> LOGIND) & ((1 << TAGSIZE) - 1);       // Calculate tag

    for (int i = ind; i < ind + WAY; i++)
    {
        // If a matching block is found, set hit and valid values
        // Return based on current and past counts of iterations
        if (table[i].tag == ptag)
        {
            hit = i;
            is_valid = (table[i].confidence == 3);
            if (table[i].current_iter + 1 == table[i].past_iter)
            {
                loop_pred = 0;
                return 0;
            }
            loop_pred = 1;
            return 1;
        }
    }

    // No matching entry found in table; return false
    is_valid = false;
    loop_pred = 0;
    return 0;
}

// Updates the predictor table based on the prediction and actually taken/not taken branch
void LoopPred::update_entry(uint8_t taken, uint8_t tage_pred)
{
    if (hit >= 0)
    {
        Entry &entry = table[ind + hit];
        if (is_valid)
        {
            // If the predicton was wrong, free the entry
            if (taken != loop_pred)
            {
                entry.past_iter = 0;
                entry.age = 0;
                entry.confidence = 0;
                entry.current_iter = 0;
                return;
            }

            if (taken != tage_pred)
            {
                if (entry.age < AGE)
                    entry.age++;
            }
        }

        entry.current_iter++;
        entry.current_iter &= ((1 << ITERSIZE) - 1);

        // If the iteration is greater than what was seen last time, free the entry
        if (entry.current_iter > entry.past_iter)
        {
            entry.confidence = 0;
            if (entry.past_iter != 0)
            {
                entry.past_iter = 0;
                entry.age = 0;
                entry.confidence = 0;
            }
        }

        if (!taken)
        {
            if (entry.current_iter == entry.past_iter)
            {
                // Increase the confidence if correct
                if (entry.confidence < 3)
                    entry.confidence++;
                
                // We do not care for loops with < 3 iterations
                if ((entry.past_iter > 0) && (entry.past_iter < 3))
                {
                    entry.past_iter = 0;
                    entry.age = 0;
                    entry.confidence = 0;
                }
            }
            else
            {
                // Set the newly allocated entry
                if (entry.past_iter == 0)
                {
                    entry.confidence = 0;
                    entry.past_iter = entry.current_iter;
                }
                // else free the entry
                else
                {
                    entry.past_iter = 0;
                    entry.age = 0;
                    entry.confidence = 0;
                }
            }
            entry.current_iter = 0;
        }
    }
    // If the branch is taken but there is no entry, we must allocate one entry in the table
    else if (taken)
    {
        seed = (seed + 1) & 3;
        for (int i = 0; i < WAY; i++)
        {
            int j = ind + ((seed + i) & 3);
            if (table[j].age == 0)
            {
                table[j].tag = ptag;
                table[j].past_iter = 0;
                table[j].current_iter = 1;
                table[j].age = AGE;
                table[j].confidence = 0;
                break;
            }
            else if (table[j].age > 0)
                table[j].age--;
        }
    }
}