#ifndef PROCESS_H
#define PROCESS_H

#define MAX_PROCESSES 20
#define MAX_NAME_LEN  32

typedef enum {
    READY,
    RUNNING,
    WAITING,
    TERMINATED
} ProcessState;

typedef enum {
    PRIORITY_LOW    = 1,
    PRIORITY_MEDIUM = 2,
    PRIORITY_HIGH   = 3
} ProcessPriority;

typedef struct {
    int            pid;
    char           name[MAX_NAME_LEN];
    ProcessState   state;
    ProcessPriority priority;
    int            burstTime;
    int            remainingTime;
    int            arrivalTime;
    int            waitingTime;
    int            turnaroundTime;
    int            memoryRequired;
} PCB;

void     initProcessManager(void);
int      createProcess(const char *name, int burstTime, int arrivalTime,
                       ProcessPriority priority, int memoryRequired);
int      terminateProcess(int pid);
int      setProcessState(int pid, ProcessState state);
PCB     *getProcess(int pid);
PCB    **getAllProcesses(int *count);
void     displayProcesses(void);
void     displayProcessByPID(int pid);
const char *stateToString(ProcessState state);
const char *priorityToString(ProcessPriority priority);

#endif
