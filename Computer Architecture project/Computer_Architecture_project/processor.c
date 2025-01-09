#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>




/******************************************************************************
* Define 
*******************************************************************************/

#define NUM_REGS 16
#define SIZE_REG 32 

#define MEM_LENGTH 1024
#define DSRAM_LENGTH 256
#define TSRAM_LENGTH 64
#define TSRAM_TAG_LENGTH 12
#define CACHE_BLOCK_SIZE 4
#define SIZE_SRAM 32 


#define FETCH 0
#define DECODE 1
#define EXEC 2
#define MEM 3
#define WB 4


/******************************************
MESI States
***************************************** */
#define INVALID 0
#define SHARED 1
#define EXCLUSIVE 2
#define MODIFIED 3


/******************************************************************************
* Structs 
*******************************************************************************/
typedef struct cmd
{
  char inst[13]; //contains the line as String
  int  opcode;
  int  rd;
  int  rs;
  int  rt;
  int  rm;
  int  imm;  
  int state; 

}Command;


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

/**********************************************************************
MEMORY structs 

************************************************************************/
// memory

SRAM* MEM; 
SRAM* CACHE_DSRAM;
SRAM* CACHE_TSRAM;





/******************************************************************************
*utilies 
*******************************************************************************/

/******************************************************************************
* Function: HexCharToInt
*
* Description: Hex Char -> Int  
*******************************************************************************/
int HexCharToInt(char h)
{
  short res;

  switch (h) {
	case 'A': res = 10;	break;
	case 'B': res = 11; break;
	case 'C': res = 12;	break;
	case 'D': res = 13;	break;
	case 'E': res = 14;	break;
	case 'F': res = 15;	break;
	case 'a': res = 10;	break;
	case 'b': res = 11;	break;
	case 'c': res = 12;	break;
	case 'd': res = 13;	break;
	case 'e': res = 14;	break;
	case 'f': res = 15;	break;
	default:
		res = atoi(&h); // if char < 10 change it to int
		break;
	}
	return res;
}

/******************************************************************************
* Function: Hex_2_Int_2s_Comp
*
* Description: Signed Hex -> Int 
*******************************************************************************/
int Hex_2_Int_2s_Comp(char * h)
{
    int i;
    int res = 0;
    int len = strlen(h);
    int msc; /* most significant chrachter */
	#ifdef change
    	if (ind) printf("***** string %s\n",h);
	#endif
    for(i=0;i<len-1;i++)
    {
        res += HexCharToInt(h[len - 1 - i]) * (1<<(4*i));
    }

    msc = HexCharToInt(h[0]);

    if (msc < 8)
    {
        res += msc * (1<<(4*(len-1)));
    }
    else
    {
        msc = msc - 8;
        res += msc * (1<<(4*(len-1))) + -8*(1<<(4*(len-1)));
    }
	#ifdef change
		if (ind){
			printf("*** res is %d\n", res);
			ind = 0;
		}
    #endif

    return res;
}

/******************************************************************************
* Function: Int_2_Hex
*
* Description: signed integer -> hex char array of length 8
*******************************************************************************/

void Int_2_Hex(int dec_num, char hex_num[9])
{
  if (dec_num < 0) 
	dec_num = dec_num + 4294967296; 

  sprintf(hex_num, "%08X", dec_num); 
}


// registers 
/* IO register array*/
char register_file[NUM_REGS][9] = {  "00000000" /* 0 - always 0 */
                             	    ,"00000000" /* 1  - non writable, contains immediate*/
						  			,"00000000" /* 2 */
						  			,"00000000" /* 3 */
						  			,"00000000" /* 4 */
						  			,"00000000" /* 5 */
								    ,"00000000" /* 6 */
						  			,"00000000" /* 7 */
						  			,"00000000" /* 8 */
						  			,"00000000" /* 9 */
						  			,"00000000" /* 10 */
						  			,"00000000" /* 11 */
						  			,"00000000" /* 12 */
						  			,"00000000" /* 13 */
						  			,"00000000" /* 14 */
						  			,"00000000" /* 15 */
						  			,"00000000" /* 16 */					  
						   };

char pc; 
int clk; 

// pipeline architecture

// cache
// instructions objects
void load_mem(mem_file); 
void branch_resolution();

/******************************************************************************
* Function: ExecuteCommand
*
* Description: executes the assembly command and updates the registers, data and memory
*******************************************************************************/

void ExecuteCommand(Command * com, FILE * trace, FILE * cycles, FILE * dmemout, FILE * regout, FILE * leds, FILE * diskout, FILE * display, FILE * hwregtrace, FILE * monitor_t,FILE * monitor_b)
{
    Write2Trace(com, trace); //write to trace before the command
    reg_arr[1] = com->imm1; // assign imm values to imm registers
	reg_arr[2] = com->imm2;

	#ifdef print_cycle
		printf("cycle: %d\n", cycle);
	#endif

    switch (com->opcode) { //cases for the opcode according to the assignment
	case 0: //add
		if (com->rd!=0 && com->rd!=1 && com->rd!=2)
		{
			reg_arr[com->rd] = reg_arr[com->rs] + reg_arr[com->rt] + reg_arr[com->rm] ;
		}
		pc++;
		break;

	case 1: //sub
		if (com->rd!=0 && com->rd!=1 && com->rd!=2)
		{
			reg_arr[com->rd] = reg_arr[com->rs] - reg_arr[com->rt] - reg_arr[com->rm];
		}
		pc++;
		break;
		
	case 2: //and
		if (com->rd != 0 && com->rd != 1 && com->rd!=2)
		{
			reg_arr[com->rd] = reg_arr[com->rs] & reg_arr[com->rt] & reg_arr[com->rm];
		}
		pc++;
		break;

	case 3: //or
		if (com->rd != 0 && com->rd != 1 && com->rd!=2)
		{
			reg_arr[com->rd] = reg_arr[com->rs] | reg_arr[com->rt] | reg_arr[com->rm];
		}
		pc++;
		break;

	case 4: //xor
		if (com->rd != 0 && com->rd != 1 && com->rd!=2)
		{
			reg_arr[com->rd] = reg_arr[com->rs] ^ reg_arr[com->rt] ^ reg_arr[com->rm];
		}
		pc++;
		break;

	case 5: //mul
		if (com->rd != 0 && com->rd != 1 && com->rd!=2)
		{
			reg_arr[com->rd] = reg_arr[com->rs] << reg_arr[com->rt];			
		}
		pc++;
		break;
	case 6: //sll
		if (com->rd != 0 && com->rd != 1 && com->rd!=2)
		{
			reg_arr[com->rd] = reg_arr[com->rs] << reg_arr[com->rt];			
		}
		pc++;
		break;

	case 7: //sra
		if (com->rd != 0 && com->rd != 1 && com->rd!=2)
		{
			reg_arr[com->rd] = reg_arr[com->rs] >> reg_arr[com->rt];
		}
		pc++;
		break;

	case 8: //srl
		if (com->rd != 0 && com->rd != 1 && com->rd!=2)
		{
			reg_arr[com->rd] = (int)((unsigned int)reg_arr[com->rs] >> reg_arr[com->rt]);
		}
		pc++;
		break;

	case 9: //beq
		if (reg_arr[com->rs]==reg_arr[com->rt])
		{ 
  		  pc = reg_arr[com->rm];
		}
		else
		  pc++;
		break;

	case 10: //bne 
		if (reg_arr[com->rs] != reg_arr[com->rt])
		{
  		  pc = reg_arr[com->rm];
		}
		else
  		  pc++;
		break;

	case 11: //blt
		if (reg_arr[com->rs] < reg_arr[com->rt])
		{
		  pc = reg_arr[com->rm];
		}
		else
		{
		  pc++;
		}
		break;

	case 12: //bgt
		if (reg_arr[com->rs] > reg_arr[com->rt])
		{
		  pc = reg_arr[com->rm];		  
		}
		else
			pc++;
		break;

	case 13: //ble
		if (reg_arr[com->rs] <= reg_arr[com->rt])
		{
  		  pc = reg_arr[com->rm];
		}
		else
		  pc++;
		break;

	case 14: //bge
		if (reg_arr[com->rs] >= reg_arr[com->rt])
		{
		  pc = reg_arr[com->rm];
		}
		else
			pc++;	
		break;

	case 15: //jal
		if (com->rd != 0 && com->rd != 1 && com->rd!=2)
		{
			reg_arr[com->rd] = (pc + 1);
			pc = reg_arr[com->rm];			
		}
		else
		{
			pc++;
		}
		break;

	case 16: //lw
		if (com->rd!=0 && com->rd!=1 && com->rd!=2 && 0 <= (reg_arr[com->rs] + reg_arr[com->rt]) && (reg_arr[com->rs] + reg_arr[com->rt]) <= 4095)
		{
		  #ifdef change
		  	ind = 1;
		  #endif
		  reg_arr[com->rd] = Hex_2_Int_2s_Comp(data_arr[(reg_arr[com->rs] + reg_arr[com->rt])]) + reg_arr[com->rm] ;
		}
		pc++;
		break;

	case 17: //sw
		if (0 <= (reg_arr[com->rs] + reg_arr[com->rt]) && (reg_arr[com->rs] + reg_arr[com->rt]) <= 4095)
		{
			char hex_num_temp[9];
		  	Int_2_Hex(reg_arr[com->rd] + reg_arr[com->rm], hex_num_temp);
			strcpy(data_arr[(reg_arr[com->rs] + reg_arr[com->rt])], hex_num_temp);
		}
		pc++;
		break;

	case 20: //halt 
		halt_flag = 1;
		break;
	}
    
}


void BuildCommand(char * command_line, Command * com)
{
    strcpy(com->inst, command_line);

	com->opcode = (int) strtol ((char[]) { command_line[0], command_line[1], 0 }, NULL, 16);
	com->rd     = (int) strtol ((char[]) { command_line[2], 0 }, NULL, 16);
	com->rs     = (int) strtol ((char[]) { command_line[3], 0 }, NULL, 16);
	com->rt     = (int) strtol ((char[]) { command_line[4], 0 }, NULL, 16);
	com->rm     = (int) strtol ((char[]) { command_line[5], 0 }, NULL, 16);
	com->imm   = (int) strtol ((char[]) { command_line[6], command_line[7], command_line[8], 0 }, NULL, 16);
    com->state = DECODE; 
	if (com->imm >= 2048)
  	  {
      com->imm -= 4096;  // if the number is greater then 2048 it means that sign bit it on and hence we have to deduce 2^12 from the number
      
      }
    register_file[1]  = com->imm;  // save immediate in reg 1
 #ifdef print
    printf ("command: CC=%d op=%d rd=%d rs=%d rt=%d rm=%d imm1=%d imm2=%d \n",cycle,com->opcode,com->rd,com->rs,com->rt,com->rm,com->imm1,com->imm2) ;
 #endif
}

/******************************************************************************
* Function: BuildCommand
*
* Description: extracts the data from the command line and places it in the command struct
*******************************************************************************/



/******************************************************************************
* Function: BuildCommand
*
* Description: extracts the data from the command line and places it in the command struct
*******************************************************************************/


public static int DECODE
{

}



public static int  pipeline()
{
// FETCH
// DECODE


// EXECUTE
// MEM
//WB
return 0; 
}


