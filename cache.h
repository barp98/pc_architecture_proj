#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Constants for memory and cache configuration
#define MAIN_MEMORY_SIZE (1 << 20)  // 2^20 words (1MB of memory)
#define CACHE_SIZE 256             // 256 words
#define BLOCK_SIZE 4               // 4 words per block
#define NUM_BLOCKS (CACHE_SIZE / BLOCK_SIZE) // Number of cache blocks
#define BLOCK_OFFSET_BITS 2        // log2(4) = 2 bits to index a word within a block
#define INDEX_BITS 6               // log2(64) = 6 bits for cache index
#define TAG_BITS (32 - INDEX_BITS - BLOCK_OFFSET_BITS) // Assuming 32-bit address


//#define INVALID   0  // Cache line is invalid
//#define SHARED    1  // Cache line is valid and shared with other caches
//#define EXCLUSIVE 2  // Cache line is valid and only present in this cache
//#define MODIFIED  3  // Cache line is valid, modified, and dirty

// Main Memory (1MB)
extern uint32_t main_memory[MAIN_MEMORY_SIZE];

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

typedef struct
{
    int bus_origid; //Originator of this transaction 0: core 0 1: core 1 2: core 2 3: core 3 4: main memory
    BusOperation bus_cmd; // 0: no command, 1: busrd, 2:busrdx, 3:flush
    int bus_addr; //word address
    int bus_data;// word data
    int bus_shared; // set to 1 when answering a BusRd transaction if any of the cores has the data in the cache, otherwise set to 0.
    FILE *logfile;
} MESI_bus;

// Function Prototypes
void log_mesibus(MESI_bus *bus, int cycle);
// Function to initialize the DSRAM (cache) with the value 2
void init_dsr_cache(DSRAM *dsram);

// Function to initialize the main memory with some values
void init_main_memory();

// Function to extract the tag, index, and block offset from an address
void get_cache_address_parts(uint32_t address, uint32_t *tag, uint32_t *index, uint32_t *block_offset);

// Function to log the cache state to a text file
void log_cache_state(DSRAM *dsram, TSRAM *tsram);

// Function to write the cache state to a file
//void write_cache_to_file(DSRAM *dsram);

// Function to print the main memory to a text file
void write_main_memory_to_file(FILE *file);

// Cache read operation, returns true for cache hit
bool cache_read(DSRAM dsram, TSRAM tsram, int orig_id, uint32_t address, uint32_t *data, MESI_bus *mesi_bus);

// Cache write operation
void cache_write(DSRAM *dsram, TSRAM *tsram, uint32_t address, uint32_t data, MESI_bus *mesi_bus);

int read_from_main_memory(int *main_memory, int address);

void snoop_bus(DSRAM *dsram, TSRAM *tsram, BusOperation op, uint32_t address, uint32_t *data_out, MESI_bus *mesi_bus);

void init_caches(DSRAM dsrams[], TSRAM tsrams[]);

void initialize_DSRAM(DSRAM *dsram, const char *log_filename);

void initialize_TSRAM(TSRAM *tsram, const char *log_filename);

void initialize_mesi_bus(MESI_bus *bus, const char *log_filename);

int* check_shared_bus(TSRAM tsrams[], DSRAM dsrams[], int origid, int address);
