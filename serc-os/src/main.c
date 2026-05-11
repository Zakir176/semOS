#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logger.h"
#include "ui_cli.h"
#include "ui_gui.h"

static void printLauncher(void) {
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║          SMART EMERGENCY RESPONSE CENTER (SERC)          ║\n");
    printf("║                  Mini-OS Simulation v1.0                 ║\n");
    printf("║                 Copperbelt University                    ║\n");
    printf("║                       CS 225                             ║\n");
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║                                                          ║\n");
    printf("║   Select Interface:                                      ║\n");
    printf("║                                                          ║\n");
    printf("║     [1]  Command Line Interface  (CLI)                   ║\n");
    printf("║     [2]  Graphical Interface     (GUI)                   ║\n");
    printf("║     [0]  Exit                                            ║\n");
    printf("║                                                          ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("  Choice: ");
}

int main(int argc, char *argv[]) {
    initLogger();
    logEvent("SYSTEM", "SERC OS launched");

    int choice = -1;

    if (argc > 1) {
        if (strcmp(argv[1], "--cli") == 0) choice = 1;
        else if (strcmp(argv[1], "--gui") == 0) choice = 2;
    }

    if (choice == -1) {
        printLauncher();
        if (scanf("%d", &choice) != 1) choice = 0;
    }

    switch (choice) {
        case 1:
            logEvent("SYSTEM", "CLI selected");
            cli_run();
            break;
        case 2:
            logEvent("SYSTEM", "GUI selected");
            gui_run(argc, argv);
            break;
        case 0:
            printf("\n  Exiting. Goodbye.\n\n");
            break;
        default:
            printf("\n  Invalid selection. Exiting.\n\n");
            break;
    }

    closeLogger();
    return 0;
}
