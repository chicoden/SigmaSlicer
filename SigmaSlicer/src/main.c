#include <stdio.h>
#include <string.h>
#include "printer.h"

int parseCommandLine(int argc, char* argv[], char** inputPath, char** outputPath) {
    for (int argi = 1; argi < argc; argi++) {
        if (strcmp(argv[argi], "--stl") == 0) {
            if (++argi >= argc) return 0;
            *inputPath = argv[argi];
        } else if (strcmp(argv[argi], "--gcode") == 0) {
            if (++argi >= argc) return 0;
            *outputPath = argv[argi];
        } else {
            return 0;
        }
    }

    return 1;
}

int main(int argc, char* argv[]) {
    char* inputPath = NULL;
    char* outputPath = "tests/output.gcode";
    if (!parseCommandLine(argc, argv, &inputPath, &outputPath)) {
        printf("Invalid command line arguments\n");
        return 0;
    }

    printf("Input: %s\n", inputPath ? inputPath : "default (test)");
    printf("Output: %s\n", outputPath);

    PrintTask task;
    task.units = MILLIMETERS;
    task.decimalPrecision = 2;
    task.fanSpeed = 0;
    task.bedTempCelsius = 60;
    task.extruderTempCelsius = 200;
    task.layerThickness = 0.2;
    task.extrusionThickness = 0.4;
    task.movementSpeed = 400.0;
    task.extrusionRate = 0.14;

    if (!beginPrintTask(outputPath, &task)) {
        printf("Failed to initialize\n");
        return 0;
    }

    printTestCylinder(2.0, 10.0, 10, &task);

    endPrintTask(&task);
    return 0;
}