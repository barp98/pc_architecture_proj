
#include "headers/cpu_structs.h"
// Function to initialize the main memory with the value 2

// Function to initialize the main memory with the value 0 and log "0x00000000" in the file
void initialize_main_memory(MainMemory* main_memory, const char* log_filename) {
    // Allocate memory for the main memory array
    // Ensure the memory is allocated
    printf("Allocating memory for main memory...\n");
    if (main_memory->memory_data == NULL) {
        fprintf(stderr, "Memory not allocated. Allocate before initializing.\n");
        return;
    }

    // Initialize all memory locations to zero
    memset(main_memory->memory_data, 0, (1 << 20) * sizeof(int));  // 2^20 integers

    // Optionally, print the value to verify
    printf("Memory initialized. Value at index 0: %d\n", main_memory->memory_data[0]);

    // Open logfile for DSRAM
    main_memory->logfile = fopen(log_filename, "w");
    if (main_memory->logfile == NULL) {
        perror("Error opening log file for DSRAM");
        exit(EXIT_FAILURE);
    }

    printf("Main memory initialized with %d addresses.\n", MAIN_MEMORY_SIZE);
}



void initialize_DSRAM(DSRAM *dsram, FILE *log_file) {
    // Initialize cache data
    dsram = (DSRAM *)malloc(sizeof(DSRAM));

     if (dsram == NULL) {
        perror("Memory allocation failed for dsram");
        // Free already allocated memory
        free(dsram);
        
    }

    for (int i = 0; i < NUM_BLOCKS; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            dsram->cache[i].data[j] = 0;  // Clear data
        }
    }
    dsram->cycle_count = 0;  // Reset cycle count

    // Open logfile for DSRAM
    dsram->logfile = log_file;
    if (dsram->logfile == NULL) {
        perror("Error opening log file for DSRAM");
        exit(EXIT_FAILURE);
    }
}

// Function to initialize TSRAM
void initialize_TSRAM(TSRAM *tsram, FILE *log_file) {

    tsram = (TSRAM *)malloc(sizeof(TSRAM));

     if (tsram == NULL) {
        perror("Memory allocation failed for cmd");
        // Free already allocated memory
        free(tsram);
        
    }
    // Initialize cache state and tags
    for (int i = 0; i < NUM_BLOCKS; i++) {
        tsram->cache[i].tag = 0;  // Clear the tag
        tsram->cache[i].mesi_state = INVALID;  // Set state to INVALID
    }
    tsram->cycle_count = 0;  // Reset cycle count
    // Open logfile for TSRAM
    tsram->logfile = log_file;
    if (tsram->logfile == NULL) {
        perror("Error opening log file for TSRAM");
        exit(EXIT_FAILURE);
    }
}


void initialize_mesi_bus(MESI_bus *bus, const char *log_filename) 
{
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


void initialize_register_file(char (*register_file)[9]) {
    // Initialize each register with 8 '0' characters
    for (int i = 0; i < NUM_REGS; i++) {
        for (int j = 0; j < 8; j++) {
            register_file[i][j] = '0';  // Fill with '0' character
        }
        register_file[i][8] = '\0';  // Null terminate the string
    }
}


void initialize_command(Command* cmd)
{
        // Initialize current_instruction fields

    cmd = (Command *)malloc(sizeof(Command));

     if (cmd == NULL) {
        perror("Memory allocation failed for cmd");
        // Free already allocated memory
        free(cmd);
        
    }

    strcpy(cmd->inst, "NOP");
    cmd->opcode = 0;
    cmd->rd = 0;
    cmd->rs = 0;
    cmd->rt = 0;
    cmd->rm = 0;
    cmd->imm = 0;
    cmd->state = 0;
     if (cmd == NULL) {
            perror("Memory allocation failed for instruction_array[i]");
            exit(1);
     }
}

void initialize_instruction_array(Command** instruction_array, int instruction_count)
{
    // Allocate memory for instruction_array (pointer to an array of Command pointers)
    instruction_array = (Command **)malloc(instruction_count * sizeof(Command *));
    if (instruction_array == NULL) {
        perror("Memory allocation failed for instruction_array");
        free(instruction_array);  // Free previously allocated memory
    }

    // Allocate memory for each Command in instruction_array
    for (int i = 0; i < instruction_count; i++) {
       initialize_command(instruction_array[i]);  // Assuming initialize_command() handles memory allocation
    }
}

void initialize_core_buffers(Core* core)
{
        // Initialize buffers to zero or default values
    core->decode_buf.rs_value = 0;
    core->decode_buf.rt_value = 0;
    core->decode_buf.rd_value = 0;
    core->decode_buf.rs = -1;
    core->decode_buf.rt = -1;
    core->decode_buf.rd = -1;
    core->decode_buf.is_branch = 0;

    core->execute_buf.alu_result = 0;
    core->execute_buf.mem_address = 0;
    core->execute_buf.rd_value = 0;
    core->execute_buf.destination = -1;
    core->execute_buf.memory_or_not = 0;
    core->execute_buf.mem_busy = 0;
    core->execute_buf.branch_resolved = 0;
    core->execute_buf.is_branch = 0;
    core->mem_buf.load_result = 0;
    core->mem_buf.destination_register = -1;
}

void initialize_cache(CACHE* cache, FILE *DSRAM_log_filename, FILE *TSRAM_log_filename, int cache_id)
{
    cache = malloc(sizeof(CACHE));  // Allocate memory for the Core struct
    if (cache == NULL) {
        perror("Memory allocation failed for Core");
    }
    cache->cache_id =cache_id;
    cache->ack = 0;  // Initially no acknowledgment

    initialize_DSRAM(cache->dsram, DSRAM_log_filename);
    initialize_TSRAM(cache->tsram, TSRAM_log_filename);

}

Core* initialize_core(int core_id, int instruction_count, FILE* imem_file, FILE* DSRAM_log_filename, FILE* TSRAM_log_filename) {
    Core* core = (Core *)malloc(sizeof(Core));  // Allocate memory for the Core struct
    if (core == NULL) {
        perror("Memory allocation failed for Core");
        return NULL;
    }

    core->core_id = core_id;
    core->pc = 0;
    core->IC = instruction_count;
    core->instruction_file = imem_file;
    initialize_instruction_array(core->instruction_array, core->IC); 
    initialize_register_file(core->register_file); 
    initialize_command(core->current_instruction);
    initialize_core_buffers(core);
    initialize_cache(core->cache,DSRAM_log_filename, TSRAM_log_filename, core->core_id ); 
    printf("Core %d initialized.\n", core_id);

    return core;
}