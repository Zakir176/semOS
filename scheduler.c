#include <stdio.h>
#include <string.h>
#include "scheduler.h"
#include "process.h"
#include "logger.h"

void runFCFS() {
    if (count == 0) {
        printf("No tasks to schedule.\n");
        return;
    }

    int total_waiting = 0;
    int total_turnaround = 0;

    processes[0].waiting_time = 0;

    for (int i = 1; i < count; i++) {
        processes[i].waiting_time = 
            processes[i - 1].waiting_time + processes[i - 1].burst_time;
    }

    for (int i = 0; i < count; i++) {
        processes[i].turnaround_time = 
            processes[i].waiting_time + processes[i].burst_time;

        total_waiting += processes[i].waiting_time;
        total_turnaround += processes[i].turnaround_time;

        processes[i].state = RUNNING;
    }

    printf("\nFCFS Scheduling Results:\n");

    printf("\n%-5s %-15s %-10s %-10s %-10s %-15s %-15s\n",
           "PID", "Name", "Priority", "Burst", "State", "Wait", "Turnaround");

    for (int i = 0; i < count; i++) {
        printf("%-5d %-15s %-10s %-10d %-10s %-15d %-15d\n",
               processes[i].pid,
               processes[i].name,
               priority_to_string(processes[i].priority),
               processes[i].burst_time,
               state_to_string(processes[i].state),
               processes[i].waiting_time,
               processes[i].turnaround_time);
    }

    printf("\nAverage Waiting Time: %.2f\n", 
           (float)total_waiting / count);

    printf("Average Turnaround Time: %.2f\n", 
           (float)total_turnaround / count);
           
    logEvent("Ran FCFS Scheduler");
}
