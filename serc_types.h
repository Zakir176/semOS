#ifndef SERC_TYPES_H
#define SERC_TYPES_H

#define MAX 100

typedef struct {
    int pid;
    char name[50];
    int priority;
    int burst_time;
    int waiting_time;
    int turnaround_time;
    char state[20];
} Process;

#endif
