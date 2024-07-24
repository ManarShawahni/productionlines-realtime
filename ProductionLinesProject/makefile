CC = gcc
CFLAGS = -Wall
LDFLAGS = -lrt -lGL -lGLU -lglut -lpthread -lm  # Link with real-time, OpenGL, pthread, and math libraries

# Default target
all: clean main

# Compile main.c, employee.c, opengl.c, productionlines.c, and settings.c into an executable named main
main: main.o employee.o opengl.o productionlines.o settings.o
	$(CC) $(CFLAGS) -o main main.o employee.o opengl.o productionlines.o settings.o $(LDFLAGS)

main.o: main.c settings.h
	$(CC) $(CFLAGS) -c main.c

employee.o: employee.c 
	$(CC) $(CFLAGS) -c employee.c

opengl.o: opengl.c
	$(CC) $(CFLAGS) -c opengl.c

productionlines.o: productionlines.c settings.h
	$(CC) $(CFLAGS) -c productionlines.c

settings.o: settings.c settings.h
	$(CC) $(CFLAGS) -c settings.c

# Clean target to remove object files and the main executable
clean:
	rm -f *.o main

