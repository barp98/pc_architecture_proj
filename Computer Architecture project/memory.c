#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>




/******************************************
MESI States
***************************************** */
#define INVALID 0
#define SHARED 1
#define EXCLUSIVE 2
#define MODIFIED 3




/******************************************
memory size parameters 
***************************************** */
#define MEM_LENGTH 1024
#define DSRAM_LENGTH 256
#define TSRAM_LENGTH 64
#define TSRAM_TAG_LENGTH 12
#define CACHE_BLOCK_SIZE 4
#define SIZE_SRAM 32 

/**********************************************************************
MEMORY structs 

************************************************************************/
// memory

SRAM* MEM; 
SRAM* CACHE_DSRAM;
SRAM* CACHE_TSRAM;


typedef struct sram
{
  int  block_size;
  int  length;
  int state; 

}SRAM;

typedef struct tsram
{
  char inst[13]; //contains the line as String
  int  opcode;
  int  rd;
  int  rs;
  int  rt;
  int  rm;
  int  imm;  
  int state; 

}TSRAM;