#include <stdio.h>
#include <string.h>
#include "process.h"
#include "ui.h"
#include "logger.h"

PCB processes[MAX_PROCESSES];
int count = 0;

void createTask() {
    PCB p;
    p.pid = count + 1;

    printf("Enter Task Name (Ambulance/Fire/Police): ");
    scanf("%s", p.name);

    int raw_priority = getSafeInt("Enter Priority (1 = Critical, 2 = High, 3 = Medium, 4 = Low): ");
    switch(raw_priority) {
        case 1: p.priority = CRITICAL; break;
        case 2: p.priority = HIGH;     break;
        case 3: p.priority = MEDIUM;   break;
        case 4: p.priority = LOW;      break;
        default: p.priority = LOW;      break;
    }
    
    p.burst_time = getSafeInt("Enter Burst Time: ");
    p.remaining_time = p.burst_time;
    p.state = READY;
    processes[count] = p;
    count++;

    printf("Task Created Successfully!\n");
    
    char logMsg[100];
    sprintf(logMsg, "Created task: %s (PID: %d)", p.name, p.pid);
    logEvent(logMsg);
}

void viewTasks() {
    if (count == 0) {
        printf("No tasks available.\n");
        return;
    }

    printf("\n%-5s %-15s %-10s %-10s %-10s\n", 
           "PID", "Name", "Priority", "Burst", "State");

    for (int i = 0; i < count; i++) {
        printf("%-5d %-15s %-10s %-10d %-10s\n",
               processes[i].pid,
               processes[i].name,
               priority_to_string(processes[i].priority),
               processes[i].burst_time,
               state_to_string(processes[i].state));
    }
}
