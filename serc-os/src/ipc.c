#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include "ipc.h"
#include "logger.h"

static const char *unitName(UnitType type) {
    switch (type) {
        case UNIT_AMBULANCE: return "Ambulance Unit";
        case UNIT_FIRE:      return "Fire Unit";
        case UNIT_POLICE:    return "Police Unit";
        case UNIT_RESCUE:    return "Rescue Team";
        default:             return "Unknown Unit";
    }
}

void runPipeDemo(void) {
    printf("\n===== IPC: Anonymous Pipe =====\n");

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        logError("IPC", "Pipe creation failed");
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        logError("IPC", "Fork failed for pipe demo");
        return;
    }

    if (pid == 0) {
        close(pipefd[1]);
        char buf[MSG_MAX_TEXT];
        ssize_t n = read(pipefd[0], buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            printf("  [Child  PID=%d] Received dispatch: \"%s\"\n", getpid(), buf);
        }
        close(pipefd[0]);
        exit(0);
    } else {
        close(pipefd[0]);
        const char *msg = "Dispatch 2x Ambulance Unit to Grid-5 Incident";
        write(pipefd[1], msg, strlen(msg));
        printf("  [Parent PID=%d] Sent dispatch: \"%s\"\n", getpid(), msg);
        close(pipefd[1]);
        wait(NULL);
    }

    logEvent("IPC", "Pipe demo completed");
    printf("===============================\n");
}

void runMessageQueueDemo(void) {
    printf("\n===== IPC: Message Queue =====\n");

    key_t key = ftok("/tmp", 'S');
    if (key == -1) {
        key = 0x5ABC;
    }

    int msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("msgget");
        logError("IPC", "Message queue creation failed");
        return;
    }

    SERCMessage dispatch;
    dispatch.mtype     = UNIT_FIRE;
    dispatch.unitType  = UNIT_FIRE;
    dispatch.unitCount = 3;
    snprintf(dispatch.text, sizeof(dispatch.text),
             "Dispatch %d %s to Building Fire on Kitwe Central",
             dispatch.unitCount, unitName(dispatch.unitType));

    if (msgsnd(msgid, &dispatch, sizeof(dispatch) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        logError("IPC", "Message send failed");
        msgctl(msgid, IPC_RMID, NULL);
        return;
    }
    printf("  [Dispatch Center] Sent: \"%s\"\n", dispatch.text);

    SERCMessage received;
    if (msgrcv(msgid, &received, sizeof(received) - sizeof(long), UNIT_FIRE, 0) == -1) {
        perror("msgrcv");
        logError("IPC", "Message receive failed");
    } else {
        printf("  [Field Unit     ] Received: \"%s\"\n", received.text);
        printf("  [Field Unit     ] Unit Type: %s  Count: %d\n",
               unitName(received.unitType), received.unitCount);
    }

    msgctl(msgid, IPC_RMID, NULL);
    logEvent("IPC", "Message queue demo completed");
    printf("==============================\n");
}

void runSharedMemoryDemo(void) {
    printf("\n===== IPC: Shared Memory =====\n");

    key_t key = ftok("/tmp", 'R');
    if (key == -1) {
        key = 0x5DEF;
    }

    int shmid = shmget(key, sizeof(SharedStatus), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        logError("IPC", "Shared memory creation failed");
        return;
    }

    SharedStatus *status = (SharedStatus *)shmat(shmid, NULL, 0);
    if (status == (SharedStatus *)-1) {
        perror("shmat");
        logError("IPC", "Shared memory attach failed");
        shmctl(shmid, IPC_RMID, NULL);
        return;
    }

    status->activeIncidents = 4;
    status->dispatchedUnits = 9;
    snprintf(status->lastIncident, sizeof(status->lastIncident),
             "Multi-vehicle accident at Ndola Roundabout — 3 Ambulance, 2 Police dispatched");

    printf("  [Writer] Wrote SERC Status to shared memory:\n");
    printf("           Active Incidents : %d\n", status->activeIncidents);
    printf("           Dispatched Units : %d\n", status->dispatchedUnits);
    printf("           Last Incident    : %s\n", status->lastIncident);

    SharedStatus *reader = (SharedStatus *)shmat(shmid, NULL, SHM_RDONLY);
    if (reader != (SharedStatus *)-1) {
        printf("\n  [Reader] Read SERC Status from shared memory:\n");
        printf("           Active Incidents : %d\n", reader->activeIncidents);
        printf("           Dispatched Units : %d\n", reader->dispatchedUnits);
        printf("           Last Incident    : %s\n", reader->lastIncident);
        shmdt(reader);
    }

    shmdt(status);
    shmctl(shmid, IPC_RMID, NULL);
    logEvent("IPC", "Shared memory demo completed");
    printf("==============================\n");
}

void ipcMenu(void) {
    int choice;
    do {
        printf("\n===== IPC Mechanisms =====\n");
        printf("  1. Anonymous Pipe\n");
        printf("  2. Message Queue\n");
        printf("  3. Shared Memory\n");
        printf("  4. Run All Demos\n");
        printf("  0. Back\n");
        printf("  Choice: ");
        if (scanf("%d", &choice) != 1) { choice = -1; while(getchar() != '\n'); }

        switch (choice) {
            case 1: runPipeDemo();          break;
            case 2: runMessageQueueDemo();  break;
            case 3: runSharedMemoryDemo();  break;
            case 4:
                runPipeDemo();
                runMessageQueueDemo();
                runSharedMemoryDemo();
                break;
            case 0: break;
            default: printf("  Invalid option.\n");
        }
    } while (choice != 0);
}
