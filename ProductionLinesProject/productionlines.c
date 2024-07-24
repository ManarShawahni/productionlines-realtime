// productionlines.c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <limits.h>
#include "settings.h"
#include "productionlines.h"
#include <errno.h>
#include <stdio.h>

#define LIQUID 0
#define PILL 1
#define NUM_EMPLOYEES_PER_LINE 7 // Define the number of employees per production line
#define NUM_EMPLOYEES 15
#define MAX_PRODUCTION_LINES 50
#define WORKLOAD_CHECK_INTERVAL 2
#define WORKLOAD_THRESHOLD 6

extern pthread_mutex_t lock;
extern int available_employees;
extern int total_produced_medicines;
extern int num_production_lines;
extern int max_produced_medicines;
int lines_memo_key = 2181; 
int var_memo_key = 3512;// Define the shared memory key
int my_line;
int emp_idx;

/*-------------------- function prototype ---------------------------*/
int check_simulation_end();
void *employee_thread_function(void *arg);
extern void *employee_thread_liquid(void *arg);
extern void *employee_thread_pill(void *arg);
void initialize_employees(int line_id, int line_pid);
void setup_shared_memory();
int find_least_busy_line(int i);
void print_employees(int line_id);
void print_shared_memory_data();
void initialize_employee_transfer_status();

/*----------------------------------------------------------------*/


/*------------------------ msg queue ------------------------------*/
extern int msgqid;
/*----------------------------------------------------------------*/


typedef struct {
    int write_idx; // the line ID that writes to the shared memory
    int from; // the line ID that sends an employee
    int to_line; // the line ID that should receive the employee
    Employee employee; // the employee being transferred
    int read; // flag to indicate if the entry has been processed
} LineSharedMemory;

typedef struct {
    int from_line; // the line ID with least workload
    int to_line; // the line ID with most workload
    int read; // flag to indicate if the entry has been processed
} MonitorSharedMemory;

int lineSharedMemID;
int lineSharedMemKey = 5353;
LineSharedMemory *lineSharedMem;
int monitorSharedMemID;
int monitorSharedMemKey = 6735;
MonitorSharedMemory *monitorSharedMem;

void initialize_new_employees(int i , Employee new_employee, int new_line_id , int new_line_pid);

// Structures for medicine messages
struct liquid_medicine_message
{
    long type; // Message type
    LiquidMedicine medicine;
};

struct pill_medicine_message
{
    long type; // Message type
    PillMedicine medicine;
};

pthread_t threads[NUM_EMPLOYEES_PER_LINE];
struct line *everyline;    // Shared memory for lines
struct shVar *var;

void initialize_employee_transfer_status() {
    for (int i = 0; i < NUM_EMPLOYEES; i++) {
        transferStatus[i].is_transferred = false;
        transferStatus[i].original_line = -1; // -1 indicates not transferred
    }
}


int finddidx(int pid){
	for (int i = 0; i < NUM_EMPLOYEES_PER_LINE; i++){		
		if(employees[i].origin_line == pid){
			return i;
		}		
	}
	return -1;
}

int find_high_workload_line() {
    int max_workload = -1;
    int line_id_with_max_workload = -1;

    for (int i = 0; i < num_production_lines; i++) {
        int current_workload = everyline[i].produced_medicines - everyline[i].packaged_medicines;
        if (current_workload > max_workload) {
            max_workload = current_workload;
            line_id_with_max_workload = i;
        }
    }

    return line_id_with_max_workload;
}


void signal_handler(int sig) {
    if (sig == SIGUSR1) {
        printf("-------------------------------------------\n");
        printf("SIGUSR1 received at line %d\n", my_line);
         // Check shared memory for a reassignment task
        if (!monitorSharedMem->read) {
            printf("Received SIGUSR1 at line %d\n", my_line);
            monitorSharedMem->read = 1; // Mark as read
            int most_busy_line = monitorSharedMem->to_line;
            printf("Determined most busy line is %d from line %d\n", most_busy_line, my_line);

        for (int i = 0; i < NUM_EMPLOYEES; i++) {

            if (employees[i].active == 1) {
                printf("Inactivating employee %d for reassignment\n", employees[i].id);
                employees[i].active = 0; // Mark as inactive
                employees[i].current_line_id = most_busy_line; 
                employees[i].current_line = everyline[most_busy_line].pid; 
                printf("ohayo2\n");
                
                // Write to LineSharedMemory for the destination line
                    lineSharedMem[most_busy_line].employee = employees[i];
                    lineSharedMem[most_busy_line].from = my_line;
                    lineSharedMem[most_busy_line].to_line = most_busy_line;
                    lineSharedMem[most_busy_line].write_idx = i;
                    lineSharedMem[most_busy_line].read = 0; // Not yet processed
                    printf("Employee %d reassigned from line %d to %d\n", employees[i].id, my_line, most_busy_line);
                
                 // Decrement employee count at the source (current line)
                    pthread_mutex_lock(&lock);
                    everyline[my_line].num_employees--;
                    pthread_mutex_unlock(&lock);
                kill(everyline[most_busy_line].pid, SIGUSR2); // Notify the busiest line to receive the employee
                break;
             }
            }
        }
    } else if (sig == SIGUSR2) {
            printf("------------------------------------------\n");
            printf("SIGUSR2 received at line %d\n", my_line);
            // Check LineSharedMemory for incoming employee
        for (int j = 0; j < MAX_PRODUCTION_LINES; j++) {
            if (!lineSharedMem[j].read && lineSharedMem[j].to_line == my_line) {
                printf("Processing received employee data for line %d\n", my_line);
                lineSharedMem[j].read = 1; // Mark as read
                Employee transferred_employee = lineSharedMem[j].employee;
                printf("Employee %d starting on new line %d\n", transferred_employee.id, my_line);
                initialize_new_employees(emp_idx,transferred_employee, my_line, everyline[my_line].pid);
           
           if (my_line % 2 == 0)
           {
                    pthread_create(&threads[emp_idx], NULL, employee_thread_liquid, &employees[emp_idx]);
           }
           else
           {
                    pthread_create(&threads[emp_idx], NULL, employee_thread_pill, &employees[emp_idx]);
           }
                // Increment the transferred employee count at the destination
                pthread_mutex_lock(&lock);
                everyline[my_line].num_transferred_employees++;
                pthread_mutex_unlock(&lock);
                    
                employees[emp_idx].active = 1;
                transferStatus[transferred_employee.id].is_transferred = true;
                transferStatus[transferred_employee.id].original_line = lineSharedMem[j].from;
                transferStatus[transferred_employee.id].to_where = my_line;
                printf("Employee %d reassigned to line %d\n", transferred_employee.id, my_line);
                emp_idx++;
                break;
            }
        }
    }
}

int calculate_workload(int line_id) {
    return (everyline[line_id].produced_medicines - everyline[line_id].packaged_medicines) / (everyline[line_id].num_employees + everyline[line_id].num_transferred_employees);
}


void *workload_monitor(void *arg) {
    
    while ( var->liq_sem_running && var->pill_sem_running) {

        sleep(WORKLOAD_CHECK_INTERVAL);
        pthread_mutex_lock(&lock);
      	printf("----------------------------------------------------------------\n");
      	printf("DEBUG: Workload monitor cycle started\n");
        for (int i = 0; i < num_production_lines; i++) {
        	if(!var->liq_sem_running || !var->pill_sem_running){
        	    pthread_mutex_unlock(&lock);
        	    exit(0);
        	}
            int workload = calculate_workload(i);
            printf("DEBUG: Line %d has workload %d\n", i, workload);
            everyline[i].workload = workload;
            
            printf("line %d  workload %d\n", i, everyline[i].workload);
            fflush(stdout);
            if (workload > WORKLOAD_THRESHOLD) {

            	int least_busy_line = find_least_busy_line(i);
            	int most_busy_line = i;
            	printf("DEBUG: Workload imbalance detected, moving employee from line %d to %d\n", least_busy_line, most_busy_line);

            	// Update MonitorSharedMemory
                monitorSharedMem->from_line = least_busy_line;
                monitorSharedMem->to_line = most_busy_line;
                monitorSharedMem->read = 0; // Not yet processed
                kill(everyline[least_busy_line].pid, SIGUSR1); // Signal the line with least workload
           	 sleep(3);
            }
        }
        pthread_mutex_unlock(&lock);
    }
    pthread_exit(NULL);
    exit(0);
}

int find_least_busy_line(int exclude_line_id) {
    int min_workload = INT_MAX;
    int line_id = -1;
    for (int i = 0; i < num_production_lines; i++) {
        if (i == exclude_line_id) continue; // Skip the excluded line
        int current_workload = everyline[i].produced_medicines - everyline[i].packaged_medicines;
        if (current_workload < min_workload) {
            min_workload = current_workload;
            line_id = i;
        }
    }
    return line_id;
}


void produc_medicine(int line_id) {
    struct line *current_line = &everyline[line_id];
    int rate = (current_line->speed / 10);  // Scale the speed to a suitable range for sleeping
    int sleep_interval = (1000000 / rate);
    printf("line %d speed %d\n", line_id, rate);
  //  printf("rate  --------- %d\n", rate);
   // printf("sleep_interval --------- %d\n", sleep_interval);
 
    while (var->liq_sem_running && var->pill_sem_running) {
        if (line_id % 2 == 0) {
        
            // Liquid medicine production
            struct liquid_medicine_message msg = {.type = line_id + 1};
            
            for (int i = 0; i < liquidCount; i++) {
            
            
				if (!var->liq_sem_running && !var->pill_sem_running)
				{
				    printf("\n----------- liq lines Simulation stopped -----------\n");
				    sleep(10);
				    exit(0);
				}
				            
            
                msg.medicine = liquidMedicines[i];
                pthread_mutex_lock(&everyline[line_id].lock);
                everyline[line_id].medicine_status[everyline[line_id].med_indx] = IN_PRODUCTION;
   	        everyline[line_id].produced_medicines++;
                pthread_mutex_unlock(&everyline[line_id].lock);
                if (msgsnd(msgqid, &msg, sizeof(msg.medicine), 0) == -1) {
                    perror("msgsnd error");
                   // exit(EXIT_FAILURE);
                }
                
                // Sleep after producing each medicine
               // printf("Produced liquid medicine %s on line %d, sleeping for %d seconds........\n", msg.medicine.name, line_id, sleep_interval);
                usleep(sleep_interval);
            }
        } else {
            // Pill medicine production
            struct pill_medicine_message msg = {.type = line_id + 1};
            for (int i = 0; i < pillCount; i++) {
            
            
            	if (!var->liq_sem_running && !var->pill_sem_running)
				{
				    printf("\n----------- Pill lines Simulation stopped -----------\n");
				    exit(0);
				}
        
                msg.medicine = pillMedicines[i];
                pthread_mutex_lock(&everyline[line_id].lock);
                everyline[line_id].medicine_status[everyline[line_id].med_indx] = IN_PRODUCTION;
   	        everyline[line_id].produced_medicines++;
                pthread_mutex_unlock(&everyline[line_id].lock);
                if (msgsnd(msgqid, &msg, sizeof(msg.medicine), 0) == -1) {
                    perror("msgsnd error");
                    //exit(EXIT_FAILURE);
                }

                // Sleep after producing each medicine
               // printf("Produced pill medicine %s on line %d, sleeping for %d seconds......\n", msg.medicine.name, line_id, sleep_interval);
                usleep(sleep_interval);
            }
        }
    }
    printf("----------- lines Simulation stopped -----------\n");
    exit(0);
}


void detach_shared_memory(void *shm_addr) {
    if (shmdt(shm_addr) == -1) {
        perror("shmdt failed");
        exit(EXIT_FAILURE);
    }
}

void cleanup_shared_memory(int shm_id) {
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl IPC_RMID failed");
        exit(EXIT_FAILURE);
    }
}


int start_production_lines(int num_lines)
{

    pthread_t monitor_thread;
    pthread_create(&monitor_thread, NULL, workload_monitor, NULL);
    printf("Workload monitor thread started.\n");
    

	var->liq_sem_running = 1;
	var->pill_sem_running = 1;    
        pid_t pid;

	key_t key = ftok("somefile", 'b');
	
	int msg_queue_id = msgget(key, 0666 | IPC_CREAT);
	if (msg_queue_id == -1) {
		perror("msgget failed");
		exit(1);
	}

	if (msgctl(msg_queue_id, IPC_RMID, NULL) == -1) {
                perror("msgctl");
                exit(EXIT_FAILURE);
            }
            
  


    // key_t key = ftok("queuefile", 63);

    key_t queue_key = ftok("productionlines.c", 'b');
    if (queue_key == -1)
    {
        perror("ftok error");
        return 1;
    }

    msgqid = msgget(queue_key, IPC_CREAT | 0660);
    if (msgqid == -1)
    {
        perror("msgget error");
        return 1;
    }
    
    if (msgctl(msgqid, IPC_RMID, NULL) == -1) {
                perror("msgctl");
                exit(EXIT_FAILURE);
            }
    msgqid = msgget(queue_key, IPC_CREAT | 0660);
    if (msgqid == -1)
    {
        perror("msgget error");
        return 1;
    }
    
    // creat all production lines

    for (int i = 0; i < num_lines; i++)
    {
        pid = fork();
        if (pid == -1)
        {
            perror("Failed to fork");
            exit(1);
        }

        if (pid == 0)
        {
            srand(time(NULL) ^ (getpid() << 16));
			
			 // Setup signal handlers
			signal(SIGUSR1, signal_handler);
			signal(SIGUSR2, signal_handler);
			//signal(SIGTERM, signal_handler);
			
          
            my_line = i;


            pthread_mutex_lock(&lock);
            everyline[i].pid = getpid();
            everyline[i].line_id = i;
            everyline[i].workload = 0; // Initialize workload to 0 or any other appropriate value
            everyline[i].speed = rand() % 41 + 10;
            everyline[i].produced_medicines=0;
            for (int j = 0; j < MAX_MEDICINES; j++) {
            everyline[i].medicine_status[j] = NOT_PROCESSED;
            }
            everyline[i].med_indx=0;
            
            pthread_mutex_unlock(&lock);

            initialize_employees(everyline[i].line_id, getpid());
            // print_employees(getpid());

            for ( emp_idx = 0; emp_idx < NUM_EMPLOYEES_PER_LINE; emp_idx++)
             {

                if (i % 2 == 0)
                {
                    pthread_create(&threads[emp_idx], NULL, employee_thread_liquid, &employees[emp_idx]);
                }
                else
                {
                    pthread_create(&threads[emp_idx], NULL, employee_thread_pill, &employees[emp_idx]);
                }
            }
            
            produc_medicine(i);
            
            for (int j = 0; j < NUM_EMPLOYEES_PER_LINE; j++)
            {
                pthread_join(threads[j], NULL);
            }  
            exit(0);
        }
    }
    // print_shared_memory_data();

    while ((pid = wait(NULL)) > 0)
    {
    }
    return 1;
  
}


void setup_shared_memory()
{

//----------------------------------------------------
lineSharedMemID = shmget(lineSharedMemKey, sizeof(LineSharedMemory) * MAX_PRODUCTION_LINES, IPC_CREAT | 0666);
    if (lineSharedMemID < 0) {
        perror("Failed to create line shared memory");
        exit(1);
    }
    lineSharedMem = (LineSharedMemory *)shmat(lineSharedMemID, NULL, 0);
    if (lineSharedMem == (void *)-1) {
        perror("shmat failed for line shared memory");
        exit(1);
    }

    // Initialize Monitor Shared Memory
    monitorSharedMemID = shmget(monitorSharedMemKey, sizeof(MonitorSharedMemory), IPC_CREAT | 0666);
    if (monitorSharedMemID < 0) {
        perror("Failed to create monitor shared memory");
        exit(1);
    }
    monitorSharedMem = (MonitorSharedMemory *)shmat(monitorSharedMemID, NULL, 0);
    if (monitorSharedMem == (void *)-1) {
        perror("shmat failed for monitor shared memory");
        exit(1);
    }

    // Initial clear of shared memory
    memset(lineSharedMem, 0, sizeof(LineSharedMemory) * MAX_PRODUCTION_LINES);
    memset(monitorSharedMem, 0, sizeof(MonitorSharedMemory));
//----------------------------------------------------
    int shmID;
    shmID = shmget(lines_memo_key, sizeof(struct line) * 20, 0666 | IPC_CREAT);
    if (shmID < 0)
    {
        printf("Failed to create shared memory for lines\n");
        exit(1);
    }

    everyline = (struct line *)shmat(shmID, NULL, 0);
    if (everyline == (void *)-1)
    {
        perror("shmat failed");
        exit(1);
    }
    
    detach_shared_memory(everyline);
    cleanup_shared_memory(shmID);
    
    shmID = shmget(lines_memo_key, sizeof(struct line) * 20, 0666 | IPC_CREAT);
    if (shmID < 0)
    {
        printf("Failed to create shared memory for lines\n");
        exit(1);
    }

    everyline = (struct line *)shmat(shmID, NULL, 0);
    if (everyline == (void *)-1)
    {
        perror("shmat failed");
        exit(1);
    }
    
    //----------------------------------------------------------------//

    int shmvarID;
    shmvarID = shmget(var_memo_key, sizeof(struct shVar), 0666 | IPC_CREAT);
    if (shmID < 0)
    {
        printf("Failed to create shared memory for lines\n");
        exit(1);
    }

    var = (struct shVar *)shmat(shmvarID, NULL, 0);
    if (var == (void *)-1)
    {
        perror("shmat failed");
        exit(1);
    }
    
    detach_shared_memory(var);
    cleanup_shared_memory(shmvarID);
    
    shmvarID = shmget(var_memo_key, sizeof(struct shVar), 0666 | IPC_CREAT);
    if (shmID < 0)
    {
        printf("Failed to create shared memory for lines\n");
        exit(1);
    }

    var = (struct shVar *)shmat(shmvarID, NULL, 0);
    if (var == (void *)-1)
    {
        perror("shmat failed");
        exit(1);
    }
    
}


void print_employees(int line_id)
{
    printf("Employees for Production Line %d:\n", line_id);
    for (int i = 0; i < NUM_EMPLOYEES_PER_LINE; i++)
    {
        printf("Employee ID: %d, line id: %d, Origin Line: %d, Current Line: %d\n",
               employees[i].id, employees[i].line_id, employees[i].origin_line, employees[i].current_line);
    }
}


void print_shared_memory_data()
{
    printf("Shared Memory Data for Production Lines:\n");
    for (int i = 0; i < num_production_lines; i++)
    {
        printf("Line ID: %d, PID: %d, Workload: %d\n", everyline[i].line_id, everyline[i].pid, everyline[i].workload);
    }
}
