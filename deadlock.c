#include <stdio.h>
#include <string.h>
#include "deadlock.h"
#include "ui.h"
#include "logger.h"

const char *resource_names[MAX_RESOURCES] = {
    "Ambulance Units",
    "Fire Units",
    "Police Units"
};

int available[MAX_RESOURCES];
int maximum[MAX_PROCESSES][MAX_RESOURCES];
int allocation[MAX_PROCESSES][MAX_RESOURCES];
int need[MAX_PROCESSES][MAX_RESOURCES];
int num_processes = 0;

static int visited[MAX_PROCESSES];
static int rec_stack[MAX_PROCESSES];
static int wait_for[MAX_PROCESSES][MAX_PROCESSES];

void initDeadlock(void) {
    num_processes = 0;
    available[0] = 5;
    available[1] = 4;
    available[2] = 6;
    memset(maximum,    0, sizeof(maximum));
    memset(allocation, 0, sizeof(allocation));
    memset(need,       0, sizeof(need));
    logEvent("Deadlock module initialised");
}

void loadDeadlockData(void) {
    printf("\n===== Load Process Resource Data =====\n");
    printf("Resources: %s, %s, %s\n",
           resource_names[0], resource_names[1], resource_names[2]);

    num_processes = getSafeInt("Enter number of processes (max 10): ");

    if (num_processes <= 0 || num_processes > MAX_PROCESSES) {
        printf("Invalid process count.\n");
        num_processes = 0;
        return;
    }

    printf("Enter currently AVAILABLE resources:\n");
    for (int j = 0; j < MAX_RESOURCES; j++) {
        char prompt[64];
        sprintf(prompt, "  %s: ", resource_names[j]);
        available[j] = getSafeInt(prompt);
    }

    for (int i = 0; i < num_processes; i++) {
        printf("\n--- Process P%d ---\n", i);
        printf("  Maximum claim:\n");
        for (int j = 0; j < MAX_RESOURCES; j++) {
            char prompt[64];
            sprintf(prompt, "    %s: ", resource_names[j]);
            maximum[i][j] = getSafeInt(prompt);
        }
        printf("  Current allocation:\n");
        for (int j = 0; j < MAX_RESOURCES; j++) {
            char prompt[64];
            sprintf(prompt, "    %s: ", resource_names[j]);
            allocation[i][j] = getSafeInt(prompt);
            if (allocation[i][j] > maximum[i][j]) {
                printf("  Warning: allocation exceeds maximum. Clamping.\n");
                allocation[i][j] = maximum[i][j];
            }
        }
        for (int j = 0; j < MAX_RESOURCES; j++)
            need[i][j] = maximum[i][j] - allocation[i][j];
    }

    logEvent("Deadlock data loaded");
    printf("\nData loaded successfully.\n");
}

void printDeadlockState(void) {
    if (num_processes == 0) {
        printf("No data loaded yet. Use Option 1 first.\n");
        return;
    }
    printf("\n%-6s  %-18s  %-18s  %-18s\n",
           "Proc", "Allocation (A/F/P)", "Maximum    (A/F/P)", "Need       (A/F/P)");
    printf("------  ------------------  ------------------  ------------------\n");
    for (int i = 0; i < num_processes; i++) {
        printf("P%-5d  %3d %3d %3d            %3d %3d %3d            %3d %3d %3d\n",
               i,
               allocation[i][0], allocation[i][1], allocation[i][2],
               maximum[i][0],    maximum[i][1],    maximum[i][2],
               need[i][0],       need[i][1],       need[i][2]);
    }
    printf("\nAvailable:  Ambulance=%d  Fire=%d  Police=%d\n",
           available[0], available[1], available[2]);
}

void runBankersAlgorithm(void) {
    if (num_processes == 0) {
        printf("Load data first (Option 1).\n");
        return;
    }

    int work[MAX_RESOURCES];
    int finish[MAX_PROCESSES];
    int safe_sequence[MAX_PROCESSES];
    int seq_index = 0;

    for (int j = 0; j < MAX_RESOURCES; j++) work[j] = available[j];
    for (int i = 0; i < num_processes; i++) finish[i] = 0;

    printf("\n===== Banker's Safety Algorithm =====\n");
    printf("Starting Work:  Ambulance=%d  Fire=%d  Police=%d\n\n",
           work[0], work[1], work[2]);

    int found;
    do {
        found = 0;
        for (int i = 0; i < num_processes; i++) {
            if (finish[i]) continue;
            int can_allocate = 1;
            for (int j = 0; j < MAX_RESOURCES; j++)
                if (need[i][j] > work[j]) { can_allocate = 0; break; }
            if (can_allocate) {
                printf("  P%d can proceed: Need(%d,%d,%d) <= Work(%d,%d,%d)\n",
                       i, need[i][0], need[i][1], need[i][2],
                       work[0], work[1], work[2]);
                for (int j = 0; j < MAX_RESOURCES; j++)
                    work[j] += allocation[i][j];
                printf("       Work after P%d finishes: (%d,%d,%d)\n",
                       i, work[0], work[1], work[2]);
                finish[i] = 1;
                safe_sequence[seq_index++] = i;
                found = 1;
            }
        }
    } while (found);

    int all_done = 1;
    for (int i = 0; i < num_processes; i++)
        if (!finish[i]) { all_done = 0; break; }

    printf("\n");
    if (all_done) {
        printf("Result: SAFE STATE\nSafe Sequence: ");
        for (int k = 0; k < seq_index; k++) {
            printf("P%d", safe_sequence[k]);
            if (k < seq_index - 1) printf(" -> ");
        }
        printf("\n");
        logEvent("Banker's Algorithm: SAFE state confirmed");
    } else {
        printf("Result: UNSAFE STATE - No safe sequence exists!\n");
        for (int i = 0; i < num_processes; i++)
            if (!finish[i])
                printf("  P%d stuck - Need(%d,%d,%d)\n",
                       i, need[i][0], need[i][1], need[i][2]);
        logEvent("Banker's Algorithm: UNSAFE state detected");
    }
}

void requestResources(void) {
    if (num_processes == 0) {
        printf("Load data first (Option 1).\n");
        return;
    }

    printf("\n===== Resource Request =====\n");
    int pid = getSafeInt("Enter process number: ");
    if (pid < 0 || pid >= num_processes) {
        printf("Invalid process number.\n");
        return;
    }

    int request[MAX_RESOURCES];
    printf("Enter requested resources:\n");
    for (int j = 0; j < MAX_RESOURCES; j++) {
        char prompt[64];
        sprintf(prompt, "  %s: ", resource_names[j]);
        request[j] = getSafeInt(prompt);
    }

    for (int j = 0; j < MAX_RESOURCES; j++) {
        if (request[j] > need[pid][j]) {
            printf("Request denied: exceeds P%d's maximum need.\n", pid);
            logEvent("Resource request denied: exceeds maximum claim");
            return;
        }
    }
    for (int j = 0; j < MAX_RESOURCES; j++) {
        if (request[j] > available[j]) {
            printf("P%d must wait - resources not available.\n", pid);
            logEvent("Resource request blocked: insufficient resources");
            return;
        }
    }

    for (int j = 0; j < MAX_RESOURCES; j++) {
        available[j]       -= request[j];
        allocation[pid][j] += request[j];
        need[pid][j]       -= request[j];
    }

    int work[MAX_RESOURCES];
    int finish[MAX_PROCESSES];
    for (int j = 0; j < MAX_RESOURCES; j++) work[j] = available[j];
    for (int i = 0; i < num_processes; i++) finish[i] = 0;

    int found;
    do {
        found = 0;
        for (int i = 0; i < num_processes; i++) {
            if (finish[i]) continue;
            int ok = 1;
            for (int j = 0; j < MAX_RESOURCES; j++)
                if (need[i][j] > work[j]) { ok = 0; break; }
            if (ok) {
                for (int j = 0; j < MAX_RESOURCES; j++)
                    work[j] += allocation[i][j];
                finish[i] = 1;
                found = 1;
            }
        }
    } while (found);

    int safe = 1;
    for (int i = 0; i < num_processes; i++)
        if (!finish[i]) { safe = 0; break; }

    if (safe) {
        printf("Request GRANTED for P%d. System is in SAFE state.\n", pid);
        logEvent("Resource request granted");
    } else {
        for (int j = 0; j < MAX_RESOURCES; j++) {
            available[j]       += request[j];
            allocation[pid][j] -= request[j];
            need[pid][j]       += request[j];
        }
        printf("Request DENIED for P%d. Would cause UNSAFE state. Rolled back.\n", pid);
        logEvent("Resource request denied: would cause unsafe state");
    }
}

static int dfs(int u) {
    visited[u] = 1;
    rec_stack[u] = 1;
    for (int v = 0; v < num_processes; v++) {
        if (!wait_for[u][v]) continue;
        if (!visited[v] && dfs(v)) return 1;
        if (rec_stack[v])          return 1;
    }
    rec_stack[u] = 0;
    return 0;
}

void detectDeadlockRAG(void) {
    if (num_processes == 0) {
        printf("Load data first (Option 1).\n");
        return;
    }

    printf("\n===== RAG Cycle Detection =====\n");
    memset(wait_for, 0, sizeof(wait_for));

    for (int i = 0; i < num_processes; i++)
        for (int j = 0; j < MAX_RESOURCES; j++) {
            if (need[i][j] <= 0)            continue;
            if (need[i][j] <= available[j]) continue;
            for (int k = 0; k < num_processes; k++)
                if (k != i && allocation[k][j] > 0)
                    wait_for[i][k] = 1;
        }

    printf("Wait-For Graph:\n");
    int any_edge = 0;
    for (int i = 0; i < num_processes; i++)
        for (int k = 0; k < num_processes; k++)
            if (wait_for[i][k]) {
                printf("  P%d --> P%d\n", i, k);
                any_edge = 1;
            }
    if (!any_edge) printf("  (no wait-for edges)\n");

    memset(visited,   0, sizeof(visited));
    memset(rec_stack, 0, sizeof(rec_stack));

    int cycle_found = 0;
    for (int i = 0; i < num_processes; i++)
        if (!visited[i] && dfs(i)) { cycle_found = 1; break; }

    printf("\n");
    if (cycle_found) {
        printf("Result: DEADLOCK DETECTED - Cycle found in wait-for graph.\n");
        logEvent("RAG cycle detection: DEADLOCK detected");
    } else {
        printf("Result: No deadlock. No cycle found.\n");
        logEvent("RAG cycle detection: no deadlock");
    }
}

void deadlockMenu(void) {
    int choice;
    do {
        printf("\n===== Deadlock Management =====\n");
        printf("1. Load Process/Resource Data\n");
        printf("2. Run Banker's Algorithm\n");
        printf("3. Request Resources\n");
        printf("4. Detect Deadlock (RAG)\n");
        printf("5. View Current State\n");
        printf("6. Back to Main Menu\n");
        choice = getSafeInt("Enter your choice: ");
        switch (choice) {
            case 1: loadDeadlockData();      break;
            case 2: runBankersAlgorithm();   break;
            case 3: requestResources();      break;
            case 4: detectDeadlockRAG();     break;
            case 5: printDeadlockState();    break;
            case 6: break;
            default: printf("Invalid choice. Try again.\n");
        }
    } while (choice != 6);
}