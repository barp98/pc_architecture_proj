#ifndef CPU_SIMULATION_H
#define CPU_SIMULATION_H
#include "cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

/******************************************************************************
* Constants
*******************************************************************************/
#define NUM_REGS 16
#define SIZE_REG 32 
#define MEMORY_SIZE (1 << 20) // 2^20 words

#define FETCH 0
#define DECODE 1
#define EXEC 2
#define MEM 3
#define WB 4
#define STALL -1

#define INSTRUCTION_LENGTH 9

/******************************************************************************
* Struct Definitions
*******************************************************************************/

// Forward declaration of DSRAM
struct dsram;


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
    Command* instruction_file;     // Pointer to the instruction file
    char current_instruction[INSTRUCTION_LENGTH + 1]; // Current fetched instruction
    DecodeBuffers decode_buf;   // Decode buffers
    ExecuteBuffer execute_buf; // Execute buffer for each core
    MemBuffer mem_buf;  // Memory buffer for each core
    char register_file[NUM_REGS][9]; // Register file
    DSRAM dsram; // Core-specific memory
} Core;

/******************************************************************************
* Function Prototypes
*******************************************************************************/
int HexCharToInt(char h);
int Hex_2_Int_2s_Comp(char * h);
void Int_2_Hex(int dec_num, char hex_num[9]);
void initialize_memory(void);
void print_register_file_to_file(const char *filename);
void BuildCommand(char * command_line, Command * com);
int fetch_instruction(Core *core, char *command_line);
void decode(Core *core, Command *com, char command_line[10]);
void execute(Core *core, Command *com);
void memory_state(Command *com, Core *core);
void writeback_state(Command *com, Core *core);

#endif // CPU_SIMULATION_H
