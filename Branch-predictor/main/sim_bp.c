#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "sim_bp.h"

bp_params params;       // look at sim_bp.h header file for the the definition of struct bp_params
bimodal b_type;
output out_print;
int select_1 = 0;

uint64_t indexing(uint64_t addr);
void bimodal_f(uint64_t addr, char outcome);
void init_b();
void printing();
void g_share_f(uint64_t addr, char outcome);
void g_share_u(char outcome);
void g_share_u(char outcome);
void hybrid_f(uint64_t addr, char outcome);

int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;

    char outcome;           // Variable holds branch outcome
    
    uint64_t addr; //changed to this for shifting purpose
    
    if (!(argc == 4 || argc == 5 || argc == 7))
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    params.bp_name  = argv[1];
    
    // strtoul() converts char* to unsigned long. It is included in <stdlib.h>
    if(strcmp(params.bp_name, "bimodal") == 0)              // Bimodal
    {
        if(argc != 4)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M2       = strtoul(argv[2], NULL, 10);
        trace_file      = argv[3];
        select_1 = 1;
        printf("COMMAND\n%s %s %lu %s\n", argv[0], params.bp_name, params.M2, trace_file);
    }
    else if(strcmp(params.bp_name, "gshare") == 0)          // Gshare
    {
        if(argc != 5)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M1       = strtoul(argv[2], NULL, 10);
        params.N        = strtoul(argv[3], NULL, 10);
        trace_file      = argv[4];
        select_1 = 2;
        printf("COMMAND\n%s %s %lu %lu %s\n", argv[0], params.bp_name, params.M1, params.N, trace_file);

    }
    else if(strcmp(params.bp_name, "hybrid") == 0)          // Hybrid
    {
        if(argc != 7)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.K        = strtoul(argv[2], NULL, 10);
        params.M1       = strtoul(argv[3], NULL, 10);
        params.N        = strtoul(argv[4], NULL, 10);
        params.M2       = strtoul(argv[5], NULL, 10);
        trace_file      = argv[6];
        select_1 = 3;
        printf("COMMAND\n%s %s %lu %lu %lu %lu %s\n", argv[0], params.bp_name, params.K, params.M1, params.N, params.M2, trace_file);

    }
    else
    {
        printf("Error: Wrong branch predictor name:%s\n", params.bp_name);
        exit(EXIT_FAILURE);
    }
    
    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }
    init_b();
    char str[2];
    while(fscanf(FP, "%lx %s", &addr, str) != EOF)
    {
        outcome = str[0];
        if(select_1 == 1)
            bimodal_f(addr,outcome);
        if(select_1 == 2)
            g_share_f(addr,outcome);
        if(select_1 == 3)
            hybrid_f(addr,outcome);
        g_share_u(outcome);
        out_print.Predict_count++;
    }
    printing();
    return 0;
}

void hybrid_f(uint64_t addr, char outcome)
{
    uint64_t index = addr >> 2;
    index = index << (64-params.K);
    index = index >> (64 - params.K);

    if(b_type.hybrid[index] >= 2)
    b_type.flag_h = 2;
    else
    b_type.flag_h = 1;

    bimodal_f(addr,outcome);
    g_share_f(addr,outcome);
    if(b_type.hybrid[index] >= 2)
    {
        if(b_type.flag_g == 1)
        {
            out_print.Mispredict_h++;
        }
    }
    else
    {
        if(b_type.flag_b == 1)
        {
            out_print.Mispredict_h++;
        }
    }
    if ((b_type.flag_b == 1)&&(b_type.flag_g == 2))
    {
        if(b_type.hybrid[index] < 3)
        {
            b_type.hybrid[index]++;
        }
    }
    else if ((b_type.flag_b == 2)&&(b_type.flag_g == 1))
    {
        if(b_type.hybrid[index] > 0)
        {
            b_type.hybrid[index]--;
        }
    }
    b_type.flag_b =0;
    b_type.flag_g=0;
}

void bimodal_f(uint64_t addr, char outcome)
{
    uint64_t index = indexing(addr);
    uint8_t prediction = b_type.pridictor[index];
    if((prediction == 2)||(prediction == 3))
    {
        if(outcome == 't')
        {
            if(prediction == 2)
            {
                if(select_1 == 3)
                {
                    if(b_type.flag_h == 1)
                        b_type.pridictor[index]++;
                }
                else
                {
                    b_type.pridictor[index]++;
                }
            }
            b_type.flag_b = 2;
        }
        else
        {
            if(select_1 == 3)
                {
                    if(b_type.flag_h == 1)
                        b_type.pridictor[index]--;
                }
                else
                {
                    b_type.pridictor[index]--;
                }
            b_type.flag_b = 1;
            out_print.Mispredict_b++;
        }
    }
    else if((prediction == 1)||(prediction == 0))
    {
        if(outcome == 'n')
        {
            if(prediction == 1)
            {
                if(select_1 == 3)
                {
                    if(b_type.flag_h == 1)
                        b_type.pridictor[index]--;
                }
                else
                {
                    b_type.pridictor[index]--;
                }
            }
            b_type.flag_b = 2;
        }
        else
        {
            if(select_1 == 3)
                {
                    if(b_type.flag_h == 1)
                        b_type.pridictor[index]++;
                }
                else
                {
                    b_type.pridictor[index]++;
                }
            b_type.flag_b = 1;
            out_print.Mispredict_b++;
        }
    }
}

void g_share_f(uint64_t addr, char outcome)
{
    uint64_t temp = addr >> 2;
    temp = temp << (63-(params.M1 - 1));
    temp = temp >> (63-(params.M1 - 1));
    uint64_t N = temp >> (params.M1 - params.N);
    uint64_t temp2 = temp << (63-(params.M1 - 1));
    temp2 = temp2 << params.N;
    temp2 = temp2 >> params.N;
    temp2 = temp2 >> (63-(params.M1 - 1));
    uint64_t temp3 = (b_type.GBH >> (64-params.N))^N;
    uint64_t index = temp3;
    index = index << (params.M1 - params.N);
    index = index | temp2;
    uint8_t prediction = b_type.pridictor_2[index];
    if((prediction == 2)||(prediction == 3))
    {
        if(outcome == 't')
        {
            if(prediction == 2)
            {
                if(select_1 == 3)
                {
                    if(b_type.flag_h == 2)
                        b_type.pridictor_2[index]++;
                }
                else
                {
                    b_type.pridictor_2[index]++;
                }
            }
            b_type.flag_g = 2;
        }
        else
        {
            if(select_1 == 3)
                {
                    if(b_type.flag_h == 2)
                        b_type.pridictor_2[index]--;
                }
                else
                {
                    b_type.pridictor_2[index]--;
                }
            b_type.flag_g = 1;
            out_print.Mispredict_g++;
        }
    }
    else if((prediction == 1)||(prediction == 0))
    {
        if(outcome == 'n')
        {
            if(prediction == 1)
            {
               if(select_1 == 3)
                {
                    if(b_type.flag_h == 2)
                        b_type.pridictor_2[index]--;
                }
                else
                {
                    b_type.pridictor_2[index]--;
                }
            }
            b_type.flag_g = 2;
        }
        else
        {
            if(select_1 == 3)
                {
                    if(b_type.flag_h == 2)
                        b_type.pridictor_2[index]++;
                }
                else
                {
                    b_type.pridictor_2[index]++;
                }
            b_type.flag_g = 1;
            out_print.Mispredict_g++;
        }
    }
}

void g_share_u(char outcome)
{
    uint64_t temp = b_type.GBH;
    uint64_t mask = 0xFFFFFFFFFFFFFFFF;
    uint64_t mask1 = 0x7FFFFFFFFFFFFFFF;
        temp = temp >> 1;
        if (outcome == 't')
        {
            temp = temp | ~mask1;
        }
        mask = mask >> params.N;
        mask = ~mask;
        temp = temp & mask;
        b_type.GBH = temp;
}

void init_b()
{
    int i;
    for (i=0; i<(int)pow(2,params.M2); i++)
    {
        b_type.pridictor[i] = 2;
    }
    for (i=0; i<(int)pow(2,params.M1); i++)
    {
        b_type.pridictor_2[i] = 2;
    }
    for (i=0; i<(int)pow(2,params.K); i++)
    {
        b_type.hybrid[i] = 1;
    }
    b_type.GBH = 0;
}

uint64_t indexing(uint64_t addr)
{
    uint64_t index;
        index = addr >> 2;
        index = index << (63-(params.M2 - 1));
        index = index >> (63-(params.M2 - 1));
        return index;
}
void printing()
{
    int i;
    if(select_1 == 1)
    {
        out_print.rate_b = (float)(((float)out_print.Mispredict_b/(float)out_print.Predict_count)*100);
        printf("OUTPUT\n");
        printf("number of predictions:\t%lu\n",out_print.Predict_count);
        printf("number of mispredictions:\t%lu\n",out_print.Mispredict_b);
        printf("misprediction rate:\t%.2f%%\n",out_print.rate_b);
        printf("FINAL BIMODAL CONTENTS\n");
    for (i=0; i<(int)pow(2,params.M2); i++)
    {
        printf("%d\t%d\n",i,b_type.pridictor[i]);
    }
    }
    if(select_1 == 2)
    {
        out_print.rate_g = (float)(((float)out_print.Mispredict_g/(float)out_print.Predict_count)*100);
        printf("OUTPUT\n");
        printf("number of predictions:\t%lu\n",out_print.Predict_count);
        printf("number of mispredictions:\t%lu\n",out_print.Mispredict_g);
        printf("misprediction rate:\t%.2f%%\n",out_print.rate_g);
        printf("FINAL GSHARE CONTENTS\n");
    for (i=0; i<(int)pow(2,params.M1); i++)
    {
        printf("%d\t%d\n",i,b_type.pridictor_2[i]);
    }
    }
    if(select_1 == 3)
    {
        out_print.rate_h = (float)(((float)out_print.Mispredict_h/(float)out_print.Predict_count)*100);
        printf("OUTPUT\n");
        printf("number of predictions:\t%lu\n",out_print.Predict_count);
        printf("number of mispredictions:\t%lu\n",out_print.Mispredict_h);
        printf("misprediction rate:\t%.2f%%\n",out_print.rate_h);
        printf("FINAL CHOOSER CONTENTS\n");
    for (i=0; i<(int)pow(2,params.K); i++)
    {
        printf("%d\t%d\n",i,b_type.hybrid[i]);
    }
    printf("FINAL GSHARE CONTENTS\n");
    for (i=0; i<(int)pow(2,params.M1); i++)
    {
        printf("%d\t%d\n",i,b_type.pridictor_2[i]);
    }
    printf("FINAL BIMODAL CONTENTS\n");
    for (i=0; i<(int)pow(2,params.M2); i++)
    {
        printf("%d\t%d\n",i,b_type.pridictor[i]);
    }
    }
}