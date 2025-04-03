
//#define _CRTDBG_MAP_ALLOC
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
//#include<crtdbg.h> //only for debug
#include "sim.h"

/*
uint32_t* Valid_bit_L1;
uint32_t* Dirty_bit_L1;
uint32_t* Tag_bits_L1;
uint32_t* LRU_L1;
uint32_t* Valid_bit_L2;
uint32_t* Dirty_bit_L2;
uint32_t* Tag_bits_L2;*/
L1L2 mem;

bit_length add_div;  //this structure holds parameters like tag bits, index bits, and offsetbits.
cache_params_t params;	// Look at the sim.h header file for the definition of struct cache_params_t.
output_report output;

//uint32_t settemp; //remove this in final code
void Cache_creater();
uint32_t CurrentLRU(uint32_t row,uint32_t L1_OR_L2);
void L1_cache(uint32_t address, char read);
void L2_cache(uint32_t address, char read);
void Main_memory();
void UpdateLRUL1(uint32_t row, uint32_t colomn);
void UpdateLRUL2(uint32_t row, uint32_t colomn);
void PrintResults();
void Miss_rate();
void PrintCacheL1(uint32_t SET);
void PrintCacheL2(uint32_t SET);
//void printLRU();

int main (int argc, char *argv[]) {
   // if this code don't work 
   //add the below line to the new code downloaded from the moodel
   //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
   /*argc = 9;
   argv[0] = "./sim";
   argv[1] = "32";
   argv[2] = "1024";
   argv[3] = "2";
   argv[4] = "12288";
   argv[5] = "6";
   argv[6] = "0";
   argv[7] = "0";
   argv[8] = "gcc_trace.txt";*/
   FILE *fp;			// File pointer.
   char *trace_file;		// This variable holds the trace file name.
   
   
   
   char rw;			// This variable holds the request's type (read or write) obtained from the trace.
   uint32_t addr;		// This variable holds the request's address obtained from the trace.
				// The header file <inttypes.h> above defines signed and unsigned integers of various sizes in a machine-agnostic way.  "uint32_t" is an unsigned integer of 32 bits.

   // Exit with an error if the number of command-line arguments is incorrect.
   if (argc != 9) {
      printf("Error: Expected 8 command-line arguments but was provided %d.\n", (argc - 1));
      exit(EXIT_FAILURE);
   }
    
   // "atoi()" (included by <stdlib.h>) converts a string (char *) to an integer (int).
   params.BLOCKSIZE = (uint32_t) atoi(argv[1]);
   params.L1_SIZE   = (uint32_t) atoi(argv[2]);
   params.L1_ASSOC  = (uint32_t) atoi(argv[3]);
   params.L2_SIZE   = (uint32_t) atoi(argv[4]);
   params.L2_ASSOC  = (uint32_t) atoi(argv[5]);
   params.PREF_N    = (uint32_t) atoi(argv[6]);
   params.PREF_M    = (uint32_t) atoi(argv[7]);
   trace_file       = argv[8];


   // Open the trace file for reading.
   fp = fopen(trace_file, "r");
   if (fp == (FILE *) NULL) {
      // Exit with an error if file open failed.
      printf("Error: Unable to open file %s\n", trace_file);
      exit(EXIT_FAILURE);
   }
    
   // Print simulator configuration.
   printf("===== Simulator configuration =====\n");
   printf("BLOCKSIZE:  %u\n", params.BLOCKSIZE);
   printf("L1_SIZE:    %u\n", params.L1_SIZE);
   printf("L1_ASSOC:   %u\n", params.L1_ASSOC);
   printf("L2_SIZE:    %u\n", params.L2_SIZE);
   printf("L2_ASSOC:   %u\n", params.L2_ASSOC);
   printf("PREF_N:     %u\n", params.PREF_N);
   printf("PREF_M:     %u\n", params.PREF_M);
   printf("trace_file: %s\n", trace_file);
   printf("===================================\n\n");
   /*
   -------------------------------------------------------------------------
   finding all the parameters like tag, index, and offset bits
   -------------------------------------------------------------------------
   */
   uint32_t setsL1 = 0;
   uint32_t setsL2 = 0;
   //printf("==== Tag, index, and offset bits ====\n");
   if(params.L1_SIZE > 0)
   {
   setsL1 =  (params.L1_SIZE)/(params.BLOCKSIZE*params.L1_ASSOC);
   add_div.index_sizeL1 = log2(setsL1);
   add_div.offset_sizeL1 = log2(params.BLOCKSIZE);
   add_div.tag_sizeL1 = (32-add_div.index_sizeL1-add_div.offset_sizeL1);
   }
   if(params.L2_SIZE > 0)
   {
   setsL2 =  (params.L2_SIZE)/(params.BLOCKSIZE*params.L2_ASSOC);
   add_div.index_sizeL2 = log2(setsL2);
   add_div.offset_sizeL2 = log2(params.BLOCKSIZE);
   add_div.tag_sizeL2 = (32-add_div.index_sizeL2-add_div.offset_sizeL2);
   }
   if((params.L1_SIZE == 0)&&(params.L2_SIZE == 0))
   {
      printf("No cache created");
      setsL1 =0;
      setsL2 = 0;
      exit(EXIT_FAILURE);   
   }
   //settemp =setsL1;
   Cache_creater(setsL1,setsL2);

   // Read requests from the trace file and echo them back.
   while (fscanf(fp, "%c %x\n", &rw, &addr) == 2) {	// Stay in the loop if fscanf() successfully parsed two tokens as specified.
      if (rw == 'r'){
         //L1_cache(addr,rw);
         }
      else if (rw == 'w'){
         //L1_cache(addr,rw);
         }
      else {
         printf("Error: Unknown request type %c.\n", rw);
	 exit(EXIT_FAILURE);
      }
      L1_cache(addr,rw);
      output.L1_request = output.L1_request + 1;
      
    }
   Miss_rate();
   PrintCacheL1(setsL1);
   PrintCacheL2(setsL2);
   PrintResults();
   return(0);
}

void Cache_creater(uint32_t SETL1,uint32_t SETL2)
{
   int i,j;
   for(i=0;i<SETL1;i++)
   {
      for (j =0;j<params.L1_ASSOC;j++)
      {
         mem.LRU_L1[i][j] = j;  //loading LRU serially
         //printf("%d",mem.LRU_L1[i][j]);
      }
     // printf("\n");
   }
   //printf("==================\n");
   for(i=0;i<SETL2;i++)
   {
      for (j =0;j<params.L2_ASSOC;j++)
      {
         mem.LRU_L2[i][j] = j;  //loading LRU serially
         //printf("%d",mem.LRU_L2[i][j]);
      }
      //printf("\n");
   }
}
void L1_cache(uint32_t address, char read)
{
   int i;
   char Hit = 'f';
   uint32_t LRU_colomn;
   uint32_t Current_Tag;
   uint32_t Current_Index;
   uint32_t Construct_address;
   Current_Tag = address >> add_div.offset_sizeL1;
   Current_Tag = Current_Tag >> add_div.index_sizeL1;
   Current_Index = address << add_div.tag_sizeL1;
   Current_Index = Current_Index >> add_div.tag_sizeL1;
   Current_Index = Current_Index >> add_div.offset_sizeL1;

   for(i =0; i<params.L1_ASSOC;i++)
   {
      if(mem.Valid_bit_L1[Current_Index][i] == 1)
      {
         if((mem.Tag_bits_L1[Current_Index][i]==Current_Tag)&&(Hit != 't'))
         {
            Hit = 't';
            UpdateLRUL1(Current_Index,i);
            if(read == 'w')
            {
               output.L1_write = output.L1_write + 1;
               mem.Dirty_bit_L1[Current_Index][i] = 1;  //==========error memory leak================
            }
            else
               output.L1_read = output.L1_read + 1;
            //mem.Tag_bits_L1[Current_Index][i] = Current_Tag;
         }
      }
   }
  if(Hit != 't')
  {
  LRU_colomn = CurrentLRU(Current_Index,1); //provide the most LRU block in the given set
   if(mem.Valid_bit_L1[Current_Index][LRU_colomn] == 1)
   {
      if(mem.Dirty_bit_L1[Current_Index][LRU_colomn] == 1)
      {
         mem.Valid_bit_L1[Current_Index][LRU_colomn] = 1;//try removing this line
         Construct_address = mem.Tag_bits_L1[Current_Index][LRU_colomn]<<add_div.index_sizeL1;//left shifting previous tag bits with index
         Construct_address = (Construct_address|Current_Index);
         Construct_address = Construct_address<<add_div.offset_sizeL1;
         if(params.L2_SIZE != 0)
            L2_cache(Construct_address,'w'); //writing back to L2
         else
            Main_memory();
         output.L1_writeback = output.L1_writeback + 1;
         mem.Dirty_bit_L1[Current_Index][LRU_colomn] = 0;  //clear Dirty bit for the same postion after write back
         /*
         ==============================================================
                  L1 is missed so sending a request to L2
               if prefech is present then write here for L1
         ==============================================================
         */
         Construct_address = Current_Tag<<add_div.index_sizeL1;
         Construct_address = (Construct_address|Current_Index);
         Construct_address = Construct_address<<add_div.offset_sizeL1;
         //printf("current address: %x\n Constructed address: %x\n",address,Construct_address);
         if(params.L2_SIZE != 0)
            L2_cache(Construct_address,'r');
         else
            Main_memory();
         if(read == 'w')      //Checking for write condition ==========error memory leak================
            {
               mem.Dirty_bit_L1[Current_Index][LRU_colomn] = 1; //if true then update the dirty bit
               output.L1_write = output.L1_write + 1;//==========error memory leak================
               output.L1_write_miss = output.L1_write_miss +1;
            }
         else{
               output.L1_read = output.L1_read + 1;
               output.L1_read_miss = output.L1_read_miss +1;
         }
         mem.Tag_bits_L1[Current_Index][LRU_colomn] = Current_Tag;
         UpdateLRUL1(Current_Index,LRU_colomn);  //LRU register Updating
      }
      else
      {
             //Tag Value Updating
         mem.Valid_bit_L1[Current_Index][LRU_colomn] = 1;    //Valid Bit Updating//======Error Memory leak======
         /*
         ==============================================================
                  L1 is missed so sending a request to L2
               if prefech is present then write here for L1
         ==============================================================
         */
        
         Construct_address = Current_Tag<<add_div.index_sizeL1;
         Construct_address = (Construct_address|Current_Index);
         Construct_address = Construct_address<<add_div.offset_sizeL1;
         //printf("current address: %x\n Constructed address: %x\n",address,Construct_address);
         if(params.L2_SIZE != 0)
            L2_cache(Construct_address,'r');
         else
            Main_memory();
         if(read == 'w')      //Checking for write condition
            {
               mem.Dirty_bit_L1[Current_Index][LRU_colomn] = 1; //if true then update the dirty bit
               output.L1_write = output.L1_write + 1;//======= Error memory leak======
               output.L1_write_miss = output.L1_write_miss +1;
            }
         else{
               output.L1_read = output.L1_read + 1;
               output.L1_read_miss = output.L1_read_miss +1;
         }
         mem.Tag_bits_L1[Current_Index][LRU_colomn] = Current_Tag;
         UpdateLRUL1(Current_Index,LRU_colomn);  //LRU register Updating
      }
   }
   else
   {
          //Tag Bit Updating
         /*
         ==============================================================
                  L1 is missed so sending a request to L2
               if prefech is present then write here for L1
         ==============================================================
         */
        
         Construct_address = Current_Tag<<add_div.index_sizeL1;
         Construct_address = (Construct_address|Current_Index);
         Construct_address = Construct_address<<add_div.offset_sizeL1;
         //printf("address %x: reconstructed address %x\n",address,Construct_address);
         //printf("current address: %x\n Constructed address: %x\n",address,Construct_address);
         if(params.L2_SIZE != 0)
            L2_cache(Construct_address,'r');
         else
            Main_memory();
         mem.Valid_bit_L1[Current_Index][LRU_colomn] = 1;       //Valid Bit updating ============error of memory leak===============
         if(read == 'w')      //Checking for write condition=================error memory leak=================
            {
               mem.Dirty_bit_L1[Current_Index][LRU_colomn] = 1; //if true then update the dirty bit
               output.L1_write = output.L1_write + 1;//==========error memory leak================
               output.L1_write_miss = output.L1_write_miss +1;
            }
         else{
               output.L1_read = output.L1_read + 1;
               output.L1_read_miss = output.L1_read_miss +1;
         }
         mem.Tag_bits_L1[Current_Index][LRU_colomn] = Current_Tag;
         UpdateLRUL1(Current_Index,LRU_colomn);    //LRU Register Updating
   }
  } 
}
void L2_cache(uint32_t address, char read)
{
   //L1 will only send tag+index bits but we wont get the offset in the address
   //has in write back condition so for that we only have tags and index in that case its not possible to get the offset
   int i;
   char Hit = 'f';
   uint32_t LRU_colomn;
   uint32_t Current_Tag;
   uint32_t Current_Index;
   output.L2_request = output.L2_request + 1;
   Current_Tag = address >> add_div.offset_sizeL2;
   //printf("Current: %x \n",Current_Tag);
   Current_Tag = Current_Tag >> add_div.index_sizeL2;
   //Current_Tag = address >> 6;
   Current_Index = address << add_div.tag_sizeL2;
   Current_Index = Current_Index >> add_div.tag_sizeL2;
   Current_Index = Current_Index >> add_div.offset_sizeL2;
   //printf("Current: %x \n",Current_Tag);
   for(i =0; i<params.L2_ASSOC;i++)
   {
      if(mem.Valid_bit_L2[Current_Index][i] == 1)
      {
         if((mem.Tag_bits_L2[Current_Index][i]==Current_Tag)&&(Hit != 't'))
         {
            Hit = 't';
            
            if(read == 'w')
            {
               output.L2_write = output.L2_write + 1;
               mem.Dirty_bit_L2[Current_Index][i] = 1;  //==========error memory leak================
            }
            else
               output.L2_read = output.L2_read + 1;
            mem.Tag_bits_L2[Current_Index][i] = Current_Tag;
            UpdateLRUL2(Current_Index,i);
         }
      }
   }
  if(Hit != 't')
  {
  LRU_colomn = CurrentLRU(Current_Index,2); //provide the most LRU block in the given set
   if(mem.Valid_bit_L2[Current_Index][LRU_colomn] == 1)
   {
      if(mem.Dirty_bit_L2[Current_Index][LRU_colomn] == 1)
      {
         mem.Valid_bit_L2[Current_Index][LRU_colomn] = 1;
        Main_memory();     //writing back to main memory
         output.L2_writeback = output.L2_writeback + 1;
         mem.Dirty_bit_L2[Current_Index][LRU_colomn] = 0;  //clear Dirty bit for the same postion after write back
         /*
         ==============================================================
              L2 is missed so sending a request to MainMemory
               if prefech is present then write here for L2
         ==============================================================
         */
        Main_memory();  //L2 cache is missed so requesting main memory
         if(read == 'w')      //Checking for write condition ==========error memory leak================
            {
               mem.Dirty_bit_L2[Current_Index][LRU_colomn] = 1; //if true then update the dirty bit
               output.L2_write = output.L2_write + 1;//==========error memory leak================
               output.L2_write_miss = output.L2_write_miss +1;
            }
         else{
               output.L2_read = output.L2_read + 1;
               output.L2_read_miss = output.L2_read_miss +1;
         }
         mem.Tag_bits_L2[Current_Index][LRU_colomn] = Current_Tag;
         UpdateLRUL2(Current_Index,LRU_colomn);  //LRU register Updating
      }
      else
      {
             //Tag Value Updating
         /*
         ==============================================================
              L2 is missed so sending a request to MainMemory
               if prefech is present then write here for L2
         ==============================================================
         */
        Main_memory();  //L2 cache is missed so requesting main memory
         mem.Valid_bit_L2[Current_Index][LRU_colomn] = 1;    //Valid Bit Updating//======Error Memory leak======
         if(read == 'w')      //Checking for write condition
            {
               mem.Dirty_bit_L2[Current_Index][LRU_colomn] = 1; //if true then update the dirty bit
               output.L2_write = output.L2_write + 1;//======= Error memory leak======
               output.L2_write_miss = output.L2_write_miss +1;
            }
         else{
               output.L2_read = output.L2_read + 1;
               output.L2_read_miss = output.L2_read_miss +1;
         }
         mem.Tag_bits_L2[Current_Index][LRU_colomn] = Current_Tag;
         UpdateLRUL2(Current_Index,LRU_colomn);  //LRU register Updating
         
      }
   }
   else
   {
          //Tag Bit Updating
         /*
         ==============================================================
              L2 is missed so sending a request to MainMemory
               if prefech is present then write here for L2
         ==============================================================
         */
        Main_memory();  //L2 cache is missed so requesting main memory
         mem.Valid_bit_L2[Current_Index][LRU_colomn] = 1;       //Valid Bit updating ============error of memory leak===============
         if(read == 'w')      //Checking for write condition=================error memory leak=================
            {
               mem.Dirty_bit_L2[Current_Index][LRU_colomn] = 1; //if true then update the dirty bit
               output.L2_write = output.L2_write + 1;//==========error memory leak================
               output.L2_write_miss = output.L2_write_miss +1;
            }
         else{
               output.L2_read = output.L2_read + 1;
               output.L2_read_miss = output.L2_read_miss +1;
         }
         mem.Tag_bits_L2[Current_Index][LRU_colomn] = Current_Tag;
         UpdateLRUL2(Current_Index,LRU_colomn);    //LRU Register Updating
   }
  } 
}
void Main_memory()
{
   output.Memory_traffic = output.Memory_traffic+1;
}
uint32_t CurrentLRU(uint32_t row, uint32_t L1_OR_L2)
{
   uint32_t j;
   if (L1_OR_L2 == 1)
   for(j = 0;j<params.L1_ASSOC;j++)
   {
      if(mem.LRU_L1[row][j] == (params.L1_ASSOC-1)){
         return j;
      }
   }
   else if (L1_OR_L2 == 2)
   for(j = 0;j<params.L2_ASSOC;j++)
   {
      if(mem.LRU_L2[row][j] == (params.L2_ASSOC-1)){
         return j;
      }
   }
   printf("Problem finding the LRU in LRU array\n");     //remove this line in final code this is only for testing
   return 0xFFFFFFFF;
}
void UpdateLRUL1(uint32_t row, uint32_t colomn)
{
   //LRU must be changed in hit and Miss
   /*
   'H' is for Hit LRU Update
   'M' is for Miss LRU Update
   */
  int count = 0;
  int j;
      for(j = 0; j<params.L1_ASSOC;j++)
      {
         if(mem.LRU_L1[row][j]<mem.LRU_L1[row][colomn])
         {
            mem.LRU_L1[row][j] = mem.LRU_L1[row][j] + 1;
            
         }
         count++;
      }
      mem.LRU_L1[row][colomn] = 0;
}
void UpdateLRUL2(uint32_t row, uint32_t colomn)
{
   //LRU must be changed in hit and Miss
   /*
   'H' is for Hit LRU Update
   'M' is for Miss LRU Update
   */
  int count = 0;
  int j;
      for(j = 0; j<params.L2_ASSOC;j++)
      {
         if(mem.LRU_L2[row][j]<mem.LRU_L2[row][colomn])
         {
            mem.LRU_L2[row][j] = mem.LRU_L2[row][j] + 1;
            
         }
         count++;
      }
      mem.LRU_L2[row][colomn] = 0;
}
void PrintResults()
{
   printf("\n===== Measurements =====\n");
   printf("a. L1 reads:                   %d\n",output.L1_read);
   printf("b. L1 read misses:             %d\n",output.L1_read_miss);
   printf("c. L1 writes:                  %d\n",output.L1_write);
   printf("d. L1 write misses:            %d\n",output.L1_write_miss);
   printf("e. L1 miss rate:               %f\n",output.L1_miss_rate);
   printf("f. L1 writebacks:              %d\n",output.L1_writeback);
   printf("g. L1 prefetches:              %d\n",output.L1_writeback);
   printf("h. L2 reads (demand):          %d\n",output.L2_read);
   printf("i. L2 read misses (demand):    %d\n",output.L2_read_miss);
   printf("j. L2 reads (prefetch):        %d\n",output.L1_writeback);
   printf("k. L2 read misses (prefetch):  %d\n",output.L1_writeback);
   printf("l. L2 writes:                  %d\n",output.L2_write);
   printf("m. L2 write misses:            %d\n",output.L2_write_miss);
   printf("n. L2 miss rate:               %f\n",output.L2_miss_rate);
   printf("o. L2 writebacks:              %d\n",output.L2_writeback);
   printf("p. L2 prefetches:              %d\n",output.L1_writeback);
   printf("q. memory traffic:             %d\n",output.Memory_traffic);
}
void PrintCacheL1(uint32_t SET)
{
   int i,j;
   printf("\n===== L1 contents =====\n");
   for(i = 0;i<SET; i++)
   {
      printf("set    %u",i);
      for(j=0;j<params.L1_ASSOC;j++)
      {
         printf("\t%x",mem.Tag_bits_L1[i][j]);
         if (mem.Dirty_bit_L1[i][j] == 1)
         printf("\tD");
         else
         printf("\t ");
      }
      printf("\n");
   }
}
void PrintCacheL2(uint32_t SET)
{
   int i,j;
   printf("\n===== L2 contents =====\n");
   for(i = 0;i<SET; i++)
   {
      printf("set    %u",i);
      for(j=0;j<params.L2_ASSOC;j++)
      {
         printf("\t");
         printf("%x",mem.Tag_bits_L2[i][j]);
         if (mem.Dirty_bit_L2[i][j] == 1)
         printf("\tD");
         else
         printf("\t ");
      }
      printf("\n");
   }
}
void Miss_rate()
{
   output.L1_miss_rate = ((output.L1_read_miss+output.L1_write_miss)/output.L1_request);
   if(params.L2_SIZE != 0)
   output.L2_miss_rate = ((output.L2_read_miss+output.L2_write_miss)/output.L2_request);
}