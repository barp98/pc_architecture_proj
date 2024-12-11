#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/******************************************************************************
* Define 
*******************************************************************************/

#define NUM_REGS 16
#define SIZE_REG 32 


#define FETCH 0
#define DECODE 1
#define EXEC 2
#define MEM 3
#define WB 4

#define INSTRUCTION_LENGTH 9
//#define MEMORY_SIZE 256

/******************************************************************************
* Structs 
*******************************************************************************/
typedef struct cmd
{
  char inst[13]; //contains the line as String_   no idea...
  int  opcode;
  int  rd;
  int  rs;
  int  rt;
  int  rm;
  int  imm;  
  int state; 

}Command;

typedef struct control_signals {
    //int reg_dst;
    int alu_src;
    int mem_to_reg;
    int reg_write;
    int mem_read;
    int mem_write;
    int branch;
    int jump;
    int halt; // Add halt signal for halt instruction
} ControlSignals;


typedef struct decode_buffers {
    int rs_value;
    int rt_value;
    int rd_value; // Only for R-type instructions
    int rs;
    int rt;
    int rd;
} DecodeBuffers;

typedef struct {
    int alu_result;    // Store the result from the ALU operation
    //int mem_result;    // Store the result from memory (if needed)
    //int rd_value;      // Value for rd (used in R-type instructions)
    //int rs_value;      // Value for rs (used in some operations)
    //int rt_value;      // Value for rt (used in some operations)
    int destination;   // Destination register (rd or rt)
} ExecuteBuffer;


// registers 
/* IO register array*/
char register_file[NUM_REGS][9] = {
    "00000000", /* 0 - always 0 */
    "00000000", /* 1 - non-writable, contains immediate */
    "00000005", /* 2 */
    "00000007", /* 3 */
    "00000000", /* 4 */
    "00000000", /* 5 */
    "00000000", /* 6 */
    "00000000", /* 7 */
    "00000000", /* 8 */
    "00000000", /* 9 */
    "00000000", /* 10 */
    "00000000", /* 11 */
    "00000000", /* 12 */
    "00000000", /* 13 */
    "00000000", /* 14 */
    "00000000"  /* 15 */
};

int pc; 
int clk; 
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


/******************************************************************************
* Function: BuildCommand
*
* Description: extracts the data from the command line and places it in the command struct
*******************************************************************************/
void BuildCommand(char * command_line, Command * com)
{
    strcpy(com->inst, command_line);

	com->opcode = (int) strtol ((char[]) { command_line[0], command_line[1], 0 }, NULL, 16);
	com->rd     = (int) strtol ((char[]) { command_line[2], 0 }, NULL, 16);
	com->rs     = (int) strtol ((char[]) { command_line[3], 0 }, NULL, 16);
	com->rt     = (int) strtol ((char[]) { command_line[4], 0 }, NULL, 16);
	com->imm   = (int) strtol ((char[]) { command_line[5], command_line[6], command_line[7], 0 }, NULL, 16);
    com->state = DECODE; 
	if (com->imm >= 2048)
  	  {
      com->imm -= 4096;  // if the number is greater then 2048 it means that sign bit it on and hence we have to deduce 2^12 from the number
      }

}

char* fetch_instruction(FILE *instruction_file, int *pc) {
    static char instruction[INSTRUCTION_LENGTH + 1]; // Buffer to hold the fetched instruction (+1 for null terminator)

    // Move to the correct position in the file: we need to account for the newline character
    fseek(instruction_file, (*pc) * (INSTRUCTION_LENGTH + 1), SEEK_SET);

    // Read the instruction at the current PC (including the newline)
    if (fgets(instruction, INSTRUCTION_LENGTH + 1, instruction_file) != NULL) {
        (*pc)++; // Increment the program counter after reading
        return instruction;
    } else {
        return NULL; // End of instructions
    }
}

void decode(char* instruction_line, ControlSignals *control, Command *com, DecodeBuffers *decode_buf)
{
    BuildCommand(instruction_line, com);
    memset(control, 0, sizeof(ControlSignals));//check--------------------------------

    // Process the opcode and set control signals based on it
    switch (com->opcode) {
        case 0: // add
        case 1: // sub
        case 2: // and
        case 3: // or
        case 4: // xor
        case 5: // mul
            control->alu_src = 0;  // ALU operand is from register
            control->mem_to_reg = 0;  // ALU result is written to register
            control->reg_write = 1;  // Write back to register
            control->mem_read = 0;  // No memory read
            control->mem_write = 0;  // No memory write
            control->branch = 0;  // No branch
            control->jump = 0;  // No jump
            control->halt = 0;
            break;
        case 6: // sll (shift left logical)
        case 7: // sra (shift right arithmetic)
        case 8: // srl (shift right logical)
            control->alu_src = 1;      // ALU uses immediate value
            control->mem_to_reg = 0;   // Data comes from ALU
            control->reg_write = 1;    // Write back to register
            control->mem_read = 0;     // No memory read
            control->mem_write = 0;    // No memory write
            control->branch = 0;       // No branching
            control->jump = 0;         // No jump
            control->halt = 0;
            break;

        case 9: // beq (branch if equal)
        case 10: // bne (branch if not equal)
        case 11: // blt (branch if less than)
        case 12: // bgt (branch if greater than)
        case 13: // ble (branch if less or equal)
        case 14: // bge (branch if greater or equal)
            control->alu_src = 0;  // ALU operand comes from registers
            control->mem_to_reg = 0;  // No memory to register transfer
            control->reg_write = 0;  // No register write
            control->mem_read = 0;  // No memory read
            control->mem_write = 0;  // No memory write
            control->branch = 1;  // These are branch instructions
            control->jump = 0;  // No jump
            control->halt = 0;
            break;


        case 15: // jal (jump and link)
            control->alu_src = 0;     // No ALU operation, jump address
            control->mem_to_reg = 0;  // Data comes from jump address
            control->reg_write = 1;   // Write back to register
            control->mem_read = 0;    // No memory read
            control->mem_write = 0;   // No memory write
            control->branch = 0;      // No branch
            control->jump = 1;        // Jump to new address
            control->halt = 0;
            break;

        case 16: // lw (load word)
            control->alu_src = 1;     // ALU uses immediate (offset)
            control->mem_to_reg = 1;  // Data comes from memory
            control->reg_write = 1;   // Write back to register
            control->mem_read = 1;    // Memory read
            control->mem_write = 0;   // No memory write
            control->branch = 0;      // No branching
            control->jump = 0;        // No jump
            control->halt = 0;
            break;

        case 17: // sw (store word)
            control->alu_src = 1;     // ALU uses immediate (offset)
            control->mem_to_reg = 0;  // No data from memory
            control->reg_write = 0;   // No register write
            control->mem_read = 0;    // No memory read
            control->mem_write = 1;   // Memory write
            control->branch = 0;      // No branching
            control->jump = 0;        // No jump
            control->halt = 0;
            break;

        case 20: // halt
            control->alu_src = 0;  // No ALU operand is immediate (address offset)
            control->mem_to_reg = 0;  // No memory to register transfer
            control->reg_write = 0;  // No register write
            control->mem_read = 0;  // No memory read
            control->mem_write = 0;  // No Write data to memory
            control->branch = 0;  // No branch
            control->jump = 0;  // No jump
            control->halt = 1;  // Halt signal set
            break;

        default:
            printf("Unrecognized opcode: %d\n", com->opcode);
            break;
    }

    // Save register values into decode buffers
    decode_buf->rs_value = Hex_2_Int_2s_Comp(register_file[com->rs]);
    decode_buf->rt_value = Hex_2_Int_2s_Comp(register_file[com->rt]);
    decode_buf->rd_value = Hex_2_Int_2s_Comp(register_file[com->rd]); // Store rd value for R-type instructions
    decode_buf->rs = com->rs;
    decode_buf->rt = com->rt;
    decode_buf->rd = com->rd;


}

void execute(ControlSignals *control, Command *com, DecodeBuffers *decode_buf, int *pc, ExecuteBuffer *execute_buf) {
    int alu_result = 0;

    if (control->halt) {
        printf("Halt instruction encountered. Stopping execution.\n");
        return;
    }

    switch (com->opcode) {
        case 0: // ADD (R-type)
            alu_result = decode_buf->rs_value + decode_buf->rt_value;
            break;

        case 1: // SUB (R-type)
            alu_result = decode_buf->rs_value - decode_buf->rt_value;
            break;

        case 2: // AND (R-type)
            alu_result = decode_buf->rs_value & decode_buf->rt_value;
            break;

        case 3: // OR (R-type)
            alu_result = decode_buf->rs_value | decode_buf->rt_value;
            break;

        case 4: // XOR (R-type)
            alu_result = decode_buf->rs_value ^ decode_buf->rt_value;
            break;

        case 5: // MUL (R-type)
            alu_result = decode_buf->rs_value * decode_buf->rt_value;
            break;

        case 6: // SLL (Shift Left Logical)
            alu_result = decode_buf->rt_value << com->imm;
            break;

        case 7: // SRA (Shift Right Arithmetic)
            alu_result = decode_buf->rt_value >> com->imm;
            break;

        case 8: // SRL (Shift Right Logical)
            alu_result = (unsigned int)decode_buf->rt_value >> com->imm;
            break;

        case 9: // BEQ (Branch if Equal)
            if (decode_buf->rs_value == decode_buf->rt_value) {
                *pc = *pc + com->imm;  // Modify PC to branch
            }
            break;

        case 10: // BNE (Branch if Not Equal)
            if (decode_buf->rs_value != decode_buf->rt_value) {
                *pc = *pc + com->imm;  // Modify PC to branch
            }
            break;

        case 11: // BLT (Branch if Less Than)
            if (decode_buf->rs_value < decode_buf->rt_value) {
                *pc = *pc + com->imm;
            }
            break;

        case 12: // BGT (Branch if Greater Than)
            if (decode_buf->rs_value > decode_buf->rt_value) {
                *pc = *pc + com->imm;
            }
            break;

        case 13: // BLE (Branch if Less or Equal)
            if (decode_buf->rs_value <= decode_buf->rt_value) {
                *pc = *pc + com->imm;
            }
            break;

        case 14: // BGE (Branch if Greater or Equal)
            if (decode_buf->rs_value >= decode_buf->rt_value) {
                *pc = *pc + com->imm;
            }
            break;

        case 15: // JAL (Jump and Link)
            // Store the return address (PC + 1) into register 15 (link register) as a hexadecimal string
            *pc = com->imm;  // Jump to target address
            break;

        case 16: // LW (Load Word)
            // Assume memory access is handled outside this function
            alu_result = decode_buf->rs_value + com->imm;  // Address calculation
            break;

        case 17: // SW (Store Word)
            // Assume memory access is handled outside this function
            alu_result = decode_buf->rs_value + com->imm;  // Address calculation
            break;
            
        case 20: // HALT (Halt execution)
            control->halt = 1;  // Halt flag set
            printf("Halt instruction encountered. Stopping execution.\n");
            return;

        default:
            printf("Unrecognized opcode: %d\n", com->opcode);
            break;
    }
    printf("Result is: %d\n", alu_result);

    // Store ALU result in the execute buffer
    execute_buf->alu_result = alu_result;
    execute_buf->destination = com->rd;  

}

void WRITEBACK(ControlSignals *control, Command *com, DecodeBuffers *decode_buf, int *pc, ExecuteBuffer *execute_buf ) {
    // Check if the instruction writes back to a register
        switch(com->opcode)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            Int_2_Hex(execute_buf->alu_result, register_file[execute_buf->destination]);
            break;
        case 15:
            Int_2_Hex(*pc + 1, register_file[15]);
            break;
        }

    }



int main() {
    // Open the instruction memory file
    FILE *imem = fopen("imem0.txt", "r");
    if (imem == NULL) {
        perror("Error opening instruction memory file");
        return EXIT_FAILURE;
    }

    int pc = 0; // Initialize the program counter
    char *instruction;
    ControlSignals control;
    Command com;
    DecodeBuffers decode_buf;
    ExecuteBuffer execute_buf; // Buffer to hold execution results

    // Fetch, decode, execute, and print instructions until the halt signal
    printf("Fetching, decoding, executing, and writing back instructions from imem0.txt:\n");
    while ((instruction = fetch_instruction(imem, &pc)) != NULL) {
        printf("Fetched instruction: %s\n", instruction);

        // Decode the instruction and set control signals
        decode(instruction, &control, &com, &decode_buf); // Include decode_buf in decode call

        // Print the properties of the command
        printf("Command properties: ");
        printf("opcode: %d, rd: %d, rs: %d, rt: %d, imm: %d, state: %d\n",
               com.opcode, com.rd, com.rs, com.rt, com.imm, com.state);

        // Print the control signals for this instruction
        printf("Control signals for opcode %d:\n", com.opcode);
        printf("reg_dst: %d, alu_src: %d, mem_to_reg: %d, reg_write: %d, mem_read: %d, "
               "mem_write: %d, branch: %d, jump: %d, halt: %d\n\n",
               control.alu_src, control.mem_to_reg, control.reg_write,
               control.mem_read, control.mem_write, control.branch, control.jump, control.halt);

        // Print the values of decode buffers
        printf("Decode buffer values:\n");
        printf("rs_value: %d, rt_value: %d, rd_value: %d\n", decode_buf.rs_value, decode_buf.rt_value, decode_buf.rd_value);
        printf("rs: %d, rt: %d, rd: %d\n\n", decode_buf.rs, decode_buf.rt, decode_buf.rd);

        // Execute the instruction
        execute(&control, &com, &decode_buf, &pc, &execute_buf); // Assuming execute_buf is populated during execution

        // Print the results of execution
        printf("After execution:\n");
        printf("ALU result: %d, Destination register: %d\n", execute_buf.alu_result, execute_buf.destination);

        // Write back the results to the register file
        WRITEBACK(&control, &com, &decode_buf, &pc, &execute_buf);

        // Print the destination register value after writeback
        if (control.reg_write) { // Check if a write occurred
            printf("After writeback: Register %d value: %d\n", 
                   execute_buf.destination, Hex_2_Int_2s_Comp(register_file[execute_buf.destination]));
        }

        // Check for halt condition
        if (control.halt) {
            printf("Execution halted.\n");
            break; // Exit the loop if the halt signal is set
        }

        printf("Program counter (PC) after writeback: %d\n", pc);
        printf("--------------------------------------------------\n");
    }

    fclose(imem); // Close the instruction memory file
    return 0;
}
