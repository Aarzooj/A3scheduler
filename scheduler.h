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
    float wait_time;
} process;

process process_table[MAX_PROCESSES];

process* create_process(char* name);
void add_process_table(process* p);