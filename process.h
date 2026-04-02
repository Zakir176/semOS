#ifndef PROCESS_H
#define PROCESS_H
#include "serc_types.h"

extern Process processes[MAX];
extern int count;

void createTask();
void viewTasks();

#endif
