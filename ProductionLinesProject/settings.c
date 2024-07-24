//settings.c
#include "settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Declare the global variables
int num_production_lines;
int num_medicines;
int liquid_failure_rate;
int pill_failure_rate;
int max_produced_medicines;
int max_out_of_spec_liquid_medicines;
int max_out_of_spec_pill_medicines;
int simulation_duration;


PillMedicine pillMedicines[MAX_MEDICINES];
int pillCount = 0;

LiquidMedicine liquidMedicines[MAX_MEDICINES];
int liquidCount = 0;


void settings_file(char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("fopen");
        exit(1);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "num_production_lines=%d", &num_production_lines);
        sscanf(line, "num_medicines=%d", &num_medicines);
        sscanf(line, "liquid_failure_rate=%d", &liquid_failure_rate);
        sscanf(line, "pill_failure_rate=%d", &pill_failure_rate);
        sscanf(line, "max_produced_medicines=%d", &max_produced_medicines);
        sscanf(line, "max_out_of_spec_liquid_medicines=%d", &max_out_of_spec_liquid_medicines);
        sscanf(line, "max_out_of_spec_pill_medicines=%d", &max_out_of_spec_pill_medicines);
        sscanf(line, "simulation_duration=%d", &simulation_duration);
    }
    fclose(file);
}

void load_medicine_config(const char* filename) {

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file)) {
        // Skip lines with '===='
        if (strstr(buffer, "====")) continue;

        char type[10];
        if (sscanf(buffer, "%[^,],", type) == 1) {
            if (strcmp(type, "liquid") == 0) {
                LiquidMedicine lm;
                if (sscanf(buffer, "%*[^,],%d,%49[^,],%d-%d,%19[^,],%d,%d,%d",
                           &lm.id, lm.name, &lm.lower_level, &lm.upper_level, lm.color,
                           &lm.seal_quality, &lm.label_correctness, &lm.expiry_validity) == 8) {
                    
                    printf("Loaded Liquid Medicine: %s\n", lm.name);
                    liquidMedicines[liquidCount++] = lm;
                }
            } else if (strcmp(type, "pill") == 0) {
                PillMedicine pm;
                if (sscanf(buffer, "%*[^,],%d,%49[^,],%d,%19[^,],%9[^,],%d",
                           &pm.id, pm.name, &pm.num_of_pills, pm.color, pm.size,
                       &pm.expiry_validity) == 6) {
                    
                    printf("Loaded Pill Medicine: %s\n", pm.name);
                    pillMedicines[pillCount++] = pm;
                }
            }
        }
    }

    fclose(file);
}

void print_medicines() {
    printf("Liquid Medicines:\n");
    for (int i = 0; i < liquidCount; i++) {
        LiquidMedicine lm = liquidMedicines[i];
        printf("ID: %d, Name: %s, Levels: %d-%d, Color: %s, Seal: %d, Label: %d, Expiry: %d\n",
               lm.id, lm.name, lm.lower_level, lm.upper_level, lm.color, lm.seal_quality, lm.label_correctness, lm.expiry_validity);
    }

    printf("Pill Medicines:\n");
    for (int i = 0; i < pillCount; i++) {
        PillMedicine pm = pillMedicines[i];
        printf("ID: %d, Name: %s, num_of_pills: %d, Color: %s, Size: %s, Expiry: %d\n",
               pm.id, pm.name, pm.num_of_pills, pm.color, pm.size, pm.expiry_validity);
    }
}


