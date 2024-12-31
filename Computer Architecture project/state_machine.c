#include "headers/utils.h"
#include "headers/cpu_structs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

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
            exit(0);
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

