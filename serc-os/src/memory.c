#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "logger.h"

static MemoryPartition partitions[MAX_PARTITIONS];
static int             partitionCount = 0;
static int             totalMemoryKB  = 0;

void initMemory(int totalKB) {
    totalMemoryKB  = totalKB;
    partitionCount = 1;
    memset(partitions, 0, sizeof(partitions));

    partitions[0].id            = 0;
    partitions[0].startAddress  = 0;
    partitions[0].size          = totalKB;
    partitions[0].status        = PARTITION_FREE;
    partitions[0].allocatedPID  = -1;

    char msg[64];
    snprintf(msg, sizeof(msg), "Memory initialized: %d KB", totalKB);
    logEvent("MEMORY", msg);
}

static int splitPartition(int index, int sizeKB, int pid) {
    if (partitionCount >= MAX_PARTITIONS) return -1;

    int remainder = partitions[index].size - sizeKB;

    if (remainder > 0) {
        for (int i = partitionCount; i > index + 1; i--)
            partitions[i] = partitions[i - 1];

        partitions[index + 1].id           = partitionCount;
        partitions[index + 1].startAddress = partitions[index].startAddress + sizeKB;
        partitions[index + 1].size         = remainder;
        partitions[index + 1].status       = PARTITION_FREE;
        partitions[index + 1].allocatedPID = -1;
        partitionCount++;
    }

    partitions[index].size         = sizeKB;
    partitions[index].status       = PARTITION_OCCUPIED;
    partitions[index].allocatedPID = pid;
    return 0;
}

int allocateMemory(int pid, int sizeKB, FitStrategy strategy) {
    if (sizeKB <= 0 || sizeKB > totalMemoryKB) {
        logError("MEMORY", "Invalid allocation size");
        return -1;
    }

    int selected = -1;

    if (strategy == FIT_FIRST) {
        for (int i = 0; i < partitionCount; i++) {
            if (partitions[i].status == PARTITION_FREE && partitions[i].size >= sizeKB) {
                selected = i;
                break;
            }
        }
    } else if (strategy == FIT_BEST) {
        int minWaste = totalMemoryKB + 1;
        for (int i = 0; i < partitionCount; i++) {
            if (partitions[i].status == PARTITION_FREE && partitions[i].size >= sizeKB) {
                int waste = partitions[i].size - sizeKB;
                if (waste < minWaste) {
                    minWaste = waste;
                    selected = i;
                }
            }
        }
    } else if (strategy == FIT_WORST) {
        int maxSize = -1;
        for (int i = 0; i < partitionCount; i++) {
            if (partitions[i].status == PARTITION_FREE && partitions[i].size >= sizeKB) {
                if (partitions[i].size > maxSize) {
                    maxSize  = partitions[i].size;
                    selected = i;
                }
            }
        }
    }

    if (selected == -1) {
        logError("MEMORY", "Allocation failed: insufficient contiguous memory");
        return -1;
    }

    splitPartition(selected, sizeKB, pid);

    char msg[128];
    snprintf(msg, sizeof(msg), "Allocated %d KB to PID=%d using %s Fit at 0x%04X",
             sizeKB, pid, fitStrategyToString(strategy),
             partitions[selected].startAddress);
    logEvent("MEMORY", msg);
    return partitions[selected].startAddress;
}

int freeMemory(int pid) {
    int freed = 0;

    for (int i = 0; i < partitionCount; i++) {
        if (partitions[i].allocatedPID == pid && partitions[i].status == PARTITION_OCCUPIED) {
            partitions[i].status       = PARTITION_FREE;
            partitions[i].allocatedPID = -1;
            freed = 1;
        }
    }

    if (!freed) {
        logError("MEMORY", "Free failed: PID not found in memory");
        return -1;
    }

    int i = 0;
    while (i < partitionCount - 1) {
        if (partitions[i].status == PARTITION_FREE &&
            partitions[i + 1].status == PARTITION_FREE) {
            partitions[i].size += partitions[i + 1].size;
            for (int j = i + 1; j < partitionCount - 1; j++)
                partitions[j] = partitions[j + 1];
            partitionCount--;
        } else {
            i++;
        }
    }

    char msg[64];
    snprintf(msg, sizeof(msg), "Memory freed for PID=%d", pid);
    logEvent("MEMORY", msg);
    return 0;
}

MemoryStats getMemoryStats(void) {
    MemoryStats stats = {0};
    stats.totalMemory   = totalMemoryKB;
    stats.partitionCount = partitionCount;

    for (int i = 0; i < partitionCount; i++) {
        if (partitions[i].status == PARTITION_OCCUPIED)
            stats.usedMemory += partitions[i].size;
        else {
            stats.freeMemory += partitions[i].size;
            stats.fragmentCount++;
        }
    }
    return stats;
}

void displayMemoryMap(void) {
    printf("\n===== Memory Map =====\n");
    printf("%-6s %-12s %-10s %-8s %-6s\n",
           "Part", "Address", "Size(KB)", "Status", "PID");
    printf("----------------------------------------------\n");

    for (int i = 0; i < partitionCount; i++) {
        MemoryPartition *mp = &partitions[i];
        char pid_str[16];
        if (mp->status == PARTITION_FREE)
            snprintf(pid_str, sizeof(pid_str), "---");
        else
            snprintf(pid_str, sizeof(pid_str), "%d", mp->allocatedPID);

        printf("%-6d 0x%-10X %-10d %-8s %s\n",
               i, mp->startAddress, mp->size,
               mp->status == PARTITION_FREE ? "FREE" : "USED",
               pid_str);
    }

    printf("==============================================\n");
}

void displayMemoryStats(void) {
    MemoryStats s = getMemoryStats();
    printf("\n===== Memory Statistics =====\n");
    printf("  Total Memory    : %d KB\n", s.totalMemory);
    printf("  Used Memory     : %d KB\n", s.usedMemory);
    printf("  Free Memory     : %d KB\n", s.freeMemory);
    printf("  Partitions      : %d\n", s.partitionCount);
    printf("  Free Fragments  : %d\n", s.fragmentCount);
    printf("  Utilization     : %.1f%%\n",
           ((double)s.totalMemory > 0) ? (double)s.usedMemory / (double)s.totalMemory * 100.0 : 0.0);
    printf("=============================\n");
}

const char *fitStrategyToString(FitStrategy strategy) {
    switch (strategy) {
        case FIT_FIRST: return "First";
        case FIT_BEST:  return "Best";
        case FIT_WORST: return "Worst";
        default:        return "Unknown";
    }
}
