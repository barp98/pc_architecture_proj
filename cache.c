#include "cache.h"


#define MAIN_MEMORY_SIZE (1 << 20) // 2^20 words (1MB of memory)
#define CACHE_SIZE 256            // 256 words
#define BLOCK_SIZE 4              // 4 words per block
#define NUM_BLOCKS (CACHE_SIZE / BLOCK_SIZE)  // Number of cache blocks
#define BLOCK_OFFSET_BITS 2       // log2(4) = 2 bits to index a word within a block
#define INDEX_BITS 6              // log2(64) = 6 bits for cache index
#define TAG_BITS (32 - INDEX_BITS - BLOCK_OFFSET_BITS) // Assuming 32-bit address


void snoop_bus(DSRAM *dsram, TSRAM *tsram, BusOperation op, uint32_t address, uint32_t *data_out) {
    uint32_t tag, index, block_offset;
    get_cache_address_parts(address, &tag, &index, &block_offset);
    printf("address:%xx0, tag:%d, index:%d, offset: %d\n", address,  tag, index, block_offset);
    CacheLine *dsram_line = &dsram->cache[index];
    CacheLine_TSRAM *tsram_line = &tsram->cache[index];
    //printf("%d %d %d\n", line->state, line->tag, line->valid);
    //printf("line tag?: %d, tag?: %d, line->tag == tag?: %d\n", line->tag,tag, line->tag == tag);
    //printf("line valid?:%d\n", line->valid);
    //printf("if is true:? %d\n", line->valid && line->tag == tag);


    if (tsram_line->mesi_state != INVALID  && tsram_line->tag == tag) {
        switch (op) {
            case BUS_READ:
                if (tsram_line->mesi_state == MODIFIED) {
                    // Write back to memory and transition to SHARED
                    printf("Snooping BUS_READ: Writing back modified data.\n");
                    for (int i = 0; i < BLOCK_SIZE; i++) {
                        main_memory[(address / BLOCK_SIZE) * BLOCK_SIZE + i] = dsram_line->data[i];
                    }
                    tsram_line->mesi_state = SHARED;
                } else if (tsram_line->mesi_state == EXCLUSIVE || tsram_line->mesi_state == SHARED) {
                    // Transition to SHARED
                    printf("Snooping BUS_READ: Transitioning to SHARED.\n");
                    tsram_line->mesi_state = SHARED;
                }
                break;

            case BUS_READ_EXCLUSIVE:
                // Invalidate the cache line
                printf("Snooping BUS_READ_EXCLUSIVE: Invalidating cache line.\n");
                tsram_line->mesi_state = INVALID;
                break;

            case BUS_INVALIDATE:
                // Invalidate the cache line
                printf("Snooping BUS_INVALIDATE: Invalidating cache line.\n");
                tsram_line->mesi_state = INVALID;
                break;

            case BUS_WRITE_BACK:
                if (tsram_line->mesi_state == MODIFIED) {
                    // Write back the modified data
                    printf("Snooping BUS_WRITE_BACK: Writing back modified data.\n");
                    for (int i = 0; i < BLOCK_SIZE; i++) {
                        main_memory[(address / BLOCK_SIZE) * BLOCK_SIZE + i] = dsram_line->data[i];
                    }                    
                    tsram_line->mesi_state = INVALID;
                } else if (tsram_line->mesi_state == SHARED) {
                    // No need to write back as other caches may already have it
                    printf("Snooping BUS_WRITE_BACK: No write needed, already shared.\n");
                }
                break;

            default:
                printf("Unknown bus operation.\n");
        }
    } else if (op == BUS_READ || op == BUS_READ_EXCLUSIVE || op == BUS_INVALIDATE || op == BUS_WRITE_BACK) {
        // No valid line found or the line is not for this tag
        printf("No valid cache line found for the given address or state mismatch.\n");
    }
}

// Main Memory (2^20 words)
uint32_t main_memory[MAIN_MEMORY_SIZE];

void initialize_DSRAM(DSRAM *dsram, const char *log_filename) {
    // Initialize cache data
    for (int i = 0; i < NUM_BLOCKS; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            dsram->cache[i].data[j] = 0;  // Clear data
        }
    }
    dsram->cycle_count = 0;  // Reset cycle count

    // Open logfile for DSRAM
    dsram->logfile = fopen(log_filename, "w");
    if (dsram->logfile == NULL) {
        perror("Error opening log file for DSRAM");
        exit(EXIT_FAILURE);
    }
}

// Function to initialize TSRAM
void initialize_TSRAM(TSRAM *tsram, const char *log_filename) {
    // Initialize cache state and tags
    for (int i = 0; i < NUM_BLOCKS; i++) {
        tsram->cache[i].tag = 0;  // Clear the tag
        tsram->cache[i].mesi_state = INVALID;  // Set state to INVALID
    }
    tsram->cycle_count = 0;  // Reset cycle count

    // Open logfile for TSRAM
    tsram->logfile = fopen(log_filename, "w");
    if (tsram->logfile == NULL) {
        perror("Error opening log file for TSRAM");
        exit(EXIT_FAILURE);
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
void log_cache_state(DSRAM *dsram, TSRAM *tsram) {
    // Log DSRAM state
    if (dsram->logfile == NULL) {
        printf("Error: Logfile not initialized for DSRAM.\n");
        return; // Prevent further execution if logfile is not available
    }

    fprintf(dsram->logfile, "DSRAM Cache State:%d\n", dsram->cycle_count);
    fprintf(dsram->logfile, "Block Number |Data[0]    Data[1]    Data[2]    Data[3]\n");
    fprintf(dsram->logfile, "---------------------------------------------------\n");

    for (int i = 0; i < NUM_BLOCKS; i++) {
        CacheLine *dsram_line = &dsram->cache[i];
        fprintf(dsram->logfile, "%12d | ", i);  // Log block number
        
        for (int j = 0; j < BLOCK_SIZE; j++) {
            fprintf(dsram->logfile, "0x%08X ", dsram_line->data[j]);  // Log data in each block
        }
        fprintf(dsram->logfile, "\n");
    }

    fprintf(dsram->logfile, "---------------------------------------------------\n");
    fflush(dsram->logfile);

    // Log TSRAM state
    if (tsram->logfile == NULL) {
        printf("Error: Logfile not initialized for TSRAM.\n");
        return; // Prevent further execution if logfile is not available
    }

    fprintf(tsram->logfile, "TSRAM Cache State:\n");
    fprintf(tsram->logfile, "Block Number | MESI State | Tag        \n");
    fprintf(tsram->logfile, "---------------------------------------------------\n");

    for (int i = 0; i < NUM_BLOCKS; i++) {
        CacheLine_TSRAM *tsram_line = &tsram->cache[i];
        fprintf(tsram->logfile, "%12d | %-11s | 0x%08X\n", 
                i, 
                (tsram_line->mesi_state == INVALID ? "INVALID" :
                 (tsram_line->mesi_state == SHARED ? "SHARED" :
                 (tsram_line->mesi_state == EXCLUSIVE ? "EXCLUSIVE" : "MODIFIED"))),
                tsram_line->tag);  // Log MESI state and tag
    }

    fprintf(tsram->logfile, "---------------------------------------------------\n");
    fflush(tsram->logfile);
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

bool cache_read(DSRAM *dsram, TSRAM *tsram, uint32_t address, uint32_t *data) {
    uint32_t tag, index, block_offset;
    get_cache_address_parts(address, &tag, &index, &block_offset);

    CacheLine *dsram_line = &dsram->cache[index];
    CacheLine_TSRAM *tsram_line = &tsram->cache[index];
    uint64_t cycles = 0;

    // Snooping the bus to ensure cache coherence before reading
    snoop_bus(dsram,tsram, BUS_READ, address, data);

    if (tsram_line->mesi_state == INVALID  && tsram_line->tag == tag) {
        // Cache hit, return the data (hit takes 1 cycle)
        *data = dsram_line->data[block_offset];
        printf("Cache hit! Data: %u\n", *data);
        cycles = 1;  // 1 cycle for cache hit
    } else {
        // Cache miss, data needs to be fetched from memory (or allocate a new block)
        printf("Cache miss! Fetching block from main memory...\n");

        // Calculate the starting address of the block in main memory
        uint32_t block_start = (address / BLOCK_SIZE) * BLOCK_SIZE;

        // Fetch the entire block from main memory
        for (int i = 0; i < BLOCK_SIZE; i++) {
            dsram_line->data[i] = main_memory[block_start + i];
        }
        tsram_line->tag = tag;
        tsram_line->mesi_state = SHARED;
        // Return the data from the correct block offset
        *data = dsram_line->data[block_offset];

        cycles = 16;  // Assume 16 cycles for fetching a block from main memory
    }

    dsram->cycle_count += cycles;  // Update total cycle count
    printf("Cycles after operation: %lu\n", dsram->cycle_count);  // Print total cycles after each operation

    // Log the cache state after this operation
    log_cache_state(dsram, tsram);
    //write_cache_to_file(dsram); // Write cache state to separate file

    return (cycles == 1);  // Return true for cache hit
}

void cache_write(DSRAM *dsram, TSRAM *tsram, uint32_t address, uint32_t data) {
    uint32_t tag, index, block_offset;
    get_cache_address_parts(address, &tag, &index, &block_offset);

    CacheLine *dsram_line = &dsram->cache[index];
    CacheLine_TSRAM *tsram_line = &tsram->cache[index];
    uint64_t cycles = 0;

    // Snooping the bus to maintain cache coherence
    snoop_bus(dsram, tsram, BUS_WRITE_BACK, address, &data);

    // Check if it's a cache hit
    if (tsram_line->mesi_state != INVALID && tsram_line->tag == tag) {
        if (tsram_line->mesi_state == EXCLUSIVE) {
            // Cache hit in EXCLUSIVE state, write data to the cache
            dsram_line->data[block_offset] = data;
            tsram_line->mesi_state = MODIFIED;  // Transition to MODIFIED because the cache line has been updated
            printf("Cache write hit (EXCLUSIVE)! Data written to index %u, block offset %u\n", index, block_offset);
            cycles = 1;  // 1 cycle for cache write hit
        } else {
            // Cache hit, write data to the cache in MODIFIED or SHARED state
            dsram_line->data[block_offset] = data;
            tsram_line->mesi_state = MODIFIED;  // Set dirty bit because the cache line has been modified
            printf("Cache write hit! Data written to index %u, block offset %u\n", index, block_offset);
            cycles = 1;  // 1 cycle for cache write hit
        }
    } else {
        // Cache miss, allocate a new block in the cache
        // Check if the cache line is dirty and needs to be written back to memory
        if (tsram_line->mesi_state != INVALID && tsram_line->mesi_state == MODIFIED) {
            uint32_t block_start_address = 
                (tsram_line->tag << (INDEX_BITS + BLOCK_OFFSET_BITS)) | (index << BLOCK_OFFSET_BITS);
            
            printf("Writing back dirty data to main memory at addresses: 0x%08X - 0x%08X\n",
                   block_start_address, block_start_address + BLOCK_SIZE - 1);
            fprintf(dsram->logfile, "Writing back dirty data to main memory at addresses: 0x%08X - 0x%08X\n",
                    block_start_address, block_start_address + BLOCK_SIZE - 1);

            // Update main memory and log each word being written back
            for (int i = 0; i < BLOCK_SIZE; i++) {
                main_memory[block_start_address + i] = dsram_line->data[i];
                fprintf(dsram->logfile, "Main Memory [0x%08X] = 0x%08X\n",
                        block_start_address + i, dsram_line->data[i]);
            }
        }

        // Allocate the block and load data into the cache
        uint32_t block_start_address = 
            (tag << (INDEX_BITS + BLOCK_OFFSET_BITS)) | (index << BLOCK_OFFSET_BITS);
        for (int i = 0; i < BLOCK_SIZE; i++) {
            dsram_line->data[i] = main_memory[block_start_address + i];
        }

        tsram_line->tag = tag;  // Update the tag
        dsram_line->data[block_offset] = data;  // Write the new data to the cache
        tsram_line->mesi_state = MODIFIED;  // Set dirty bit because the cache line has been modified

        printf("Cache write miss! Data written to index %u, block offset %u\n", index, block_offset);
        cycles = 16;  // Assume 16 cycles for cache miss (fetch and write)
    }

    dsram->cycle_count += cycles;  // Update total cycle count
    printf("Cycles after write: %lu\n", dsram->cycle_count);  // Print total cycles after each write operation

    // Log the cache state after this operation
    log_cache_state(dsram, tsram);  // Log both DSRAM and TSRAM states
    //write_cache_to_file(dsram);  // Write cache state to separate file
}

