# Pharmaceutical Production Simulation 🏭

https://github.com/user-attachments/assets/7fad2628-0c63-420b-bb4a-7de1af74e5e1

## Description
This simulation project is designed to model and visualize the pharmaceutical production process for both liquid and pill medicines (lines). It uses advanced programming concepts including multi-threading, inter-process communication (IPC), and shared memory, combined with real-time visualization using OpenGL. The core objective is to manage a production environment where conditions such as workload and resource allocation dynamically change and require real-time adjustments.
We will have 4 stages for each line and according to its type (Pill or Liquid), it will perfrom these stages.

### Key Features:
- **Real-time Visualization**: Implements OpenGL to dynamically display the production process, updating in real time as the simulation progresses.
- **Concurrency Management**: Utilizes threads and processes extensively to simulate concurrent operations in medicine inspection and packaging.
- **IPC Mechanisms**: Employs message queues and shared memory for communication between processes, ensuring coordinated workflow across different stages of the production line.
- **Dynamic Resource Allocation**: Handles unexpected shifts in production demands by reallocating resources such as employees between lines based on workload, demonstrated through signal handling.
- **Robust Error Handling**: Includes comprehensive error checking and handling to manage and recover from operational anomalies during runtime.

## Technologies Used:
- **C**: For the core simulation logic and process management.
- **OpenGL (GLUT)**: For real-time graphical representation of the production line.
- **POSIX Threads (pthread)**: For managing concurrent processes and synchronization.
- **Linux Signals**: For inter-process notifications and status updates.
- **IPC**: For message queues and shared memory management.

## Setup and Installation
Ensure you have GCC, Make, and OpenGL libraries installed on your Linux system to compile and run the simulation.

1. **Clone the repository**:
   ```bash
   git clone https://github.com/your-username/pharmaceutical-production.git
   cd pharmaceutical-production
   ```

- Or you can open the project file on virtualbox or SEED
  - Open the project file on terminal
  - Build the project:
     ```bash
     make
     ```
  - Run the program:
     ```bash
     ./main
     ```
  
