#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define MAX_COMMAND_LENGTH 100
#define MAX_PROGRAM_NAME 50
#define MAX_PROCESSES 10

typedef struct {
    pid_t pid;
    char name[MAX_PROGRAM_NAME];
    time_t start_time;
    time_t end_time;
    time_t wait_time;
} Process;

Process processes[MAX_PROCESSES];
int num_processes = 0;

int ready_queue[MAX_PROCESSES];
int ready_front = 0;
int ready_rear = -1;
int ready_size = 0;

void enqueue(int pid) {
    ready_rear = (ready_rear + 1) % MAX_PROCESSES;
    ready_queue[ready_rear] = pid;
    ready_size++;
}

int dequeue() {
    if (ready_size <= 0) return -1;
    int pid = ready_queue[ready_front];
    ready_front = (ready_front + 1) % MAX_PROCESSES;
    ready_size--;
    return pid;
}

void scheduler(int ncpu, int tslice) {
    int current_process = 0;
    while (num_processes > 0) {
        if (processes[current_process].pid != -1) {
            int pid = dequeue();
            if (pid != -1) {
                // Send signal to start execution
                kill(pid, SIGCONT);

                // Wait for the time slice
                sleep(tslice);

                // Send signal to stop execution
                kill(pid, SIGSTOP);

                // Calculate wait and end times
                for (int i = 0; i < num_processes; i++) {
                    if (processes[i].pid == pid) {
                        processes[i].end_time = time(NULL);
                        processes[i].wait_time =
                            processes[i].end_time - processes[i].start_time;
                        printf("Process %s (PID %d) completed.\n", processes[i].name,
                               processes[i].pid);
                        break;
                    }
                }
            }
        }

        // Move to the next process in a round-robin fashion
        current_process = (current_process + 1) % num_processes;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <NCPU> <TSLICE>\n", argv[0]);
        return 1;
    }

    int ncpu = atoi(argv[1]);
    int tslice = atoi(argv[2]);

    char command[MAX_COMMAND_LENGTH];
    while (true) {
        printf("SimpleShell$ ");
        fgets(command, sizeof(command), stdin);
        command[strlen(command) - 1] = '\0';

        if (strcmp(command, "run") == 0) {
            scheduler(ncpu, tslice);
            return 0;
        } else if (strncmp(command, "submit", 6) == 0) {
            char program[MAX_PROGRAM_NAME];
            sscanf(command, "submit %s", program);

            pid_t pid = fork();
            if (pid == 0) {  // Child process
                // Wait for the scheduler signal before starting execution
                raise(SIGSTOP);

                execl(program, program, (char *)NULL);
                fprintf(stderr, "Error executing %s\n", program);
                return 1;
            } else if (pid > 0) {  // Parent process
                // Add the process to the list
                strcpy(processes[num_processes].name, program);
                processes[num_processes].pid = pid;
                processes[num_processes].start_time = time(NULL);
                enqueue(pid);
                num_processes++;
            } else {
                fprintf(stderr, "Fork failed\n");
                return 1;
            }
        }
    }
}
