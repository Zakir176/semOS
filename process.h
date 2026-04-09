#ifndef PROCESS_H
#define PROCESS_H
#include "serc_types.h"

extern PCB processes[MAX_PROCESSES];
extern int count;

void createTask();
void viewTasks();

#endif
