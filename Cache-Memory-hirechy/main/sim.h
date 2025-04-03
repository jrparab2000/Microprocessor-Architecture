#ifndef SIM_CACHE_H
#define SIM_CACHE_H

typedef 
struct {
   uint32_t BLOCKSIZE;
   uint32_t L1_SIZE;
   uint32_t L1_ASSOC;
   uint32_t L2_SIZE;
   uint32_t L2_ASSOC;
   uint32_t PREF_N;
   uint32_t PREF_M;
} cache_params_t;

// Put additional data structures here as per your requirement.
typedef
struct {
   int tag_sizeL1;
   int index_sizeL1;
   int offset_sizeL1;
   int tag_sizeL2;
   int index_sizeL2;
   int offset_sizeL2;
} bit_length;

typedef
struct{
   uint32_t LRU;
}L1_cache_param;

typedef
struct
{
   int L1_request;
   float L1_miss_rate;
   int L1_write_miss;
   int L1_read_miss;
   int L1_write;
   int L1_read;
   int L1_writeback;
   int L2_request;
   
   float L2_miss_rate;
   int L2_write_miss;
   int L2_read_miss;
   int L2_write;
   int L2_read;
   int L2_writeback;
   int Memory_traffic;
}output_report;

typedef
struct
{
   uint32_t Valid_bit_L1[1000][1000];
   uint32_t Dirty_bit_L1[1000][1000];
   uint32_t Tag_bits_L1[1000][1000];
   uint32_t LRU_L1[1000][1000];
   uint32_t Valid_bit_L2[1000][1000];
   uint32_t Dirty_bit_L2[1000][1000];
   uint32_t Tag_bits_L2[1000][1000];/* data */
   uint32_t LRU_L2[1000][1000];
}L1L2;


/*
typedef
struct{
   boolean *valid_bits[][];
   uint32_t *tag_bits[][];
   uint32_t *LRU[];
}L2_cache_param;*/
#endif
