#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <math.h>
#include "printer.h"

const double PI = 3.141592653589793;

int beginPrintTask(const char* outputPath, PrintTask* task) {
    if (!(task->gcode = fopen(outputPath, "w"))) return 0;

    // Select units
    if (task->units == INCHES) fprintf(task->gcode, "G20\n");
    else if (task->units == MILLIMETERS) fprintf(task->gcode, "G21\n");

    fprintf(task->gcode, "G17\n"); // Set XY plane to working surface
    fprintf(task->gcode, "G90\n"); // Use absolute positioning

    // Set fan speed
    if (task->fanSpeed > 0) {
        fprintf(task->gcode, "M106 S%i\n", task->fanSpeed);
    } else {
        // Turn fan completely off if fanSpeed <= 0
        fprintf(task->gcode, "M107\n");
    }

    fprintf(task->gcode, "M109 S%i\n", task->extruderTempCelsius); // Set extruder temperature
    fprintf(task->gcode, "M190 S%i\n", task->bedTempCelsius); // Set heating bed temperature

    return 1;
}

void endPrintTask(PrintTask* task) {
    fclose(task->gcode);
}

void movePrintHeadTo(double x, double y, double z, PrintTask* task) {
    // Update position
    task->position[0] = x;
    task->position[1] = y;
    task->position[2] = z;

    // Print gcode command
    fprintf(
        task->gcode,
        "G00 X%.*f Y%.*f Z%.*f\n",
        task->decimalPrecision, x,
        task->decimalPrecision, y,
        task->decimalPrecision, z
    );
}

void extrudeLineTo(double x, double y, PrintTask* task) {
    // Calculate length to be extruded
    double deltaX = x - task->position[0];
    double deltaY = y - task->position[1];
    double moveDistance = sqrt(deltaX * deltaX + deltaY * deltaY);

    // Update position
    task->position[0] = x;
    task->position[1] = y;

    // Print gcode command
    fprintf(
        task->gcode,
        "G01 X%.*f Y%.*f E%.*f F%.*f\n",
        task->decimalPrecision, x,
        task->decimalPrecision, y,
        task->decimalPrecision, task->extrusionRate * moveDistance,
        task->decimalPrecision, task->movementSpeed
    );
}

void printTestCylinder(double height, double radius, int sides, PrintTask* task) {
    double stepAngle = 2.0 * PI / sides;
    double cosStepAngle = cos(stepAngle);
    double sinStepAngle = sin(stepAngle);
    for (double z = 0.0; z < height; z += task->layerThickness) {
        double x = radius, y = 0.0;
        movePrintHeadTo(x, y, z, task);
        for (int step = 0; step < sides; step++) {
            double ox = x, oy = y;
            x = ox * cosStepAngle - oy * sinStepAngle;
            y = oy * cosStepAngle + ox * sinStepAngle;
            extrudeLineTo(x, y, task);
        }
    }
}