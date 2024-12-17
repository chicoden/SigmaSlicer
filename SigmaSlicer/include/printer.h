#pragma once

#include <stdio.h>

typedef struct {
    enum { INCHES, MILLIMETERS } units;
    int decimalPrecision;
    int fanSpeed;
    int bedTempCelsius;
    int extruderTempCelsius;
    double layerThickness;
    double extrusionThickness;
    double movementSpeed;
    double extrusionRate;
    double position[3];
    FILE* gcode;
} PrintTask;

int beginPrintTask(const char* outputPath, PrintTask* task);
void endPrintTask(PrintTask* task);
void movePrintHeadTo(double x, double y, double z, PrintTask* task);
void extrudeLineTo(double x, double y, PrintTask* task);
void printTestCylinder(double height, double radius, int sides, PrintTask* task);