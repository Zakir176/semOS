#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#define MAX_FILES      32
#define MAX_FILENAME   64
#define MAX_FILE_DATA  512

typedef enum {
    FILE_TYPE_INCIDENT,
    FILE_TYPE_DISPATCH,
    FILE_TYPE_REPORT,
    FILE_TYPE_LOG
} FileType;

typedef struct {
    int      id;
    char     name[MAX_FILENAME];
    FileType type;
    char     data[MAX_FILE_DATA];
    int      size;
    int      isOpen;
    int      ownerPID;
    long     createdAt;
    long     modifiedAt;
} VirtualFile;

void         initFileManager(void);
int          createFile(const char *name, FileType type, int ownerPID);
int          openFile(int fileId);
int          closeFile(int fileId);
int          writeFile(int fileId, const char *data);
int          readFile(int fileId, char *buffer, int bufSize);
int          deleteFile(int fileId);
VirtualFile *findFileByName(const char *name);
VirtualFile *findFileById(int fileId);
void         listFiles(void);
void         displayFile(int fileId);
const char  *fileTypeToString(FileType type);
void         fileMenu(void);

#endif
