#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

const double M_PI = 3.141592653589793;

typedef struct {
    double normal[3];
    double vertices[3][3];
} Facet;

typedef struct {
    Facet *facets;
    int numFacets;
} Mesh;

typedef struct {
    enum { INCHES, MILLIMETERS } units;
    int fanSpeed;
    int bedTempCelsius;
    int extruderTempCelsius;
    double layerThickness;
    double movementSpeed;
    double extrusionRate;
    int decimalPrecision;
    double position[3];
    FILE *gcode;
} PrintTask;

int loadSTL(char *path, Mesh *mesh) {
    FILE *stl = fopen(path, "rb");
    if (stl == NULL) return 0;

    uint8_t header[80];
    fread(header, sizeof(uint8_t), 80, stl);

    uint32_t numFacets;
    fread(&numFacets, sizeof(uint32_t), 1, stl);
    mesh->numFacets = numFacets;
    mesh->facets = malloc(mesh->numFacets * sizeof(Facet));

    for (int fi = 0; fi < mesh->numFacets; fi++) {
        Facet *facet = &mesh->facets[fi];
        float vector[3];

        fread(vector, sizeof(float), 3, stl);
        facet->normal[0] = vector[0];
        facet->normal[1] = vector[1];
        facet->normal[2] = vector[2];

        for (int vi = 0; vi < 3; vi++) {
            fread(vector, sizeof(float), 3, stl);
            facet->vertices[vi][0] = vector[0];
            facet->vertices[vi][1] = vector[1];
            facet->vertices[vi][2] = vector[2];
        }

        uint16_t attribByteCount;
        fread(&attribByteCount, sizeof(uint16_t), 1, stl);
    }

    fclose(stl);
    return 1;
}

void destroyMesh(Mesh *mesh) {
    free(mesh->facets);
}

int beginPrintTask(char *outputPath, PrintTask *task) {
    task->gcode = fopen(outputPath, "w");
    if (task->gcode == NULL) {
        printf("Failed to open GCODE file :(\n");
        return 0;
    }

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

void endPrintTask(PrintTask *task) {
    fclose(task->gcode);
}

void movePrintHeadTo(double x, double y, double z, PrintTask *task) {
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

void extrudeLineTo(double x, double y, PrintTask *task) {
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

int main(int argc, char *argv[]) {
    // Define a new print task
    PrintTask task;
    task.units = MILLIMETERS;
    task.fanSpeed = 0;
    task.bedTempCelsius = 60;
    task.extruderTempCelsius = 200;
    task.layerThickness = 0.2;
    task.movementSpeed = 400.0;
    task.extrusionRate = 0.14;
    task.decimalPrecision = 2;

    // Parse command line arguments
    char *modelPath = NULL;
    char *outputPath = "output.gcode";
    for (int argi = 0; argi < argc; argi++) {
        if (strcmp(argv[argi], "--stl") == 0) {
            if (++argi >= argc) {
                printf("Invalid arguments\n");
                return 0;
            } modelPath = argv[argi];
        } else if (strcmp(argv[argi], "--gcode") == 0) {
            if (++argi >= argc) {
                printf("Invalid arguments\n");
                return 0;
            } outputPath = argv[argi];
        }
    }

    // Start printing
    if (!beginPrintTask(outputPath, &task)) {
        printf("Failed to start print task :(\n");
        return 0;
    }

    if (modelPath == NULL) {
        // Print a 10 layer tall cyclinder
        double radius = 10.0;
        double deltaAngle = 2.0 * M_PI / 10.0;
        for (int layer = 0; layer < 10; layer++) {
            movePrintHeadTo(radius, 0.0, layer * task.layerThickness, &task);
            for (double angle = deltaAngle; angle < 2.0 * M_PI + deltaAngle; angle += deltaAngle) {
                extrudeLineTo(radius * cos(angle), radius * sin(angle), &task);
            }
        }

        endPrintTask(&task);
        return 0;
    }

    // Load mesh file
    Mesh mesh;
    if (!loadSTL(modelPath, &mesh)) {
        printf("Failed to load STL model :(\n");
        endPrintTask(&task);
        return 0;
    }

    // Finish printing
    destroyMesh(&mesh);
    endPrintTask(&task);
    return 0;
}
