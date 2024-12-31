#include <stdio.h>
#include <stdlib.h>

#define CACHE_SETS 4   // Number of sets in the cache
#define WAYS 4         // 4-way set-associative cache
#define MEMORY_SIZE 512 // Size of the memory

typedef struct {
    int valid;
    int tag;
} CacheLine;

typedef struct {
    CacheLine lines[WAYS];
} CacheSet;

typedef struct {
    CacheSet sets[CACHE_SETS];
} Cache;

// Initialize the cache
void initialize_cache(Cache *cache) {
    for (int i = 0; i < CACHE_SETS; i++) {
        for (int j = 0; j < WAYS; j++) {
            cache->sets[i].lines[j].valid = 0;
            cache->sets[i].lines[j].tag = -1;
        }
    }
}

// Simulate cache access
int access_cache(Cache *cache, int address) {
    int set_index = (address / WAYS) % CACHE_SETS; // Determine set index
    int tag = address / (WAYS * CACHE_SETS);      // Determine tag

    CacheSet *set = &cache->sets[set_index];

    // Check for a cache hit
    for (int i = 0; i < WAYS; i++) {
        if (set->lines[i].valid && set->lines[i].tag == tag) {
            return 1; // Cache hit
        }
    }

    // Cache miss, replace an entry using a simple policy (e.g., FIFO)
    for (int i = 0; i < WAYS; i++) {
        if (!set->lines[i].valid) {
            set->lines[i].valid = 1;
            set->lines[i].tag = tag;
            return 0; // Cache miss
        }
    }

    // If all lines are valid, evict the first (FIFO) and replace it
    set->lines[0].tag = tag;

    // Rotate entries to simulate FIFO
    CacheLine temp = set->lines[0];
    for (int i = 1; i < WAYS; i++) {
        set->lines[i - 1] = set->lines[i];
    }
    set->lines[WAYS - 1] = temp;

    return 0; // Conflict miss
}

int main() {
    Cache cache;
    initialize_cache(&cache);

    int memory_accesses[] = {0, 128, 256, 384, 0, 129, 256, 384}; // Example access pattern
    int num_accesses = sizeof(memory_accesses) / sizeof(memory_accesses[0]);

    printf("Simulating 4-way set-associative cache...\n");
    for (int i = 0; i < num_accesses; i++) {
        int address = memory_accesses[i];
        int result = access_cache(&cache, address);

        if (result) {
            printf("Accessing address %d: Cache Hit\n", address);
        } else {
            printf("Accessing address %d: Cache Miss\n", address);
        }
    }

    return 0;
}
