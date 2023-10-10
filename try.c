#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define READY 0
#define RUNNING 1

#define MAX_PROCESSES 100
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
    long long execution_time;
    float wait_time;
} process;

process process_table[MAX_PROCESSES];
int index = 0;

void add_process_table(process p){
    process_table[index] = p;
    index++;
}

process ready_queue[MAX_PROCESSES];
int front = 0;
int rear = -1;
int num_processes = 0;

void add_process(process p){
    rear++;
    ready_queue[rear] = p;
    num_processes++;
}

process remove_process(process p){
    if (front > rear) {
        process empty_process;
        empty_process.pid = -1;
        return empty_process;
    }
    process p = ready_queue[front];
    front++;
    num_processes--;
    return p;
}




int main(int argc , char* argv[]){
    NCPU = atoi(argv[1]);
    TSLICE  =atoi(argv[2]);

}





