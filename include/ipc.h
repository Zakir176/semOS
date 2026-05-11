#ifndef IPC_H
#define IPC_H

#include <sys/types.h>

#define SHM_SIZE         1024
#define MSG_MAX_TEXT     256
#define IPC_KEY_BASE     0x5ERC

typedef enum {
    UNIT_AMBULANCE = 1,
    UNIT_FIRE      = 2,
    UNIT_POLICE    = 3,
    UNIT_RESCUE    = 4
} UnitType;

typedef struct {
    long    mtype;
    UnitType unitType;
    int     unitCount;
    char    text[MSG_MAX_TEXT];
} SERCMessage;

typedef struct {
    int     activeIncidents;
    int     dispatchedUnits;
    char    lastIncident[MSG_MAX_TEXT];
} SharedStatus;

void ipcMenu(void);

void runPipeDemo(void);
void runMessageQueueDemo(void);
void runSharedMemoryDemo(void);

#endif
