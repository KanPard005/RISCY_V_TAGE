#include "ooo_cpu.h"

#define TAGE_BIMODAL_TABLE_SIZE 16384
#define TAGE_PREDICTOR_TABLE_SIZE 1024
#define TAGE_NUM_COMPONENTS 5
#define TAGE_BASE_COUNTER_BITS 2
#define TAGE_COUNTER_BITS 3  
#define TAGE_USEFUL_BITS 2
#define TAGE_TAG_BITS 9
#define TAGE_HISTORY_LENGTH 500

class Tage
{
private:
    /* data */
    int bimodal_table[NUM_CPUS][TAGE_BIMODAL_TABLE_SIZE];  // range 0-3
    struct entry{
        uint8_t ctr;  // range 0-7
        uint16_t tag; 
        uint8_t useful;  // range 0-3
    } predictor_table[NUM_CPUS][TAGE_NUM_COMPONENTS][TAGE_PREDICTOR_TABLE_SIZE];
    
    uint8_t global_history[TAGE_HISTORY_LENGTH];
    

public:
    void init(uint32_t cpu);
    uint8_t predict(uint32_t cpu, uint64_t ip);
    void update(uint32_t cpu, uint64_t ip, uint8_t taken);
    uint16_t get_bimodal_index(uint64_t ip);
    uint16_t get_predictor_index(uint64_t ip, uint8_t *global_history);

    Tage(/* args */);
    ~Tage();
};

void Tage::init(uint32_t cpu){
    for (int i=0; i < TAGE_BIMODAL_TABLE_SIZE; i++){
        bimodal_table[cpu][i] = 2;  // weakly taken
    }
    for (int i=0; i < TAGE_NUM_COMPONENTS; i++){
        for (int j=0; j < TAGE_PREDICTOR_TABLE_SIZE; j++){
            bimodal_table[cpu][i][j].ctr = 4;  // weakly taken
            bimodal_table[cpu][i][j].useful = 0;  // not useful
            bimodal_table[cpu][i][j].tag = 0;
        }
    }
}

uint8_t Tage::predict(uint32_t cpu, uint64_t ip){

}

void Tage::update(uint32_t cpu, uint64_t ip, uint8_t taken){

}

uint16_t Tage::get_bimodal_index(uint64_t ip){

}

uint16_t Tage::get_predictor_index(uint64_t ip, uint8_t *global_history){

}


Tage::Tage()
{
    
}



Tage::~Tage()
{
}
