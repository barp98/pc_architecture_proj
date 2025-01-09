#ifndef CPU_STRUCTS_H
#define CPU_STRUCTS_H
#include "cache.h"  // Include cache functions and constants

/******************************************************************************
* Definitions
******************************************************************************/

#define NUM_REGS 16
#define SIZE_REG 32
#define MEMORY_SIZE (1 << 20) // 2^20 words
#define NUM_CORES 4
#define FETCH 0
#define DECODE 1
#define EXEC 2
#define MEM 3
#define WB 4
#define STALL 1

#define INSTRUCTION_LENGTH 9
#define MAX_LINE_LENGTH 10

/******************************************************************************
* Structs
******************************************************************************/

typedef struct control_signals {
    int alu_src;
    int mem_to_reg;
    int reg_write;
    int mem_read;
    int mem_write;
    int branch;
    int jump;
    int halt; // Add halt signal for halt instruction
} ControlSignals;

typedef struct cmd {
    char inst[13];
    int opcode;
    int rd;
    int rs;
    int rt;
    int rm;
    int imm;  
    int state; 
    ControlSignals control_signals;
} Command;

typedef struct decode_buffers {
    int rs_value;
    int rt_value;
    int rd_value; // Only for R-type instructions
    int rs;
    int rt;
    int rd;
    int is_branch; 
} DecodeBuffers;

typedef struct {
    int alu_result;    // Store the result from the ALU operation
    int mem_address;    // Store the result from memory (if needed)
    int rd_value;      // Value for rd (used in R-type instructions)
    int destination;   // Destination register (rd or rt)
    int memory_or_not;
    int mem_busy;
    int branch_resolved; 
    int is_branch;
} ExecuteBuffer;

typedef struct {
    int load_result;   // Result from memory (after read)
    int destination_register;  // Address of the memory operation (calculated in execute)
} MemBuffer;

typedef struct Core {
    int core_id;
    int pc;                     // Program counter
    int IC;    
    FILE* instruction_file; 
    Command* current_instruction; // Current fetched instruction
    DecodeBuffers decode_buf;   // Decode buffers
    ExecuteBuffer execute_buf; // Execute buffer for each core
    MemBuffer mem_buf;  // Memory buffer for each core
    char register_file[NUM_REGS][9]; // Register file
    CACHE* cache; 
    Command** instruction_array;     // Pointer to the instruction file
    MESI_bus* bus; 

} Core;

#endif // CPU_STRUCTS_H
