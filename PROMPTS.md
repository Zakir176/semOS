# SERC OS Prompt Input Guide

This file lists the interactive prompts shown by `bin/serc-os` when running the program.

## Launcher
- `Choice:`
  - `1` → Command Line Interface (CLI)
  - `2` → Graphical Interface (GUI)
  - `0` → Exit

## CLI Main Menu
After selecting CLI, the main menu prompt is:
- `Choice:`
  - `1` → Process Management
  - `2` → CPU Scheduling
  - `3` → Memory Management
  - `4` → IPC Mechanisms
  - `5` → Deadlock Management
  - `6` → File Management
  - `0` → Exit

## Process Management
### Menu options
- `Choice:`
  - `1` → Create Process
  - `2` → Terminate Process
  - `3` → List All Processes
  - `4` → View Process by PID
  - `5` → Set Process State
  - `0` → Back

### Create Process inputs
- `Process name:` → text string (single token)
- `Burst time:` → integer > 0
- `Arrival time:` → integer (time unit)
- `Priority (1=Low 2=Medium 3=High):` → `1`, `2`, or `3`
- `Memory required (KB):` → integer > 0

### Terminate Process
- `PID to terminate:` → process ID integer

### View Process by PID
- `PID:` → process ID integer

### Set Process State
- `PID:` → process ID integer
- `State (0=Ready 1=Running 2=Waiting 3=Terminated):` → `0`, `1`, `2`, or `3`

## CPU Scheduling
### Menu options
- `Choice:`
  - `1` → Run FCFS
  - `2` → Run SJF
  - `3` → Run Round Robin
  - `0` → Back

### Round Robin input
- `Time quantum:` → integer > 0

## Memory Management
### Menu options
- `Choice:`
  - `1` → Allocate Memory
  - `2` → Free Memory
  - `3` → View Memory Map
  - `4` → View Memory Statistics
  - `0` → Back

### Allocate Memory inputs
- `PID:` → process ID integer
- `Size (KB):` → integer > 0
- `Strategy (0=First 1=Best 2=Worst):` → `0`, `1`, or `2`

### Free Memory
- `PID to free:` → process ID integer

## IPC Mechanisms
### Menu options
- `Choice:`
  - `1` → Anonymous Pipe Demo
  - `2` → Message Queue Demo
  - `3` → Shared Memory Demo
  - `4` → Run All Demos
  - `0` → Back

## Deadlock Management
### Menu options
- `Choice:`
  - `1` → Run Safety Algorithm
  - `2` → Request Resources
  - `3` → Display Banker State
  - `4` → Add Wait-For Edge
  - `5` → Detect Cycle (RAG)
  - `6` → Display Wait-For Graph
  - `7` → Clear Wait-For Graph
  - `0` → Back

### Request Resources inputs
- `Process ID (0-N):` → integer process ID in the current Banker state range
- `Request (Ambulance Fire Police Rescue):` → four integers separated by spaces

### Add Wait-For Edge inputs
- `From process:` → integer PID or process index
- `To process  :` → integer PID or process index

## File Management
### Menu options
- `Choice:`
  - `1` → Create File
  - `2` → List Files
  - `3` → Open File
  - `4` → Write to File
  - `5` → Read File
  - `6` → Display File Details
  - `7` → Close File
  - `8` → Delete File
  - `0` → Back

### Create File inputs
- `File name:` → text string (single token)
- `Type (0=Incident 1=Dispatch 2=Report 3=Log):` → `0`, `1`, `2`, or `3`

### Open/Write/Read/Display/Close/Delete File inputs
- `File ID:` → file ID integer

### Write to File additional input
- `Data (no spaces):` → text string without spaces

## GUI Dialog Prompts
The GUI uses the same underlying labels for prompt dialogs:
- `Process Name:`
- `Burst Time:`
- `Arrival Time:`
- `Priority (1=Low 2=Medium 3=High):`
- `Memory Required (KB):`
- `PID to terminate:`
- `Time Quantum:`
- `PID:`
- `Size (KB):`
- `Strategy (0=First 1=Best 2=Worst):`
- `PID to free:`
- `From process (P_n):`
- `To process (P_n):`
- `Type (0=Incident 1=Dispatch 2=Report 3=Log):`
- `File ID:`
- `Content:`

## Notes
- Invalid numeric choice values will usually result in `Invalid option.` or `Invalid priority.` and return to the menu.
- In CLI mode, input is read with `scanf` and text fields generally accept a single token without spaces.
- In GUI mode, prompts appear as dialog boxes with the same labels.
