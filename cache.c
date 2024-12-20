#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAIN_MEMORY_SIZE (1 << 20) // 2^20 words (1MB of memory)
#define CACHE_SIZE 256            // 256 words
#define BLOCK_SIZE 4              // 4 words per block
#define NUM_BLOCKS (CACHE_SIZE / BLOCK_SIZE)  // Number of cache blocks
#define BLOCK_OFFSET_BITS 2       // log2(4) = 2 bits to index a word within a block
#define INDEX_BITS 6              // log2(64) = 6 bits for cache index
#define TAG_BITS (32 - INDEX_BITS - BLOCK_OFFSET_BITS) // Assuming 32-bit address

// Main Memory (2^20 words)
uint32_t main_memory[MAIN_MEMORY_SIZE];

// Cache Line structure
typedef struct {
    uint32_t data[BLOCK_SIZE];  // Data: 4 words (32 bits each)
    uint32_t tag;               // Tag for comparison
    bool valid;                 // Valid bit
    bool dirty;                 // Dirty bit (write-back)
} CacheLine;

// Data Cache structure
typedef struct {
    CacheLine cache[NUM_BLOCKS];  // Cache array with 64 blocks
    uint64_t cycle_count;         // Total cycles counter
} DSRAM;

// Function to initialize the DSRAM (cache) with the value 2
void init_dsr_cache(DSRAM *dsram) {
    dsram->cycle_count = 0;  // Initialize cycle count to zero
    for (int i = 0; i < NUM_BLOCKS; i++) {
        dsram->cache[i].valid = false;   // Initially invalid
        dsram->cache[i].dirty = false;   // Initially not dirty
        // Initialize all data in the cache block to 2
        for (int j = 0; j < BLOCK_SIZE; j++) {
            dsram->cache[i].data[j] = 0; // Initialize all words to 2
        }
    }
}

// Function to initialize the main memory with the value 2
void init_main_memory() {
    for (int i = 0; i < MAIN_MEMORY_SIZE; i++) {
        main_memory[i] = i; // Initialize memory with the value 2
    }
}

// Extract the tag, index, and block offset from an address
void get_cache_address_parts(uint32_t address, uint32_t *tag, uint32_t *index, uint32_t *block_offset) {
    *block_offset = address & ((1 << BLOCK_OFFSET_BITS) - 1);  // Extract block offset (2 bits)
    *index = (address >> BLOCK_OFFSET_BITS) & ((1 << INDEX_BITS) - 1);  // Extract index (6 bits)
    *tag = address >> (BLOCK_OFFSET_BITS + INDEX_BITS);  // Extract the remaining bits as tag
}

// Function to log cache state to a text file
void log_cache_state(DSRAM *dsram, FILE *logfile) {
    fprintf(logfile, "Cache State:\n");
    for (int i = 0; i < NUM_BLOCKS; i++) {
        CacheLine *line = &dsram->cache[i];
        fprintf(logfile, "Block %d: ", i);
        fprintf(logfile, "Valid: %d, Dirty: %d, Tag: 0x%X, Data: ", line->valid, line->dirty, line->tag);
        for (int j = 0; j < BLOCK_SIZE; j++) {
            fprintf(logfile, "0x%X ", line->data[j]);
        }
        fprintf(logfile, "\n");
    }
    fprintf(logfile, "End of Cache State\n\n");
}

// Function to write cache state to a separate text file
void write_cache_to_file(DSRAM *dsram) {
    FILE *cachefile = fopen("cache_state_log.txt", "w");
    if (cachefile == NULL) {
        perror("Error opening cache state file");
        return;
    }

    fprintf(cachefile, "Cache State:\n");
    for (int i = 0; i < NUM_BLOCKS; i++) {
        CacheLine *line = &dsram->cache[i];
        fprintf(cachefile, "Block %d: ", i);
        fprintf(cachefile, "Valid: %d, Dirty: %d, Tag: 0x%X, Data: ", line->valid, line->dirty, line->tag);
        for (int j = 0; j < BLOCK_SIZE; j++) {
            fprintf(cachefile, "0x%08X ", line->data[j]);
        }
        fprintf(cachefile, "\n");
    }
    fprintf(cachefile, "End of Cache State\n\n");

    fclose(cachefile);
}

int read_from_main_memory(int *main_memory, int address) {
    // Check if the address is within the valid range
    if (address < 0 || address >= MAIN_MEMORY_SIZE) {
        fprintf(stderr, "Error: Invalid memory address %d\n", address);
        exit(EXIT_FAILURE); // Exit the program or handle the error appropriately
    }

    // Return the value at the specified memory address
    return (int)main_memory[address];
}

// Function to print the main memory to a text file
void write_main_memory_to_file(FILE *file) {
    fprintf(file, "Main Memory State:\n");
    for (int i = 0; i < MAIN_MEMORY_SIZE; i++) {
        // Print memory in a readable format (e.g., words in hex)
        fprintf(file, "Address 0x%08X: 0x%08X\n", i, main_memory[i]);
    }
    fprintf(file, "End of Main Memory State\n\n");
}

bool cache_read(DSRAM *dsram, uint32_t address, uint32_t *data, FILE *logfile) {
    uint32_t tag, index, block_offset;
    get_cache_address_parts(address, &tag, &index, &block_offset);

    CacheLine *line = &dsram->cache[index];
    uint64_t cycles = 0;

    if (line->valid && line->tag == tag) {
        // Cache hit, return the data (hit takes 1 cycle)
        *data = line->data[block_offset];
        printf("Cache hit! Data: %u\n", *data);
        cycles = 1;  // 1 cycle for cache hit
    } else {
        // Cache miss, data needs to be fetched from memory (or allocate a new block)
        printf("Cache miss! Fetching block from main memory...\n");

        // Calculate the starting address of the block in main memory
        uint32_t block_start = (address / BLOCK_SIZE) * BLOCK_SIZE;

        // Fetch the entire block from main memory
        for (int i = 0; i < BLOCK_SIZE; i++) {
            line->data[i] = main_memory[block_start + i];
        }
        line->tag = tag;
        line->valid = true;
        *data = line->data[block_offset];

        cycles = 16;  // Assume 16 cycles for fetching a block from main memory
    }

    dsram->cycle_count += cycles;  // Update total cycle count
    printf("Cycles after operation: %lu\n", dsram->cycle_count);  // Print total cycles after each operation

    // Log the cache state after this operation
    log_cache_state(dsram, logfile);
    write_cache_to_file(dsram); // Write cache state to separate file

    return (cycles == 1);  // Return true for cache hit
}

void cache_write(DSRAM *dsram, uint32_t address, uint32_t data, FILE *logfile) {
    uint32_t tag, index, block_offset;
    get_cache_address_parts(address, &tag, &index, &block_offset);

    CacheLine *line = &dsram->cache[index];
    uint64_t cycles = 0;

    if (line->valid && line->tag == tag) {
        // Cache hit, write data to the cache line (hit takes 1 cycle)
        line->data[block_offset] = data;
        line->dirty = true;  // Set dirty bit because the cache line has been modified
        printf("Cache write hit! Data written to index %u, block offset %u\n", index, block_offset);
        cycles = 1;  // 1 cycle for cache write hit
    } else {
        // Cache miss, allocate a new block in the cache
        if (line->valid && line->dirty) {
            // Write back the dirty cache line to memory (if necessary)
            uint32_t block_start_address = 
                (line->tag << (INDEX_BITS + BLOCK_OFFSET_BITS)) | (index << BLOCK_OFFSET_BITS);
            
            printf("Writing back dirty data to main memory at addresses: 0x%08X - 0x%08X\n",
                   block_start_address, block_start_address + BLOCK_SIZE - 1);
            fprintf(logfile, "Writing back dirty data to main memory at addresses: 0x%08X - 0x%08X\n",
                    block_start_address, block_start_address + BLOCK_SIZE - 1);

            // Update main memory and log each word being written back
            for (int i = 0; i < BLOCK_SIZE; i++) {
                main_memory[block_start_address + i] = line->data[i];
                fprintf(logfile, "Main Memory [0x%08X] = 0x%08X\n",
                        block_start_address + i, line->data[i]);
            }
        }

        // Allocate the block and load data
        uint32_t block_start_address = 
            (tag << (INDEX_BITS + BLOCK_OFFSET_BITS)) | (index << BLOCK_OFFSET_BITS);
        for (int i = 0; i < BLOCK_SIZE; i++) {
            line->data[i] = main_memory[block_start_address + i];
        }
        line->valid = true;
        line->tag = tag;
        line->data[block_offset] = data;  // Write data to cache
        line->dirty = true;  // Set dirty bit because the cache line has been modified

        printf("Cache write miss! Data written to index %u, block offset %u\n", index, block_offset);
        cycles = 16;  // Assume 16 cycles for cache miss (fetch and write)
    }

    dsram->cycle_count += cycles;  // Update total cycle count
    printf("Cycles after write: %lu\n", dsram->cycle_count);  // Print total cycles after each write operation
}

