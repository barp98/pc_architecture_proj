#ifndef CPU_SIMULATOR_H
#define CPU_SIMULATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "cache.h"  // Include cache functions and constants

/****************************************************************************** 
 * Define 
 ******************************************************************************/

#define NUM_REGS 16
#define SIZE_REG 32 
#define MEMORY_SIZE (1 << 20) // 2^20 words

#define FETCH 0
#define DECODE 1
#define EXEC 2
#define MEM 3
#define WB 4

#define INSTRUCTION_LENGTH 9
//#define MEMORY_SIZE 256

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
    char inst[13]; // contains the line as String
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
} DecodeBuffers;

typedef struct execute_buffer {
    int alu_result;    // Store the result from the ALU operation
    int mem_address;    // Store the result from memory (if needed)
    int rd_value;      // Value for rd (used in R-type instructions)
    int destination;   // Destination register (rd or rt)
    int memory_or_not;
} ExecuteBuffer;

typedef struct mem_buffer {
    int load_result;   // Result from memory (after read)
    int destination_register;  // Address of the memory operation (calculated in execute)
} MemBuffer;

extern char register_file[NUM_REGS][9]; // IO register array
extern int pc; 
extern int clk;
extern int main_memory[MEMORY_SIZE]; // Main memory array

/****************************************************************************** 
 * Function Declarations
 ******************************************************************************/

int HexCharToInt(char h);
int Hex_2_Int_2s_Comp(char *h);
void Int_2_Hex(int dec_num, char hex_num[9]);
void initialize_memory(void);
void print_register_file_to_file(const char *filename);
void BuildCommand(char *command_line, Command *com);
char* fetch_instruction(FILE *instruction_file, int *pc);
void decode(char* instruction_line, Command *com, DecodeBuffers *decode_buf);
void execute(Command *com, DecodeBuffers *decode_buf, int *pc, ExecuteBuffer *execute_buf);
void memory_state(Command *com, ExecuteBuffer *execute_buf, MemBuffer *mem_buf, int *pc, DSRAM *dsram);
void WRITEBACK(Command *com, int *pc, MemBuffer *mem_buf);

#endif // CPU_SIMULATOR_H
