# Deadlock Module — SERC OS

## Overview

The deadlock module implements two classic deadlock-handling techniques from operating systems theory, themed around SERC (South African Emergency Response Coordination) resource management:

1. **Banker's Algorithm** — A deadlock-avoidance algorithm that checks whether resource requests can be granted without leading to an unsafe state.
2. **Wait-For Graph & Cycle Detection** — A deadlock-detection approach that models processes waiting for each other and checks for cycles using DFS.

The four resource types are modelled as SERC emergency units:
- **Ambulance Units** (index 0)
- **Fire Units** (index 1)
- **Police Units** (index 2)
- **Rescue Teams** (index 3)

---

## Key Data Structures

### `BankerState` (defined in `include/deadlock.h`)

```c
#define MAX_DL_PROCESSES  10
#define MAX_RESOURCES      4

typedef struct {
    int allocation[MAX_DL_PROCESSES][MAX_RESOURCES];  // resources currently held by each process
    int maximum[MAX_DL_PROCESSES][MAX_RESOURCES];     // max resources each process may ever request
    int available[MAX_RESOURCES];                     // resources currently free
    int need[MAX_DL_PROCESSES][MAX_RESOURCES];        // calculated: maximum - allocation
    int processCount;
    int resourceCount;
} BankerState;
```

The `need` matrix is computed on the fly by `computeNeed()`:

```c
static void computeNeed(BankerState *state) {
    for (int i = 0; i < state->processCount; i++)
        for (int j = 0; j < state->resourceCount; j++)
            state->need[i][j] = state->maximum[i][j] - state->allocation[i][j];
}
```

### Wait-For Graph (static globals in `src/deadlock.c`)

```c
static int waitGraph[MAX_DL_PROCESSES][MAX_DL_PROCESSES];  // adjacency matrix
static int graphSize = 0;                                   // number of processes tracked
```

An edge `waitGraph[from][to] = 1` means process `from` is waiting for a resource held by process `to`.

---

## Banker's Algorithm

### `runSafetyAlgorithm()` — Full Annotated Walkthrough

```c
int runSafetyAlgorithm(BankerState *state, int *safeSequence) {
    computeNeed(state);                              // 1. Refresh need matrix

    int work[MAX_RESOURCES];                         // 2. Copy available into work
    int finish[MAX_DL_PROCESSES] = {0};              // 3. All processes start as unfinished
    int seq[MAX_DL_PROCESSES];
    int seqIndex = 0;

    for (int j = 0; j < state->resourceCount; j++)
        work[j] = state->available[j];

    int found;
    do {
        found = 0;
        for (int i = 0; i < state->processCount; i++) {
            if (finish[i]) continue;                 // 4. Skip already-finished processes

            int canRun = 1;
            for (int j = 0; j < state->resourceCount; j++) {
                if (state->need[i][j] > work[j]) {   // 5. Check if need <= work for all resources
                    canRun = 0;
                    break;
                }
            }

            if (canRun) {
                for (int j = 0; j < state->resourceCount; j++)
                    work[j] += state->allocation[i][j];  // 6. Assume process finishes, release resources
                finish[i]           = 1;
                seq[seqIndex++]     = i;
                found               = 1;                 // 7. At least one process could run this pass
            }
        }
    } while (found);                                     // 8. Repeat until no more processes can run

    for (int i = 0; i < state->processCount; i++) {
        if (!finish[i]) return 0;                        // 9. Any unfinished process → UNSAFE
    }

    if (safeSequence)
        for (int i = 0; i < state->processCount; i++)
            safeSequence[i] = seq[i];                    // 10. Copy safe sequence to output

    return 1;                                            // 11. All processes finished → SAFE
}
```

**How it works step by step:**

1. Compute `need = maximum - allocation` for every process.
2. `work` starts as a copy of `available` — the resources currently free.
3. All processes are marked unfinished.
4. The outer loop keeps scanning until a full pass finds no runnable process.
5. For each unfinished process, check if its *remaining need* can be satisfied by the current `work`.
6. If yes, pretend it runs to completion and releases its allocation back into `work`.
7. Mark the process finished and record it in the safe sequence.
8. If no process could run in a full pass, exit the loop.
9. If any process is still unfinished, the system is **unsafe** (deadlock risk).
10. Otherwise, the sequence is a valid safe ordering.

### `requestResources()` — Resource Request Lifecycle

```c
int requestResources(BankerState *state, int pid, int request[]) {
    computeNeed(state);

    // Phase 1: Validate the request
    for (int j = 0; j < state->resourceCount; j++) {
        if (request[j] > state->need[pid][j]) {       // Cannot ask for more than declared maximum
            logError("DEADLOCK", "Request exceeds maximum claim");
            return -1;                                 // Invalid request
        }
        if (request[j] > state->available[j]) {        // Resources not immediately available
            logEvent("DEADLOCK", "Resources unavailable — process must wait");
            return 0;                                  // Must wait (not unsafe, just blocked)
        }
    }

    // Phase 2: Tentatively allocate the resources
    for (int j = 0; j < state->resourceCount; j++) {
        state->available[j]        -= request[j];
        state->allocation[pid][j]  += request[j];
        state->need[pid][j]        -= request[j];
    }

    // Phase 3: Check if the new state is safe
    int seq[MAX_DL_PROCESSES];
    if (!runSafetyAlgorithm(state, seq)) {
        // Phase 4: Rollback if unsafe
        for (int j = 0; j < state->resourceCount; j++) {
            state->available[j]        += request[j];
            state->allocation[pid][j]  -= request[j];
            state->need[pid][j]        += request[j];
        }
        logEvent("DEADLOCK", "Request denied — would lead to unsafe state");
        return 0;
    }

    logEvent("DEADLOCK", "Request granted");
    return 1;                                          // Safe — allocation stands
}
```

**Key insight:** The rollback in Phase 4 is what makes this *deadlock avoidance* rather than just detection. The system never enters an unsafe state — it rejects requests that would lead there.

---

## Wait-For Graph & Cycle Detection

### `addWaitEdge()` / `clearWaitGraph()`

```c
void addWaitEdge(int from, int to) {
    if (from < MAX_DL_PROCESSES && to < MAX_DL_PROCESSES) {
        waitGraph[from][to] = 1;
        if (from >= graphSize) graphSize = from + 1;
        if (to   >= graphSize) graphSize = to   + 1;
    }
}

void clearWaitGraph(void) {
    memset(waitGraph, 0, sizeof(waitGraph));
    graphSize = 0;
}
```

### `detectCycle()` — DFS with Recursion Stack

```c
static int dfsVisit(int node, int visited[], int recStack[]) {
    visited[node]  = 1;          // Mark as visited
    recStack[node] = 1;          // Add to current recursion stack

    for (int j = 0; j < graphSize; j++) {
        if (!waitGraph[node][j]) continue;          // No edge → skip
        if (!visited[j] && dfsVisit(j, visited, recStack))
            return 1;                                // Cycle found deeper in the graph
        if (recStack[j])
            return 1;                                // Back edge → cycle detected
    }

    recStack[node] = 0;          // Remove from recursion stack (backtrack)
    return 0;
}

int detectCycle(void) {
    int visited[MAX_DL_PROCESSES]  = {0};
    int recStack[MAX_DL_PROCESSES] = {0};

    for (int i = 0; i < graphSize; i++) {
        if (!visited[i] && dfsVisit(i, visited, recStack)) {
            logEvent("DEADLOCK", "Cycle detected — DEADLOCK");
            return 1;
        }
    }
    logEvent("DEADLOCK", "No cycle detected");
    return 0;
}
```

**How cycle detection works:**

- `visited[]` tracks which nodes have been fully explored.
- `recStack[]` tracks nodes currently on the DFS path (the "call stack").
- If we encounter a node already on the recursion stack, we've found a **back edge** → a cycle → deadlock.
- This is a standard O(V+E) DFS-based cycle detection for directed graphs.

---

## Default Banker State

When you enter the Deadlock Management menu, a default 5-process, 4-resource scenario is loaded:

```c
static void buildDefaultState(BankerState *state) {
    state->processCount  = 5;
    state->resourceCount = 4;

    int alloc[5][4] = {
        {1, 0, 1, 0},   // P0: holds 1 Ambulance, 1 Police
        {0, 1, 0, 1},   // P1: holds 1 Fire, 1 Rescue
        {2, 0, 1, 0},   // P2: holds 2 Ambulance, 1 Police
        {0, 1, 0, 0},   // P3: holds 1 Fire
        {1, 0, 0, 1}    // P4: holds 1 Ambulance, 1 Rescue
    };
    int maxim[5][4] = {
        {3, 2, 2, 1},   // P0 may need up to 3 Ambulance, 2 Fire, 2 Police, 1 Rescue
        {1, 2, 1, 2},
        {4, 1, 3, 1},
        {1, 3, 1, 1},
        {2, 1, 1, 2}
    };
    int avail[4] = {2, 1, 1, 2};   // 2 Ambulance, 1 Fire, 1 Police, 2 Rescue free

    memcpy(state->allocation, alloc, sizeof(alloc));
    memcpy(state->maximum, maxim, sizeof(maxim));
    memcpy(state->available, avail, sizeof(avail));
    computeNeed(state);
}
```

This state is **safe** — running the safety algorithm produces a sequence like `P1 → P3 → P4 → P0 → P2`.

---

## Example Run

```
===== Deadlock Management =====
  1. Run Safety Algorithm
  2. Request Resources
  3. Display Banker State
  4. Add Wait-For Edge
  5. Detect Cycle (RAG)
  6. Display Wait-For Graph
  7. Clear Wait-For Graph
  0. Back
  Choice: 1

  Status : SAFE
  Safe Sequence: P1 -> P3 -> P4 -> P0 -> P2

  Choice: 2
  Process ID (0-4): 2
  Request (Ambulance Fire Police Rescue): 2 1 2 1
  Request DENIED — unsafe state or insufficient resources.

  Choice: 4
  From process: 1
  To process  : 2
  Edge P1 -> P2 added.

  Choice: 4
  From process: 2
  To process  : 3
  Edge P2 -> P3 added.

  Choice: 4
  From process: 3
  To process  : 1
  Edge P3 -> P1 added.

  Choice: 5
  Result: DEADLOCK DETECTED — cycle found in wait-for graph.
```

---

## Menu Structure

The `deadlockMenu()` function in `src/deadlock.c` ties everything together:

| Option | Function Called | Description |
|--------|----------------|-------------|
| 1 | `runSafetyAlgorithm()` | Check if current Banker state is safe |
| 2 | `requestResources()` | Request resources (with safety check) |
| 3 | `displayBankerState()` | Print allocation/max/available/need tables |
| 4 | `addWaitEdge()` | Add a directed edge to the wait-for graph |
| 5 | `detectCycle()` | Run DFS cycle detection on the wait-for graph |
| 6 | `displayWaitForGraph()` | Print all edges in the wait-for graph |
| 7 | `clearWaitGraph()` | Reset the wait-for graph to empty |
| 0 | — | Return to main menu |