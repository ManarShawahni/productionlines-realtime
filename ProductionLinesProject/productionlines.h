//productionlines.h
#ifndef PRODUCTIONLINES_H
#define PRODUCTIONLINES_H

#include <sys/types.h>
#include <pthread.h> // Ensure to include this for pthread_mutex_t
#include <stdbool.h>

#define MAX_MEDICINES 100
// Enum for medicine status
typedef enum {
    NOT_PROCESSED,
    IN_PRODUCTION,
    IN_INSPECTION,
    PACKAGE,
    INSPECTION_PASSED,
    INSPECTION_FAILED,
    DONE
} MedicineStatus;

struct line{
    pid_t pid;
    int line_id;
    int workload; 
    int produced_medicines;
    int packaged_medicines;
    int speed;  // Added missing semicolon here
    MedicineStatus medicine_status[400];  // Array to track status of medicines
    int med_indx;
    pthread_mutex_t lock;  // Mutex for synchronization
    int num_employees;           // Current number of employees on the line
    int num_transferred_employees;
};

struct shVar{
	int out_of_spec_bottled_medicines;
	int out_of_spec_pill_medicines;
	int liq_sem_running;
	int pill_sem_running;
};

typedef enum
{
    INSPECTION,
    PACKAGING
} Role;

typedef struct
{
    int id;
    int line_id;
    int origin_line;  // pid
    int current_line; // pid
    int current_line_id;
    Role role;       
    int active; 
    pthread_t thread;
    pthread_cond_t cond; // Each thread has its own condition variable
} Employee;

Employee employees[NUM_EMPLOYEES];

typedef struct {
    bool is_transferred; // Indicates if the employee has been transferred
    int original_line; // Original line ID from where the employee was transferred
    int to_where;  
} EmployeeTransferStatus;

EmployeeTransferStatus transferStatus[NUM_EMPLOYEES];

extern struct shVar *var;
extern struct line *everyline;
extern int msgqid;  // Declare external if defined elsewhere
extern int lines_memo_key; // Key for shared memory

void print_shared_memory_data();

#endif // PRODUCTIONLINES_H
