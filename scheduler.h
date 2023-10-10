#include <stdio.h>

#define READY 0
#define RUNNING 1
#define MAX_PROCESSES 100

int num_processes = 0;
int i = 0;

typedef struct Process {
    pid_t pid;
    int state;
    char* name;
    struct timespec start_time;
    struct timespec end_time;
    long long execution_time;
    double wait_time;
} process;

process* create_process(char* name);