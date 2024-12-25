#include <stdio.h>
#include <stdlib.h>
#include "cache.h"

void test_quad_cache_operations(DSRAM *cache1, DSRAM *cache2, DSRAM *cache3, DSRAM *cache4, FILE *logfile1, FILE *logfile2, FILE *logfile3, FILE *logfile4) {
    uint32_t data;

    printf("Testing quad cache operations with bus snooping...\n");

    // Test 1: Cache 1 reads (causes a miss), other caches snoop BUS_READ
    uint32_t address1 = 0x00000100;  // Address within range
    printf("\nTest 1: Cache 1 reads address 0x%08X (should cause a miss)\n", address1);
    if (!cache_read(cache1, address1, &data, logfile1)) {
        printf("Cache 1 miss as expected. Data loaded: 0x%08X\n", data);
    }
    printf("Other caches snoop BUS_READ for the same address.\n");
    snoop_bus(cache2, BUS_READ, address1, NULL);
    snoop_bus(cache3, BUS_READ, address1, NULL);
    snoop_bus(cache4, BUS_READ, address1, NULL);

    // Log cache states after the operation
    log_cache_state(cache1, logfile1);
    log_cache_state(cache2, logfile2);
    log_cache_state(cache3, logfile3);
    log_cache_state(cache4, logfile4);

    // Test 2: Cache 2 writes (causes a miss), other caches snoop BUS_READ_EXCLUSIVE
    uint32_t address2 = 0x00000200;  // Another address within range
    uint32_t write_data = 0xDEADBEEF;
    printf("\nTest 2: Cache 2 writes to address 0x%08X (should cause a miss)\n", address2);
    cache_write(cache2, address2, write_data, logfile2);
    printf("Other caches snoop BUS_READ_EXCLUSIVE for the same address.\n");
    snoop_bus(cache1, BUS_READ_EXCLUSIVE, address2, NULL);
    snoop_bus(cache3, BUS_READ_EXCLUSIVE, address2, NULL);
    snoop_bus(cache4, BUS_READ_EXCLUSIVE, address2, NULL);

    // Log cache states after the operation
    log_cache_state(cache1, logfile1);
    log_cache_state(cache2, logfile2);
    log_cache_state(cache3, logfile3);
    log_cache_state(cache4, logfile4);

    // Test 3: Cache 3 writes (causes a hit), other caches snoop BUS_INVALIDATE
    printf("\nTest 3: Cache 3 writes to address 0x%08X (should cause a hit)\n", address1);
    cache_write(cache3, address1, write_data, logfile3);
    printf("Other caches snoop BUS_INVALIDATE for the same address.\n");
    snoop_bus(cache1, BUS_INVALIDATE, address1, NULL);
    snoop_bus(cache2, BUS_INVALIDATE, address1, NULL);
    snoop_bus(cache4, BUS_INVALIDATE, address1, NULL);

    // Log cache states after the operation
    log_cache_state(cache1, logfile1);
    log_cache_state(cache2, logfile2);
    log_cache_state(cache3, logfile3);
    log_cache_state(cache4, logfile4);

    // Test 4: Cache 4 reads (causes a miss), other caches snoop BUS_READ
    printf("\nTest 4: Cache 4 reads address 0x%08X (should cause a miss)\n", address2);
    if (!cache_read(cache4, address2, &data, logfile4)) {
        printf("Cache 4 miss as expected. Data loaded: 0x%08X\n", data);
    }
    printf("Other caches snoop BUS_READ for the same address.\n");
    snoop_bus(cache1, BUS_READ, address2, NULL);
    snoop_bus(cache2, BUS_READ, address2, NULL);
    snoop_bus(cache3, BUS_READ, address2, NULL);

    // Log cache states after the operation
    log_cache_state(cache1, logfile1);
    log_cache_state(cache2, logfile2);
    log_cache_state(cache3, logfile3);
    log_cache_state(cache4, logfile4);

    // Test 5: Verify main memory content after write-back
    printf("\nTest 5: Verifying main memory content after write-back...\n");
    uint32_t memory_value = read_from_main_memory(main_memory, address2);
    printf("Main memory value at address 0x%08X: 0x%08X\n", address2, memory_value);

    printf("\nAll quad cache tests completed.\n");
}

int main() {
    // Initialize DSRAM (caches) and main memory
    DSRAM cache1, cache2, cache3, cache4;
    init_dsr_cache(&cache1);
    init_dsr_cache(&cache2);
    init_dsr_cache(&cache3);
    init_dsr_cache(&cache4);
    init_main_memory();

    // Open log files for each cache
    FILE *logfile1 = fopen("cache1_log.txt", "w");
    FILE *logfile2 = fopen("cache2_log.txt", "w");
    FILE *logfile3 = fopen("cache3_log.txt", "w");
    FILE *logfile4 = fopen("cache4_log.txt", "w");
    if (logfile1 == NULL || logfile2 == NULL || logfile3 == NULL || logfile4 == NULL) {
        perror("Error opening test log files");
        return EXIT_FAILURE;
    }

    // Run quad cache tests
    test_quad_cache_operations(&cache1, &cache2, &cache3, &cache4, logfile1, logfile2, logfile3, logfile4);

    // Close log files
    fclose(logfile1);
    fclose(logfile2);
    fclose(logfile3);
    fclose(logfile4);

    return 0;
}
