#ifndef CPU_SIMULATION_H
#define CPU_SIMULATION_H

#include "cpu_structs.h"
#include "utils.h" 
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



/******************************************************************************
* Function Prototypes
*******************************************************************************/
void initialize_memory(void);
void print_register_file_to_file(const char *filename);
void BuildCommand(char * command_line, Command * com);
int fetch_instruction(Core *core, char *command_line);
void decode(Core *core, Command *com, char command_line[10]);
void execute(Core *core, Command *com);
void memory_state(Command *com, Core *core);
void writeback_state(Command *com, Core *core);

#endif // CPU_SIMULATION_H
