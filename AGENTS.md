# AGENTS

## Purpose
Provide a brief, agent-friendly reference for working with this repository and focusing on runtime errors, edge cases, and race-condition risks.

## Project overview
- Language: C
- Simulation of a small operating system concept with process management, CPU scheduling, memory allocation, IPC demos, deadlock handling, and file management.
- Source files live in `src/`; headers in `include/`.
- No automated test framework was detected in the repository.

## Build and run
- Build the full app: `make`
- Build CLI-only binary: `make cli`
- Binaries are produced in `bin/`: `bin/serc-os` and `bin/serc-os-cli`
- GUI build depends on GTK+ 3.0 via `pkg-config gtk+-3.0`
- Log file: `logs/serc.log`

## Key areas for code review
- `src/main.c` — launcher and interface selection logic
- `src/ui_cli.c` — interactive CLI menus, user input, and state transitions
- `src/ui_gui.c` — GUI entrypoint and GTK-based interface (compiled only when GTK is available)
- `src/process.c` / `include/process.h` — static process table and process lifecycle
- `src/scheduler.c` / `include/scheduler.h` — FCFS, SJF, and Round Robin scheduling logic
- `src/memory.c` / `include/memory.h` — memory partitioning, fit strategies, allocation, and fragmentation
- `src/ipc.c` / `include/ipc.h` — pipe, message queue, shared memory demos
- `src/logger.c` / `include/logger.h` — global logging to a single file
- `src/deadlock.c`, `src/filemanager.c`, `src/ui_gui.c` — auxiliary simulation features

## Safety and review guidance
When checking for errors, edge cases, or race conditions, focus on:

- Input validation and bounds:
  - `scanf` use in CLI menus and `getSafeInt`
  - `strncpy` length limits and string termination
  - `MAX_PROCESSES` and `MAX_GANTT_SLOTS` assumptions
  - `queue` size in `runRoundRobin` and `partitionCount` limits in `memory.c`

- Scheduler correctness and corner cases:
  - `runSJF` selection when no process has arrived yet
  - `runRoundRobin` with `quantum <= 0`
  - idle time handling and `currentTime` advancement
  - recomputing average metrics when `count == 0`

- Memory allocation edge conditions:
  - allocation requests larger than total memory
  - `splitPartition` behavior when remaining size is zero
  - adjacent free-partition coalescing in `freeMemory`
  - stale `allocatedPID` values and fragmentation accounting

- IPC and resource cleanup:
  - proper `msgctl(..., IPC_RMID, NULL)` cleanup after message queue use
  - `shmdt` and `shmctl(..., IPC_RMID, NULL)` usage in shared memory demos
  - lack of synchronization for shared memory reader/writer access; demo-only semantics
  - `fork` behavior in `runPipeDemo`

- Logger and global state:
  - `logger.c` uses a single shared `FILE *` and is not thread-safe
  - static arrays in `process.c` and `memory.c` are globally shared
  - `getAllProcesses` returns a static pointer array, so callers should not assume thread safety

- GUI vs CLI mode:
  - the main binary selects CLI or GUI via `--cli`/`--gui`
  - `make cli` compiles `SERC_CLI_ONLY`, disabling GUI support

## Notes for future agents
- Do not assume this is a multi-threaded codebase; concurrency exists only via IPC demos and `fork`.
- There are no tests in the repository, so use `make` and manual runtime checks when validating fixes.
- Prefer minimal code changes that preserve the existing `Makefile` interface and CLI behavior.
