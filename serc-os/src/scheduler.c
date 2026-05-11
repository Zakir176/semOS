#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "scheduler.h"
#include "logger.h"

static void resetProcessTimes(PCB **processes, int count) {
    for (int i = 0; i < count; i++) {
        processes[i]->waitingTime    = 0;
        processes[i]->turnaroundTime = 0;
        processes[i]->remainingTime  = processes[i]->burstTime;
        processes[i]->state          = READY;
    }
}

static void computeAverages(ScheduleResult *result, PCB **processes, int count) {
    float totalWait = 0, totalTAT = 0;
    int   totalBurst = 0, totalTime = 0;

    for (int i = 0; i < count; i++) {
        totalWait  += processes[i]->waitingTime;
        totalTAT   += processes[i]->turnaroundTime;
        totalBurst += processes[i]->burstTime;
    }
    if (result->slotCount > 0) {
        totalTime = result->slots[result->slotCount - 1].end;
    }

    result->avgWaitingTime    = count > 0 ? totalWait / count : 0;
    result->avgTurnaroundTime = count > 0 ? totalTAT  / count : 0;
    result->cpuUtilization    = totalTime > 0 ? (float)totalBurst / totalTime * 100.0f : 0;
}

ScheduleResult runFCFS(PCB **processes, int count) {
    ScheduleResult result;
    memset(&result, 0, sizeof(result));

    if (count == 0) return result;

    resetProcessTimes(processes, count);

    PCB *sorted[MAX_PROCESSES];
    memcpy(sorted, processes, count * sizeof(PCB *));
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (sorted[j]->arrivalTime < sorted[i]->arrivalTime) {
                PCB *tmp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = tmp;
            }
        }
    }

    int currentTime = 0;
    for (int i = 0; i < count; i++) {
        PCB *p = sorted[i];
        if (currentTime < p->arrivalTime)
            currentTime = p->arrivalTime;

        p->waitingTime    = currentTime - p->arrivalTime;
        p->state          = RUNNING;
        result.slots[result.slotCount].pid   = p->pid;
        result.slots[result.slotCount].start = currentTime;
        currentTime += p->burstTime;
        result.slots[result.slotCount].end   = currentTime;
        result.slotCount++;

        p->turnaroundTime = currentTime - p->arrivalTime;
        p->state          = TERMINATED;
    }

    computeAverages(&result, processes, count);
    logEvent("SCHEDULER", "FCFS scheduling completed");
    return result;
}

ScheduleResult runSJF(PCB **processes, int count) {
    ScheduleResult result;
    memset(&result, 0, sizeof(result));

    if (count == 0) return result;

    resetProcessTimes(processes, count);

    int completed  = 0;
    int currentTime = 0;
    int done[MAX_PROCESSES] = {0};

    while (completed < count) {
        int shortest = -1;
        int minBurst = INT_MAX;

        for (int i = 0; i < count; i++) {
            if (!done[i] && processes[i]->arrivalTime <= currentTime) {
                if (processes[i]->burstTime < minBurst) {
                    minBurst  = processes[i]->burstTime;
                    shortest  = i;
                }
            }
        }

        if (shortest == -1) {
            currentTime++;
            continue;
        }

        PCB *p = processes[shortest];
        p->waitingTime    = currentTime - p->arrivalTime;
        p->state          = RUNNING;
        result.slots[result.slotCount].pid   = p->pid;
        result.slots[result.slotCount].start = currentTime;
        currentTime += p->burstTime;
        result.slots[result.slotCount].end   = currentTime;
        result.slotCount++;

        p->turnaroundTime = currentTime - p->arrivalTime;
        p->state          = TERMINATED;
        done[shortest]    = 1;
        completed++;
    }

    computeAverages(&result, processes, count);
    logEvent("SCHEDULER", "SJF scheduling completed");
    return result;
}

ScheduleResult runRoundRobin(PCB **processes, int count, int quantum) {
    ScheduleResult result;
    memset(&result, 0, sizeof(result));

    if (count == 0 || quantum <= 0) return result;

    resetProcessTimes(processes, count);

    int remaining[MAX_PROCESSES];
    int arrived[MAX_PROCESSES];

    for (int i = 0; i < count; i++) {
        remaining[i] = processes[i]->burstTime;
        arrived[i]   = 0;
    }

    int currentTime = 0;
    int completed   = 0;
    int queue[MAX_PROCESSES * 100];
    int head = 0, tail = 0;

    for (int i = 0; i < count; i++) {
        if (processes[i]->arrivalTime == 0) {
            queue[tail++] = i;
            arrived[i]    = 1;
        }
    }

    while (completed < count) {
        if (head == tail) {
            currentTime++;
            for (int i = 0; i < count; i++) {
                if (!arrived[i] && processes[i]->arrivalTime <= currentTime) {
                    queue[tail++] = i;
                    arrived[i]    = 1;
                }
            }
            continue;
        }

        int idx = queue[head++];
        PCB *p  = processes[idx];

        int slice = (remaining[idx] < quantum) ? remaining[idx] : quantum;

        result.slots[result.slotCount].pid   = p->pid;
        result.slots[result.slotCount].start = currentTime;
        currentTime   += slice;
        remaining[idx] -= slice;
        result.slots[result.slotCount].end = currentTime;
        result.slotCount++;

        for (int i = 0; i < count; i++) {
            if (!arrived[i] && processes[i]->arrivalTime <= currentTime) {
                queue[tail++] = i;
                arrived[i]    = 1;
            }
        }

        if (remaining[idx] == 0) {
            p->turnaroundTime = currentTime - p->arrivalTime;
            p->waitingTime    = p->turnaroundTime - p->burstTime;
            p->state          = TERMINATED;
            completed++;
        } else {
            queue[tail++] = idx;
        }
    }

    computeAverages(&result, processes, count);
    char msg[64];
    snprintf(msg, sizeof(msg), "Round Robin scheduling completed (quantum=%d)", quantum);
    logEvent("SCHEDULER", msg);
    return result;
}

void displayGanttChart(const ScheduleResult *result) {
    if (result->slotCount == 0) {
        printf("  No Gantt data.\n");
        return;
    }

    printf("\n  Gantt Chart:\n  ");
    for (int i = 0; i < result->slotCount; i++)
        printf("+-------");
    printf("+\n  ");

    for (int i = 0; i < result->slotCount; i++)
        printf("| P%-4d ", result->slots[i].pid);
    printf("|\n  ");

    for (int i = 0; i < result->slotCount; i++)
        printf("+-------");
    printf("+\n  ");

    printf("%-4d", result->slots[0].start);
    for (int i = 0; i < result->slotCount; i++)
        printf("    %-4d", result->slots[i].end);
    printf("\n");
}

void displayScheduleResult(const ScheduleResult *result, PCB **processes, int count) {
    printf("\n===== Scheduling Results =====\n");
    printf("%-6s %-20s %-10s %-12s %-14s\n",
           "PID", "Name", "Arrival", "Waiting", "Turnaround");
    printf("--------------------------------------------------------------\n");

    for (int i = 0; i < count; i++) {
        PCB *p = processes[i];
        printf("%-6d %-20s %-10d %-12d %-14d\n",
               p->pid, p->name, p->arrivalTime,
               p->waitingTime, p->turnaroundTime);
    }

    printf("--------------------------------------------------------------\n");
    printf("  Avg Waiting Time    : %.2f\n", result->avgWaitingTime);
    printf("  Avg Turnaround Time : %.2f\n", result->avgTurnaroundTime);
    printf("  CPU Utilization     : %.2f%%\n", result->cpuUtilization);

    displayGanttChart(result);
    printf("==============================================================\n");
}
