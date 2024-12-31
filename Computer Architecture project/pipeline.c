#include "headers/state_machine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LINE_LENGTH 10
#define FETCH 0
#define DECODE 1
#define EXEC 2
#define MEM 3
#define WB 4

void modify_file_line(const char *filename, int line_number, const char *new_content) {
 FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    FILE *temp = fopen("temp.txt", "w");
    if (!temp) {
        perror("Error opening temporary file");
        fclose(file);
        return;
    }

    char buffer[1024];
    int current_line = 1;

    // Copy the original file to the temporary file, appending to the target line.
    while (fgets(buffer, sizeof(buffer), file)) {
        if (current_line == line_number) {
            // Remove the newline character from the existing line before appending.
            buffer[strcspn(buffer, "\n")] = '\0';
            fprintf(temp, "%s%s\n", buffer, new_content);  // Append new content.
        } else {
            fprintf(temp, "%s", buffer);  // Copy the original line.
        }
        current_line++;
    }

    // If the target line is beyond the end of the file, append new lines and the content.
    while (current_line <= line_number) {
        if (current_line == line_number) {
            fprintf(temp, "%s\n", new_content);
        } else {
            fprintf(temp, "\n");
        }
        current_line++;
    }

    fclose(file);
    fclose(temp);

    // Replace the original file with the temporary file.
    if (remove(filename) != 0 || rename("temp.txt", filename) != 0) {
        perror("Error replacing the original file");
    }
}


int detect_hazard(DecodeBuffers *decode_buf, ExecuteBuffer *execute_buf)
{
       // Data Hazard Detection
    if (decode_buf->rs == execute_buf->rd_value && execute_buf->rd_value != 0) {   // rs1 is in use 
        printf("Data hazard detected: Read-after-Write (RAW) on rs1\n");
        return 1; // Stall
    }
    if (decode_buf->rt == execute_buf->rd_value && execute_buf->rd_value != 0) { // rs1 is in use 
        printf("Data hazard detected: Read-after-Write (RAW) on rs2\n");
        return 1; // Stall
    }

    // Control Hazard Detection
    if (decode_buf->is_branch && !execute_buf->branch_resolved) {
        printf("Control hazard detected: Branch not resolved\n");
        return 1; // Stall
    }

    // Structural Hazard Detection
    if (execute_buf->mem_busy) {
        printf("Structural hazard detected: Memory unit busy\n");
        return 1; // Stall
    } 

    // No hazards detected
    return 0;
}int state_machine(Core* core, int stall, int num) {
    int hazard;
    Command *com = &core->instruction_file[num];
    switch (com->state) {
    case FETCH:
        if (fetch_instruction(core, core->current_instruction) == -1) {
            return -1; // No more instructions to fetch
        }
        modify_file_line("pipeline_output.txt", num + 1, "f ");
        break;

    case DECODE:
        // hazard = detect_hazard(&core->decode_buf, &core->execute_buf);
        // if (hazard) {
        //     stall = 1; // Set stall flag
        //     modify_file_line("pipeline_output->txt", num + 1, "- "); // Indicate stall
        //     return STALL;
        // }
        decode(core, com, core->instruction_file[num].inst );
        modify_file_line("pipeline_output.txt", num + 1, "d ");
        printf("\nDECODE PIPELINE Command properties: ");
        printf("opcode: %d, rd: %d, rs: %d, rt: %d, imm: %d\n", com->opcode, com->rd, com->rs, com->rt, com->imm);
        break;

    case EXEC:
        //printf("--------------------BEFORE EXECUTE Command properties: ");
        //printf("opcode: %d, rd: %d, rs: %d, rt: %d, imm: %d\n", com->opcode, com->rd, com->rs, com->rt, com->imm);

        printf("\npre Execute PIPELINE Command properties: ");
        printf("opcode: %d, rd: %d, rs: %d, rt: %d, imm: %d\n", com->opcode, com->rd, com->rs, com->rt, com->imm);
        execute(core, com);
        //printf("EXECUTE Command properties: ");
        //printf("opcode: %d, rd: %d, rs: %d, rt: %d, imm: %d\n", com->opcode, com->rd, com->rs, com->rt, com->imm);

        printf("\nExecute PIPELINE Command properties: ");
        printf("opcode: %d, rd: %d, rs: %d, rt: %d, imm: %d\n", com->opcode, com->rd, com->rs, com->rt, com->imm);
        modify_file_line("pipeline_output.txt", num + 1, "e ");
        break;

    case MEM:
        memory_state(com, core);
        modify_file_line("pipeline_output.txt", num + 1, "m ");
        break;

    case WB:
        writeback_state(com, core);
        modify_file_line("pipeline_output.txt", num + 1, "wb");
        break;

    case STALL:
        // Check if the stall condition has resolved
        // hazard = detect_hazard(&core->decode_buf, &core->execute_buf);
        if (!hazard) {
            stall = 0;          // Clear the stall flag
            com->state = DECODE; // Retry decoding
        }
        return STALL;

    default:
        fprintf(stderr, "Error: Invalid pipeline state: %d\n", com->state);
        return -1;
    }

    return 0; // Indicate success
}


// Example initialization
Core initialize_core(int core_id, char * filename) {
    
    Core core = {
        .core_id = core_id,
        .pc = 0,
        //.IC = IC,
       // .instruction_file = (Command*)malloc(sizeof(Command) * IC),
        .decode_buf = {0}, // Adjust based on DecodeBuffers initialization
        .execute_buf = {0}, // Adjust based on ExecuteBuffer initialization
        .mem_buf = {0},     // Adjust based on MemBuffer initialization
        .register_file = {{0}}, // Initialize register file to zero
        .dsram = {0}            // Adjust based on DSRAM initialization
    };
    Command init_command = { .opcode = 0, .rd = 0, .rs = 0, .rt = 0, .rm = 0, .imm = 0, .state = 0 };
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
    }

    int instruction_count = 0;
    char line[MAX_LINE_LENGTH];

    while (fgets(line, sizeof(line), file)) {
        // Remove trailing newline character
        line[strcspn(line, "\n")] = '\0';

        // Allocate space for the new instruction if needed
        if (instruction_count >= core.IC) {
            // Reallocate to expand the instruction array
            core.instruction_file = realloc(core.instruction_file, sizeof(Command) * (instruction_count + 1));

            if (!core.instruction_file) {
                perror("Failed to reallocate memory for instructions");
                fclose(file);
            }
            core.IC++;
        }


        core.instruction_file[instruction_count] = init_command;
        strncpy(core.instruction_file[instruction_count].inst, line, sizeof(core.instruction_file[instruction_count].inst) - 1); // Copy the line into the instruction's `inst` field

                                            
        core.instruction_file[instruction_count].inst[sizeof(core.instruction_file[instruction_count].inst) - 1] = '\0'; // Null-terminate
        core.instruction_file[instruction_count].state = 0;
        // Initialize control_signals as needed
        instruction_count++;
    }

    fclose(file);

    return core;
}
int pipeline(char* filename, int core_id) {
    int clock = 0;
    int num_executed_commands = 0;
    int first_command = 0;
    int last_command = 0;
    int stall = 0;
    int output;
    Core core= initialize_core(core_id, filename); 
    int IC=core.IC; 
    // Print the content of the array
       printf("%d Loaded Instructions:\n\n", IC);

    for (int i = 0; i < IC; i++) {
        printf("%s\n", core.instruction_file[i].inst);
    }

    	
    printf("\n\n\n");
    const int max_cycles = 10000; // Prevent infinite loops
    int i;  // Line index to be retrieved 
    char line[MAX_LINE_LENGTH];  // Array to store the line
    while (num_executed_commands < core . IC) {
        printf("%s\n\n\n-------------------------- Cycle number %d ------------------------: \n\n", BLUE, clock);
        // Process commands in the pipeline
        for (int j = first_command; j <= last_command; j++) {

            // Handle Write-back (WB) state
            if (core.instruction_file[j].state == WB) {
                first_command++;          // Move first command forward
                num_executed_commands++;  // Count completed commands
            }

            printf("\n%s -------------------------- Command instruction: %s ------------------------: \n", RED, core.instruction_file[j].inst);

            printf("%sState: %s\n", WHITE, (char *[]){"fetch", "decode", "execute", "memory", "writeback"}[core.instruction_file[j].state]);
  

            output = state_machine(&core, stall, j);
                      printf("opcode: %d, rd: %d, rs: %d, rt: %d, imm: %d\n", core.instruction_file[j].opcode, core.instruction_file[j].rd,
             core.instruction_file[j].rs, core.instruction_file[j].rt, core.instruction_file[j].imm);
        printf("\n%s ------------------------------------------------------------------------------: \n%s", RED, WHITE);


            // Advance command state if no stall
            if (core.instruction_file[j].state < WB) {  //!stall && 

                core.instruction_file[j].state++;  // Advance to next stage
            }


            // // Handle end of instruction fetch
            // if (output == -1) {
            //     stall = 0; // Clear stall if no more instructions
            //     break;
            // }


        }

        printf("%s\n\n\n------------------------------------------------------------: %s\n\n", BLUE, WHITE, clock);
        printf("\n\n");
        // Increment the clock cycle
        clock++;

        // If no stall and more commands exist, load the next command
        if (!stall && last_command < core.IC - 1) {
            last_command++;
            modify_file_line("pipeline_output.txt", last_command + 1, "  ");
        }

        // Prevent infinite loops by capping the clock
        if (clock > max_cycles) {
            fprintf(stderr, "Error: Exceeded maximum cycles. Terminating to prevent infinite loop.\n");
            break;
        }
    }

    return clock;  // Return the number of clock cycles used
}


int main() {
    // Open instruction memory file
    FILE *file = fopen("log_files/pipeline_log.txt", "w");
    if (file == NULL) {
        perror("Error opening file");
        fclose(file);
        return EXIT_FAILURE;
    }
    fclose(file);
    // Count instructions and allocate memory for commands
    //Command *commands = malloc(IC * sizeof(Command));
    // Call pipeline function to execute the instructions
    int clock_cycles = pipeline("mem_files/imem0.txt", 1);

    // Output the result
    printf("Pipeline executed in %d clock cycles\n", clock_cycles);

    // Clean up
    //free(commands);
    //fclose(imem);

    return EXIT_SUCCESS;
}