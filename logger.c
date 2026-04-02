#include <stdio.h>
#include <time.h>
#include "logger.h"

void initLogger() {
    FILE *f = fopen("events.log", "a");
    if (f) {
        fprintf(f, "\n=== SERC System Started ===\n");
        fclose(f);
    }
}

void logEvent(const char* message) {
    FILE *f = fopen("events.log", "a");
    if (f) {
        time_t now;
        time(&now);
        char *date = ctime(&now);
        date[24] = '\0'; // Remove newline
        fprintf(f, "[%s] %s\n", date, message);
        fclose(f);
    }
}
