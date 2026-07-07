# SERC OS Prompt Input Guide

This file lists every interactive prompt shown when running `bin/serc-os` (or `bin/serc-os-cli`), along with **example inputs** you can type.

---

## Launcher

When you start `bin/serc-os`, you'll see:

```
Choice:
```

| Input | Result |
|-------|--------|
| `1` | Open Command Line Interface (CLI) |
| `2` | Open Graphical Interface (GUI) |
| `0` | Exit the program |

**Example:**
```
Choice: 1
```

---

## CLI Main Menu

After selecting CLI mode:

```
Choice:
```

| Input | Result |
|-------|--------|
| `1` | Process Management |
| `2` | CPU Scheduling |
| `3` | Memory Management |
| `4` | IPC Mechanisms |
| `5` | Deadlock Management |
| `6` | File Management |
| `0` | Go back / Exit |

**Example:**
```
Choice: 1
```

---

## 1. Process Management

### Menu
```
Choice:
```

| Input | Action |
|-------|--------|
| `1` | Create Process |
| `2` | Terminate Process |
| `3` | List All Processes |
| `4` | View Process by PID |
| `5` | Set Process State |
| `0` | Back |

### Create Process
```
Process name: MyProcess
Burst time: 10
Arrival time: 0
Priority (1=Low 2=Medium 3=High): 2
Memory required (KB): 512
```

| Field | Example | Rules |
|-------|---------|-------|
| Process name | `MyProcess` | Single word, no spaces |
| Burst time | `10` | Integer > 0 |
| Arrival time | `0` | Integer (time unit) |
| Priority | `2` | `1` (Low), `2` (Medium), or `3` (High) |
| Memory required | `512` | Integer > 0 (KB) |

### Terminate Process
```
PID to terminate: 3
```

### View Process by PID
```
PID: 3
```

### Set Process State
```
PID: 3
State (0=Ready 1=Running 2=Waiting 3=Terminated): 1
```

| State | Value |
|-------|-------|
| Ready | `0` |
| Running | `1` |
| Waiting | `2` |
| Terminated | `3` |

---

## 2. CPU Scheduling

### Menu
```
Choice:
```

| Input | Action |
|-------|--------|
| `1` | Run FCFS (First-Come, First-Served) |
| `2` | Run SJF (Shortest Job First) |
| `3` | Run Round Robin |
| `0` | Back |

### Round Robin (option 3)
```
Time quantum: 4
```

| Field | Example | Rules |
|-------|---------|-------|
| Time quantum | `4` | Integer > 0 |

---

## 3. Memory Management

### Menu
```
Choice:
```

| Input | Action |
|-------|--------|
| `1` | Allocate Memory |
| `2` | Free Memory |
| `3` | View Memory Map |
| `4` | View Memory Statistics |
| `0` | Back |

### Allocate Memory
```
PID: 3
Size (KB): 256
Strategy (0=First 1=Best 2=Worst): 0
```

| Field | Example | Rules |
|-------|---------|-------|
| PID | `3` | Process ID integer |
| Size | `256` | Integer > 0 (KB) |
| Strategy | `0` | `0` (First Fit), `1` (Best Fit), `2` (Worst Fit) |

### Free Memory
```
PID to free: 3
```

---

## 4. IPC Mechanisms

### Menu
```
Choice:
```

| Input | Action |
|-------|--------|
| `1` | Anonymous Pipe Demo |
| `2` | Message Queue Demo |
| `3` | Shared Memory Demo |
| `4` | Run All Demos |
| `0` | Back |

Each option runs automatically — no additional input required.

---

## 5. Deadlock Management

### Menu
```
Choice:
```

| Input | Action |
|-------|--------|
| `1` | Run Safety Algorithm (Banker's) |
| `2` | Request Resources |
| `3` | Display Banker State |
| `4` | Add Wait-For Edge |
| `5` | Detect Cycle (RAG) |
| `6` | Display Wait-For Graph |
| `7` | Clear Wait-For Graph |
| `0` | Back |

### Request Resources (option 2)
```
Process ID (0-N): 0
Request (Ambulance Fire Police Rescue): 1 2 0 1
```

| Field | Example | Rules |
|-------|---------|-------|
| Process ID | `0` | Integer in the current Banker state range |
| Request | `1 2 0 1` | Four integers separated by spaces |

### Add Wait-For Edge (option 4)
```
From process: 1
To process  : 2
```

| Field | Example |
|-------|---------|
| From process | `1` |
| To process | `2` |

---

## 6. File Management

### Menu
```
Choice:
```

| Input | Action |
|-------|--------|
| `1` | Create File |
| `2` | List Files |
| `3` | Open File |
| `4` | Write to File |
| `5` | Read File |
| `6` | Display File Details |
| `7` | Close File |
| `8` | Delete File |
| `0` | Back |

### Create File
```
File name: incident1
Type (0=Incident 1=Dispatch 2=Report 3=Log): 0
```

| Field | Example | Rules |
|-------|---------|-------|
| File name | `incident1` | Single word, no spaces |
| Type | `0` | `0` (Incident), `1` (Dispatch), `2` (Report), `3` (Log) |

### Open / Write / Read / Display Details / Close / Delete File
All require the same input:
```
File ID: 3
```

| Field | Example | Rules |
|-------|---------|-------|
| File ID | `3` | File ID integer (shown when you list files) |

### Write to File (additional input)
After entering the File ID, you'll be prompted for data:
```
Data (no spaces): CAD_Report_2026
```

| Field | Example | Rules |
|-------|---------|-------|
| Data | `CAD_Report_2026` | Single token, no spaces |

---

## GUI Dialog Prompts

When running in GUI mode (option `2` in the launcher), the same prompts appear as dialog boxes. The labels are identical to the CLI prompts above:

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

Follow the same input rules as the CLI examples above.

---

## Notes

- All menu `Choice:` prompts expect a single digit.
- Invalid numeric choices usually show `Invalid option.` and return to the menu.
- CLI mode reads input with `scanf` — text fields accept a single word (no spaces).
- GUI mode shows the same labels in dialog boxes.