#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include "ipc.h"
#include "ui.h"
#include "logger.h"

typedef struct {
    long mtype;
    char mtext[128];
} SERCMessage;

void demoPipe(void) {
    printf("\n===== IPC Demo: Anonymous Pipe =====\n");
    int fd[2];
    if (pipe(fd) == -1) { perror("pipe() failed"); return; }

    pid_t pid = fork();
    if (pid < 0) { perror("fork() failed"); return; }

    if (pid == 0) {
        close(fd[1]);
        char buffer[128];
        int bytes = read(fd[0], buffer, sizeof(buffer) - 1);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            printf("[Child  PID=%d] Received via pipe: \"%s\"\n", getpid(), buffer);
        }
        close(fd[0]);
        exit(0);
    } else {
        close(fd[0]);
        const char *alert = "SERC ALERT: Fire at Grid-7. Deploy Fire Units!";
        printf("[Parent PID=%d] Sending alert via pipe...\n", getpid());
        write(fd[1], alert, strlen(alert));
        close(fd[1]);
        waitpid(pid, NULL, 0);
        printf("[Parent] Pipe communication complete.\n");
        logEvent("IPC: pipe demo completed");
    }
}

void demoMessageQueue(void) {
    printf("\n===== IPC Demo: System V Message Queue =====\n");
    key_t key = ftok(".", PROJ_ID);
    if (key == -1) { perror("ftok() failed"); return; }

    int msqid = msgget(key, IPC_CREAT | 0666);
    if (msqid == -1) { perror("msgget() failed"); return; }

    pid_t pid = fork();
    if (pid < 0) { perror("fork() failed"); msgctl(msqid, IPC_RMID, NULL); return; }

    if (pid == 0) {
        SERCMessage msg;
        if (msgrcv(msqid, &msg, sizeof(msg.mtext), MSG_TYPE, 0) == -1) {
            perror("msgrcv() failed");
            exit(1);
        }
        printf("[Child  PID=%d] Dispatch received: \"%s\"\n", getpid(), msg.mtext);
        exit(0);
    } else {
        SERCMessage msg;
        msg.mtype = MSG_TYPE;
        snprintf(msg.mtext, sizeof(msg.mtext),
                 "DISPATCH: Ambulance Unit 3 to Hospital Road - PRIORITY HIGH");
        printf("[Parent PID=%d] Sending dispatch via message queue...\n", getpid());
        if (msgsnd(msqid, &msg, sizeof(msg.mtext), 0) == -1)
            perror("msgsnd() failed");
        waitpid(pid, NULL, 0);
        msgctl(msqid, IPC_RMID, NULL);
        printf("[Parent] Message queue removed. Demo complete.\n");
        logEvent("IPC: message queue demo completed");
    }
}

void demoSharedMemory(void) {
    printf("\n===== IPC Demo: System V Shared Memory =====\n");
    key_t key = ftok(".", PROJ_ID + 1);
    if (key == -1) { perror("ftok() failed"); return; }

    int shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1) { perror("shmget() failed"); return; }

    char *shm_ptr = (char *)shmat(shmid, NULL, 0);
    if (shm_ptr == (char *)-1) {
        perror("shmat() failed");
        shmctl(shmid, IPC_RMID, NULL);
        return;
    }

    snprintf(shm_ptr, SHM_SIZE, "INCIDENT: Active | Location: North Sector | Units: 0");
    printf("[Parent PID=%d] Wrote to shared memory:\n  \"%s\"\n", getpid(), shm_ptr);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork() failed");
        shmdt(shm_ptr);
        shmctl(shmid, IPC_RMID, NULL);
        return;
    }

    if (pid == 0) {
        char *child_ptr = (char *)shmat(shmid, NULL, 0);
        if (child_ptr == (char *)-1) { perror("shmat() in child"); exit(1); }
        printf("[Child  PID=%d] Read: \"%s\"\n", getpid(), child_ptr);
        snprintf(child_ptr, SHM_SIZE,
                 "INCIDENT: Contained | Location: North Sector | Units: 3 deployed");
        printf("[Child  PID=%d] Updated: \"%s\"\n", getpid(), child_ptr);
        shmdt(child_ptr);
        exit(0);
    } else {
        waitpid(pid, NULL, 0);
        printf("[Parent PID=%d] Final state: \"%s\"\n", getpid(), shm_ptr);
        shmdt(shm_ptr);
        shmctl(shmid, IPC_RMID, NULL);
        printf("[Parent] Shared memory removed. Demo complete.\n");
        logEvent("IPC: shared memory demo completed");
    }
}

void ipcMenu(void) {
    int choice;
    do {
        printf("\n===== Inter-Process Communication =====\n");
        printf("1. Demo: Anonymous Pipe\n");
        printf("2. Demo: System V Message Queue\n");
        printf("3. Demo: System V Shared Memory\n");
        printf("4. Back to Main Menu\n");
        choice = getSafeInt("Enter your choice: ");
        switch (choice) {
            case 1: demoPipe();         break;
            case 2: demoMessageQueue(); break;
            case 3: demoSharedMemory(); break;
            case 4: break;
            default: printf("Invalid choice. Try again.\n");
        }
    } while (choice != 4);
}