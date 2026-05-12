#ifndef MEMORY_H
#define MEMORY_H

#define MAX_MEMORY_KB   1024
#define MAX_PARTITIONS  32

typedef enum {
    PARTITION_FREE,
    PARTITION_OCCUPIED
} PartitionStatus;

typedef enum {
    FIT_FIRST,
    FIT_BEST,
    FIT_WORST
} FitStrategy;

typedef struct {
    int             id;
    int             startAddress;
    int             size;
    PartitionStatus status;
    int             allocatedPID;
} MemoryPartition;

typedef struct {
    int totalMemory;
    int usedMemory;
    int freeMemory;
    int partitionCount;
    int fragmentCount;
} MemoryStats;

void             initMemory(int totalKB);
int              allocateMemory(int pid, int sizeKB, FitStrategy strategy);
int              freeMemory(int pid);
MemoryStats      getMemoryStats(void);
void             displayMemoryMap(void);
void             displayMemoryStats(void);
const char      *fitStrategyToString(FitStrategy strategy);

#endif
