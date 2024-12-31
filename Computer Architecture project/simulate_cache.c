#include "cache.h"
#include <stdio.h>
#include <stdlib.h>

// Define the number of caches
#define NUM_CACHES 4

// Function to initialize all caches



// Function to simulate cache operations
void simulate_cache_operations(DSRAM dsrams[], TSRAM tsrams[], MESI_bus *bus) {
    uint32_t data;
    uint32_t address;
    bool hit;
    int cycle = 0;

    printf("Simulating cache operations...\n");

    // First cache access (Write to address 0x00001000)
    address = 0x00001000;
    printf("\nCycle %d: Writing data to address 0x%08X...\n", cycle, address);
    cache_write(&dsrams[0], &tsrams[0], address, 42, bus);
    log_mesibus(bus, cycle++);

    // Second cache access (Read from address 0x00001000)
    address = 0x00001000;
    hit = cache_read(&dsrams[0], &tsrams[0], address, &data, bus);
    printf("Cycle %d: Read from address 0x%08X: Data = %u (Hit: %s)\n", cycle, address, data, hit ? "Yes" : "No");
    log_mesibus(bus, cycle++);

    // Third cache access (Write to address 0x00002000)
    address = 0x00002000;
    printf("\nCycle %d: Writing data to address 0x%08X...\n", cycle, address);
    cache_write(&dsrams[1], &tsrams[1], address, 84, bus);
    log_mesibus(bus, cycle++);

    // Fourth cache access (Read from address 0x00002000)
    address = 0x00002000;
    hit = cache_read(&dsrams[1], &tsrams[1], address, &data, bus);
    printf("Cycle %d: Read from address 0x%08X: Data = %u (Hit: %s)\n", cycle, address, data, hit ? "Yes" : "No");
    log_mesibus(bus, cycle++);

    // Fifth cache access (Write to address 0x00001000 for cache 2)
    address = 0x00001000;
    printf("\nCycle %d: Writing data to address 0x%08X in cache 2...\n", cycle, address);
    cache_write(&dsrams[2], &tsrams[2], address, 100, bus);
    log_mesibus(bus, cycle++);

    // Sixth cache access (Read from address 0x00001000 in cache 2)
    address = 0x00001000;
    hit = cache_read(&dsrams[2], &tsrams[2], address, &data, bus);
    printf("Cycle %d: Read from address 0x%08X in cache 2: Data = %u (Hit: %s)\n", cycle, address, data, hit ? "Yes" : "No");
    log_mesibus(bus, cycle++);

    // Simulate bus snooping operations
    printf("\nSimulating bus snooping operations...\n");

    // Simulate a bus read operation
    address = 0x00001000;
    snoop_bus(&dsrams[0], &tsrams[0], BUS_READ, address, &data, bus);
    log_mesibus(bus, cycle++);

    // Simulate a bus write-back operation
    address = 0x00002000;
    snoop_bus(&dsrams[1], &tsrams[1], BUS_WRITE_BACK, address, &data, bus);
    log_mesibus(bus, cycle++);

    // Simulate a bus invalidate operation
    address = 0x00001000;
    snoop_bus(&dsrams[3], &tsrams[3], BUS_INVALIDATE, address, &data, bus);
    log_mesibus(bus, cycle++);

    // Simulate another cache read
    address = 0x00001000;
    hit = cache_read(&dsrams[0], &tsrams[0], address, &data, bus);
    printf("Cycle %d: Read from address 0x%08X after snooping: Data = %u (Hit: %s)\n", cycle, address, data, hit ? "Yes" : "No");
    log_mesibus(bus, cycle++);

    // Final state logging
    log_cache_state(&dsrams[0], &tsrams[0]);
    log_cache_state(&dsrams[1], &tsrams[1]);
    log_cache_state(&dsrams[2], &tsrams[2]);
    log_cache_state(&dsrams[3], &tsrams[3]);
}

int main() {

    // Declare and initialize DSRAM and TSRAM
// Declare arrays of DSRAM and TSRAM (assuming 4 caches for example)
    printf("hello");
    DSRAM dsrams[4];
    TSRAM tsrams[4];
    MESI_bus bus;
    int clock = 0;

    // Initialize DSRAM and TSRAM caches
    initialize_mesi_bus(&bus, "bustrace.txt");
    initialize_DSRAM(&dsrams[0], "dsram_log_0.txt");
    initialize_DSRAM(&dsrams[1], "dsram_log_1.txt");
    initialize_DSRAM(&dsrams[2], "dsram_log_2.txt");
    initialize_DSRAM(&dsrams[3], "dsram_log_3.txt");

    initialize_TSRAM(&tsrams[0], "tsram_log_0.txt");
    initialize_TSRAM(&tsrams[1], "tsram_log_1.txt");
    initialize_TSRAM(&tsrams[2], "tsram_log_2.txt");
    initialize_TSRAM(&tsrams[3], "tsram_log_3.txt");
    printf("hello");
    // Simulate cache operations
    simulate_cache_operations(dsrams, tsrams, &bus);

    return 0;





    // Continue with further processing...
    return 0;

}
