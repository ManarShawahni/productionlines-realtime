//employee.c
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include "settings.h"
#include <sys/wait.h>
#include <sys/msg.h>
#include "productionlines.h"
#include <GL/glut.h>
#include <unistd.h>


extern pthread_mutex_t lock;
//extern pthread_cond_t cond;

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

extern LiquidMedicine liquidMedicines[];
extern PillMedicine pillMedicines[];
extern int num_liquid_medicines;
extern int num_pill_medicines;
extern int max_out_of_spec_liquid_medicines;
extern int max_out_of_spec_pill_medicines;
int sleep_duration;
int msgqid; // Declare external if defined elsewhere




extern void initialize_employees(int line_id, int line_pid);
extern void initialize_new_employees(int i , Employee new_employee, int new_line_id , int new_line_pid);
bool inspect_pill(PillMedicine *medicine, int line_id);
bool inspect_liquid(LiquidMedicine *medicine, int line_id);
void employee_package_liquid(LiquidMedicine *medicine, int line_id);
void employee_package_pill(PillMedicine *medicine, int line_id);

void initialize_new_employees(int i , Employee new_employee, int new_line_id , int new_line_pid)
{
    
        employees[i].id = i;
        employees[i].line_id = new_employee.line_id;
        employees[i].origin_line = new_employee.origin_line;
        employees[i].current_line = new_line_pid;
        employees[i].current_line_id = new_line_id;
        employees[i].active = 1;
        employees[i].role = INSPECTION; // Default role as inspection
        printf("new Employee %d initialized for line %d. origin line id %d activation %d\n", i, employees[i].current_line_id, employees[i].line_id, employees[i].active );
    
}
void initialize_employees(int line_id, int line_pid)
{
    everyline[line_id].num_employees = NUM_EMPLOYEES_PER_LINE;  // Initialize the number of employees per line
    everyline[line_id].num_transferred_employees = 0;  // Initially, no employees are transferred
    for (int i = 0; i < NUM_EMPLOYEES_PER_LINE; i++)
    {
        employees[i].id = i;
        employees[i].line_id = line_id;
        employees[i].origin_line = line_pid;
        employees[i].current_line = line_pid;
        employees[i].active = 1;
        
        employees[i].role = INSPECTION; // Default role as inspection
        printf("Employee %d initialized for line %d. line id %d activation %d\n", i, line_pid, employees[i].line_id, employees[i].active );
    }
}

void increment_med_index(int line_id) {
    pthread_mutex_lock(&lock);
    everyline[line_id].med_indx++;
    pthread_mutex_unlock(&lock);
}


bool inspect_liquid(LiquidMedicine *medicine, int line_id)
{
   // printf("Inspecting liquid medicine: %s\n", medicine->name);
    bool pass = true;
    //sleep(3);
    pthread_mutex_lock(&lock);
    everyline[line_id].medicine_status[everyline[line_id].med_indx] = IN_INSPECTION;
    pthread_mutex_unlock(&lock);

    // Liquid level check
    if (rand() % 100 < 5)
    { // 5% chance the level is out of spec
     //   printf("FAIL: Liquid level out of spec for %s\n", medicine->name);
        pass = false;
    }

    // Color check
    if (rand() % 100 < 5)
    { // 5% chance of color mismatch
     //   printf("FAIL: Color mismatch for %s\n", medicine->name);
        pass = false;
    }

    // Seal quality check
    if (rand() % 100 < 5)
    { // 5% chance of seal failure
     //   printf("FAIL: Seal quality issue for %s\n", medicine->name);
        pass = false;
    }

    // Label correctness check
    if (rand() % 100 < 5)
    { // 5% chance of label being incorrect
     //   printf("FAIL: Label issue for %s\n", medicine->name);
        pass = false;
    }

    // Expiry validity check
    if (rand() % 100 < 5)
    { // 5% chance of invalid expiry date
      //  printf("FAIL: Expiry date issue for %s\n", medicine->name);
        pass = false;
    }

    if (pass)
    {
      //        printf("PASS: All checks passed for %s\n", medicine->name);
    }
    
    usleep(300000);
    pthread_mutex_lock(&lock);
    int med_index = everyline[line_id].med_indx;
    everyline[line_id].medicine_status[med_index] = pass ? INSPECTION_PASSED : INSPECTION_FAILED;
    
    if (!var->liq_sem_running || !var->pill_sem_running){
		exit(0);
	}   
   
        if (!pass)
    {
       var->out_of_spec_bottled_medicines++; 
    }
    
    pthread_mutex_unlock(&lock);

    if (pass) {
        employee_package_liquid(medicine, line_id);
    }

    increment_med_index(line_id);  // Increment index after processing
    return pass;
    
}

bool inspect_pill(PillMedicine *medicine, int line_id)
{
   // printf("Inspecting Pill Medicine %s:\n", medicine->name);
    bool pass = true;
    //sleep(3);
    pthread_mutex_lock(&lock);
    everyline[line_id].medicine_status[everyline[line_id].med_indx] = IN_INSPECTION;
    pthread_mutex_unlock(&lock);
	sleep_duration = 300000 + rand() % 500000;  // 100000 + 0 to 399999
	usleep(sleep_duration);
	
	
    // Pill count check
    if (rand() % 100 < 5)
    { // 5% chance of missing pills
        //printf("FAIL: Missing pills for %s\n", medicine->name);
        pass = false;
    }

    // Color check
    if (rand() % 100 < 5)
    { // 5% chance of color mismatch
        //printf("FAIL: Color mismatch for %s\n", medicine->name);
        pass = false;
    }

    // Size check
    if (rand() % 100 < 5)
    { // 5% chance of size mismatch
       // printf("FAIL: Size mismatch for %s\n", medicine->name);
        pass = false;
    }

    // Expiry validity check
    if (rand() % 100 < 5)
    { // 5% chance of invalid expiry date
        //printf("FAIL: Expiry date issue for %s\n", medicine->name);
        pass = false;
    }

   /* if (pass)
    {
      printf("PASS: All checks passed for %s\n", medicine->name);
    }*/
    
	if (!var->liq_sem_running || !var->pill_sem_running){
		exit(0);
	}   
   
    pthread_mutex_lock(&lock);
    int med_index = everyline[line_id].med_indx;
    everyline[line_id].medicine_status[med_index] = pass ? INSPECTION_PASSED : INSPECTION_FAILED;
    
    if (!pass)
    {
        var->out_of_spec_pill_medicines++; 
    }
    
    pthread_mutex_unlock(&lock);
    

    if (pass) {
        employee_package_pill(medicine, line_id);
    }

    increment_med_index(line_id);  // Increment index after processing
    return pass;
}


void employee_package_liquid(LiquidMedicine *medicine, int line_id) {
    pthread_mutex_lock(&lock);
    int med_index = everyline[line_id].med_indx - 1;  // Adjust for latest medicine
    if (everyline[line_id].medicine_status[med_index] == INSPECTION_PASSED) {
        //printf("Packaging Liquid Medicine: %s\n", medicine->name);
        
        
        everyline[line_id].medicine_status[med_index] = PACKAGE;
        // Simulate packaging delay
        
        sleep_duration = 300000 + rand() % 500000;  // 100000 + 0 to 399999
		usleep(sleep_duration);
        everyline[line_id].medicine_status[med_index] = DONE;
    }
    pthread_mutex_unlock(&lock);
}

void employee_package_pill(PillMedicine *medicine, int line_id)
{
    pthread_mutex_lock(&lock);
    int med_index = everyline[line_id].med_indx - 1; 
    if (everyline[line_id].medicine_status[med_index] == INSPECTION_PASSED) {
       // printf("Packaging Pill Medicine: %s\n", medicine->name);
       sleep_duration = 300000 + rand() % 500000;   // 100000 + 0 to 399999
		usleep(sleep_duration);
        everyline[line_id].medicine_status[med_index] = PACKAGE;
        // Simulate packaging delay
       
       sleep_duration = 300000 + rand() % 500000;    // 100000 + 0 to 399999
		usleep(sleep_duration);
			
        everyline[line_id].medicine_status[med_index] = DONE;
    }
    pthread_mutex_unlock(&lock); 
}

void *employee_thread_liquid(void *arg)
{
    Employee *employee = (Employee *)arg;

    struct liquid_medicine_message msg;
    while (var->liq_sem_running && var->pill_sem_running)
    {
    
        //sleep(3);
        if (msgrcv(msgqid, &msg, sizeof(msg.medicine), employee->line_id+1, 0) == -1)
        {
            //perror("msgrcv error");
            break;
        }
        
       // printf("out_of_spec_bottled_medicines+++++++++++ %d\n", var->out_of_spec_bottled_medicines);
        pthread_mutex_lock(&lock);
        if (var->out_of_spec_bottled_medicines >= max_out_of_spec_liquid_medicines)
        {
        	
            printf("Out of spec liquid medicines %d\n", var->out_of_spec_bottled_medicines);
            fflush(stdout);
            var->liq_sem_running=0;
            pthread_mutex_unlock(&lock);
           
            break;
        }
        pthread_mutex_unlock(&lock);
		
		
        if (inspect_liquid(&msg.medicine, employee->line_id)) {
            
    		
            employee_package_liquid(&msg.medicine, employee->line_id);
            everyline[employee->line_id].packaged_medicines++;
        }
       
    }
    
    pthread_exit(NULL);
}

void *employee_thread_pill(void *arg)
{
    Employee *employee = (Employee *)arg;
    //printf("employee %d in line %d start to new line %d\n\n",employee->id, employee->line_id, employee->current_line_id);
    struct pill_medicine_message msg;
    while (var->liq_sem_running && var->pill_sem_running)
    {
        key_t key = ftok("productionlines.c", 'b');
        msgqid = msgget(key, 0666 | IPC_CREAT);
        if (msgrcv(msgqid, &msg, sizeof(msg.medicine), employee->line_id+1, 0) == -1)
        {
            //perror("msgrcv error");
            break;
        }
        
        pthread_mutex_lock(&lock);
        if (var->out_of_spec_pill_medicines >= max_out_of_spec_pill_medicines)
        {
           
            printf("Out of spec Bill medicines %d\n", var->out_of_spec_pill_medicines);
            var->pill_sem_running = 0;
            pthread_mutex_unlock(&lock);
            break;
        }
        pthread_mutex_unlock(&lock);

    if (inspect_pill(&msg.medicine, employee->line_id)) {
    		
            employee_package_pill(&msg.medicine, employee->line_id);
            
            everyline[employee->line_id].packaged_medicines++;
        }
        
    }
    //kill(employee->origin_line, SIGTERM);
    pthread_exit(NULL);
}
