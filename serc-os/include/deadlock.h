#ifndef DEADLOCK_H
#define DEADLOCK_H

#define MAX_DL_PROCESSES  10
#define MAX_RESOURCES      4

typedef struct {
    int allocation[MAX_DL_PROCESSES][MAX_RESOURCES];
    int maximum[MAX_DL_PROCESSES][MAX_RESOURCES];
    int available[MAX_RESOURCES];
    int need[MAX_DL_PROCESSES][MAX_RESOURCES];
    int processCount;
    int resourceCount;
} BankerState;

void initDeadlock(void);
int  runSafetyAlgorithm(BankerState *state, int *safeSequence);
int  requestResources(BankerState *state, int pid, int request[]);
int  detectCycle(void);
void addWaitEdge(int from, int to);
void removeWaitEdge(int from, int to);
void clearWaitGraph(void);
void displayBankerState(const BankerState *state);
void displayWaitForGraph(void);
void deadlockMenu(void);

#endif
