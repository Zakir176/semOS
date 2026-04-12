#ifndef IPC_H
#define IPC_H

#define SHM_SIZE  256
#define MSG_TYPE  1L
#define PROJ_ID   42

void ipcMenu(void);
void demoPipe(void);
void demoMessageQueue(void);
void demoSharedMemory(void);

#endif