#ifndef SIM_BP_H
#define SIM_BP_H

typedef struct bp_params{
    unsigned long int K;
    unsigned long int M1;
    unsigned long int M2;
    unsigned long int N;
    char*             bp_name;
}bp_params;

// Put additional data structures here as per your requirement
typedef struct bimodal{
    uint8_t pridictor[10000000];
    uint8_t pridictor_2[10000000];
    uint8_t hybrid[10000000];
    uint8_t flag_b;
    uint8_t flag_g;
    uint8_t flag_h;
    uint64_t GBH;
}bimodal;

typedef struct output{
    unsigned long int Predict_count;
    unsigned long int Mispredict_b;
    unsigned long int Mispredict_g;
    unsigned long int Mispredict_h;
    float rate_b;
    float rate_g;
    float rate_h;
}output;

#endif
