#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"

#define MAX_GANTT_SLOTS 256

typedef struct {
    int pid;
    int start;
    int end;
} GanttSlot;

typedef struct {
    GanttSlot slots[MAX_GANTT_SLOTS];
    int       slotCount;
    float     avgWaitingTime;
    float     avgTurnaroundTime;
    float     cpuUtilization;
} ScheduleResult;

ScheduleResult runFCFS(PCB **processes, int count);
ScheduleResult runSJF(PCB **processes, int count);
ScheduleResult runRoundRobin(PCB **processes, int count, int quantum);
void           displayScheduleResult(const ScheduleResult *result, PCB **processes, int count);
void           displayGanttChart(const ScheduleResult *result);

#endif
