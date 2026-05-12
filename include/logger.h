#ifndef LOGGER_H
#define LOGGER_H

#define LOG_FILE "logs/serc.log"

void logEvent(const char *module, const char *message);
void logError(const char *module, const char *message);
void initLogger(void);
void closeLogger(void);

#endif
