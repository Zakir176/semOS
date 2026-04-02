#include <stdio.h>
#include "logger.h"
#include "ui.h"
#include "process.h"
#include "scheduler.h"

int main() {
    int choice;
    
    initLogger();

    while (1) {
        printMenu();
        
        choice = getSafeInt("Enter your choice: ");

        switch (choice) {
            case 1:
                createTask();
                break;
            case 2:
                viewTasks();
                break;
            case 3:
                runFCFS();
                break;
            case 4:
                printf("Allocating Memory...\n");
                break;
            case 5:
                printf("Viewing Logs...\n");
                break;
            case 6:
                printf("Exiting program...\n");
                logEvent("System Shutdown");
                return 0;
            default:
                printf("Invalid choice. Try again.\n");
        }
    }

    return 0;
}