#include <stdio.h>
#include "ui.h"

void printMenu() {
    printf("\n===== Smart Emergency Response System =====\n");
    printf("1. Create Emergency Task\n");
    printf("2. View Tasks\n");
    printf("3. Run Scheduler\n");
    printf("4. Allocate Memory\n");
    printf("5. View Logs\n");
    printf("6. Exit\n");
}

int getSafeInt(const char* prompt) {
    int value;
    printf("%s", prompt);
    if (scanf("%d", &value) != 1) {
        // Clear input buffer on failure
        while(getchar() != '\n'); 
        return -1;
    }
    return value;
}
