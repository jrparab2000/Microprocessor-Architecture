#ifndef SIM_PROC_H
#define SIM_PROC_H

typedef struct proc_params{
    long int rob_size;
    long int iq_size;
    long int width;
}proc_params;

typedef struct duration_struct{
    int valid;
    int begin_cycle;
    int duration;
}duration_struct;

typedef struct data_struct{
    int Sequence;
    uint64_t PC;
    int op_type, dest, src1, src2, valid;  // Variables are read from trace file
    int rdy1;
    int rdy2;
    int mode1; //if this is 1 look in the ROB table else look in the ARF table
    int mode2;
    duration_struct FE_out;
    duration_struct DE_out;
    duration_struct RN_out;
    duration_struct RR_out;
    duration_struct DI_out;
    duration_struct IS_out;
    duration_struct EX_out;
    duration_struct WB_out;
    duration_struct RT_out;
}data_struct;

typedef struct ROB_struct{
    data_struct basic;
    uint64_t PC;
    int mis;
    int exc;
    int ready;
    int dst;
}ROB_struct;


typedef struct bypass{
    int dst;
    int valid;
}bypass;

typedef struct Issue_Queue_struct{
    data_struct basic;
    int valid;
    int dst;
    int rdy1;
    int rdy2;
    int src1;
    int src2;
}Issue_Queue_struct;

// Put additional data structures here as per your requirement

#endif
