
#include "cache_structs.h"
#include "utils.h"
#ifndef CACHE_H
#define CACHE_H
// Function Prototypes

void log_main_memory(MainMemory *main_memory);
void log_mesibus(MESI_bus *bus, int cycle);
// Function to initialize the DSRAM (cache) with the value 2
//void init_dsr_cache(DSRAM *dsram);

// Function to extract the tag, index, and block offset from an address
void get_cache_address_parts(uint32_t address, uint32_t *tag, uint32_t *index, uint32_t *block_offset);

// Function to log the cache state to a text file
void log_cache_state(CACHE *cache);

// Function to write the cache state to a file
//void write_cache_to_file(DSRAM *dsram);

// Function to print the main memory to a text file
void write_main_memory_to_file(FILE *file);

void write_after_busrdx(CACHE *requesting, int origid, uint32_t address, uint32_t data, MESI_bus *mesi_bus);

// Cache read operation, returns true for cache hit
bool cache_read(CACHE *cache, uint32_t address, uint32_t *data, MESI_bus *mesi_bus);

// Cache write operation
bool cache_write(CACHE *cache, uint32_t address, MESI_bus *mesi_bus, int data);

//int read_from_main_memory(int *main_memory, int address);

int snoop_bus(CACHE *caches[], uint32_t address, MESI_bus *bus, MainMemory *main_memory);
//void init_caches(DSRAM dsrams[], TSRAM tsrams[]);

int* check_shared_bus(CACHE *caches[], int origid, int address);

void send_data_to_bus(MESI_bus *bus, int data, int origid, int bus_shared, int address, int requesting_id);

void send_data_from_main_memory_to_bus(MainMemory *main_memory, MESI_bus *bus, int address);

void dsram_read_data_from_bus(CACHE *cache, int address, MESI_bus *bus);

void dsram_write_data_to_bus(CACHE *cache, int address, MESI_bus *bus);

#endif // CACHE_STRUCTS_H
