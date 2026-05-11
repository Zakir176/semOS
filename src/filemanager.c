#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "filemanager.h"
#include "logger.h"

static VirtualFile fileTable[MAX_FILES];
static int         fileCount  = 0;
static int         nextFileId = 1;

void initFileManager(void) {
    memset(fileTable, 0, sizeof(fileTable));
    fileCount  = 0;
    nextFileId = 1;
    logEvent("FILEMGR", "File manager initialized");
}

int createFile(const char *name, FileType type, int ownerPID) {
    if (fileCount >= MAX_FILES) {
        logError("FILEMGR", "File table full");
        return -1;
    }
    if (findFileByName(name)) {
        logError("FILEMGR", "File already exists");
        return -1;
    }

    VirtualFile *f = &fileTable[fileCount++];
    f->id         = nextFileId++;
    strncpy(f->name, name, MAX_FILENAME - 1);
    f->name[MAX_FILENAME - 1] = '\0';
    f->type       = type;
    f->size       = 0;
    f->isOpen     = 0;
    f->ownerPID   = ownerPID;
    f->createdAt  = time(NULL);
    f->modifiedAt = f->createdAt;
    memset(f->data, 0, sizeof(f->data));

    char msg[128];
    snprintf(msg, sizeof(msg), "Created file '%s' ID=%d Type=%s",
             f->name, f->id, fileTypeToString(f->type));
    logEvent("FILEMGR", msg);
    return f->id;
}

int openFile(int fileId) {
    VirtualFile *f = findFileById(fileId);
    if (!f) {
        logError("FILEMGR", "Open failed: file not found");
        return -1;
    }
    f->isOpen = 1;
    char msg[128];
    snprintf(msg, sizeof(msg), "Opened file ID=%d '%s'", fileId, f->name);
    logEvent("FILEMGR", msg);
    return 0;
}

int closeFile(int fileId) {
    VirtualFile *f = findFileById(fileId);
    if (!f) {
        logError("FILEMGR", "Close failed: file not found");
        return -1;
    }
    f->isOpen = 0;
    char msg[128];
    snprintf(msg, sizeof(msg), "Closed file ID=%d '%s'", fileId, f->name);
    logEvent("FILEMGR", msg);
    return 0;
}

int writeFile(int fileId, const char *data) {
    VirtualFile *f = findFileById(fileId);
    if (!f) {
        logError("FILEMGR", "Write failed: file not found");
        return -1;
    }
    if (!f->isOpen) {
        logError("FILEMGR", "Write failed: file not open");
        return -1;
    }

    int len = strlen(data);
    if (len >= MAX_FILE_DATA) {
        logError("FILEMGR", "Write failed: data exceeds file capacity");
        return -1;
    }

    strncpy(f->data, data, MAX_FILE_DATA - 1);
    f->data[MAX_FILE_DATA - 1] = '\0';
    f->size       = len;
    f->modifiedAt = time(NULL);

    char msg[64];
    snprintf(msg, sizeof(msg), "Wrote %d bytes to file ID=%d", len, fileId);
    logEvent("FILEMGR", msg);
    return len;
}

int readFile(int fileId, char *buffer, int bufSize) {
    VirtualFile *f = findFileById(fileId);
    if (!f) {
        logError("FILEMGR", "Read failed: file not found");
        return -1;
    }
    if (!f->isOpen) {
        logError("FILEMGR", "Read failed: file not open");
        return -1;
    }

    int n = f->size < bufSize - 1 ? f->size : bufSize - 1;
    strncpy(buffer, f->data, n);
    buffer[n] = '\0';

    char msg[64];
    snprintf(msg, sizeof(msg), "Read %d bytes from file ID=%d", n, fileId);
    logEvent("FILEMGR", msg);
    return n;
}

int deleteFile(int fileId) {
    for (int i = 0; i < fileCount; i++) {
        if (fileTable[i].id == fileId) {
            char msg[128];
            snprintf(msg, sizeof(msg), "Deleted file ID=%d '%s'",
                     fileId, fileTable[i].name);
            for (int j = i; j < fileCount - 1; j++)
                fileTable[j] = fileTable[j + 1];
            fileCount--;
            logEvent("FILEMGR", msg);
            return 0;
        }
    }
    logError("FILEMGR", "Delete failed: file not found");
    return -1;
}

VirtualFile *findFileByName(const char *name) {
    for (int i = 0; i < fileCount; i++) {
        if (strncmp(fileTable[i].name, name, MAX_FILENAME) == 0)
            return &fileTable[i];
    }
    return NULL;
}

VirtualFile *findFileById(int fileId) {
    for (int i = 0; i < fileCount; i++) {
        if (fileTable[i].id == fileId)
            return &fileTable[i];
    }
    return NULL;
}

void listFiles(void) {
    printf("\n===== File System =====\n");
    printf("%-5s %-24s %-12s %-6s %-6s\n",
           "ID", "Name", "Type", "Size", "Open");
    printf("----------------------------------------------\n");

    if (fileCount == 0) {
        printf("  No files in system.\n");
    } else {
        for (int i = 0; i < fileCount; i++) {
            VirtualFile *f = &fileTable[i];
            printf("%-5d %-24s %-12s %-6d %-6s\n",
                   f->id, f->name, fileTypeToString(f->type),
                   f->size, f->isOpen ? "Yes" : "No");
        }
    }
    printf("==============================================\n");
}

void displayFile(int fileId) {
    VirtualFile *f = findFileById(fileId);
    if (!f) {
        printf("  File ID=%d not found.\n", fileId);
        return;
    }

    char timeBuf[32];
    struct tm *tm_info;
    tm_info = localtime(&f->createdAt);
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", tm_info);

    printf("\n===== File Details =====\n");
    printf("  ID         : %d\n", f->id);
    printf("  Name       : %s\n", f->name);
    printf("  Type       : %s\n", fileTypeToString(f->type));
    printf("  Size       : %d bytes\n", f->size);
    printf("  Owner PID  : %d\n", f->ownerPID);
    printf("  Created    : %s\n", timeBuf);
    printf("  Is Open    : %s\n", f->isOpen ? "Yes" : "No");
    printf("  Contents   :\n");
    printf("  %s\n", f->size > 0 ? f->data : "(empty)");
    printf("========================\n");
}

const char *fileTypeToString(FileType type) {
    switch (type) {
        case FILE_TYPE_INCIDENT: return "INCIDENT";
        case FILE_TYPE_DISPATCH: return "DISPATCH";
        case FILE_TYPE_REPORT:   return "REPORT";
        case FILE_TYPE_LOG:      return "LOG";
        default:                 return "UNKNOWN";
    }
}

void fileMenu(void) {
    int choice;
    do {
        printf("\n===== File Management =====\n");
        printf("  1. Create File\n");
        printf("  2. List Files\n");
        printf("  3. Open File\n");
        printf("  4. Write to File\n");
        printf("  5. Read File\n");
        printf("  6. Display File Details\n");
        printf("  7. Close File\n");
        printf("  8. Delete File\n");
        printf("  0. Back\n");
        printf("  Choice: ");
        if (scanf("%d", &choice) != 1) { choice = -1; while(getchar() != '\n'); }

        switch (choice) {
            case 1: {
                char name[MAX_FILENAME];
                int  type;
                printf("  File name: ");
                scanf("%63s", name);
                printf("  Type (0=Incident 1=Dispatch 2=Report 3=Log): ");
                scanf("%d", &type);
                if (type < 0 || type > 3) { printf("  Invalid type.\n"); break; }
                int id = createFile(name, (FileType)type, 0);
                if (id > 0) printf("  File created with ID=%d\n", id);
                break;
            }
            case 2:
                listFiles();
                break;
            case 3: {
                int id;
                printf("  File ID: ");
                scanf("%d", &id);
                if (openFile(id) == 0) printf("  File opened.\n");
                break;
            }
            case 4: {
                int  id;
                char data[MAX_FILE_DATA];
                printf("  File ID: ");
                scanf("%d", &id);
                printf("  Data (no spaces): ");
                scanf("%511s", data);
                int n = writeFile(id, data);
                if (n >= 0) printf("  Wrote %d bytes.\n", n);
                break;
            }
            case 5: {
                int  id;
                char buf[MAX_FILE_DATA];
                printf("  File ID: ");
                scanf("%d", &id);
                int n = readFile(id, buf, sizeof(buf));
                if (n >= 0) printf("  Content: %s\n", buf);
                break;
            }
            case 6: {
                int id;
                printf("  File ID: ");
                scanf("%d", &id);
                displayFile(id);
                break;
            }
            case 7: {
                int id;
                printf("  File ID: ");
                scanf("%d", &id);
                if (closeFile(id) == 0) printf("  File closed.\n");
                break;
            }
            case 8: {
                int id;
                printf("  File ID: ");
                scanf("%d", &id);
                if (deleteFile(id) == 0) printf("  File deleted.\n");
                break;
            }
            case 0: break;
            default: printf("  Invalid option.\n");
        }
    } while (choice != 0);
}
