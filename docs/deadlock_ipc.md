# Deadlock & IPC Modules — SERC OS

---

## Part 1: Deadlock Module

### Overview

The deadlock module implements two classic deadlock-handling techniques from operating systems theory, themed around SERC (South African Emergency Response Coordination) resource management:

1. **Banker's Algorithm** — A deadlock-avoidance algorithm that checks whether resource requests can be granted without leading to an unsafe state.
2. **Wait-For Graph & Cycle Detection** — A deadlock-detection approach that models processes waiting for each other and checks for cycles using DFS.

The four resource types are modelled as SERC emergency units:
- **Ambulance Units** (index 0)
- **Fire Units** (index 1)
- **Police Units** (index 2)
- **Rescue Teams** (index 3)

---

### Key Data Structures

#### `BankerState` (defined in `include/deadlock.h`)

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

#### Wait-For Graph (static globals in `src/deadlock.c`)

```c
static int waitGraph[MAX_DL_PROCESSES][MAX_DL_PROCESSES];  // adjacency matrix
static int graphSize = 0;                                   // number of processes tracked
```

An edge `waitGraph[from][to] = 1` means process `from` is waiting for a resource held by process `to`.

---

### Banker's Algorithm

#### `runSafetyAlgorithm()` — Full Annotated Walkthrough

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

#### `requestResources()` — Resource Request Lifecycle

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

### Wait-For Graph & Cycle Detection

#### `addWaitEdge()` / `clearWaitGraph()`

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

#### `detectCycle()` — DFS with Recursion Stack

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

### Default Banker State

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

### Example Run

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

### Deadlock Menu Structure

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

---

## Part 2: IPC Module

### Overview

The IPC module demonstrates three classic UNIX inter-process communication mechanisms, themed around an emergency dispatch command-and-control system (SERC — South African Emergency Response Coordination). Each mechanism is wrapped in a self-contained demo function accessible from the IPC menu.

**Three mechanisms covered:**
1. **Anonymous Pipe** — One-way byte stream between parent and child processes (`fork()` + `pipe()`)
2. **System V Message Queue** — Message-passing with typed messages (`msgget` / `msgsnd` / `msgrcv`)
3. **System V Shared Memory** — Shared memory segment with reader/writer semantics (`shmget` / `shmat`)

---

### Key Data Structures

#### `SERCMessage` (defined in `include/ipc.h`)

```c
#define MSG_MAX_TEXT     256
#define IPC_KEY_BASE     0x5ERC

typedef enum {
    UNIT_AMBULANCE = 1,
    UNIT_FIRE      = 2,
    UNIT_POLICE    = 3,
    UNIT_RESCUE    = 4
} UnitType;

typedef struct {
    long    mtype;                      // message type (for message queues)
    UnitType unitType;                  // type of emergency unit
    int     unitCount;                  // number of units dispatched
    char    text[MSG_MAX_TEXT];         // dispatch message text
} SERCMessage;
```

#### `SharedStatus` (defined in `include/ipc.h`)

```c
typedef struct {
    int     activeIncidents;            // number of active incidents
    int     dispatchedUnits;            // total units dispatched
    char    lastIncident[MSG_MAX_TEXT]; // description of most recent incident
} SharedStatus;
```

#### Helper: `unitName()`

```c
static const char *unitName(UnitType type) {
    switch (type) {
        case UNIT_AMBULANCE: return "Ambulance Unit";
        case UNIT_FIRE:      return "Fire Unit";
        case UNIT_POLICE:    return "Police Unit";
        case UNIT_RESCUE:    return "Rescue Team";
        default:             return "Unknown Unit";
    }
}
```

---

### 1. Anonymous Pipe Demo (`runPipeDemo()`)

```c
void runPipeDemo(void) {
    printf("\n===== IPC: Anonymous Pipe =====\n");

    int pipefd[2];
    if (pipe(pipefd) == -1) {           // 1. Create pipe (pipefd[0]=read, pipefd[1]=write)
        perror("pipe");
        logError("IPC", "Pipe creation failed");
        return;
    }

    pid_t pid = fork();                 // 2. Fork — child inherits pipefd
    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        // CHILD PROCESS
        close(pipefd[1]);               // 3a. Child closes write end
        char buf[MSG_MAX_TEXT];
        ssize_t n = read(pipefd[0], buf, sizeof(buf) - 1);  // 4a. Read from pipe
        if (n > 0) {
            buf[n] = '\0';
            printf("  [Child  PID=%d] Received dispatch: \"%s\"\n", getpid(), buf);
        }
        close(pipefd[0]);
        exit(0);
    } else {
        // PARENT PROCESS
        close(pipefd[0]);               // 3b. Parent closes read end
        const char *msg = "Dispatch 2x Ambulance Unit to Grid-5 Incident";
        write(pipefd[1], msg, strlen(msg));  // 4b. Write dispatch to pipe
        printf("  [Parent PID=%d] Sent dispatch: \"%s\"\n", getpid(), msg);
        close(pipefd[1]);
        wait(NULL);                     // 5. Wait for child to finish
    }

    logEvent("IPC", "Pipe demo completed");
    printf("===============================\n");
}
```

**Lifecycle:**

```
Parent                     Pipe                     Child
  │                                                  │
  ├─ close(pipefd[0])      ◀─────── read ────────    │
  ├─ write("Dispatch...")  ──────── write ──────►    │
  ├─ wait(NULL)                                      │
  │                                                  ├─ close(pipefd[1])
  │                                                  ├─ read → "Dispatch..."
  │                                                  └─ exit(0)
```

1. `pipe(pipefd)` creates a unidirectional byte stream. `pipefd[0]` is the read end, `pipefd[1]` is the write end.
2. `fork()` creates a child process that inherits both file descriptors.
3. Each process closes the unused end (parent closes read, child closes write) — standard pipe discipline.
4. Parent writes the dispatch message; child reads it from the other end.
5. Parent calls `wait(NULL)` to reap the child before returning.

---

### 2. Message Queue Demo (`runMessageQueueDemo()`)

```c
void runMessageQueueDemo(void) {
    printf("\n===== IPC: Message Queue =====\n");

    key_t key = ftok("/tmp", 'S');      // 1. Generate a unique IPC key
    if (key == -1) key = 0x5ABC;        //    Fallback if ftok fails

    int msgid = msgget(key, IPC_CREAT | 0666);  // 2. Create message queue
    if (msgid == -1) {
        perror("msgget");
        logError("IPC", "Message queue creation failed");
        return;
    }

    // 3. Prepare and send a message with mtype = UNIT_FIRE
    SERCMessage dispatch;
    dispatch.mtype     = UNIT_FIRE;     // Message type = 2 (Fire units)
    dispatch.unitType  = UNIT_FIRE;
    dispatch.unitCount = 3;
    snprintf(dispatch.text, sizeof(dispatch.text),
             "Dispatch %d %s to Building Fire on Kitwe Central",
             dispatch.unitCount, unitName(dispatch.unitType));

    if (msgsnd(msgid, &dispatch, sizeof(dispatch) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        logError("IPC", "Message send failed");
        msgctl(msgid, IPC_RMID, NULL);   // Cleanup and abort
        return;
    }
    printf("  [Dispatch Center] Sent: \"%s\"\n", dispatch.text);

    // 4. Receive the message back (same process, simulating a field unit)
    SERCMessage received;
    if (msgrcv(msgid, &received, sizeof(received) - sizeof(long), UNIT_FIRE, 0) == -1) {
        perror("msgrcv");
        logError("IPC", "Message receive failed");
    } else {
        printf("  [Field Unit     ] Received: \"%s\"\n", received.text);
        printf("  [Field Unit     ] Unit Type: %s  Count: %d\n",
               unitName(received.unitType), received.unitCount);
    }

    msgctl(msgid, IPC_RMID, NULL);       // 5. Destroy the message queue
    logEvent("IPC", "Message queue demo completed");
    printf("==============================\n");
}
```

**Key points about message queues:**

- **`ftok()`** generates a standard IPC key from a pathname and a project identifier.
- **`msgget()`** creates (or opens) a message queue with the given key and permissions `0666`.
- **`msgsnd()`** sends a message. The `sizeof(dispatch) - sizeof(long)` excludes the `mtype` field from the message payload (the kernel reads `mtype` separately).
- **`msgrcv()`** receives a message. The fourth argument `UNIT_FIRE` means "receive the first message whose `mtype` equals `UNIT_FIRE`". You could pass `0` to receive the first message regardless of type, or a negative value to receive messages with type ≤ that value.
- **`msgctl(IPC_RMID)`** destroys the queue. This is critical — the queue persists in the kernel until explicitly removed.

**Type-based filtering:**

```
Message Queue
─────────────────────────────────────────
│ mtype=1 (Ambulance)  │  message text  │
│ mtype=2 (Fire)       │  message text  │  ← msgrcv with mtype=2 reads this
│ mtype=3 (Police)     │  message text  │
│ mtype=4 (Rescue)     │  message text  │
─────────────────────────────────────────
```

---

### 3. Shared Memory Demo (`runSharedMemoryDemo()`)

```c
void runSharedMemoryDemo(void) {
    printf("\n===== IPC: Shared Memory =====\n");

    key_t key = ftok("/tmp", 'R');      // 1. Generate IPC key
    if (key == -1) key = 0x5DEF;

    int shmid = shmget(key, sizeof(SharedStatus), IPC_CREAT | 0666);  // 2. Create segment
    if (shmid == -1) {
        perror("shmget");
        logError("IPC", "Shared memory creation failed");
        return;
    }

    // 3. Attach as WRITER (read-write)
    SharedStatus *status = (SharedStatus *)shmat(shmid, NULL, 0);
    if (status == (SharedStatus *)-1) {
        perror("shmat");
        logError("IPC", "Shared memory attach failed");
        shmctl(shmid, IPC_RMID, NULL);
        return;
    }

    // 4. Write SERC incident status into shared memory
    status->activeIncidents = 4;
    status->dispatchedUnits = 9;
    snprintf(status->lastIncident, sizeof(status->lastIncident),
             "Multi-vehicle accident at Ndola Roundabout — 3 Ambulance, 2 Police dispatched");

    printf("  [Writer] Wrote SERC Status to shared memory:\n");
    printf("           Active Incidents : %d\n", status->activeIncidents);
    printf("           Dispatched Units : %d\n", status->dispatchedUnits);
    printf("           Last Incident    : %s\n", status->lastIncident);

    // 5. Attach as READER (read-only)
    SharedStatus *reader = (SharedStatus *)shmat(shmid, NULL, SHM_RDONLY);
    if (reader != (SharedStatus *)-1) {
        printf("\n  [Reader] Read SERC Status from shared memory:\n");
        printf("           Active Incidents : %d\n", reader->activeIncidents);
        printf("           Dispatched Units : %d\n", reader->dispatchedUnits);
        printf("           Last Incident    : %s\n", reader->lastIncident);
        shmdt(reader);                   // 6a. Detach reader
    }

    shmdt(status);                       // 6b. Detach writer
    shmctl(shmid, IPC_RMID, NULL);       // 7. Remove shared memory segment
    logEvent("IPC", "Shared memory demo completed");
    printf("==============================\n");
}
```

**Lifecycle:**

```
shmget(IPC_CREAT)  ──►  Kernel allocates segment (sizeof(SharedStatus))
       │
shmat(RDWR)         ──►  Writer attaches → writes data
       │
shmat(SHM_RDONLY)   ──►  Reader attaches → reads data
       │
shmdt(reader)       ──►  Reader detaches
shmdt(status)       ──►  Writer detaches
       │
shmctl(IPC_RMID)    ──►  Kernel destroys segment
```

**Key points about shared memory:**

- **`shmget()`** creates a shared memory segment of size `sizeof(SharedStatus)`. All processes that know the key can access it.
- **`shmat()`** attaches the segment into the process's virtual address space. Returns a pointer you can dereference directly. With `SHM_RDONLY`, writes to the mapped memory will segfault.
- **`shmdt()`** detaches the segment (does not destroy it).
- **`shmctl(IPC_RMID)`** marks the segment for removal. The kernel actually destroys it only after the last process detaches.
- In this demo, the same process does both the write and the read. In a real system, separate processes (a dispatch center and a field terminal) would each attach independently.

---

### Demo Comparison

| Mechanism | Direction | Persistence | Setup Cost | Best For |
|-----------|-----------|-------------|------------|----------|
| Anonymous Pipe | Unidirectional (one-way) | None (kernel buffer, destroyed when both ends close) | Low | Parent-child communication, streaming data |
| Message Queue | Bidirectional (typed messages) | Kernel persists until `IPC_RMID` | Medium | Structured messages with type-based routing |
| Shared Memory | Bidirectional (direct reads/writes) | Kernel persists until `IPC_RMID` | Medium | High-speed shared data (no kernel copies) |

---

### IPC Menu Structure

The `ipcMenu()` function in `src/ipc.c` ties everything together:

| Option | Function Called | Description |
|--------|----------------|-------------|
| 1 | `runPipeDemo()` | Anonymous pipe — parent sends dispatch to child |
| 2 | `runMessageQueueDemo()` | Message queue — send and receive a typed SERC dispatch |
| 3 | `runSharedMemoryDemo()` | Shared memory — write and read incident status |
| 4 | All three | Runs pipe, message queue, and shared memory demos sequentially |
| 0 | — | Return to main menu |

### Example Run

```
===== IPC Mechanisms =====
  1. Anonymous Pipe
  2. Message Queue
  3. Shared Memory
  4. Run All Demos
  0. Back
  Choice: 1

===== IPC: Anonymous Pipe =====
  [Parent PID=11234] Sent dispatch: "Dispatch 2x Ambulance Unit to Grid-5 Incident"
  [Child  PID=11235] Received dispatch: "Dispatch 2x Ambulance Unit to Grid-5 Incident"
===============================

  Choice: 2

===== IPC: Message Queue =====
  [Dispatch Center] Sent: "Dispatch 3 Fire Unit to Building Fire on Kitwe Central"
  [Field Unit     ] Received: "Dispatch 3 Fire Unit to Building Fire on Kitwe Central"
  [Field Unit     ] Unit Type: Fire Unit  Count: 3
==============================

  Choice: 3

===== IPC: Shared Memory =====
  [Writer] Wrote SERC Status to shared memory:
           Active Incidents : 4
           Dispatched Units : 9
           Last Incident    : Multi-vehicle accident at Ndola Roundabout — 3 Ambulance, 2 Police dispatched

  [Reader] Read SERC Status from shared memory:
           Active Incidents : 4
           Dispatched Units : 9
           Last Incident    : Multi-vehicle accident at Ndola Roundabout — 3 Ambulance, 2 Police dispatched
==============================
```

### Cleanup Notes

- All three demos clean up after themselves (pipe ends are closed, message queue is removed with `IPC_RMID`, shared memory is detached and removed).
- The pipe demo uses `fork()`, so the child process is a true separate process with its own address space. The pipe acts as the communication channel.
- The message queue and shared memory demos run in a single process for simplicity, but the same System V primitives work identically across unrelated processes as long as they share the IPC key.