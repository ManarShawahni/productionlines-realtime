// settings.h

#ifndef SETTINGS_H
#define SETTINGS_H
#define MAX_MEDICINES 100  // Adjust as necessary for the maximum number of medicines

#define NUM_EMPLOYEES_PER_LINE 7  // Define the number of employees per production line
#define NUM_EMPLOYEES 15
#define MAX_PRODUCTION_LINES 50
#define LIQUID 0
#define PILL 1
extern int simulation_running;  // Declaration of the simulation running flag


extern int num_production_lines;
extern int num_medicines;
extern int liquid_failure_rate;
extern int pill_failure_rate;
extern int max_produced_medicines;
extern int max_out_of_spec_liquid_medicines;
extern int max_out_of_spec_pill_medicines;
extern int simulation_duration;


void settings_file(char *filename);
void print_medicines();

typedef struct {
    int id;
    char name[100];
    int lower_level;
    int upper_level;
    char color[20];
    int seal_quality;  // 1 for good, 0 for bad
    int label_correctness;  // 1 for correct, 0 for incorrect
    int expiry_validity;  // 1 for valid, 0 for invalid
    float failure_rate;  // Probability of a failure in inspection
} LiquidMedicine;
 int msgqid;  // Declare external if defined elsewhere

typedef struct {
    int id;
    char name[100];
    int num_of_pills; // Total number of pills that should be in the package
    char color[20];
    char size[10];
    int expiry_validity; // 1 for valid, 0 for invalid
    float failure_rate; // Probability of a failure in inspection
} PillMedicine;

extern int production_line_type[MAX_PRODUCTION_LINES];

LiquidMedicine liquidMedicines[MAX_MEDICINES];
PillMedicine pillMedicines[MAX_MEDICINES];
extern int liquidCount;
extern int pillCount;

void load_medicine_config(const char* filename);


#endif

