#include "cache.h"  // Include cache functions and constants


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

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
    int mem_busy;
    int is_branch;
} ExecuteBuffer;

typedef struct {
    int load_result;   // Result from memory (after read)
    int destination_register;  // Address of the memory operation (calculated in execute)

} MemBuffer;


typedef struct Core {
    int core_id;                         // Core identifier
    int pc;                              // Program counter
    int IC;                              // Number of instructions
    Command* instruction_file;           // Pointer to the instruction file
    DecodeBuffers decode_buf;            // Decode buffers
    ExecuteBuffer execute_buf;           // Execute buffer for each core
    MemBuffer mem_buf;                   // Memory buffer for each core
    char register_file[NUM_REGS][9];     // Register file (each register is 8 chars + '\0')
    DSRAM dsram;                         // Core-specific memory
} Core;


//int main_memory[MEMORY_SIZE]; // Main memory array
// registers 
/* IO register array*/
char register_file[NUM_REGS][9] = {
    "00000000", /* 0 - always 0 */
    "00000000", /* 1 - non-writable, contains immediate */
    "00000005", /* 2 */
    "00000005", /* 3 */
    "00000005", /* 4 */
    "00000005", /* 5 */
    "00000005", /* 6 */
    "00000005", /* 7 */
    "00000005", /* 8 */
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

int fetch_instruction(Core *core, char *command_line) {
    
    static char instruction[INSTRUCTION_LENGTH + 1]; // Buffer for fetched instruction

    // Move to the correct position in the instruction file
    //fseek(core->instruction_file, core->pc * (INSTRUCTION_LENGTH + 1), SEEK_SET);

    // Fetch the instruction
    //if (fgets(instruction, INSTRUCTION_LENGTH + 1, core->instruction_file) != NULL) {


    if(core->pc < core->IC){
        core->pc++; // Increment program counter
       // strcpy(command_line, instruction); // Copy instruction to command_line
        return 1;
    } else {
        return 0; // End of instructions
    }


}

void decode(Core *core, Command *com, char command_line[9]) {
    // Build the Command struct from the command line
    BuildCommand(command_line, com);
        printf("\nentered decode: %s\n", com->inst);

    printf("opcode of com is: %d\n", com->opcode);
    printf("%s", command_line);

    

    // Process the opcode and set control signals based on it
    switch (com->opcode) {
        case 0: // add
        case 1: // sub
        case 2: // and
        case 3: // or
        case 4: // xor
        case 5: // mul
            com->control_signals.alu_src = 0;
            com->control_signals.mem_to_reg = 0;
            com->control_signals.reg_write = 1;
            com->control_signals.mem_read = 0;
            com->control_signals.mem_write = 0;
            com->control_signals.branch = 0;
            com->control_signals.jump = 0;
            com->control_signals.halt = 0;
            break;

        case 6: // sll (shift left logical)
        case 7: // sra (shift right arithmetic)
        case 8: // srl (shift right logical)
            com->control_signals.alu_src = 1;
            com->control_signals.mem_to_reg = 0;
            com->control_signals.reg_write = 1;
            com->control_signals.mem_read = 0;
            com->control_signals.mem_write = 0;
            com->control_signals.branch = 0;
            com->control_signals.jump = 0;
            com->control_signals.halt = 0;
            break;

        case 9: // beq (branch if equal)
        case 10: // bne (branch if not equal)
        case 11: // blt (branch if less than)
        case 12: // bgt (branch if greater than)
        case 13: // ble (branch if less or equal)
        case 14: // bge (branch if greater or equal)
            com->control_signals.alu_src = 0;
            com->control_signals.mem_to_reg = 0;
            com->control_signals.reg_write = 0;
            com->control_signals.mem_read = 0;
            com->control_signals.mem_write = 0;
            com->control_signals.branch = 1;
            com->control_signals.jump = 0;
            com->control_signals.halt = 0;
            break;

        case 15: // jal (jump and link)
            com->control_signals.alu_src = 0;
            com->control_signals.mem_to_reg = 0;
            com->control_signals.reg_write = 1;
            com->control_signals.mem_read = 0;
            com->control_signals.mem_write = 0;
            com->control_signals.branch = 0;
            com->control_signals.jump = 1;
            com->control_signals.halt = 0;
            break;

        case 16: // lw (load word)
            com->control_signals.alu_src = 1;
            com->control_signals.mem_to_reg = 1;
            com->control_signals.reg_write = 1;
            com->control_signals.mem_read = 1;
            com->control_signals.mem_write = 0;
            com->control_signals.branch = 0;
            com->control_signals.jump = 0;
            com->control_signals.halt = 0;
            break;

        case 17: // sw (store word)
            com->control_signals.alu_src = 1;
            com->control_signals.mem_to_reg = 0;
            com->control_signals.reg_write = 0;
            com->control_signals.mem_read = 0;
            com->control_signals.mem_write = 1;
            com->control_signals.branch = 0;
            com->control_signals.jump = 0;
            com->control_signals.halt = 0;
            break;

        case 20: // halt
            com->control_signals.alu_src = 0;
            com->control_signals.mem_to_reg = 0;
            com->control_signals.reg_write = 0;
            com->control_signals.mem_read = 0;
            com->control_signals.mem_write = 0;
            com->control_signals.branch = 0;
            com->control_signals.jump = 0;
            com->control_signals.halt = 1;
            break;

        default:
            printf("Core %d: Unrecognized opcode: %d\n", core->core_id, com->opcode);
            break;
    }

    // Save register values into decode buffers
    core->decode_buf.rs_value = Hex_2_Int_2s_Comp(core->register_file[com->rs]);
    core->decode_buf.rt_value = Hex_2_Int_2s_Comp(core->register_file[com->rt]);
    core->decode_buf.rd_value = Hex_2_Int_2s_Comp(core->register_file[com->rd]);
    core->decode_buf.rs = com->rs;
    core->decode_buf.rt = com->rt;
    core->decode_buf.rd = com->rd;
}
void execute(Core *core, Command *com) {
    com->state = EXEC;
    int alu_result = 0;
    int address = 0;
    int memory_or_not = 0;
    printf("\nentered exec: %s\n", com->inst);
    if (com->control_signals.halt) {
        printf("Core %d: Halt instruction encountered. Stopping execution.\n", core->core_id);
        return;
    }

    // Structural Hazard: Check if memory is busy
    if (core->execute_buf.mem_busy) {
        printf("Core %d: Memory unit is busy. Execution stalled.\n", core->core_id);
        return;  // Stall execution
    }

    switch (com->opcode) {
        case 0: // ADD (R-type)
            alu_result = core->decode_buf.rs_value + core->decode_buf.rt_value;
            memory_or_not = 0;
            break;

        case 1: // SUB (R-type)
            alu_result = core->decode_buf.rs_value - core->decode_buf.rt_value;
            memory_or_not = 0;
            break;

        case 2: // AND (R-type)
            alu_result = core->decode_buf.rs_value & core->decode_buf.rt_value;
            memory_or_not = 0;
            break;

        case 3: // OR (R-type)
            alu_result = core->decode_buf.rs_value | core->decode_buf.rt_value;
            memory_or_not = 0;
            break;

        case 4: // XOR (R-type)
            alu_result = core->decode_buf.rs_value ^ core->decode_buf.rt_value;
            memory_or_not = 0;
            break;

        case 5: // MUL (R-type)
            alu_result = core->decode_buf.rs_value * core->decode_buf.rt_value;
            memory_or_not = 0;
            break;

        case 6: // SLL (Shift Left Logical)
            alu_result = core->decode_buf.rt_value << com->imm;
            memory_or_not = 0;
            break;

        case 7: // SRA (Shift Right Arithmetic)
            alu_result = core->decode_buf.rt_value >> com->imm;
            memory_or_not = 0;
            break;

        case 8: // SRL (Shift Right Logical)
            alu_result = (unsigned int)core->decode_buf.rt_value >> com->imm;
            memory_or_not = 0;
            break;

        case 9: // BEQ (Branch if Equal)
            if (core->decode_buf.rs_value == core->decode_buf.rt_value) {
                core->pc = core->pc + (com->imm << 2);  // Branch calculation (multiply immediate by 4)
            }
            core->execute_buf.is_branch = 1;
            memory_or_not = 0;
            break;

        case 10: // BNE (Branch if Not Equal)
            if (core->decode_buf.rs_value != core->decode_buf.rt_value) {
                core->pc = core->pc + (com->imm << 2);
            }
            core->execute_buf.is_branch = 1;
            memory_or_not = 0;
            break;

        case 11: // BLT (Branch if Less Than)
            if (core->decode_buf.rs_value < core->decode_buf.rt_value) {
                core->pc = core->pc + (com->imm << 2);
            }
            core->execute_buf.is_branch = 1;
            memory_or_not = 0;
            break;

        case 12: // BGT (Branch if Greater Than)
            if (core->decode_buf.rs_value > core->decode_buf.rt_value) {
                core->pc = core->pc + (com->imm << 2);
            }
            core->execute_buf.is_branch = 1;
            memory_or_not = 0;
            break;

        case 13: // BLE (Branch if Less or Equal)
            if (core->decode_buf.rs_value <= core->decode_buf.rt_value) {
                core->pc = core->pc + (com->imm << 2);
            }
            core->execute_buf.is_branch = 1;
            memory_or_not = 0;
            break;

        case 14: // BGE (Branch if Greater or Equal)
            if (core->decode_buf.rs_value >= core->decode_buf.rt_value) {
                core->pc = core->pc + (com->imm << 2);
            }
            core->execute_buf.is_branch = 1;
            memory_or_not = 0;
            break;

        case 15: // JAL (Jump and Link)
            core->pc = com->imm;  // Jump to target address
            core->execute_buf.is_branch = 1;
            memory_or_not = 0;
            break;

        case 16: // LW (Load Word)
            address = core->decode_buf.rs_value + com->imm;  // Base + offset
            memory_or_not = 1;
            break;

        case 17: // SW (Store Word)
            address = core->decode_buf.rs_value + com->imm;  // Base + offset
            memory_or_not = 1;
            break;

        case 20: // HALT
            com->control_signals.halt = 1;
            memory_or_not = 0;
            printf("Core %d: Halt instruction encountered. Stopping execution.\n", core->core_id);
            return;

        default:
            printf("Core %d: Unrecognized opcode: %d\n", core->core_id, com->opcode);
            break;
    }

    printf("Core %d: ALU Result = %d\n", core->core_id, alu_result);

    // Update the execute buffer
    core->execute_buf.alu_result = alu_result;
    core->execute_buf.destination = com->rd;
    core->execute_buf.mem_address = address;
    core->execute_buf.memory_or_not = memory_or_not;
    core->execute_buf.rd_value = core->decode_buf.rd_value;
    core->execute_buf.is_branch = core->execute_buf.is_branch || 0;  // Ensure default value is 0
    core->execute_buf.mem_busy = memory_or_not;  // Set mem_busy if a memory operation is in progress
}

void memory_state(Command *com, Core *core) {
    com->state = MEM;
    int address = 0;
    uint32_t data;

    // If memory access is required (i.e., for Load/Store operations)
    if (core->execute_buf.memory_or_not == 1) {  // Memory operation indicator (load/store)
        if (com->control_signals.mem_read == 1) {  // Load Word (LW)
            // Cache read instead of direct memory read for the specific core
            bool hit = cache_read(&(core->dsram), core->execute_buf.mem_address, &data, core->dsram.logfile);  // Pass the logfile

            if (hit) {
                core->mem_buf.load_result = data;  // Store loaded data in the buffer
                printf("Memory Read (Cache hit): Loaded value %d from address %d\n", core->mem_buf.load_result, core->execute_buf.mem_address);
            } else {
                // Handle cache miss (fetch data from memory if needed)
                printf("Memory Read (Cache miss): Fetching data from memory\n");
                data = read_from_main_memory(main_memory, core->execute_buf.mem_address);
                core->mem_buf.load_result = data;
            }

            core->mem_buf.destination_register = core->execute_buf.destination;  // Store destination register for writing back
        }

        if (com->control_signals.mem_write == 1) {  // Store Word (SW)
            // Cache write instead of direct memory write for the specific core
            cache_write(&(core->dsram), core->execute_buf.mem_address, core->execute_buf.rd_value, core->dsram.logfile);  // Pass the logfile
            printf("Memory Write (Cache): Stored value %d to address %d\n", core->execute_buf.rd_value, core->execute_buf.mem_address);
        }
    } else {
        // No memory operation, directly use the ALU result
        core->mem_buf.load_result = core->execute_buf.alu_result;
        core->mem_buf.destination_register = core->execute_buf.destination;
        printf("No memory operation needed.\n");
    }
}

void writeback_state(Command *com, Core *core) {
    // Check if the instruction writes back to a register
    switch (com->opcode) {
        case 0: // ADD
        case 1: // SUB
        case 2: // AND
        case 3: // OR
        case 4: // XOR
        case 5: // MUL
        case 6: // SLL (Shift Left Logical)
        case 7: // SRA (Shift Right Arithmetic)
        case 8: // SRL (Shift Right Logical)
            // Write back the result from the memory buffer to the register file
            Int_2_Hex(core->mem_buf.load_result, core->register_file[core->mem_buf.destination_register]);
            //printf("data written into register = %d", Hex_2_Int_2s_Comp(core->register_file[core->mem_buf.destination_register]));
            break;
        
        case 15: // JAL (Jump and Link)
            // Store the return address (PC + 1) into the link register (usually $ra or $15)
            Int_2_Hex(core->pc + 1, core->register_file[15]);
            break;

        case 16: // LW (Load Word)
            // Write the loaded data to the destination register
            Int_2_Hex(core->mem_buf.load_result, core->register_file[core->mem_buf.destination_register]);
            break;

        case 17: // SW (Store Word) - No write-back to register
            break;

        case 20: // HALT - No write-back
            break;

        default:
            printf("Unrecognized opcode %d for writeback.\n", com->opcode);
            break;
    }
}

// int main() {
//     init_main_memory();
//     // Initialize the core structure with instruction data
//     // Open instruction memory file
//     FILE *imem = fopen("imem0.txt", "r");
//     if (!imem) {
//         perror("Error opening instruction memory file");
//         exit(EXIT_FAILURE);
//     }

//     FILE *logfile1 = fopen("cache1_logfile.txt", "w");
//     if (!imem) {
//         perror("Error opening memory state file");
//         fclose(imem);
//         exit(1);
//     }


// Core p1 = {
//     .pc = 0,
//     .register_file = {
//         "00000000", /* 0 - always 0 */
//         "00000000", /* 1 - non-writable, contains immediate */
//         "000000A1", /* 2 */
//         "000000A1", /* 3 */
//         "000000A1", /* 4 */
//         "00000005", /* 5 */
//         "00000005", /* 6 */
//         "00000005", /* 7 */
//         "00000005", /* 8 */
//         "00000000", /* 9 */
//         "00000000", /* 10 */
//         "00000000", /* 11 */
//         "00000000", /* 12 */
//         "00000000", /* 13 */
//         "00000000", /* 14 */
//         "00000000"  /* 15 */
//     },
//     .instruction_file = imem,
//     .dsram = {
//         .cycle_count = 0,  // Initialize cycle count to zero
//         .logfile = logfile1 // Logfile pointer to the log file created
//     }
// };


//     FILE *memfile = fopen("main_memory_state_log.txt", "w");
//     if (!memfile) {
//         perror("Error opening memory state file");
//         fclose(logfile1);
//         fclose(imem);
//         exit(1);
//     }


//     // Open regout0.txt to reset the file before starting
//     FILE *regout = fopen("regout0.txt", "w");
//     if (!regout) {
//         perror("Error opening regout0.txt for writing");
//         fclose(imem);
//         fclose(logfile1);
//         fclose(memfile);
//         exit(EXIT_FAILURE);
//     }
//     fclose(regout);  // Close the file immediately

//     printf("Fetching, decoding, executing, and writing back instructions from imem0.txt:\n");

//     // Loop to process instructions until the halt condition
//     char command_line[INSTRUCTION_LENGTH + 1];
//     Command com;

//     while (fetch_instruction(&p1, command_line)) {
//         printf("Fetched instruction: %s\n", command_line);

//         // Decode instruction and set control signals
//         decode(&p1, &com, command_line);
//         printf("Command properties: ");
//         printf("opcode: %d, rd: %d, rs: %d, rt: %d, imm: %d\n",
//                com.opcode, com.rd, com.rs, com.rt, com.imm);

//         // Print control signals
//         printf("Control signals for opcode %d:\n", com.opcode);
//         printf("alu_src: %d, mem_to_reg: %d, reg_write: %d, mem_read: %d, "
//                "mem_write: %d, branch: %d, jump: %d, halt: %d\n\n",
//                com.control_signals.alu_src, com.control_signals.mem_to_reg,
//                com.control_signals.reg_write, com.control_signals.mem_read, com.control_signals.mem_write,
//                com.control_signals.branch, com.control_signals.jump, com.control_signals.halt);

//         // Print the decode buffer values
//         printf("Decode buffer values:\n");
//         printf("rs_value: %d, rt_value: %d, rd_value: %d\n", 
//                p1.decode_buf.rs_value, p1.decode_buf.rt_value, p1.decode_buf.rd_value);
//         printf("rs: %d, rt: %d, rd: %d\n\n", p1.decode_buf.rs, p1.decode_buf.rt, p1.decode_buf.rd);

//         // Execute instruction
//         execute(&p1, &com);
//         printf("After execution:\n");
//         printf("ALU result: %d, Destination register: %d\n", p1.execute_buf.alu_result, p1.execute_buf.destination);

//         // Perform memory operation if needed
//         memory_state(&com, &p1);

//         // Writeback operation to registers
//         writeback_state(&com, &p1);

//         // Print the destination register value after writeback if a write occurred
//         if (com.control_signals.reg_write) {
//             printf("After writeback: Register %d value: %d\n", 
//                    p1.mem_buf.destination_register, Hex_2_Int_2s_Comp(p1.register_file[p1.mem_buf.destination_register]));
//         }

//         // Check for halt condition
//         if (com.control_signals.halt) {
//             printf("Execution halted.\n");
//             break;
//         }

//         // Print the register file to file
//         print_register_file_to_file("regout0.txt");

//         // Print the program counter after writeback
//         printf("Program counter (PC) after writeback: %d\n", p1.pc);
//         printf("--------------------------------------------------");

//         // Write the main memory state to file
//         write_main_memory_to_file(memfile);
//     }

//     // Clean up file pointers
//     fclose(memfile);
//     fclose(logfile1);
//     fclose(imem);

//     printf("Program execution complete.\n");

// } 
