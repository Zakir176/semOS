#include <stdio.h>

int main() {
    int choice;

    while (1) {
        printf("\n===== Smart Emergency Response System =====\n");
        printf("1. Create Emergency Task\n");
        printf("2. View Tasks\n");
        printf("3. Run Scheduler\n");
        printf("4. Allocate Memory\n");
        printf("5. View Logs\n");
        printf("6. Exit\n");

        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("Creating Emergency Task...\n");
                break;
            case 2:
                printf("Viewing Tasks...\n");
                break;
            case 3:
                printf("Running Scheduler...\n");
                break;
            case 4:
                printf("Allocating Memory...\n");
                break;
            case 5:
                printf("Viewing Logs...\n");
                break;
            case 6:
                printf("Exiting program...\n");
                return 0;
            default:
                printf("Invalid choice. Try again.\n");
        }
    }

    return 0;
}