#include <stdio.h>
#include <math.h>    // Include math library for ceil and sqrt functions
#include <GL/glut.h> // Header File For The GLUT Library
#include "settings.h"
#include "productionlines.h"

void initialize_opengl();

int production_line_type[MAX_PRODUCTION_LINES];
int num_production_lines = MAX_PRODUCTION_LINES; // Initialize with a valid value
int window_width = 1500;
int window_height = 900;
float spacing = 10.0f; // Space between squares
float sem_end_x, sem_end_y,  sem_end_rectWidth,  sem_end_rectHeight;
#define NUM_SECTIONS 4 // Production, Inspection, Packaging, Done

void timer(int value)
{
    glutPostRedisplay();         // Mark the current window as needing to be redisplayed
    glutTimerFunc(16, timer, 0); // Set up the next timer. 16 ms -> ~60 FPS
}

void drawSectionTitles(float x, float y, float rectWidth, float rectHeight)
{
    // Define section titles
    char titles[4][20] = {
        "production",
        "inspection",
        "Packaging",
        "DONE"};

    // Define title colors
    float titleColor[3] = {0.0f, 0.0f, 0.0f}; // Black color

    // Define title position offsets
    float xOffset = rectWidth / 8.0f;   // Offset from left edge of rectangle
    float yOffset = rectHeight * 0.95f; // Offset from top edge of rectangle

    // Set font
    void *font = GLUT_BITMAP_HELVETICA_10;

    // Draw section titles
    for (int i = 0; i < 4; ++i)
    {
        glColor3fv(titleColor);
        glRasterPos2f(x + xOffset + i * rectWidth / 4.0f, y + yOffset - (rectHeight / 3));
        // glutBitmapString(font, (unsigned char *)titles[i]);
        for (const char *c = titles[i]; *c != '\0'; c++)
        {
            glutBitmapCharacter(font, *c);
        }
    }
}

void initialize_production_lines()
{
    for (int i = 0; i < MAX_PRODUCTION_LINES; i++)
    {
        // Alternating line types based on the index
        production_line_type[i] = (i % 2 == 0) ? LIQUID : PILL;
    }
}

void reshape(int width, int height)
{
    if (height == 0)
        height = 1; // Prevent division by zero
    window_width = width;
    window_height = height;
    glViewport(0, 0, (GLsizei)width, (GLsizei)height); // Set the viewport to cover the new window
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, (GLdouble)width, 0.0, (GLdouble)height);
}
void drawEndSem() {
    // Display the information text
    glColor3f(1.0f, 1.0f, 1.0f);  //  color for text
    char infoText[20];
    sprintf(infoText, "END SIMULATION");
    glRasterPos2f(sem_end_x + 10, sem_end_y + sem_end_rectHeight - 80);  // Position the text inside the rectangle
    for (const char *c = infoText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

void drawMedicines(int line_id, float x, float y, float rectWidth, float rectHeight)
{

	if (!var->liq_sem_running || !var->pill_sem_running){
		drawEndSem();
	}
	
    int produced_medicines = everyline[line_id].produced_medicines;
    float sectionWidth = rectWidth / NUM_SECTIONS;
    float circleRadius = sectionWidth / 25;
    float spacing = circleRadius * 2.5;
    int perRow = (int)(sectionWidth / spacing);
    float startY = y + circleRadius + spacing;

    for (int i = 0; i < produced_medicines; i++)
    {
        MedicineStatus status = everyline[line_id].medicine_status[i];
        int sectionIndex = 0;

        switch (status)
        {
        case IN_PRODUCTION:
            sectionIndex = 0;
            break;
        case IN_INSPECTION:
            sectionIndex = 1;
            break;
        case PACKAGE:
            sectionIndex = 2;
            break;
        case DONE:
            sectionIndex = 3;
            break;
        case INSPECTION_PASSED:
            sectionIndex = 1;
            break;
        case INSPECTION_FAILED:
            sectionIndex = 1;
            break;
        default:
            continue;
        }

        int row = i / perRow;
        int col = i % perRow;
        float medX = x + sectionIndex * sectionWidth + col * spacing + circleRadius;
        float medY = startY + row * spacing;
        if (medX + circleRadius > x + (sectionIndex + 1) * sectionWidth)
        {
            continue; // Prevent drawing beyond the section boundary
        }

        // Set color based on status
        if (status == INSPECTION_FAILED)
        {
            glColor3ub(255, 0, 0); // Red for failed inspection
        }
        else if (status == INSPECTION_PASSED)
        {
            glColor3ub(255, 255, 255); // White for passed inspection
        }
        else if (status == PACKAGE)
        {
            glColor3ub(106, 90, 205);
        }
        else if (status == DONE)
        {
            glColor3ub(204, 152, 31);
        }
        else
        {
            // Default colors for other statuses
            if (production_line_type[line_id] == LIQUID)
            {
                glColor3ub(191, 120, 149); // Pink color for liquid medicines
            }
            else
            {
                glColor3ub(0, 156, 123); // Green color for pill medicines
            }
        }

        // Draw circle for each medicine
        glBegin(GL_POLYGON);
        for (int angle = 0; angle < 360; angle++)
        {
            float rad = angle * (M_PI / 180.0f); // Converting Degrees to Radians
            glVertex2f(medX + cos(rad) * circleRadius, medY + sin(rad) * circleRadius);
        }
        glEnd();
    }
}



void drawInfoSection(float x, float y, float rectWidth, float rectHeight) {
    glColor3ub(0.0f, 0.0f, 0.0f);  // Light blue background for info section
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + rectWidth, y);
    glVertex2f(x + rectWidth, y + rectHeight);
    glVertex2f(x, y + rectHeight);
    glEnd();

    // Display the information text
    glColor3f(1.0f, 1.0f, 1.0f); 
    char infoText[40];
    sprintf(infoText, "OUT OF SPACE LIQUID BOTTL: %d", var->out_of_spec_bottled_medicines);
    glRasterPos2f(x + 10, y + rectHeight - 20);  // Position the text inside the rectangle
    for (const char *c = infoText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    
    // Display the information text
     glColor3f(1.0f, 1.0f, 1.0f); 
    
    sprintf(infoText, "OUT OF SPACE OUT PILL: %d",  var->out_of_spec_pill_medicines);
    glRasterPos2f(x + 10, y + rectHeight - 50);  // Position the text inside the rectangle
    for (const char *c = infoText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    
    if (!var->liq_sem_running || !var->pill_sem_running){
		drawEndSem();
	}
    
}

int getCurrentNumberOfEmployees(int lineIndex) {
    return everyline[lineIndex].num_employees + everyline[lineIndex].num_transferred_employees;
}



void drawEachProductionLine(int i, float x, float y, float rectWidth, float rectHeight) {
     // Set color based on production line type for the outer rectangle
     	if (!var->liq_sem_running || !var->pill_sem_running){
		drawEndSem();
	}
        if (production_line_type[i] == LIQUID)
        {
            glColor3ub(255, 203, 203); // RGB color (255, 230, 230) for liquid lines
        }
        else if (production_line_type[i] == PILL)
        {
            glColor3ub(122, 178, 178); // RGB color (109, 197, 209) for pill lines
        }

        // Draw the outer rectangle
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + rectWidth, y);
        glVertex2f(x + rectWidth, y + rectHeight);
        glVertex2f(x, y + rectHeight);
        glEnd();

        // Draw medicines within the production area
        drawMedicines(i, x, y, rectWidth, rectHeight);

        // Draw black lines at the top third of each rectangle
        glColor3f(0.0f, 0.0f, 0.0f); // Black color
        glBegin(GL_LINES);
        glVertex2f(x, y + rectHeight * (2.0f / 3.0f));             // Top-left corner
        glVertex2f(x + rectWidth, y + rectHeight * (2.0f / 3.0f)); // Top-right corner
        glEnd();

        // Draw vertical lines to divide the space below the black line into four columns
        glColor3f(0.0f, 0.0f, 0.0f); // Black color
        glBegin(GL_LINES);
        glVertex2f(x + rectWidth / 4.0f, y);                              // Start of first column
        glVertex2f(x + rectWidth / 4.0f, y + rectHeight * (2.0f / 3.0f)); // End of first column

        glVertex2f(x + rectWidth / 2.0f, y);                              // Start of second column
        glVertex2f(x + rectWidth / 2.0f, y + rectHeight * (2.0f / 3.0f)); // End of second column

        glVertex2f(x + 3 * rectWidth / 4.0f, y);                              // Start of third column
        glVertex2f(x + 3 * rectWidth / 4.0f, y + rectHeight * (2.0f / 3.0f)); // End of third column
        glEnd();

        // Draw a shaded rectangle behind the type label

        if (production_line_type[i] == LIQUID)
        {
            glColor3ub(255, 234, 227);
            // blue Lighter shade of liquid color
        }
        else if (production_line_type[i] == PILL)
        {
            // bink Lighter shade of pill color
            glColor3ub(205, 232, 229);
        }

        glBegin(GL_QUADS);
        glVertex2f(x, y + rectHeight * 0.9f);             // Bottom-left corner
        glVertex2f(x + rectWidth, y + rectHeight * 0.9f); // Bottom-right corner
        glVertex2f(x + rectWidth, y + rectHeight);        // Top-right corner
        glVertex2f(x, y + rectHeight);                    // Top-left corner
        glEnd();

        // Draw type label at the top of each rectangle
        char label[64]; // Buffer for label
        sprintf(label, "%s", (production_line_type[i] == LIQUID) ? "LIQUID Production Line" : "PILL Production Line");
        glColor3f(0.0f, 0.0f, 0.0f);                                                                // Black color for text
        float textWidth = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char *)label); // Calculate text width
        glRasterPos2f(x + (rectWidth - textWidth) / 2.0f, y + rectHeight * 0.93f);                  // Position the text
        // glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char *)label); // Draw the text
        for (const char *c = label; *c != '\0'; c++)
        {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
		
		drawSectionTitles(x, y, rectWidth, rectHeight);

		// Display workload next to the line type label
        char workload_text[64];
        sprintf(workload_text, "Load: %d", everyline[i].workload);
        glColor3f(0.0f, 0.0f, 0.0f); // Black color for workload text
        glRasterPos2f(x + (rectWidth - textWidth) , y + rectHeight * 0.93f); // Adjust x to place workload text next to the line label
        for (const char *c = workload_text; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
        
       
        
        // Write "Employees" below the shaded area
        char employees[] = "Employees";
        glColor3f(0.0f, 0.0f, 0.0f);                                                              // Black color for text
        textWidth = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char *)employees); // Calculate text width
        glRasterPos2f(x + (rectWidth - textWidth) / 2.0f, y + rectHeight * 0.9f - 10);            // Position the text
                                                                                                  // glutBitmapString(GLUT_BITMAP_HELVETICA_10, (unsigned char *)employees); // Draw the text
        for (const char *c = employees; *c != '\0'; c++)
        {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *c);
        }

        // Draw black lines at the top third of each rectangle
        glColor3f(0.0f, 0.0f, 0.0f); // Black color
        glBegin(GL_LINES);
        glVertex2f(x, y + rectHeight * (2.0f / 3.0f));             // Top-left corner
        glVertex2f(x + rectWidth, y + rectHeight * (2.0f / 3.0f)); // Top-right corner
        glEnd();

        // Draw section titles
        drawSectionTitles(x, y, rectWidth, rectHeight);

        // Draw workers (dots) in a single row at the top of each rectangle

        glPointSize(8.0f);           // Adjust point size as needed
         int numEmployees = getCurrentNumberOfEmployees(i);  // Implement this function to get the current number of employees, which includes transferred ones.
         float employee_spacing_x = rectWidth / (numEmployees + 1);
        for (int j = 0; j < numEmployees; j++)
        {
            float employee_x = x + (j + 1) * employee_spacing_x;
            float employee_y = y + rectHeight * (2.0f / 3.0f) + rectHeight * 0.15f; // Adjust Y position to place employees in the top third
            
                    // Set color based on employee's transfer status
        if (transferStatus[j].to_where == i && transferStatus[j].is_transferred) {
            glColor3ub(255, 0, 0); // Red color for transferred employees
        } else {
            glColor3ub(0, 0, 0); // Black color for original or non-transferred employees
        }


            glBegin(GL_POINTS);
            glVertex2f(employee_x, employee_y);
            glEnd();
        }
}

void drawProductionLines()
{
   	int num_info_sections = 1;
    int total_sections = num_production_lines + num_info_sections;
    int num_columns = (int)ceil(sqrt(total_sections));
    int num_rows = (int)ceil((float)total_sections / num_columns);

    // Calculate the width and height of each rectangle
    float rectWidth = (window_width - (num_columns + 1) * spacing) / num_columns;
    float rectHeight = (window_height - (num_rows + 1) * spacing) / num_rows;


	  // Iterate over production lines + one additional for the info
    for (int i = 0; i < total_sections; i++) {
        int row = i / num_columns;
        int col = i % num_columns;

        float x = col * (rectWidth + spacing) + spacing;
        float y = row * (rectHeight + spacing) + spacing;

        if (i < num_production_lines) {
            // Normal production line drawing code here
            
            drawEachProductionLine(i, x, y, rectWidth, rectHeight);
        } else {
        	sem_end_x = x;
            sem_end_y = y;
            sem_end_rectWidth =rectWidth;
            sem_end_rectHeight = rectHeight;
            // Draw the info section
            drawInfoSection(x, y, rectWidth, rectHeight);
        }
    }


   
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the screen

    // Reset the modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawProductionLines(); // Draw the production lines
	
	if (!var->liq_sem_running || !var->pill_sem_running){
		drawEndSem();
	}
    glutSwapBuffers(); // Swap buffers (double buffering)
}

void init()
{
    // Beige background
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void initOpenGL(int *argc, char **argv)
{
    glutInit(argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(window_width, window_height);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Worker Visualization");

    // OpenGL initialization function
    init();
    initialize_production_lines(); // Ensure production lines are initialized

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, timer, 0); // Initialize the timer function

    // Start the GLUT main loop
    glutMainLoop();
}

