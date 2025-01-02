#include "cache.h"


#define MAIN_MEMORY_SIZE (1 << 20) // 2^20 words (1MB of memory)
#define CACHE_SIZE 256            // 256 words
#define BLOCK_SIZE 4              // 4 words per block
#define NUM_BLOCKS (CACHE_SIZE / BLOCK_SIZE)  // Number of cache blocks
#define BLOCK_OFFSET_BITS 2       // log2(4) = 2 bits to index a word within a block
#define INDEX_BITS 6              // log2(64) = 6 bits for cache index
#define TAG_BITS (32 - INDEX_BITS - BLOCK_OFFSET_BITS) // Assuming 32-bit address


// Function to log cache state to a text file
void log_mesibus(MESI_bus *bus, int cycle) {
    // Log DSRAM state
    if (bus->logfile == NULL) {
        printf("Error: Logfile not initialized for DSRAM.\n");
        return; // Prevent further execution if logfile is not available
    }

    // Log the bus details in the required format
    fprintf(bus->logfile, "%d   %d    %d   0x%08X   %d       %d\n", 
            cycle, 
            bus->bus_origid, 
            bus->bus_cmd, 
            bus->bus_addr, 
            bus->bus_data, 
            bus->bus_shared);
}

/******************************************************************************
* Function: snoop_bus
*
* Description: simulating all caches(except the one that initiated the transaction) to check what's on the bus
* maybe they need to send their data to the bus, or invalidate their cache line 
* should occur every cycle
*******************************************************************************/
void snoop_bus(DSRAM dsrams[], TSRAM tsrams[], BusOperation op, int cache_id, uint32_t address, uint32_t *data_out, MESI_bus *bus) { //change to pointers
    uint32_t tag, index, block_offset;
    get_cache_address_parts(address, &tag, &index, &block_offset);
    bus->bus_cmd = op;  // Set the bus command
    bus->bus_addr = address;  // Set the bus address
    bus->bus_origid = cache_id;  // Set the originator of the bus transaction
    bus->bus_data = 0;  // Initialize bus data to zero

    // Debug print to see cache address parts
    printf("address:%08X, tag:%d, index:%d, offset: %d\n", address, tag, index, block_offset);
        switch (bus->bus_cmd) {
            case NO_COMMAND:
                // No command, just snooping
                printf("Snooping NO_COMMAND: No operation.\n");
                bus->bus_shared = 0;  // Data is not shared
                break;
            case BUS_RD:
                // Bus read operation : When a BusRd (Bus Read) transaction occurs on the bus, it indicates that a processor or cache is requesting a block of data from the memory system.
                printf("Snooping BUS_READ\n");
                int *cache_own_id = check_shared_bus(tsrams, dsrams,cache_id, address); // Check if the data is in another cache, if yes, return the cache id
                if (cache_own_id != NULL) {
                    // Data found in another cache
                    if(tsrams[cache_own_id[0]].cache[index].mesi_state == SHARED){
                        {
                            printf("FOUND A SHARED BLOCK: Data found in cache %d, fetching from there...\n", cache_own_id);
                            dsrams[cache_id].cache[index].data[block_offset] = dsrams[cache_own_id[0]].cache[index].data[block_offset];
                            *data_out = dsrams[cache_id].cache[index].data[block_offset];
                            bus->bus_data = dsrams[cache_own_id[0]].cache[index].data[block_offset];
                            bus->bus_shared = 1;  // Data is shared
                        }

                    }

                    else if (tsrams[cache_own_id[0]].cache[index].mesi_state == EXCLUSIVE){
                        {
                            printf("FOUND AN EXCLUSIVE BLOCK: Data found in cache %d, fetching from there...\n", cache_own_id);
                            dsrams[cache_id].cache[index].data[block_offset] = dsrams[cache_own_id[0]].cache[index].data[block_offset];
                            *data_out = dsrams[cache_id].cache[index].data[block_offset];
                            bus->bus_data = dsrams[cache_own_id[0]].cache[index].data[block_offset];
                            tsrams[cache_id].cache[index].mesi_state = SHARED;  // Data is shared ///change also in another cache!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                            bus->bus_data = dsrams[cache_own_id[0]].cache[index].data[block_offset];
                            bus->bus_shared = 1;  // Data is not shared
                        }
                    }

                    else if(tsrams[cache_own_id[0]].cache[index].mesi_state == MODIFIED){
                        {
                            printf("FOUND A MODIFIED BLOCK: Data found in cache %d, fetching from there...\n", cache_own_id);
                            snoop_bus(dsrams, tsrams, FLUSH, cache_own_id[0], address, dsrams[cache_own_id[0]].cache[index].data, bus); //perform flush to update the main memory
                            tsrams[cache_id].cache[index].mesi_state = SHARED;  // Data is shared in the requesting cache
                            tsrams[cache_own_id[0]].cache[index].mesi_state = SHARED;  // Data is shared in the cache with the modified data
                            dsrams[cache_id].cache[index].data[block_offset] = dsrams[cache_own_id[0]].cache[index].data[block_offset]; // Copy the data to the requesting cache
                            bus->bus_data = dsrams[cache_own_id[0]].cache[index].data[block_offset]; //send the data to the bus
                            bus->bus_shared = 1;  // Data is shared
                            //----------------------------------------------------------need to perform flush------------------------------------------------
                        }
                    }
                    else
                    {
                    //fetch from main memory
                    // Data not found in any cache
                    printf("Data not found in any cache, fetching from main memory...\n");
                    // Calculate the starting address of the block in main memory
                    uint32_t block_start = (address / BLOCK_SIZE) * BLOCK_SIZE;

                    // Fetch the entire block from main memory
                    for (int i = 0; i < BLOCK_SIZE; i++) {
                        dsrams[cache_id].cache[index].data[i] = main_memory[block_start + i];
                    }
                    tsrams[cache_id].cache[index].tag = tag;
                    tsrams[cache_id].cache[index].mesi_state = EXCLUSIVE;  // Data is shared
                    *data_out = dsrams[cache_id].cache[index].data[block_offset];
                    bus->bus_data = dsrams[cache_id].cache[index].data[block_offset];
                    bus->bus_shared = 0;  // Not shared anymore

                    }
                }


            case BUS_RDX:
                // Bus read exclusive operation : When a BusRdX (Bus Read Exclusive) transaction occurs on the bus, it indicates that a processor or cache is requesting a block of data from the memory system with the intent to write to it.
                printf("Snooping BUS_READ_EXCLUSIVE\n");
                int *cache_own_id = check_shared_bus(tsrams, dsrams,cache_id, address); // Check if the data is in another cache, if yes, return the cache id
                if (cache_own_id != NULL) {
                    // Data found in another cache
                    if(tsrams[cache_own_id[0]].cache[index].mesi_state == SHARED){ ////////should be a loop on the array
                        {
                            printf("FOUND A SHARED BLOCK: Data found in cache %d, fetching from there...\n", cache_own_id);
                            bus->bus_data = dsrams[cache_own_id[0]].cache[index].data[block_offset];
                            dsrams[cache_id].cache[index].data[block_offset] = dsrams[cache_own_id[0]].cache[index].data[block_offset]; // Copy the data to the requesting cache
                            tsrams[cache_id].cache[index].mesi_state = EXCLUSIVE;  // Data is exclusive in the requesting cache (will be modified later)
                            tsrams[cache_own_id[0]].cache[index].mesi_state = INVALID;  // Data is invalid in the cache with the shared data
                            
                            //*data_out = dsrams[cache_id].cache[index].data[block_offset];
                            //bus->bus_data = dsrams[cache_own_id[0]].cache[index].data[block_offset];
                            //bus->bus_shared = 1;  // Data is shared
                        }

                    }

                    else if (tsrams[cache_own_id[0]].cache[index].mesi_state == EXCLUSIVE){
                        {
                            printf("FOUND AN EXCLUSIVE BLOCK: Data found in cache %d, fetching from there...\n", cache_own_id);
                            dsrams[cache_id].cache[index].data[block_offset] = dsrams[cache_own_id[0]].cache[index].data[block_offset]; // Copy the data to the requesting cache
                            tsrams[cache_id].cache[index].mesi_state = EXCLUSIVE;  // Data is exclusive in the requesting cache (will be modified later)
                            tsrams[cache_own_id[0]].cache[index].mesi_state = INVALID;  // Data is invalid in the cache with the shared data
                            bus->bus_shared = 1;  // Data is not shared
                        }
                    }

                    else if(tsrams[cache_own_id[0]].cache[index].mesi_state == MODIFIED){
                        {   
                            printf("FOUND A MODIFIED BLOCK: Data found in cache %d, fetching from there...\n", cache_own_id);
                            //send block to the main memory
                            snoop_bus(dsrams, tsrams, FLUSH, cache_own_id[0], address, dsrams[cache_own_id[0]].cache[index].data, bus); //perform flush to update the main memory
                            tsrams[cache_id].cache[index].mesi_state = EXCLUSIVE;  // Data is exclusive in the requesting cache
                            tsrams[cache_own_id[0]].cache[index].mesi_state = INVALID;  // Data is invalid in the cache with the shared data
                        }
                    }
                    else
                    {
                    //fetch from main memory --- for later...
                    //than the block is exclusive (will be modified later)

                    }

            case FLUSH: 
                printf("Snooping FLUSH\n");
                // Flush operation: When a Flush transaction occurs on the bus, it indicates that a cache line is being invalidated.
                // Invalidate the cache line
                //write the modified block to the main memory
                //invalidate the cache line



            default:
                printf("Unknown bus operation.\n");
        }
}


void initialize_mesi_bus(MESI_bus *bus, const char *log_filename) {
    bus->bus_origid = 0;  // Set the originator of the bus transaction
    bus->bus_cmd = 0;        // Set the bus command (e.g., BUS_RD, BUS_RDX, etc.)
    bus->bus_addr = 0;       // Set the address for the bus operation
    bus->bus_data = 0;          // Set the data for write operations (optional)
    bus->bus_shared = 0;      // Set shared state for read operations (1 if shared, 0 if exclusive)
    bus->logfile = fopen(log_filename, "w");
    if (bus->logfile == NULL) {
        perror("Error opening log file for DSRAM");
        exit(EXIT_FAILURE);
    }
    fprintf(bus->logfile, "Cyc Orig Cmd    Addr      Data  Shared\n");	
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



/******************************************************************************
* Function: send_op_to_bus
*
* Description: send command to the bus
*******************************************************************************/
void send_op_to_bus(MESI_bus *bus, int origid, BusOperation cmd, int addr) {
    bus->bus_origid = origid;
    bus->bus_cmd = cmd;
    bus->bus_addr = addr;
}

/******************************************************************************
* Function: send_data_to_bus
*
* Description: send data to the bus. can happen when flushing/send data from other cache to another.
* 
*******************************************************************************/
void send_data_to_bus(MESI_bus *bus, int data, int origid, int bus_shared) {
    bus->bus_data = data;
    bus->bus_origid = origid;
    bus->bus_shared = bus_shared;
}


/******************************************************************************
* Function: read_from_bus
*
* Description: read data from the bus. will occur after a read transaction.
* if shared signal is 1, data brought from another cache, if 0, databrought from main memory
*******************************************************************************/
void read_from_bus(DSRAM *dsram, TSRAM *tsram, int *data, int address, MESI_bus *bus)
{
    uint32_t tag, index, block_offset;
    get_cache_address_parts(address, &tag, &index, &block_offset);
    CacheLine *dsram_line = &dsram->cache[index];
    CacheLine_TSRAM *tsram_line = &tsram->cache[index];
        if(bus->bus_shared == 1) //data found in another cache
        {
            *data = &dsram->cache[index].data[block_offset]; //copy the data from the cache
            tsram->cache[index].mesi_state = SHARED; //data is shared
            printf("Data found in another cache! Data: %u\n", *data);
        }
        else //data not found in any cache -> fetch from main memory
        {
            printf("Data on the bus is from memory...\n");
            *data = &dsram->cache[index].data[block_offset]; //send data to the core
            tsram->cache[index].mesi_state = EXCLUSIVE; //data is shared
        }
}

/******************************************************************************
* Function: cache_read
*
* Description: read command. first check if the data is in the cache, if not, send a bus transaction.
* after that, all other caches will see the transaction. if it's in another cache, they will send the data to the bus and shared = 1. then we call read_from_bus
*******************************************************************************/
bool cache_read(DSRAM dsram, TSRAM tsram, int orig_id, uint32_t address, uint32_t *data, MESI_bus *mesi_bus) {
    uint32_t tag, index, block_offset;
    get_cache_address_parts(address, &tag, &index, &block_offset);
    CacheLine *dsram_line = &dsram.cache[index];
    CacheLine_TSRAM *tsram_line = &tsram.cache[index];
    //3 options: cache hit, cache miss, data found in another cache, cache miss, data not found in any cache
    // 1.cache hit: valid block, tag matches, no command on the bus
    // 2.cache miss: check if the block is in another cache, if yes, bring the data from that cache(STATE SHARED). If not, fetch from main memory(STATE EXCLUSIVE)
    if (tsram_line->mesi_state != INVALID && tsram_line->tag == tag) //cache hit, valid block
    {
        *data = dsram_line->data[block_offset];
        printf("Cache hit! Data: %u\n", *data);
    }

    else { // Cache miss, check if the data is in another cache -> create bus transaction
        printf("Cache miss! check if the block is in another cache...\n");
        send_op_to_bus(mesi_bus, orig_id, BUS_RD, address);      //***********  ************/
    }
        /*
        //wait for the bus to finish the transaction...........
        if(mesi_bus->bus_shared == 1) //data found in another cache
        {
            *data = dsram.cache[index].data[block_offset]; //copy the data from the cache
            tsram.cache[index].mesi_state = SHARED; //data is shared
            printf("Data found in another cache! Data: %u\n", *data);
            cycles = 1;  // 1 cycle for cache hit
        }
        else //data not found in any cache -> fetch from main memory
        {
            printf("Data not found in any cache, fetching from main memory...\n");
            // Calculate the starting address of the block in main memory
            uint32_t block_start = (address / BLOCK_SIZE) * BLOCK_SIZE;

            // Fetch the entire block from main memory
            for (int i = 0; i < BLOCK_SIZE; i++) {
                dsram.cache[index].data[i] = main_memory[block_start + i]; //copy data to dsram
            }
            tsram.cache[index].tag = tag; //update tag in tsram
            tsram.cache[index].mesi_state = EXCLUSIVE;  // update state in tsram
            *data = dsram.cache[index].data[block_offset]; //send data to the core
        }*/
        log_cache_state(&dsram, &tsram);
}



// Function to check which caches have the data
int* check_shared_bus(TSRAM tsrams[], DSRAM dsrams[], int origid, int address) {
    printf("Bus: Issuing BUSRD for address %d\n", address);
    uint32_t tag, index, block_offset;
    get_cache_address_parts(address, &tag, &index, &block_offset);

    // Array to store the indexes of caches that have the block
    static int cache_indexes[4];  // Static array to hold the cache indexes
    int found_count = 0;  // Initialize the found count to 0

    // Check other caches
    for (int i = 0; i < 4; i++) {
        if (i == origid) continue; // Skip the requesting cache
        CacheLine_TSRAM *tsram_line = &tsrams[i].cache[index];
        if (tsram_line->mesi_state != INVALID && tsram_line->tag == tag) {
            // Data found in cache i
            printf("Bus: Data found in cache %d\n", i);
            cache_indexes[found_count] = i;  // Store the cache index
            (found_count)++;  // Increment the found count
        }
    }

    // Return an empty array if no caches have the block
    if (found_count == 0) {
        return NULL;  // Return NULL if no caches have the data
    }

    // Return the array of cache indexes
    return cache_indexes;
}




void cache_write(DSRAM *dsram, TSRAM *tsram, uint32_t address, uint32_t data, MESI_bus *mesi_bus) {
    uint32_t tag, index, block_offset;
    get_cache_address_parts(address, &tag, &index, &block_offset);

    CacheLine *dsram_line = &dsram->cache[index];
    CacheLine_TSRAM *tsram_line = &tsram->cache[index];
    uint64_t cycles = 0;
    // Snooping the bus to maintain cache coherence

    // Check if it's a cache hit
    if (tsram_line->mesi_state != INVALID && tsram_line->tag == tag) {
        if (tsram_line->mesi_state == EXCLUSIVE) {
            // Cache hit in EXCLUSIVE state, write data to the cache
            dsram_line->data[block_offset] = data;
            tsram_line->mesi_state = MODIFIED;  // Transition to MODIFIED because the cache line has been updated
            mesi_bus->bus_cmd = 0; // No command
            printf("Cache write hit (EXCLUSIVE)! Data written to index %u, block offset %u\n", index, block_offset);
            cycles = 1;  // 1 cycle for cache write hit

        } else if (tsram_line->mesi_state == MODIFIED) {
            // Cache hit, write data to the cache in MODIFIED or SHARED state
            dsram_line->data[block_offset] = data;
            tsram_line->mesi_state = MODIFIED;  // Set dirty bit because the cache line has been modified
            mesi_bus->bus_cmd = 0; // No command
            printf("Cache write hit! Data written to index %u, block offset %u\n", index, block_offset);
            cycles = 1;  // 1 cycle for cache write hit
        }

        else 
        {
            dsram_line->data[block_offset] = data;
            tsram_line->mesi_state = MODIFIED;  // Set dirty bit because the cache line has been modified
            mesi_bus->bus_cmd = 2; // BUS_RDX (Exclusive read)
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

