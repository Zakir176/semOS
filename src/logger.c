#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "logger.h"

static FILE *logFile = NULL;

static void writeLog(const char *level, const char *module, const char *message) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);

    if (logFile) {
        fprintf(logFile, "[%s] [%s] [%s] %s\n", timestamp, level, module, message);
        fflush(logFile);
    }
}

void initLogger(void) {
    logFile = fopen(LOG_FILE, "a");
    if (!logFile) {
        fprintf(stderr, "Warning: Could not open log file. Logging disabled.\n");
    }
}

void logEvent(const char *module, const char *message) {
    writeLog("INFO", module, message);
}

void logError(const char *module, const char *message) {
    writeLog("ERROR", module, message);
    fprintf(stderr, "[ERROR] [%s] %s\n", module, message);
}

void closeLogger(void) {
    if (logFile) {
        fclose(logFile);
        logFile = NULL;
    }
}
