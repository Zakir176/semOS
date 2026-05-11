#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"
#include "logger.h"

static PCB  processTable[MAX_PROCESSES];
static int  processCount = 0;
static int  nextPID      = 1;

void initProcessManager(void) {
    memset(processTable, 0, sizeof(processTable));
    processCount = 0;
    nextPID      = 1;
    logEvent("PROCESS", "Process manager initialized");
}

int createProcess(const char *name, int burstTime, int arrivalTime,
                  ProcessPriority priority, int memoryRequired) {
    if (processCount >= MAX_PROCESSES) {
        logError("PROCESS", "Process table full");
        return -1;
    }
    if (burstTime <= 0 || memoryRequired <= 0) {
        logError("PROCESS", "Invalid burst time or memory requirement");
        return -1;
    }

    PCB *p = &processTable[processCount++];
    p->pid            = nextPID++;
    strncpy(p->name, name, MAX_NAME_LEN - 1);
    p->name[MAX_NAME_LEN - 1] = '\0';
    p->state          = READY;
    p->priority       = priority;
    p->burstTime      = burstTime;
    p->remainingTime  = burstTime;
    p->arrivalTime    = arrivalTime;
    p->waitingTime    = 0;
    p->turnaroundTime = 0;
    p->memoryRequired = memoryRequired;

    char msg[128];
    snprintf(msg, sizeof(msg), "Created process '%s' PID=%d Burst=%d Priority=%s",
             p->name, p->pid, p->burstTime, priorityToString(p->priority));
    logEvent("PROCESS", msg);

    return p->pid;
}

int terminateProcess(int pid) {
    for (int i = 0; i < processCount; i++) {
        if (processTable[i].pid == pid) {
            processTable[i].state = TERMINATED;
            char msg[64];
            snprintf(msg, sizeof(msg), "Process PID=%d terminated", pid);
            logEvent("PROCESS", msg);
            return 0;
        }
    }
    logError("PROCESS", "Terminate failed: PID not found");
    return -1;
}

int setProcessState(int pid, ProcessState state) {
    for (int i = 0; i < processCount; i++) {
        if (processTable[i].pid == pid) {
            processTable[i].state = state;
            char msg[64];
            snprintf(msg, sizeof(msg), "PID=%d state -> %s", pid, stateToString(state));
            logEvent("PROCESS", msg);
            return 0;
        }
    }
    logError("PROCESS", "SetState failed: PID not found");
    return -1;
}

PCB *getProcess(int pid) {
    for (int i = 0; i < processCount; i++) {
        if (processTable[i].pid == pid)
            return &processTable[i];
    }
    return NULL;
}

PCB **getAllProcesses(int *count) {
    static PCB *ptrs[MAX_PROCESSES];
    int n = 0;
    for (int i = 0; i < processCount; i++) {
        if (processTable[i].state != TERMINATED)
            ptrs[n++] = &processTable[i];
    }
    *count = n;
    return ptrs;
}

const char *stateToString(ProcessState state) {
    switch (state) {
        case READY:      return "READY";
        case RUNNING:    return "RUNNING";
        case WAITING:    return "WAITING";
        case TERMINATED: return "TERMINATED";
        default:         return "UNKNOWN";
    }
}

const char *priorityToString(ProcessPriority priority) {
    switch (priority) {
        case PRIORITY_LOW:    return "LOW";
        case PRIORITY_MEDIUM: return "MEDIUM";
        case PRIORITY_HIGH:   return "HIGH";
        default:              return "UNKNOWN";
    }
}

void displayProcesses(void) {
    printf("\n===== Active Processes =====\n");
    printf("%-6s %-20s %-12s %-10s %-8s %-8s\n",
           "PID", "Name", "State", "Priority", "Burst", "Memory");
    printf("--------------------------------------------------------------\n");

    int found = 0;
    for (int i = 0; i < processCount; i++) {
        PCB *p = &processTable[i];
        if (p->state == TERMINATED) continue;
        printf("%-6d %-20s %-12s %-10s %-8d %-8d\n",
               p->pid, p->name, stateToString(p->state),
               priorityToString(p->priority), p->burstTime, p->memoryRequired);
        found = 1;
    }
    if (!found)
        printf("  No active processes.\n");
    printf("==============================================================\n");
}

void displayProcessByPID(int pid) {
    PCB *p = getProcess(pid);
    if (!p) {
        printf("Process PID=%d not found.\n", pid);
        return;
    }
    printf("\n===== Process Details =====\n");
    printf("  PID           : %d\n", p->pid);
    printf("  Name          : %s\n", p->name);
    printf("  State         : %s\n", stateToString(p->state));
    printf("  Priority      : %s\n", priorityToString(p->priority));
    printf("  Burst Time    : %d\n", p->burstTime);
    printf("  Remaining     : %d\n", p->remainingTime);
    printf("  Arrival Time  : %d\n", p->arrivalTime);
    printf("  Memory (KB)   : %d\n", p->memoryRequired);
    printf("===========================\n");
}
