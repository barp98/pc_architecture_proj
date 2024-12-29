#ifndef PIPELINE_H
#define PIPELINE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pipeline_cpu.h"

/* Function prototypes */
int state_machine(Core *core, Command *com, int *stall);
int detect_data_hazard(DecodeBuffers *decode_buf, ExecuteBuffer *execute_buf);
int pipeline(FILE *instruction_file, Command commands[], int IC);
int instantiate_buffers();

#endif /* PIPELINE_H */
