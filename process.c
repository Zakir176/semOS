#include <stdio.h>
#include <string.h>
#include "process.h"
#include "ui.h"
#include "logger.h"

Process processes[MAX];
int count = 0;

void createTask() {
    Process p;
    p.pid = count + 1;

    printf("Enter Task Name (Ambulance/Fire/Police): ");
    scanf("%s", p.name);

    p.priority = getSafeInt("Enter Priority (1 = High, 2 = Medium, 3 = Low): ");
    p.burst_time = getSafeInt("Enter Burst Time: ");

    strcpy(p.state, "Ready");
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
        printf("%-5d %-15s %-10d %-10d %-10s\n",
               processes[i].pid,
               processes[i].name,
               processes[i].priority,
               processes[i].burst_time,
               processes[i].state);
    }
}
