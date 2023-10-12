#include <stdio.h>
#include <unistd.h>

#define READY 0
#define RUNNING 1
#define MAX_PROCESSES 100

int num_processes = 0;
int total_processes = 0;
int i = 0;

pid_t shell_pid = -1;

typedef struct Process {
    pid_t pid;
    int state;
    char* name;
    struct timespec start_time;
    struct timespec end_time;
    long long execution_time;
    long long wait_time;
} process;

process* process_table[MAX_PROCESSES];

process* create_process(char* name);