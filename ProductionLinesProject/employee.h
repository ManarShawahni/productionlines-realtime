#ifndef EMPLOYEE_H
#define EMPLOYEE_H
#include <pthread.h>

#define NUM_EMPLOYEES_PER_LINE 7  // Define the number of employees per production line
#define NUM_EMPLOYEES 15

typedef struct {
    float liquid_level;
    char color[20];
    int is_sealed;
    char label[50];
    char expiry_date[20];
} Bottle;

typedef struct {
    int pill_count;
    char color[20];
    char size[20];
    char expiry_date[20];
} PillContainer;

void initialize_employees();
void employee_inspect_liquid(Bottle *bottle);
void employee_inspect_pill(PillContainer *pill_container);
void employee_package_liquid(Bottle *bottle);
void employee_package_pill(PillContainer *pill_container);

void *employee_thread_liquid(void *arg);
void *employee_thread_pill(void *arg);

#endif


