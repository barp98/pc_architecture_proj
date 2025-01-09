#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headers/asm.h"
#include "headers/pipeline.h"
#include "headers/initialize.h"
// Function to initialize Core

//mesi_bus

int main(int argc, char *argv[])
{
    char* program; 
    char* imem_file;
    char* dmem_file;
    char* pipeline_log_file ="log_files/pipeline_log.txt";
    // check that we have 3 cmd line parameters
    Core* cores[NUM_CORES]; 
	if (argc != 4) {
		printf("usage: asm program.asm imem.txt dmem.txt\n");
		exit(1);
	}

	// open files
	// strcpy(program,argv[1]);
	// strcpy(imem_file, argv[2]);
	// strcpy(dmem_file,argv[3]);


    printf("files are: %s, %s, %s\n", argv[1], argv[2],argv[3] );

    FILE* fp_asm = fopen(argv[1], "rt");
	FILE* fp_imemout = fopen(argv[2], "wt");
	FILE* fp_dmemout = fopen(argv[3], "wt");


	if (!fp_asm || !fp_imemout || !fp_dmemout) {
		printf("ERROR: couldn't open files\n");
		exit(1);
	}
    int instruction_count = interpret_file(fp_asm, fp_imemout, fp_dmemout);

    printf("%sFinished interepreting file %s\n", GREEN, WHITE);
        char buffer[100];


    for (int core_id=0; core_id <= NUM_CORES; core_id++)
    {
       cores[core_id] = initialize_core(core_id, instruction_count, fp_imemout); 
    }
    
    int clock_cycles = pipeline( pipeline_log_file, cores[0]);

    snoop_bus(core_id, instruction_count, fp_imemout); 
    
    
    // Output the result
    printf("Pipeline executed in %d clock cycles\n", clock_cycles);


    
    // close files
	fclose(fp_asm);
	fclose(fp_imemout);
	fclose(fp_dmemout);

    // Clean up
   // free(cores);
    //fclose(imem);
	
    return EXIT_SUCCESS;
     
}
