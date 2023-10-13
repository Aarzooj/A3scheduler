#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <stdbool.h>
#include <sys/types.h>
#include <signal.h>

#define READY 0
#define RUNNING 1
#define MAX_PROCESSES 100
#define INPUT_SIZE 256
#define HISTORY_SIZE 100
#define MAX_PROGRAM_NAME 50

int num_processes = 0;
int total_processes = 0;
// int i = 0;
int CPU_CYCLES = 0;

int TSLICE;
int NCPU;

long long start_ms;
long long end_ms;

typedef struct Process {
    pid_t pid;
    int state;
    char* name;
    struct timespec start_time;
    struct timespec end_time;
    double execution_time;
    long long wait_time;
    int prev_cycle;
    int current_cycle;
    int priority;
} process;

process* process_table[MAX_PROCESSES];

process* ready_queue1[MAX_PROCESSES];
int front1 = 0;
int rear1 = -1;
process* ready_queue2[MAX_PROCESSES];
int front2 = 0;
int rear2 = -1;
process* ready_queue3[MAX_PROCESSES];
int front3 = 0;
int rear3 = -1;
process* ready_queue4[MAX_PROCESSES];
int front4 = 0;
int rear4 = -1;

process* running_queue[MAX_PROCESSES];
int front_r = 0;
int rear_r = -1;

// queue.c
process* create_process(char* name, int priority);
void add_process_r(process* p);
void add_process(process* p);
void add_process_table(process* p);
bool is_empty();
process* remove_process(process* p);
process* remove_process_r(process* p);
void empty_process_table();

void shell_loop();

// timer.c
static void sigalrm_handler(int sig);
static int set_sigalrm(int sig, int flags, void (*handler)(int));
static int timer_handler(long long frequency_nsec);

//SimpleScheduler.c
void scheduler();
void displayProcesses();
void move_ready_to_running(int front, int rear, process **ready_queue);

#endif