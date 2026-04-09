/*  FILE: serc_types.h */

/* This #ifndef block prevents the file from being included twicee.
   Think of it as a safety lock */
#ifndef SERC_TYPES_H
#define SERC_TYPES_H

#include <stdio.h>   
#include <stdlib.h>  
#include <string.h>  

/* SECTION 1: GLOBAL CONSTANTS */

#define MAX_PROCESSES     20      /* Maximam number of processes/tasks at one time */
#define MAX_MEMORY        1024    /* Total memory available in MB                  */
#define MAX_NAME_LEN      50      /* Maximum length of a process name              */
#define MAX_RESOURCES     10      /* Maximum number of resource types              */
#define TIME_QUANTUM      3       /* Time slice for Round Robin scheduling (units) */
#define MAX_MEMORY_BLOCKS 50      /* Maximum number of memory blocks allowed       */
#define LOG_FILE_NAME     "serc_log.txt"  /* Name of the system log file           */


/* SECTION 2: PROCESS STATES */
typedef enum {
    NEW        = 0,   /* New Process  */
    READY      = 1,   /* Process is waiting for the CPU             */
    RUNNING    = 2,   /* Process is currently using the CPU         */
    WAITING    = 3,   /* Process is waiting for something (e.g. memory) */
    TERMINATED = 4    /* Process has finished and is done           */
} ProcessState;


/* EMERGENCY PRIORITY LEVELS */
typedef enum {
    LOW      = 1,   /* Low priority  */
    MEDIUM   = 2,   /* Medium priority  */
    HIGH     = 3,   /* High priority    */
    CRITICAL = 4    /* Critical      */
} PriorityLevel;


/*PCB — Process Control Block*/
typedef struct {

    /* --- Identity --- */
    int   pid;                    /* Unique Process ID (assigned automatically) */
    char  name[MAX_NAME_LEN];     /* Human-readable name e.g. "Ambulance_01"    */

    /* --- Scheduling Info (used by Member 2) --- */
    ProcessState state;           /* Current state: NEW, READY, RUNNING, etc.   */
    PriorityLevel priority;       /* Emergency priority level                    */
    int   burst_time;             /* Total CPU time this process needs (units)   */
    int   remaining_time;         /* CPU time still left to run                  */
    int   arrival_time;           /* When the process arrived (virtual clock)    */
    int   waiting_time;           /* How long it spent waiting for the CPU       */
    int   turnaround_time;        /* Total time from arrival to completion       */
    int   start_time;             /* When it first got the CPU                   */
    int   finish_time;            /* When it fully completed                     */

    /*  Memory Info  */
    int   memory_required;        /* How much memory (MB) this process needs     */
    int   memory_allocated;       /* How much memory was actually given to it    */
    int   mem_block_id;           /* Which memory block it is stored in (-1=none)*/

    /* --- IPC Info --- */
    int   shared_mem_id;          /* Shared memory segment ID (-1 if not used)   */
    int   pipe_fd[2];             /* Pipe file descriptors [0]=read, [1]=write   */
    int   msg_queue_id;           /* Message queue ID (-1 if not used)           */

    /* --- ReSource Info--- */
    int   resources_allocated[MAX_RESOURCES]; /* Resources currently held        */
    int   resources_needed[MAX_RESOURCES];    /* Resources still needed          */

} PCB;


/* MemBlock — Memory Block */
typedef struct {

    int   block_id;               /* Unique ID for this memory block             */
    int   start_address;          /* Where this block starts in memory (MB)      */
    int   size;                   /* Total size of this block (MB)               */
    int   is_free;                /* 1 = free/available,  0 = occupied           */
    int   process_id;             /* Which process is using it (-1 if free)      */

} MemBlock;


/* MEMORY TABLE */
typedef struct {

    MemBlock blocks[MAX_MEMORY_BLOCKS]; /* Array of all memory blocks            */
    int      total_blocks;              /* How many blocks currently exist        */
    int      total_memory;             /* Total memory size (always MAX_MEMORY)  */
    int      used_memory;              /* How much memory is currently occupied  */
    int      free_memory;              /* How much memory is currently free      */

} MemoryTable;


/*PROCESS TABLE*/
typedef struct {

    PCB  processes[MAX_PROCESSES];   /* Array of all process control blocks  */
    int  count;                      /* How many processes are currently stored */
    int  next_pid;                   /* The PID to assign to the next new process */

} ProcessTable;


/* HELPER FUNCTION — Convert enum to readable text */

/* Returns the name of a process state as a string */
static inline const char* state_to_string(ProcessState s) {
    switch (s) {
        case NEW:        return "NEW";
        case READY:      return "READY";
        case RUNNING:    return "RUNNING";
        case WAITING:    return "WAITING";
        case TERMINATED: return "TERMINATED";
        default:         return "UNKNOWN";
    }
}

/* Returns the name of a priority level as a string */
static inline const char* priority_to_string(PriorityLevel p) {
    switch (p) {
        case LOW:      return "LOW";
        case MEDIUM:   return "MEDIUM";
        case HIGH:     return "HIGH";
        case CRITICAL: return "CRITICAL";
        default:       return "UNKNOWN";
    }
}

#endif /* SERC_TYPES_H */
