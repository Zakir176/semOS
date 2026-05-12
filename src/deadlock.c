#include <stdio.h>
#include <string.h>
#include "deadlock.h"
#include "logger.h"

static const char *resourceNames[MAX_RESOURCES] = {
    "Ambulance Units",
    "Fire Units",
    "Police Units",
    "Rescue Teams"
};

static int waitGraph[MAX_DL_PROCESSES][MAX_DL_PROCESSES];
static int graphSize = 0;

void initDeadlock(void) {
    clearWaitGraph();
    logEvent("DEADLOCK", "Deadlock manager initialized");
}

static void computeNeed(BankerState *state) {
    for (int i = 0; i < state->processCount; i++)
        for (int j = 0; j < state->resourceCount; j++)
            state->need[i][j] = state->maximum[i][j] - state->allocation[i][j];
}

int runSafetyAlgorithm(BankerState *state, int *safeSequence) {
    computeNeed(state);

    int work[MAX_RESOURCES];
    int finish[MAX_DL_PROCESSES] = {0};
    int seq[MAX_DL_PROCESSES];
    int seqIndex = 0;

    for (int j = 0; j < state->resourceCount; j++)
        work[j] = state->available[j];

    int found;
    do {
        found = 0;
        for (int i = 0; i < state->processCount; i++) {
            if (finish[i]) continue;

            int canRun = 1;
            for (int j = 0; j < state->resourceCount; j++) {
                if (state->need[i][j] > work[j]) {
                    canRun = 0;
                    break;
                }
            }

            if (canRun) {
                for (int j = 0; j < state->resourceCount; j++)
                    work[j] += state->allocation[i][j];
                finish[i]           = 1;
                seq[seqIndex++]     = i;
                found               = 1;
            }
        }
    } while (found);

    for (int i = 0; i < state->processCount; i++) {
        if (!finish[i]) {
            logEvent("DEADLOCK", "Safety check: system is UNSAFE");
            return 0;
        }
    }

    if (safeSequence)
        for (int i = 0; i < state->processCount; i++)
            safeSequence[i] = seq[i];

    logEvent("DEADLOCK", "Safety check: system is SAFE");
    return 1;
}

int requestResources(BankerState *state, int pid, int request[]) {
    computeNeed(state);

    for (int j = 0; j < state->resourceCount; j++) {
        if (request[j] > state->need[pid][j]) {
            logError("DEADLOCK", "Request exceeds maximum claim");
            return -1;
        }
        if (request[j] > state->available[j]) {
            logEvent("DEADLOCK", "Resources unavailable — process must wait");
            return 0;
        }
    }

    for (int j = 0; j < state->resourceCount; j++) {
        state->available[j]        -= request[j];
        state->allocation[pid][j]  += request[j];
        state->need[pid][j]        -= request[j];
    }

    int seq[MAX_DL_PROCESSES];
    if (!runSafetyAlgorithm(state, seq)) {
        for (int j = 0; j < state->resourceCount; j++) {
            state->available[j]        += request[j];
            state->allocation[pid][j]  -= request[j];
            state->need[pid][j]        += request[j];
        }
        logEvent("DEADLOCK", "Request denied — would lead to unsafe state");
        return 0;
    }

    char msg[128];
    snprintf(msg, sizeof(msg), "Request granted for PID=%d", pid);
    logEvent("DEADLOCK", msg);
    return 1;
}

void addWaitEdge(int from, int to) {
    if (from < MAX_DL_PROCESSES && to < MAX_DL_PROCESSES) {
        waitGraph[from][to] = 1;
        if (from >= graphSize) graphSize = from + 1;
        if (to   >= graphSize) graphSize = to   + 1;
    }
}

void removeWaitEdge(int from, int to) {
    if (from < MAX_DL_PROCESSES && to < MAX_DL_PROCESSES)
        waitGraph[from][to] = 0;
}

void clearWaitGraph(void) {
    memset(waitGraph, 0, sizeof(waitGraph));
    graphSize = 0;
}

static int dfsVisit(int node, int visited[], int recStack[]) {
    visited[node]  = 1;
    recStack[node] = 1;

    for (int j = 0; j < graphSize; j++) {
        if (!waitGraph[node][j]) continue;
        if (!visited[j] && dfsVisit(j, visited, recStack)) return 1;
        if (recStack[j]) return 1;
    }

    recStack[node] = 0;
    return 0;
}

int detectCycle(void) {
    int visited[MAX_DL_PROCESSES]  = {0};
    int recStack[MAX_DL_PROCESSES] = {0};

    for (int i = 0; i < graphSize; i++) {
        if (!visited[i] && dfsVisit(i, visited, recStack)) {
            logEvent("DEADLOCK", "Cycle detected in wait-for graph — DEADLOCK");
            return 1;
        }
    }

    logEvent("DEADLOCK", "No cycle detected in wait-for graph");
    return 0;
}

void displayBankerState(const BankerState *state) {
    printf("\n===== Banker's Algorithm State =====\n");
    printf("  Resources: ");
    for (int j = 0; j < state->resourceCount; j++)
        printf("%-18s ", resourceNames[j]);
    printf("\n\n");

    printf("  Available: ");
    for (int j = 0; j < state->resourceCount; j++)
        printf("%-18d ", state->available[j]);
    printf("\n\n");

    printf("  %-6s %-20s %-20s %-20s\n", "PID", "Allocation", "Maximum", "Need");
    printf("  ----------------------------------------------------------------------\n");

    for (int i = 0; i < state->processCount; i++) {
        char alloc[64] = "", maxim[64] = "", need[64] = "";
        char tmp[16];
        for (int j = 0; j < state->resourceCount; j++) {
            snprintf(tmp, sizeof(tmp), "%d ", state->allocation[i][j]);
            strcat(alloc, tmp);
            snprintf(tmp, sizeof(tmp), "%d ", state->maximum[i][j]);
            strcat(maxim, tmp);
            snprintf(tmp, sizeof(tmp), "%d ", state->need[i][j]);
            strcat(need, tmp);
        }
        printf("  P%-5d %-20s %-20s %-20s\n", i, alloc, maxim, need);
    }
    printf("====================================\n");
}

void displayWaitForGraph(void) {
    printf("\n===== Wait-For Graph =====\n");
    if (graphSize == 0) {
        printf("  Graph is empty.\n");
        printf("==========================\n");
        return;
    }
    for (int i = 0; i < graphSize; i++) {
        for (int j = 0; j < graphSize; j++) {
            if (waitGraph[i][j])
                printf("  P%d  -->  P%d\n", i, j);
        }
    }
    printf("==========================\n");
}

static void buildDefaultState(BankerState *state) {
    state->processCount  = 5;
    state->resourceCount = 4;

    int alloc[5][4] = {
        {1, 0, 1, 0},
        {0, 1, 0, 1},
        {2, 0, 1, 0},
        {0, 1, 0, 0},
        {1, 0, 0, 1}
    };
    int maxim[5][4] = {
        {3, 2, 2, 1},
        {1, 2, 1, 2},
        {4, 1, 3, 1},
        {1, 3, 1, 1},
        {2, 1, 1, 2}
    };
    int avail[4] = {2, 1, 1, 2};

    memcpy(state->allocation, alloc, sizeof(alloc));
    memcpy(state->maximum, maxim, sizeof(maxim));
    memcpy(state->available, avail, sizeof(avail));
    computeNeed(state);
}

void deadlockMenu(void) {
    BankerState state;
    memset(&state, 0, sizeof(state));
    buildDefaultState(&state);

    int choice;
    do {
        printf("\n===== Deadlock Management =====\n");
        printf("  1. Run Safety Algorithm\n");
        printf("  2. Request Resources\n");
        printf("  3. Display Banker State\n");
        printf("  4. Add Wait-For Edge\n");
        printf("  5. Detect Cycle (RAG)\n");
        printf("  6. Display Wait-For Graph\n");
        printf("  7. Clear Wait-For Graph\n");
        printf("  0. Back\n");
        printf("  Choice: ");
        if (scanf("%d", &choice) != 1) { choice = -1; while(getchar() != '\n'); }

        switch (choice) {
            case 1: {
                int seq[MAX_DL_PROCESSES];
                int safe = runSafetyAlgorithm(&state, seq);
                if (safe) {
                    printf("\n  Status : SAFE\n  Safe Sequence: ");
                    for (int i = 0; i < state.processCount; i++)
                        printf("P%d%s", seq[i], i < state.processCount - 1 ? " -> " : "\n");
                } else {
                    printf("\n  Status : UNSAFE — Deadlock risk detected.\n");
                }
                break;
            }
            case 2: {
                int pid;
                int req[MAX_RESOURCES] = {0};
                printf("  Process ID (0-%d): ", state.processCount - 1);
                scanf("%d", &pid);
                printf("  Request (Ambulance Fire Police Rescue): ");
                for (int j = 0; j < state.resourceCount; j++)
                    scanf("%d", &req[j]);
                int result = requestResources(&state, pid, req);
                if (result == 1)
                    printf("  Request GRANTED.\n");
                else if (result == 0)
                    printf("  Request DENIED — unsafe state or insufficient resources.\n");
                else
                    printf("  Request INVALID — exceeds maximum claim.\n");
                break;
            }
            case 3:
                displayBankerState(&state);
                break;
            case 4: {
                int from, to;
                printf("  From process: ");
                scanf("%d", &from);
                printf("  To process  : ");
                scanf("%d", &to);
                addWaitEdge(from, to);
                printf("  Edge P%d -> P%d added.\n", from, to);
                break;
            }
            case 5: {
                int cycle = detectCycle();
                printf("\n  Result: %s\n",
                       cycle ? "DEADLOCK DETECTED — cycle found in wait-for graph."
                             : "No deadlock — no cycle detected.");
                break;
            }
            case 6:
                displayWaitForGraph();
                break;
            case 7:
                clearWaitGraph();
                printf("  Wait-for graph cleared.\n");
                break;
            case 0: break;
            default: printf("  Invalid option.\n");
        }
    } while (choice != 0);
}
