#ifndef DEADLOCK_H
#define DEADLOCK_H

#define MAX_PROCESSES  10
#define MAX_RESOURCES   3

extern const char *resource_names[MAX_RESOURCES];

extern int available[MAX_RESOURCES];
extern int maximum[MAX_PROCESSES][MAX_RESOURCES];
extern int allocation[MAX_PROCESSES][MAX_RESOURCES];
extern int need[MAX_PROCESSES][MAX_RESOURCES];
extern int num_processes;

void initDeadlock(void);
void loadDeadlockData(void);
void runBankersAlgorithm(void);
void requestResources(void);
void detectDeadlockRAG(void);
void printDeadlockState(void);
void deadlockMenu(void);

#endif