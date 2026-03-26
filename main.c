#include <stdio.h>
#include <string.h>
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

Process processes[MAX];
int count = 0;

void createTask() {
    Process p;

    p.pid = count + 1;

    printf("Enter Task Name (Ambulance/Fire/Police): ");
    scanf("%s", p.name);

    printf("Enter Priority (1 = High, 2 = Medium, 3 = Low): ");
    scanf("%d", &p.priority);

    printf("Enter Burst Time: ");
    scanf("%d", &p.burst_time);

    strcpy(p.state, "Ready");

    processes[count] = p;
    count++;

    printf("Task Created Successfully!\n");
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

        strcpy(processes[i].state, "Running");
    }

    printf("\nFCFS Scheduling Results:\n");

    printf("\n%-5s %-15s %-10s %-10s %-15s %-15s\n",
           "PID", "Name", "Priority", "Burst", "Waiting", "Turnaround");

    for (int i = 0; i < count; i++) {
        printf("%-5d %-15s %-10d %-10d %-15d %-15d\n",
               processes[i].pid,
               processes[i].name,
               processes[i].priority,
               processes[i].burst_time,
               processes[i].waiting_time,
               processes[i].turnaround_time);
    }

    printf("\nAverage Waiting Time: %.2f\n", 
           (float)total_waiting / count);

    printf("Average Turnaround Time: %.2f\n", 
           (float)total_turnaround / count);
}
int main() {
    int choice;

    while (1) {
        printf("\n===== Smart Emergency Response System =====\n");
        printf("1. Create Emergency Task\n");
        printf("2. View Tasks\n");
        printf("3. Run Scheduler\n");
        printf("4. Allocate Memory\n");
        printf("5. View Logs\n");
        printf("6. Exit\n");

        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                createTask();
                break;
            case 2:
                viewTasks();
                break;
            case 3:
                runFCFS();
                break;
            case 4:
                printf("Allocating Memory...\n");
                break;
            case 5:
                printf("Viewing Logs...\n");
                break;
            case 6:
                printf("Exiting program...\n");
                return 0;
            default:
                printf("Invalid choice. Try again.\n");
        }
    }

    return 0;
}