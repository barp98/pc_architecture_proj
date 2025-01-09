#ifndef ASM_H
#define ASM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Constants
#define IMEM_SIZE 1024
#define DMEM_SIZE 1024

// Global Variables
extern int imem[IMEM_SIZE];
extern int dmem[DMEM_SIZE];
extern int R[16];

// File Pointers
extern FILE *fp_asm;
extern FILE *fp_imemout;
extern FILE *fp_dmemout;

// Opcode and Register Names
extern char op_name[][10];
extern char reg_name[][10];
extern char reg_altname[][10];

// Labels
extern char jumplabels[IMEM_SIZE][50];
extern char labels[IMEM_SIZE][50];

// Function Declarations
/**
 * Interpret assembly file and generate memory files.
 *
 * @param program The path to the assembly file.
 * @param imem_file The path to the output instruction memory file.
 * @param dmem_file The path to the output data memory file.
 * @return 0 on success, exits on error.
 */
int interpret_file(FILE* fp_asm, FILE* fp_imemout, FILE* fp_dmemout);

#endif // ASM_H
