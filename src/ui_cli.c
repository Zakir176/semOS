#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui_cli.h"
#include "process.h"
#include "scheduler.h"
#include "memory.h"
#include "ipc.h"
#include "deadlock.h"
#include "filemanager.h"
#include "logger.h"

static void clearScreen(void) {
    printf("\033[2J\033[H");
}

static void printBanner(void) {
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║          SMART EMERGENCY RESPONSE CENTER (SERC)          ║\n");
    printf("║                  Mini-OS Simulation                      ║\n");
    printf("║                 Copperbelt University                    ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
}

static int getSafeInt(const char *prompt) {
    int val;
    printf("%s", prompt);
    while (scanf("%d", &val) != 1) {
        printf("  Invalid input. Try again: ");
        while (getchar() != '\n');
    }
    return val;
}

static void processMenu(void) {
    int choice;
    do {
        printf("\n===== Process Management =====\n");
        printf("  1. Create Process\n");
        printf("  2. Terminate Process\n");
        printf("  3. List All Processes\n");
        printf("  4. View Process by PID\n");
        printf("  5. Set Process State\n");
        printf("  0. Back\n");

        choice = getSafeInt("  Choice: ");

        switch (choice) {
            case 1: {
                char name[MAX_NAME_LEN];
                printf("  Process name: ");
                scanf("%31s", name);
                int burst    = getSafeInt("  Burst time: ");
                int arrival  = getSafeInt("  Arrival time: ");
                int priority = getSafeInt("  Priority (1=Low 2=Medium 3=High): ");
                int memory   = getSafeInt("  Memory required (KB): ");

                if (priority < 1 || priority > 3) {
                    printf("  Invalid priority.\n");
                    break;
                }

                int pid = createProcess(name, burst, arrival,
                                        (ProcessPriority)priority, memory);
                if (pid > 0)
                    printf("  Process created with PID=%d\n", pid);
                else
                    printf("  Failed to create process.\n");
                break;
            }
            case 2: {
                int pid = getSafeInt("  PID to terminate: ");
                if (terminateProcess(pid) == 0)
                    printf("  Process PID=%d terminated.\n", pid);
                else
                    printf("  Termination failed.\n");
                break;
            }
            case 3:
                displayProcesses();
                break;
            case 4: {
                int pid = getSafeInt("  PID: ");
                displayProcessByPID(pid);
                break;
            }
            case 5: {
                int pid   = getSafeInt("  PID: ");
                int state = getSafeInt("  State (0=Ready 1=Running 2=Waiting 3=Terminated): ");
                if (state < 0 || state > 3) { printf("  Invalid state.\n"); break; }
                if (setProcessState(pid, (ProcessState)state) == 0)
                    printf("  State updated.\n");
                break;
            }
            case 0: break;
            default: printf("  Invalid option.\n");
        }
    } while (choice != 0);
}

static void schedulerMenu(void) {
    int choice;
    do {
        printf("\n===== CPU Scheduling =====\n");
        printf("  1. Run FCFS\n");
        printf("  2. Run SJF\n");
        printf("  3. Run Round Robin\n");
        printf("  0. Back\n");

        choice = getSafeInt("  Choice: ");

        int count;
        PCB **procs = getAllProcesses(&count);

        if (choice >= 1 && choice <= 3 && count == 0) {
            printf("  No active processes to schedule.\n");
            continue;
        }

        switch (choice) {
            case 1: {
                ScheduleResult result = runFCFS(procs, count);
                displayScheduleResult(&result, procs, count);
                break;
            }
            case 2: {
                ScheduleResult result = runSJF(procs, count);
                displayScheduleResult(&result, procs, count);
                break;
            }
            case 3: {
                int quantum = getSafeInt("  Time quantum: ");
                ScheduleResult result = runRoundRobin(procs, count, quantum);
                displayScheduleResult(&result, procs, count);
                break;
            }
            case 0: break;
            default: printf("  Invalid option.\n");
        }
    } while (choice != 0);
}

static void memoryMenu(void) {
    int choice;
    do {
        printf("\n===== Memory Management =====\n");
        printf("  1. Allocate Memory\n");
        printf("  2. Free Memory\n");
        printf("  3. View Memory Map\n");
        printf("  4. View Memory Statistics\n");
        printf("  0. Back\n");

        choice = getSafeInt("  Choice: ");

        switch (choice) {
            case 1: {
                int pid      = getSafeInt("  PID: ");
                int size     = getSafeInt("  Size (KB): ");
                int strategy = getSafeInt("  Strategy (0=First 1=Best 2=Worst): ");
                if (strategy < 0 || strategy > 2) { printf("  Invalid strategy.\n"); break; }
                int addr = allocateMemory(pid, size, (FitStrategy)strategy);
                if (addr >= 0)
                    printf("  Allocated at address 0x%04X\n", addr);
                else
                    printf("  Allocation failed.\n");
                break;
            }
            case 2: {
                int pid = getSafeInt("  PID to free: ");
                if (freeMemory(pid) == 0)
                    printf("  Memory freed for PID=%d\n", pid);
                else
                    printf("  Free failed.\n");
                break;
            }
            case 3:
                displayMemoryMap();
                break;
            case 4:
                displayMemoryStats();
                break;
            case 0: break;
            default: printf("  Invalid option.\n");
        }
    } while (choice != 0);
}

static void printMainMenu(void) {
    printf("\n╔══════════════════════════════════════╗\n");
    printf("║           SERC Main Menu             ║\n");
    printf("╠══════════════════════════════════════╣\n");
    printf("║  1. Process Management               ║\n");
    printf("║  2. CPU Scheduling                   ║\n");
    printf("║  3. Memory Management                ║\n");
    printf("║  4. IPC Mechanisms                   ║\n");
    printf("║  5. Deadlock Management              ║\n");
    printf("║  6. File Management                  ║\n");
    printf("║  0. Exit                             ║\n");
    printf("╚══════════════════════════════════════╝\n");
}

void cli_run(void) {
    clearScreen();
    printBanner();
    printf("\n  Interface  : Command Line\n");
    printf("  Mode       : Interactive\n\n");

    initProcessManager();
    initMemory(MAX_MEMORY_KB);
    initFileManager();
    initDeadlock();

    int choice;
    do {
        printMainMenu();
        choice = getSafeInt("  Choice: ");

        switch (choice) {
            case 1: processMenu();   break;
            case 2: schedulerMenu(); break;
            case 3: memoryMenu();    break;
            case 4: ipcMenu();       break;
            case 5: deadlockMenu();  break;
            case 6: fileMenu();      break;
            case 0:
                printf("\n  Shutting down SERC OS. Goodbye.\n\n");
                logEvent("SYSTEM", "CLI session ended");
                break;
            default:
                printf("  Invalid option.\n");
        }
    } while (choice != 0);
}
