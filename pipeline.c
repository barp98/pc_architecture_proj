#include <stdio.h>
#include "pipeline.h"

int instantiate_buffers()
{
    ControlSignals control;
    Command com;
    DecodeBuffers decode_buf;
    ExecuteBuffer execute_buf; // Buffer to hold execution results


}
int state_machine(FILE *instruction_file, int clock, int pc, int state )
{
    switch (state) {
    case FETCH:
        // fetch instruction
        fetch_instruction(imem, &pc); 
        break;
    case DECODE:
        // Code to execute if expression == constant2
        decode(instruction, &control, &com, &decode_buf); // Include decode_buf in decode call
        break;
    case EXEC:
       // Execute the instruction
        execute(&control, &com, &decode_buf, &pc, &execute_buf); // Assuming execute_buf is populated during execution
        break; 
    case MEM:
        MEMORY(); 
    case WB:
        writeback(&control, &com, DecodeBuffers *decode_buf, int *pc, ExecuteBuffer *execute_buf );
        break; 
    case STALL:
        return -1; 
    default:
        // Code to execute if no case matches
        break;
    }
}

/*pipeline implementation*/

int pipeline(FILE *instruction_file, char*[] commands, int IC)
{
    int clock=0; 
    int pc=0; 
    int num_executed_commands=0;
    int first_command=0; 
    int last_command = 0;  
   
    while(num_executed_commands<IC)  // as long as not all commands have been executed
    {   
        for (int j = first_command; j <= last_command; j++)
            {   
                commands[j].state += 1;
                state_machine(commands[j].state, pc);  // run the state machine unless you hit a stall
                if (command[j].state == WB)  // command has finished
                    {   
                        first_command++; 
                        num_executed_commands++;
                    }

            }
    
    clock +=1; 
    last_command+=1; 
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
    // Fetch, decode, execute, and print instructions until the halt signal
    printf("Fetching, decoding, executing, and writing back instructions from imem0.txt:\n");
    

    fclose(imem); // Close the instruction memory file
    return 0;
}
