#ifndef PIPELINE_H
#define PIPELINE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "state_machine.h"

/* Function prototypes */
int state_machine(Core *core, Command *com, int *stall);
int detect_data_hazard(DecodeBuffers *decode_buf, ExecuteBuffer *execute_buf);
int pipeline(char* log_file,  Core* core);

#endif /* PIPELINE_H */
