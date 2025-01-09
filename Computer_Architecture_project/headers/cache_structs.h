#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef CACHE_STRUCTS_H
#define CACHE_STRUCTS_H
// Constants for memory and cache configuration
#define LINE_CONTENT 0x00000000 // Content for each memory line

#define MAIN_MEMORY_SIZE (1 << 20)  // 2^20 words (1MB of memory)
#define CACHE_SIZE 256             // 256 words
#define BLOCK_SIZE 4               // 4 words per block
#define NUM_BLOCKS (CACHE_SIZE / BLOCK_SIZE) // Number of cache blocks
#define BLOCK_OFFSET_BITS 2        // log2(4) = 2 bits to index a word within a block
#define INDEX_BITS 6               // log2(64) = 6 bits for cache index
#define TAG_BITS (32 - INDEX_BITS - BLOCK_OFFSET_BITS) // Assuming 32-bit address
#define MAIN_MEMORY_STALLS 16
#define DATA_IS_READY 1

// struct global{
//     int num_words_sent; 
//     int block_offset_counter; 
//     int main_memory_stalls_counter = 0; 
// }Global; 


    extern int num_words_sent; 
     extern int block_offset_counter; 
     extern int main_memory_stalls_counter; 

// Cache Line structure
typedef struct cacheLine{
    uint32_t data[BLOCK_SIZE];  // Data: 4 words (32 bits each)
} CacheLine;

typedef enum {
    INVALID,
    SHARED,
    EXCLUSIVE,
    MODIFIED
} MESIState;

typedef enum {
    NO_COMMAND,          // Request for shared read
    BUS_RD, // Request for exclusive read (write intent)
    BUS_RDX,    // Write data back to main memory
    FLUSH     // Invalidate cache line
} BusOperation;



// Data Cache structure (DSRAM)
typedef struct {
    CacheLine cache[NUM_BLOCKS]; // Cache array with 64 blocks
    uint64_t cycle_count;        // Total cycles counter
    FILE *logfile;
} DSRAM;

typedef struct {
    uint32_t tag;               
    MESIState mesi_state;           // Coherence state
} CacheLine_TSRAM;

typedef struct {
    CacheLine_TSRAM cache[NUM_BLOCKS]; // Cache array with 64 blocks
    uint64_t cycle_count;        // Total cycles counter
    FILE *logfile;
} TSRAM;


typedef struct {
    DSRAM* dsram; 
    TSRAM* tsram; 
    int cache_id ; 
    int ack;
} CACHE;


typedef struct
{
    int bus_origid; //Originator of this transaction 0: core 0 1: core 1 2: core 2 3: core 3 4: main memory
    BusOperation bus_cmd; // 0: no command, 1: busrd, 2:busrdx, 3:flush
    int bus_addr; //word address
    int bus_data;// word data
    int bus_shared; // set to 1 when answering a BusRd transaction if any of the cores has the data in the cache, otherwise set to 0.

    int bus_requesting_id; /********************************************************** */

    FILE *logfile;
} MESI_bus;


typedef struct {
    int *memory_data;  // Pointer to the memory array
    FILE *logfile;     // Log file pointer
} MainMemory;

#endif // CPU_STRUCTS_H
