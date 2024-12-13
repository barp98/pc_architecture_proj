#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "cache.h"  // Include cache functions and constants

/******************************************************************************
* Define 
*******************************************************************************/

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
*******************************************************************************/


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
  ControlSignals control_signals;

}Command;


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
    int mem_address;    // Store the result from memory (if needed)
    int rd_value;      // Value for rd (used in R-type instructions)
    //int rs_value;      // Value for rs (used in some operations)
    //int rt_value;      // Value for rt (used in some operations)
    int destination;   // Destination register (rd or rt)
    int memory_or_not;
} ExecuteBuffer;

typedef struct {
    int load_result;   // Result from memory (after read)
    int destination_register;  // Address of the memory operation (calculated in execute)

} MemBuffer;


//int main_memory[MEMORY_SIZE]; // Main memory array

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

// Function to initialize the memory
void initialize_memory() {
    // Initialize memory to zero using memset
    memset(main_memory, 0, sizeof(main_memory));  // Set all bytes to zero

    // Optionally, print the value to verify
    printf("Memory initialized. Value at index 0: %d\n", main_memory[0]);
}

// Function to print the register file to a file (all registers in the same row)
void print_register_file_to_file(const char *filename) {
    FILE *file = fopen(filename, "a");  // Open in append mode to add to the file
    if (file == NULL) {
        perror("Error opening file for writing");
        return;
    }

    // Iterate through the register file and print each register value in the same row
    for (int i = 0; i < NUM_REGS; i++) {
        fprintf(file, "%s ", register_file[i]);  // Print each register value separated by space
    }

    fprintf(file, "\n");  // End the row with a newline

    fclose(file);  // Close the file after writing
}

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

void decode(char* instruction_line, Command *com, DecodeBuffers *decode_buf)
{
    BuildCommand(instruction_line, com);

    // Process the opcode and set control signals based on it
    switch (com->opcode) {
        case 0: // add
        case 1: // sub
        case 2: // and
        case 3: // or
        case 4: // xor
        case 5: // mul
            com->control_signals.alu_src = 0;  // ALU operand is from register
            com->control_signals.mem_to_reg = 0;  // ALU result is written to register
            com->control_signals.reg_write = 1;  // Write back to register
            com->control_signals.mem_read = 0;  // No memory read
            com->control_signals.mem_write = 0;  // No memory write
            com->control_signals.branch = 0;  // No branch
            com->control_signals.jump = 0;  // No jump
            com->control_signals.halt = 0;
            break;
        case 6: // sll (shift left logical)
        case 7: // sra (shift right arithmetic)
        case 8: // srl (shift right logical)
            com->control_signals.alu_src = 1;      // ALU uses immediate value
            com->control_signals.mem_to_reg = 0;   // Data comes from ALU
            com->control_signals.reg_write = 1;    // Write back to register
            com->control_signals.mem_read = 0;     // No memory read
            com->control_signals.mem_write = 0;    // No memory write
            com->control_signals.branch = 0;       // No branching
            com->control_signals.jump = 0;         // No jump
            com->control_signals.halt = 0;
            break;

        case 9: // beq (branch if equal)
        case 10: // bne (branch if not equal)
        case 11: // blt (branch if less than)
        case 12: // bgt (branch if greater than)
        case 13: // ble (branch if less or equal)
        case 14: // bge (branch if greater or equal)
            com->control_signals.alu_src = 0;  // ALU operand comes from registers
            com->control_signals.mem_to_reg = 0;  // No memory to register transfer
            com->control_signals.reg_write = 0;  // No register write
            com->control_signals.mem_read = 0;  // No memory read
            com->control_signals.mem_write = 0;  // No memory write
            com->control_signals.branch = 1;  // These are branch instructions
            com->control_signals.jump = 0;  // No jump
            com->control_signals.halt = 0;
            break;


        case 15: // jal (jump and link)
            com->control_signals.alu_src = 0;     // No ALU operation, jump address
            com->control_signals.mem_to_reg = 0;  // Data comes from jump address
            com->control_signals.reg_write = 1;   // Write back to register
            com->control_signals.mem_read = 0;    // No memory read
            com->control_signals.mem_write = 0;   // No memory write
            com->control_signals.branch = 0;      // No branch
            com->control_signals.jump = 1;        // Jump to new address
            com->control_signals.halt = 0;
            break;

        case 16: // lw (load word)
            com->control_signals.alu_src = 1;     // ALU uses immediate (offset)
            com->control_signals.mem_to_reg = 1;  // Data comes from memory
            com->control_signals.reg_write = 1;   // Write back to register
            com->control_signals.mem_read = 1;    // Memory read
            com->control_signals.mem_write = 0;   // No memory write
            com->control_signals.branch = 0;      // No branching
            com->control_signals.jump = 0;        // No jump
            com->control_signals.halt = 0;
            break;

        case 17: // sw (store word)
            com->control_signals.alu_src = 1;     // ALU uses immediate (offset)
            com->control_signals.mem_to_reg = 0;  // No data from memory
            com->control_signals.reg_write = 0;   // No register write
            com->control_signals.mem_read = 0;    // No memory read
            com->control_signals.mem_write = 1;   // Memory write
            com->control_signals.branch = 0;      // No branching
            com->control_signals.jump = 0;        // No jump
            com->control_signals.halt = 0;
            break;

        case 20: // halt
            com->control_signals.alu_src = 0;  // No ALU operand is immediate (address offset)
            com->control_signals.mem_to_reg = 0;  // No memory to register transfer
            com->control_signals.reg_write = 0;  // No register write
            com->control_signals.mem_read = 0;  // No memory read
            com->control_signals.mem_write = 0;  // No Write data to memory
            com->control_signals.branch = 0;  // No branch
            com->control_signals.jump = 0;  // No jump
            com->control_signals.halt = 1;  // Halt signal set
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

void execute(Command *com, DecodeBuffers *decode_buf, int *pc, ExecuteBuffer *execute_buf) {
    com->state = EXEC;
    int alu_result = 0;
    int address = 0;
    int memory_or_not = 0;


    if (com->control_signals.halt) {
        printf("Halt instruction encountered. Stopping execution.\n");
        return;
    }

    switch (com->opcode) {
        case 0: // ADD (R-type)
            alu_result = decode_buf->rs_value + decode_buf->rt_value;
            memory_or_not = 0;
            break;

        case 1: // SUB (R-type)
            alu_result = decode_buf->rs_value - decode_buf->rt_value;
            memory_or_not = 0;
            break;

        case 2: // AND (R-type)
            alu_result = decode_buf->rs_value & decode_buf->rt_value;
            memory_or_not = 0;
            break;

        case 3: // OR (R-type)
            alu_result = decode_buf->rs_value | decode_buf->rt_value;
            memory_or_not = 0;
            break;

        case 4: // XOR (R-type)
            alu_result = decode_buf->rs_value ^ decode_buf->rt_value;
            memory_or_not = 0;
            break;

        case 5: // MUL (R-type)
            alu_result = decode_buf->rs_value * decode_buf->rt_value;
            memory_or_not = 0;
            break;

        case 6: // SLL (Shift Left Logical)
            alu_result = decode_buf->rt_value << com->imm;
            memory_or_not = 0;
            break;

        case 7: // SRA (Shift Right Arithmetic)
            alu_result = decode_buf->rt_value >> com->imm;
            memory_or_not = 0;
            break;

        case 8: // SRL (Shift Right Logical)
            alu_result = (unsigned int)decode_buf->rt_value >> com->imm;
            memory_or_not = 0;
            break;

        case 9: // BEQ (Branch if Equal)
            if (decode_buf->rs_value == decode_buf->rt_value) {
                *pc = *pc + com->imm;  // Modify PC to branch
            }
            memory_or_not = 0;
            break;

        case 10: // BNE (Branch if Not Equal)
            if (decode_buf->rs_value != decode_buf->rt_value) {
                *pc = *pc + com->imm;  // Modify PC to branch
            }
            memory_or_not = 0;
            break;

        case 11: // BLT (Branch if Less Than)
            if (decode_buf->rs_value < decode_buf->rt_value) {
                *pc = *pc + com->imm;
            }
            memory_or_not = 0;
            break;

        case 12: // BGT (Branch if Greater Than)
            if (decode_buf->rs_value > decode_buf->rt_value) {
                *pc = *pc + com->imm;
            }
            memory_or_not = 0;
            break;

        case 13: // BLE (Branch if Less or Equal)
            if (decode_buf->rs_value <= decode_buf->rt_value) {
                *pc = *pc + com->imm;
            }
            memory_or_not = 0;
            break;

        case 14: // BGE (Branch if Greater or Equal)
            if (decode_buf->rs_value >= decode_buf->rt_value) {
                *pc = *pc + com->imm;
            }
            memory_or_not = 0;
            break;

        case 15: // JAL (Jump and Link)
            // Store the return address (PC + 1) into register 15 (link register) as a hexadecimal string
            *pc = com->imm;  // Jump to target address
            memory_or_not = 0;
            break;

        case 16: // LW (Load Word)
            // Assume memory access is handled outside this function
            address = decode_buf->rs_value + decode_buf->rt_value;  // Address calculation
            memory_or_not = 1;
            break;

        case 17: // SW (Store Word)
            // Assume memory access is handled outside this function
            address = decode_buf->rs_value + decode_buf->rt_value;  // Address calculation
            memory_or_not = 1;
            break;
            
        case 20: // HALT (Halt execution)
            com->control_signals.halt = 1;  // Halt flag set
            memory_or_not = 0;
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
    execute_buf->mem_address = address;
    execute_buf->memory_or_not = memory_or_not;
    execute_buf->rd_value = decode_buf->rd_value;

}

void memory_state(Command *com, ExecuteBuffer *execute_buf, MemBuffer *mem_buf, int *pc, DSRAM *dsram)
{
    com->state = MEM;
    
    // If memory access is required (i.e., for Load/Store operations)
    if (execute_buf->memory_or_not == 1)  // Assuming memory_or_not indicates if we are working with memory
    {
        if (com->control_signals.mem_read == 1)  // Load Word (LW)
        {
            // Cache read instead of direct memory read
            uint32_t data;
            bool hit = cache_read(dsram, execute_buf->mem_address, &data, NULL);  // Assuming you don't need to log for now
            
            if (hit) {
                mem_buf->load_result = data;
                printf("Memory Read (Cache hit): Loaded value %d from address %d\n", mem_buf->load_result, execute_buf->mem_address);
            } else {
                printf("Memory Read (Cache miss): Fetching data from memory\n");
            }

            mem_buf->destination_register = execute_buf->destination;
        }

        if (com->control_signals.mem_write == 1)  // Store Word (SW)
        {
            // Cache write instead of direct memory write
            cache_write(dsram, execute_buf->mem_address, execute_buf->rd_value, NULL);
            printf("Memory Write (Cache): Stored value %d to address %d\n", execute_buf->rd_value, execute_buf->mem_address);
        }
    }
    else
    {
        // No memory operation, can proceed to the next phase or do nothing
        mem_buf->load_result = execute_buf->alu_result;
        mem_buf->destination_register = execute_buf->destination;
        printf("No memory operation needed.\n");
    }
}

void WRITEBACK(Command *com, int *pc, MemBuffer *mem_buf) {
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
            Int_2_Hex(mem_buf->load_result, register_file[mem_buf->destination_register]);
            break;
        case 15:
            Int_2_Hex(*pc + 1, register_file[15]);
            break;
        case 16:
            Int_2_Hex(mem_buf->load_result, register_file[mem_buf->destination_register]);
        }
        

    }

int main() {
    DSRAM dsram;
    init_dsr_cache(&dsram);
    init_main_memory();

    // Open a log file
    FILE *logfile = fopen("cache_log.txt", "w");
    if (logfile == NULL) {
        perror("Error opening log file");
        return 1;
    }

    // Open a file to log main memory state
    FILE *memfile = fopen("main_memory_state_log.txt", "w");
    if (memfile == NULL) {
        perror("Error opening memory state file");
        fclose(logfile);
        return 1;
    }


    
    // Open the instruction memory file
    FILE *imem = fopen("imem0.txt", "r");
    if (imem == NULL) {
        perror("Error opening instruction memory file");
        return EXIT_FAILURE;
    }

    int pc = 0; // Initialize the program counter
    char *instruction;
    Command com;            // Command structure now includes control signals
    DecodeBuffers decode_buf;
    ExecuteBuffer execute_buf; // Buffer to hold execution results
    MemBuffer mem_buf;  // Buffer to hold memory results

    // Open the regout0.txt file in write mode to reset it before starting
    FILE *regout = fopen("regout0.txt", "w");
    if (regout == NULL) {
        perror("Error opening regout0.txt for writing");
        return EXIT_FAILURE;
    }
    fclose(regout);  // Close the file immediately since we're appending later

    // Fetch, decode, execute, and print instructions until the halt signal
    printf("Fetching, decoding, executing, and writing back instructions from imem0.txt:\n");
    while ((instruction = fetch_instruction(imem, &pc)) != NULL) {
        printf("Fetched instruction: %s\n", instruction);

        // Decode the instruction and set control signals
        decode(instruction, &com, &decode_buf); // Pass com, decode_buf for decoding

        // Print the properties of the command
        printf("Command properties: ");
        printf("opcode: %d, rd: %d, rs: %d, rt: %d, imm: %d, state: %d\n",
               com.opcode, com.rd, com.rs, com.rt, com.imm, com.state);

        // Print the control signals for this instruction
        printf("Control signals for opcode %d:\n", com.opcode);
        printf("alu_src: %d, mem_to_reg: %d, reg_write: %d, mem_read: %d, "
               "mem_write: %d, branch: %d, jump: %d, halt: %d\n\n",
               com.control_signals.alu_src, com.control_signals.mem_to_reg,
               com.control_signals.reg_write, com.control_signals.mem_read, com.control_signals.mem_write,
               com.control_signals.branch, com.control_signals.jump, com.control_signals.halt);

        // Print the values of decode buffers
        printf("Decode buffer values:\n");
        printf("rs_value: %d, rt_value: %d, rd_value: %d\n", decode_buf.rs_value, decode_buf.rt_value, decode_buf.rd_value);
        printf("rs: %d, rt: %d, rd: %d\n\n", decode_buf.rs, decode_buf.rt, decode_buf.rd);

        // Execute the instruction
        execute(&com, &decode_buf, &pc, &execute_buf); // Pass com, decode_buf for execution

        // Print the results of execution
        printf("After execution:\n");
        printf("ALU result: %d, Destination register: %d\n", execute_buf.alu_result, execute_buf.destination);

        // Perform memory operations if needed
        memory_state(&com, &execute_buf, &mem_buf, &pc, &dsram); // Call memory_state

        // Write back the results to the register file
        WRITEBACK(&com, &pc, &mem_buf);

        // Print the destination register value after writeback
        if (com.control_signals.reg_write) { // Check if a write occurred
            printf("After writeback: Register %d value: %d\n", 
                   execute_buf.destination, Hex_2_Int_2s_Comp(register_file[execute_buf.destination]));
        }

        // Check for halt condition
        if (com.control_signals.halt) {
            printf("Execution halted.\n");
            break; // Exit the loop if the halt signal is set
        }

        // Print the register file after every command (in a new row in the file)
        print_register_file_to_file("regout0.txt");

        printf("Program counter (PC) after writeback: %d\n", pc);
        printf("--------------------------------------------------\n");
        write_main_memory_to_file(memfile);
    }
    fclose(memfile);
    fclose(logfile);
    //initialize_memory();

    fclose(imem); // Close the instruction memory file
    return 0;
}