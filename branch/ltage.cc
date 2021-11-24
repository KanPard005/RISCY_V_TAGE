#include <ooo_cpu.h>
#include "tage.h"
#include "loop_pred.h"

Tage tage_predictor[NUM_CPUS];
LoopPred loop_predictor[NUM_CPUS];
uint8_t tage_prediction;

void O3_CPU::initialize_branch_predictor() {
    tage_predictor[cpu].init();
    loop_predictor[cpu].init();
}

uint8_t O3_CPU::predict_branch(uint64_t ip) {
    bool res_loop = loop_predictor[cpu].get_pred(ip);
    bool res_tage = tage_predictor[cpu].predict(ip);
    tage_prediction = res_tage;
    if ((loop_predictor[cpu].is_valid) & (loop_predictor[cpu].loop_correct >= 0)) 
        return res_loop;
    return res_tage;
}

void O3_CPU::last_branch_result(uint64_t ip, uint8_t taken) {
    tage_predictor[cpu].update(ip, taken);
    loop_predictor[cpu].update_entry(taken, tage_prediction);
}