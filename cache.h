#ifndef CACHE_H
#define CACHE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>  // Include stdio.h for FILE type and file operations

// Constants for memory and cache configuration
#define MAIN_MEMORY_SIZE (1 << 20)  // 2^20 words (1MB of memory)
#define CACHE_SIZE 256             // 256 words
#define BLOCK_SIZE 4               // 4 words per block
#define NUM_BLOCKS (CACHE_SIZE / BLOCK_SIZE) // Number of cache blocks
#define BLOCK_OFFSET_BITS 2        // log2(4) = 2 bits to index a word within a block
#define INDEX_BITS 6               // log2(64) = 6 bits for cache index
#define TAG_BITS (32 - INDEX_BITS - BLOCK_OFFSET_BITS) // Assuming 32-bit address

// Main Memory (1MB)
extern uint32_t main_memory[MAIN_MEMORY_SIZE];

// Cache Line structure
typedef struct {
    uint32_t data[BLOCK_SIZE];  // Data: 4 words (32 bits each)
    uint32_t tag;               // Tag for comparison
    bool valid;                 // Valid bit
    bool dirty;                 // Dirty bit (write-back)
} CacheLine;

// Data Cache structure (DSRAM)
typedef struct {
    CacheLine cache[NUM_BLOCKS]; // Cache array with 64 blocks
    uint64_t cycle_count;        // Total cycles counter
} DSRAM;

// Function Prototypes

// Function to initialize the DSRAM (cache) with the value 2
void init_dsr_cache(DSRAM *dsram);

// Function to initialize the main memory with some values
void init_main_memory();

// Function to extract the tag, index, and block offset from an address
void get_cache_address_parts(uint32_t address, uint32_t *tag, uint32_t *index, uint32_t *block_offset);

// Function to log the cache state to a text file
void log_cache_state(DSRAM *dsram, FILE *logfile);

// Function to write the cache state to a file
void write_cache_to_file(DSRAM *dsram);

// Function to print the main memory to a text file
void write_main_memory_to_file(FILE *file);

// Cache read operation, returns true for cache hit
bool cache_read(DSRAM *dsram, uint32_t address, uint32_t *data, FILE *logfile);

// Cache write operation
void cache_write(DSRAM *dsram, uint32_t address, uint32_t data, FILE *logfile);

#endif // CACHE_H