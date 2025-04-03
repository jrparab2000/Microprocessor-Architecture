#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <vector>
#include "sim_proc.h"

using namespace std;

proc_params params;       // look at sim_bp.h header file for the the definition of struct proc_params

class Issue_queue_class
{
    private:
        int count;
        int i;
        int oldest_sequence;
    public:
        vector<Issue_Queue_struct> IQ;
        void Vector_Resize(unsigned long int size);
        int empty_space_IQ();
        int IQ_oldest();
        void Issue_Queue_print();
        int IQ_full_blank();
};

void Issue_queue_class::Vector_Resize(unsigned long int size)
{
    IQ.resize(size);
}
int Issue_queue_class::empty_space_IQ()
{
    count = 0;
    for (i = 0; i < params.iq_size; i++)
    {
        if(IQ[i].valid == 0)
            count++;
    }
    return count;
}
int Issue_queue_class::IQ_oldest()
{
    oldest_sequence = 10000000;
    count = -1;
    for(i=0; i<params.iq_size;i++)
    {
        if((IQ[i].rdy1 == 1) && (IQ[i].rdy2 == 1) && (IQ[i].valid == 1))
        {
            if(IQ[i].basic.Sequence < oldest_sequence)
            {
                oldest_sequence = IQ[i].basic.Sequence;
                count = i;
            }
        }
    }
    return count;
}

void Issue_queue_class::Issue_Queue_print()
{
    for(i=0;i<params.iq_size;i++)
    {
        printf("\n====Issue Queue %d====\n",i);
        printf("Issue valid: %d\nReady1,2: %d,%d\nSource 1,2: %d,%d\nMode 1,2: %d,%d\ndst: %d\nclockentry: %d\n",IQ[i].valid,IQ[i].rdy1,IQ[i].rdy2,IQ[i].src1,IQ[i].src2,IQ[i].basic.mode1,IQ[i].basic.mode2,IQ[i].dst,IQ[i].basic.IS_out.begin_cycle);
    }
}

int Issue_queue_class::IQ_full_blank()
{
    count = 0;
    for(i=0;i<params.iq_size;i++)
    {
        count++;
    }
    if(count == params.iq_size)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

class ROB_class
{
    private:
        int i;
    public:
        int H,T,length;
        vector<ROB_struct> ROB_v;
        void ROB_Resize(unsigned long int size);
        int ROB_empty();
        void ROB_print();
        int ROB_ready(int value);
        int ROB_retire();
};
int ROB_class::ROB_empty()
{
    if(H<T)
    {
        return (params.rob_size-(T-H));
    }
    else if(H>T)
    {
        return ((-T+H));//change similar to above
    }
    else if((H==0)&&(T==0))// this can create problem when only one space is avalible
    {
        return params.rob_size;
    }
    return -1;
}
void ROB_class::ROB_Resize(unsigned long int size)
{
    ROB_v.resize(size);
}
void ROB_class::ROB_print()
{
    for(i=0;i<params.rob_size;i++)
    {
        printf("ROBentry: %d\nDst: %d\nrdy: %d\nPC: %lx\n",i,ROB_v[i].dst,ROB_v[i].ready,ROB_v[i].PC);
        printf("%d,%d\n",ROB_v[H].basic.mode1,ROB_v[H].basic.mode2);
    }
}

int ROB_class::ROB_ready(int value)
{
    for(i=0;i<params.rob_size;i++)
    {
        if(ROB_v[i].dst == value)
        {
            return ROB_v[i].ready;
        }
    }
    return 0;
}

int ROB_class::ROB_retire()
{
    if(ROB_v[H].ready == 1)
        return 1;
    else if(ROB_v[H].ready == 0)
        return 0;
    return -1;
}

class pipeline
{
private:
    int count;
public:
    int valid;
    int valid_count;
    vector<data_struct> data_v;
    vector<int> v;
    void Vector_Resize(unsigned long int size);
    int Vector_free();
    int line_checker();
    pipeline();
};

pipeline::pipeline() {
    count = 0;
}
void pipeline::Vector_Resize(unsigned long int size)
{
    data_v.resize(size);
}

int pipeline::line_checker()
{
    if (valid_count != params.width-1)
    {
        return 1;
    }
    return 0;
}

int pipeline::Vector_free()
{
    for(int i=0;i<params.width;i++)
    {
        if(data_v[i].valid)
        {
            count++;
        }
    }
    return count;
}

int Rename_buffer[67][2];

vector<bypass> bypass1;
vector<bypass> bypass2;
vector<bypass> bypass3;

pipeline FE;
pipeline DE;
pipeline RN;
pipeline RR;
pipeline DI;
Issue_queue_class IQ;
pipeline EX21;
pipeline EX22;
pipeline EX23;
pipeline EX24;
pipeline EX25;
pipeline EX11;
pipeline EX12;
pipeline EX01;
pipeline WB0;
pipeline WB1;
pipeline WB2;
ROB_class ROB;

data_struct print;

int clock = 0;
int ROB_flag;

FILE *FP;               // File handler
int EOF_detector;
int op_type, dest, src1, src2, sequence;  // Variables are read from trace file
uint64_t pc; // Variable holds the pc read from input file;
char *trace_file;       // Variable that holds trace file name;

int temp_debug =0;

void Fetch();
void Decoder();
void Rename();
void Register_Read();
void Dispatch();
void Issue();
void Execute();
void Writeback();
void Retire();
void printfunc();
void Print_final();
int Advance_cycle();

int main (int argc, char* argv[])
{
    if (argc != 5)
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    params.rob_size     = strtoul(argv[1], NULL, 10);
    params.iq_size      = strtoul(argv[2], NULL, 10);
    params.width        = strtoul(argv[3], NULL, 10);
    trace_file          = argv[4];
    
    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }
    FE.Vector_Resize(params.width);
    DE.Vector_Resize(params.width);
    RN.Vector_Resize(params.width);
    RR.Vector_Resize(params.width);
    DI.Vector_Resize(params.width);
    IQ.Vector_Resize(params.iq_size);
    EX01.Vector_Resize(params.width);
    EX11.Vector_Resize(params.width);
    EX12.Vector_Resize(params.width);
    EX21.Vector_Resize(params.width);
    EX22.Vector_Resize(params.width);
    EX23.Vector_Resize(params.width);
    EX24.Vector_Resize(params.width);
    EX25.Vector_Resize(params.width);
    ROB.ROB_Resize(params.rob_size);
    bypass1.resize(params.width);
    bypass2.resize(params.width);
    bypass3.resize(params.width);
    WB0.Vector_Resize(params.width);
    WB1.Vector_Resize(params.width);
    WB2.Vector_Resize(params.width);
    do
    {
        Retire();
        Writeback();
        Execute();
        Issue();
        Dispatch();
        Register_Read();
        Rename();
        Decoder();
        Fetch();
        clock++;
    } while (Advance_cycle());
    printf("# === Simulator Command =========\n");
    printf("# %s %ld %ld %ld %s\n",argv[0],params.rob_size,params.iq_size,params.width,argv[4]);
    Print_final();
    return 0;
}
void printfunc()
{
    print = ROB.ROB_v[ROB.H].basic;
    printf("%d fu{%d} src{%d,%d}",print.Sequence,print.op_type,print.src1,print.src2);
    printf(" dst{%d} FE{%d,1} DE{%d,%d}",ROB.ROB_v[ROB.H].dst,print.DE_out.begin_cycle-1,print.DE_out.begin_cycle,print.DE_out.duration);
    printf(" RN{%d,%d} RR{%d,%d} DI{%d,%d}",print.RN_out.begin_cycle,print.RN_out.duration,print.RR_out.begin_cycle,print.RR_out.duration,print.DI_out.begin_cycle,print.DI_out.duration); 
    printf(" IS{%d,%d} EX{%d,%d} WB{%d,%d} RT{%d,%d}\n",print.IS_out.begin_cycle,print.IS_out.duration,print.EX_out.begin_cycle,print.EX_out.duration,print.WB_out.begin_cycle,print.WB_out.duration,print.RT_out.begin_cycle,print.RT_out.duration);
}
void Retire()
{
    int i;
    for(i=0;i<params.width;i++)
    {
        if(ROB.ROB_retire()&&(ROB.length> 0))
        {
            ROB.ROB_v[ROB.H].basic.RT_out.duration = -ROB.ROB_v[ROB.H].basic.RT_out.begin_cycle + clock;
            printfunc();
            if(ROB.ROB_v[ROB.H].dst != -1)
            {
                if(Rename_buffer[ROB.ROB_v[ROB.H].dst][1] == ROB.H)
                {
                    Rename_buffer[ROB.ROB_v[ROB.H].dst][0]=0;
                    Rename_buffer[ROB.ROB_v[ROB.H].dst][1]=0;
                }
            }
            if(ROB.H != (params.rob_size-1))
            {
                ROB.H++;
            }
            else
            {
                ROB.H = 0;
            }
            ROB.length--;
        }
    }
}
void Writeback()
{
    int i;
    for(i=0;i<params.width;i++)
    {
        
        if(WB0.data_v[i].valid == 1)
        {
            WB0.data_v[i].src1 = ROB.ROB_v[WB0.data_v[i].dest].basic.src1;
            WB0.data_v[i].src2 = ROB.ROB_v[WB0.data_v[i].dest].basic.src2;
            ROB.ROB_v[WB0.data_v[i].dest].basic = WB0.data_v[i];
            ROB.ROB_v[WB0.data_v[i].dest].basic.WB_out.duration = 1;
            ROB.ROB_v[WB0.data_v[i].dest].basic.RT_out.begin_cycle = clock;
            ROB.ROB_v[WB0.data_v[i].dest].ready = 1;
            WB0.data_v[i].valid = 0;
        }
        if(WB1.data_v[i].valid == 1)
        {
            WB1.data_v[i].src1 = ROB.ROB_v[WB1.data_v[i].dest].basic.src1;
            WB1.data_v[i].src2 = ROB.ROB_v[WB1.data_v[i].dest].basic.src2;
            ROB.ROB_v[WB1.data_v[i].dest].basic = WB1.data_v[i];
            ROB.ROB_v[WB1.data_v[i].dest].basic.WB_out.duration = 1;
            ROB.ROB_v[WB1.data_v[i].dest].basic.RT_out.begin_cycle = clock;
            ROB.ROB_v[WB1.data_v[i].dest].ready = 1;
            WB1.data_v[i].valid = 0;
        }
        if(WB2.data_v[i].valid == 1)
        {
            WB2.data_v[i].src1 = ROB.ROB_v[WB2.data_v[i].dest].basic.src1;
            WB2.data_v[i].src2 = ROB.ROB_v[WB2.data_v[i].dest].basic.src2;
            ROB.ROB_v[WB2.data_v[i].dest].basic = WB2.data_v[i];
            ROB.ROB_v[WB2.data_v[i].dest].basic.WB_out.duration = 1;
            ROB.ROB_v[WB2.data_v[i].dest].basic.RT_out.begin_cycle = clock;
            ROB.ROB_v[WB2.data_v[i].dest].ready = 1;
            WB2.data_v[i].valid = 0;
        }
    }
}
void Execute()
{
    int i;
    for(i=0; i<params.width; i++)
    {
        bypass1[i].valid = 0;
        bypass2[i].valid = 0;
        bypass3[i].valid = 0;
        
        if(EX01.data_v[i].valid == 1)
        {
            WB0.data_v[i] = EX01.data_v[i];
            WB0.data_v[i].EX_out.duration = -WB0.data_v[i].EX_out.begin_cycle + clock;
            WB0.data_v[i].WB_out.begin_cycle = clock;
            WB0.data_v[i].valid = 1;
            bypass1[i].valid = 1;
            bypass1[i].dst = EX01.data_v[i].dest;
            EX01.data_v[i].valid = 0;
        }
        if (EX11.data_v[i].valid == 1)
        {
            WB1.data_v[i] = EX11.data_v[i];
            WB1.data_v[i].EX_out.duration = -WB1.data_v[i].EX_out.begin_cycle + clock;
            WB1.data_v[i].WB_out.begin_cycle = clock;
            WB1.data_v[i].valid = 1;
            bypass2[i].valid = 1;
            bypass2[i].dst = EX11.data_v[i].dest;
            EX11.data_v[i].valid = 0;
        }
        if (EX11.data_v[i].valid == 0)
        {
            if(EX12.data_v[i].valid == 1)
            {
                EX11.data_v[i] = EX12.data_v[i];
                EX12.data_v[i].valid = 0;
                EX11.data_v[i].valid = 1;
            }
        }
        if (EX21.data_v[i].valid == 1)
        {
            WB2.data_v[i] = EX21.data_v[i];
            WB2.data_v[i].EX_out.duration = -WB2.data_v[i].EX_out.begin_cycle + clock;
            WB2.data_v[i].WB_out.begin_cycle = clock;
            WB2.data_v[i].valid = 1;
            bypass3[i].valid = 1;
            bypass3[i].dst = EX21.data_v[i].dest;
            EX21.data_v[i].valid = 0;
            
        }    
        if (EX21.data_v[i].valid == 0)
        {
            if(EX22.data_v[i].valid == 1)
            {
                EX21.data_v[i] = EX22.data_v[i];
                EX22.data_v[i].valid = 0;
                EX21.data_v[i].valid = 1;
            }
        }
        if (EX22.data_v[i].valid == 0)
        {
            if(EX23.data_v[i].valid == 1)
            {
                EX22.data_v[i] = EX23.data_v[i];
                EX23.data_v[i].valid = 0;
                EX22.data_v[i].valid = 1;
            }
        }
        if (EX23.data_v[i].valid == 0)
        {
            if(EX24.data_v[i].valid == 1)
            {
                EX23.data_v[i] = EX24.data_v[i];
                EX24.data_v[i].valid = 0;
                EX23.data_v[i].valid = 1;
                
            }
        }
        if (EX24.data_v[i].valid == 0)
        {
            if(EX25.data_v[i].valid == 1)
            {
                EX24.data_v[i] = EX25.data_v[i];
                EX25.data_v[i].valid = 0;
                EX24.data_v[i].valid = 1;
                
            }
        }
    }
}
void Issue()
{
    int i,j;
    for(j=0; j<params.iq_size;j++)
    {
        for(i=0;i<params.width;i++)
        {
            if((bypass1[i].valid == 1))
            {
                if((bypass1[i].dst == IQ.IQ[j].src1))
                {
                    IQ.IQ[j].rdy1 = 1;
                }
                if((bypass1[i].dst == IQ.IQ[j].src2))
                {
                    IQ.IQ[j].rdy2 = 1;
                }
            }
            if((bypass2[i].valid == 1))
            {
                if((bypass2[i].dst == IQ.IQ[j].src1))
                {
                    IQ.IQ[j].rdy1 = 1;
                }
                if(bypass2[i].dst == IQ.IQ[j].src2)
                {
                    IQ.IQ[j].rdy2 = 1;
                }
            }
            if(bypass3[i].valid == 1)
            {
                if((bypass3[i].dst == IQ.IQ[j].src1))
                {
                    IQ.IQ[j].rdy1 = 1;
                }
                if(bypass3[i].dst == IQ.IQ[j].src2)
                {
                    IQ.IQ[j].rdy2 = 1;
                }
            }
        }
    }
    int current_inst;
    for(i=0; i<params.width; i++)
    {
        current_inst = IQ.IQ_oldest();
        
        if(current_inst != -1)
        {
            if(IQ.IQ[current_inst].basic.op_type == 0)
            {
                if(EX01.data_v[i].valid == 0)
                {
                    EX01.data_v[i] = IQ.IQ[current_inst].basic;
                    IQ.IQ[current_inst].valid = 0;
                    EX01.data_v[i].valid = 1;
                    EX01.data_v[i].IS_out.duration = -EX01.data_v[i].IS_out.begin_cycle + clock; 
                    EX01.data_v[i].EX_out.begin_cycle=clock;
                }
            }
            if(IQ.IQ[current_inst].basic.op_type == 1)
            {
                if (EX12.data_v[i].valid == 0)
                {
                    EX12.data_v[i] = IQ.IQ[current_inst].basic;
                    EX12.data_v[i].valid = 1;
                    IQ.IQ[current_inst].valid = 0;
                    EX12.data_v[i].IS_out.duration = -EX12.data_v[i].IS_out.begin_cycle + clock; 
                    EX12.data_v[i].EX_out.begin_cycle = clock;
                }
            }
            if(IQ.IQ[current_inst].basic.op_type == 2)
            {
                if (EX25.data_v[i].valid == 0)
                {
                    EX25.data_v[i] = IQ.IQ[current_inst].basic;
                    EX25.data_v[i].valid = 1;
                    IQ.IQ[current_inst].valid = 0;
                    EX25.data_v[i].IS_out.duration = -EX25.data_v[i].IS_out.begin_cycle + clock; 
                    EX25.data_v[i].EX_out.begin_cycle=clock;
                }
            }
        }
    }     
}
void Dispatch()
{
    int i,j;
    for(i=0; i<params.width; i++){
        for(j=0;j<params.width;j++){
         if((bypass1[i].valid == 1))
            {
                if((bypass1[i].dst == DI.data_v[j].src1))
                {
                    
                    DI.data_v[j].rdy1 = 1;
                }
                if((bypass1[i].dst == DI.data_v[j].src2))
                {
                    DI.data_v[j].rdy2 = 1;
                }
            }
            if((bypass2[i].valid == 1))
            {
                if((bypass2[i].dst == DI.data_v[j].src1))
                {
                    DI.data_v[j].rdy1 = 1;
                }
                if(bypass2[i].dst == DI.data_v[j].src2)
                {
                    DI.data_v[j].rdy2 = 1;
                }
            }
            if(bypass3[i].valid == 1)
            {
                if((bypass3[i].dst == DI.data_v[j].src1))
                {
                    DI.data_v[j].rdy1 = 1;
                }
                if(bypass3[i].dst == DI.data_v[j].src2)
                {
                    DI.data_v[j].rdy2 = 1;
                }
            }
        }
        
        }
        if(IQ.empty_space_IQ() >= DI.valid_count)
        {
            if(DI.valid == 1)
            {
                for(i=0; i<DI.valid_count; i++)
                {
                    int current = 0;
                    for(j=0; j<params.iq_size;j++)
                    {
                        if(IQ.IQ[j].valid == 0)
                        {
                            current = j;
                            break;
                        }
                    }
                    IQ.IQ[current].valid = 1;
                    IQ.IQ[current].basic = DI.data_v[i];
                    IQ.IQ[current].basic.DI_out.duration = -IQ.IQ[current].basic.DI_out.begin_cycle + clock; 
                    IQ.IQ[current].basic.IS_out.begin_cycle=clock;  
                    IQ.IQ[current].rdy1 = DI.data_v[i].rdy1;
                    IQ.IQ[current].rdy2 = DI.data_v[i].rdy2;
                    IQ.IQ[current].src1 = DI.data_v[i].src1;
                    IQ.IQ[current].src2 = DI.data_v[i].src2;
                    IQ.IQ[current].dst = DI.data_v[i].dest;
                    if(IQ.IQ[current].src1 == -1)
                    {
                        IQ.IQ[current].rdy1=1;
                    }
                    if(IQ.IQ[current].src2 == -1)
                    {
                        IQ.IQ[current].rdy2=1;
                    }  
                }
                DI.valid = 0;  
            }
        }
}
void Register_Read()
{
    int i,j;
    for(j=0;j<RR.valid_count;j++){
    if(RR.data_v[j].mode1 == 1)
        {
            if((ROB.ROB_v[RR.data_v[j].src1].ready))
            {
                RR.data_v[j].rdy1 = 1;
            }
        }
        else if(RR.data_v[j].mode1 == 0)
        {
                RR.data_v[j].rdy1 = 1;
        }
        if(RR.data_v[j].mode2 == 1)
        {
            if((ROB.ROB_v[RR.data_v[j].src2].ready))
            {
                RR.data_v[j].rdy2 = 1;
            }
        }
        else if(RR.data_v[j].mode2 == 0)
        {
            RR.data_v[j].rdy2 = 1;
        }
    }
    for(i=0; i<params.width; i++){
        for(j=0;j<params.width;j++){
        if((bypass1[i].valid == 1))
            {
                if((bypass1[i].dst == RR.data_v[j].src1))
                {
                    RR.data_v[j].rdy1 = 1;
                }
                if((bypass1[i].dst == RR.data_v[j].src2))
                {
                    RR.data_v[j].rdy2 = 1;
                }
            }
            if((bypass2[i].valid == 1))
            {
                if((bypass2[i].dst == RR.data_v[j].src1))
                {
                    RR.data_v[j].rdy1 = 1;
                }
                if(bypass2[i].dst == RR.data_v[j].src2)
                {
                    RR.data_v[j].rdy2 = 1;
                }
            }
            if(bypass3[i].valid == 1)
            {
                if((bypass3[i].dst == RR.data_v[j].src1))
                {
                    RR.data_v[j].rdy1 = 1;
                }
                if(bypass3[i].dst == RR.data_v[j].src2)
                {
                    RR.data_v[j].rdy2 = 1;
                }
            }
        }   
        }
    if(DI.valid == 0)
    {
        if(RR.valid == 1)
        {
            DI.valid_count = RR.valid_count;
            for(i=0; i<RR.valid_count; i++)
            {
                DI.data_v[i] = RR.data_v[i];
                DI.data_v[i].RR_out.duration = -DI.data_v[i].RR_out.begin_cycle + clock;
                DI.data_v[i].DI_out.begin_cycle = clock;  
            }
            RR.valid = 0;
            DI.valid = 1;    
        }
    }
}
void Rename()
{
    int i;
    if(ROB.length <= params.rob_size-DE.valid_count)
    {
        if (RR.valid == 0)
        {
            if (RN.valid == 1)
            {
                RR.valid_count = RN.valid_count;
                for (i=0; i<RN.valid_count; i++)
                {
                    ROB.ROB_v[ROB.T].basic = RN.data_v[i];  
                    if(RN.data_v[i].src1 != -1)
                    {
                        if(Rename_buffer[RN.data_v[i].src1][0] == 1)
                        {
                            RN.data_v[i].src1 = Rename_buffer[RN.data_v[i].src1][1];
                            RN.data_v[i].mode1 = 1;
                        }
                    }
                    if(RN.data_v[i].src2 != -1)
                    {
                        if(Rename_buffer[RN.data_v[i].src2][0] == 1)
                        {
                            RN.data_v[i].src2 = Rename_buffer[RN.data_v[i].src2][1];
                            RN.data_v[i].mode2 = 1;
                        }
                    }
                    ROB.ROB_v[ROB.T].dst = RN.data_v[i].dest;   //storing destination register in the ROB
                    ROB.ROB_v[ROB.T].PC = RN.data_v[i].PC;  //PC loading in the ROB
                    ROB.ROB_v[ROB.T].ready = 0; //setting ROB to zero

                    if(RN.data_v[i].dest != -1)
                    {
                        Rename_buffer[RN.data_v[i].dest][0]=1;  //Rename buffer storing the ROB entry in it
                        Rename_buffer[RN.data_v[i].dest][1]=ROB.T; 
                    }
                    RN.data_v[i].dest = ROB.T;
                    if(ROB.T != (params.rob_size-1))
                    {
                        ROB.T++;
                    }
                    else
                    {
                        ROB.T = 0;
                    }
                    ROB.length++;
                    RR.data_v[i] = RN.data_v[i];
                    RR.data_v[i].RN_out.duration = -RR.data_v[i].RN_out.begin_cycle + clock; //storing the duration for which it is in DE
                    RR.data_v[i].RR_out.begin_cycle=clock;  //storing the current cycle in the entry of this state
                }
                RR.valid = 1;
                RN.valid = 0;
            }
        }
    }
    
}
void Decoder()
{
    int i;
    if (RN.valid == 0)
    {
        if (DE.valid == 1)
        {
            RN.valid_count = DE.valid_count;
            for (i=0; i<DE.valid_count; i++)
            {
                RN.data_v[i] = DE.data_v[i];  //moving data from DE to RN
                RN.data_v[i].DE_out.duration = -RN.data_v[i].DE_out.begin_cycle + clock; //storing the duration for which it is in DE
                RN.data_v[i].RN_out.begin_cycle=clock;  //storing the current cycle in the entry of this state
            }
            RN.valid = 1;
            DE.valid = 0;
        }
    }
}
void Fetch()
{
    if(DE.valid == 0)
    {
        if(FE.valid == 1)
        {
            DE.valid_count = FE.valid_count;
            for(int i=0;i<FE.valid_count;i++)
            {
                DE.data_v[i] = FE.data_v[i];
                DE.data_v[i].FE_out.duration = clock - DE.data_v[i].FE_out.begin_cycle;
                DE.data_v[i].DE_out.begin_cycle=clock;
            }
            DE.valid = 1;
            FE.valid = 0;   
        }
    }
    if(FE.valid == 0)
    {
        FE.valid_count=0;
        for(int i=0;i<params.width;i++)
        {
            if(fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF)
            {
                FE.data_v[i].PC = pc;
                FE.data_v[i].op_type = op_type;
                FE.data_v[i].dest = dest;
                FE.data_v[i].src1 = src1;
                FE.data_v[i].src2 = src2;
                FE.data_v[i].FE_out.begin_cycle=clock;
                FE.data_v[i].Sequence = sequence;
                FE.valid_count++;
                sequence++;
                FE.valid = 1;
                EOF_detector = 0;
            }
            else
            {
                EOF_detector = 1;
            }
        }
    }
}    
int Advance_cycle()
{
    int i;
    int count1 = 0;
    if(EOF_detector == 1)
    {
        if((FE.valid == 0)&&(DE.valid == 0)&&(RN.valid == 0)&&(RR.valid == 0)&&(DI.valid == 0))
        {
            if(IQ.IQ_full_blank())
            {
                for(i=0;i<params.width;i++)
                {
                    if((!EX01.data_v[i].valid)&&(!EX11.data_v[i].valid)&&(!EX12.data_v[i].valid)&&(!EX21.data_v[i].valid)&&(!EX22.data_v[i].valid)&&(!EX23.data_v[i].valid)&&(!EX24.data_v[i].valid)&&(!EX25.data_v[i].valid))
                    {
                        if((!WB0.data_v[i].valid)&&(!WB1.data_v[i].valid)&&(!WB2.data_v[i].valid))
                            count1++;
                    }
                }
            }
        }
    }
    if((count1 == params.width)&&(ROB.H == ROB.T))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
void Print_final()
{
    float IPC=(float)sequence/(float)(clock-1);
    printf("# === Processor Configuration ===\n");
    printf("# ROB_SIZE = %lu\n# IQ_SIZE  = %lu\n# WIDTH    = %lu\n", params.rob_size, params.iq_size, params.width);
    printf("# === Simulation Results ========\n");
    printf("# Dynamic Instruction Count    = %d\n",sequence);
    printf("# Cycles                       = %d\n",clock-1);
    printf("# Instructions Per Cycle (IPC) = %.2f\n",IPC);
}


