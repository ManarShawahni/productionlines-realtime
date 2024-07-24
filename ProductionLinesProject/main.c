// main.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#include "settings.h"
#include "productionlines.h"


pthread_mutex_t lock;
//pthread_cond_t cond;

// Declare a thread for the OpenGL
pthread_t opengl_thread;


int total_produced_medicines = 0;
time_t simulation_start_time;

void setup_shared_memory();
void initialize_production_lines();
int start_production_lines(int num_production_lines);
int check_simulation_end();
void* start_opengl_thread(void* arg);
void initialize_employees(int line_id);
void initOpenGL(int *argc, char **argv);



int main(int argc, char *argv[]) {
    srand(time(NULL));
    settings_file("file.txt");
    
    printf("Loading medicine configurations from %s...\n", "medicines_config.txt");
    load_medicine_config("medicines_config.txt");
    
    setup_shared_memory();
    
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process for OpenGL rendering
        initOpenGL(&argc, argv);
        exit(0);
    }

    
    print_medicines();
    
    // Initialize the mutex and condition variable
    if (pthread_mutex_init(&lock, NULL) != 0) {
        perror("pthread_mutex_init");
        return 1;
    }
  
    simulation_start_time = time(NULL);

    start_production_lines(num_production_lines);
   
    //print_shared_memory_data();

    printf("Simulation completed.\n");
    // Destroy the mutex and condition variable
    pthread_mutex_destroy(&lock);
  
    return 0;
}



int check_simulation_end() {

 return 0; 
}


