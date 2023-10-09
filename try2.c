#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define READY 0
#define RUNNING 1

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

typedef struct ready_queue {
    process p;
    struct ready_queue* next;
} queue;

queue* allocate_node(process p) {
    queue* q = (queue*)malloc(sizeof(queue));
    q->p = p;
    q->next = NULL;
    return q;
}

queue* insert_rear(queue* head, queue* q) {
    if (head == NULL) {
        return q;
    }
    queue* tmp = head;
    while (tmp->next != NULL) {
        tmp = tmp->next;
    }
    tmp->next = q;
    return head;
}

int main() {
    // Get the start time
    queue* all_process = NULL;
    process p1;
    p1.pid = getpid(); // Get the current process ID
    p1.state = READY;
    p1.name = "./a.out";
    clock_gettime(CLOCK_REALTIME, &p1.start_time);
    sleep(2);
    // Get the end time
    clock_gettime(CLOCK_REALTIME, &p1.end_time);

    // Calculate execution time in milliseconds
    start_ms = p1.start_time.tv_sec * 1000LL + p1.start_time.tv_nsec / 1000000LL;
    end_ms = p1.end_time.tv_sec * 1000LL + p1.end_time.tv_nsec / 1000000LL;
    p1.execution_time = end_ms - start_ms;

    all_process = insert_rear(all_process, allocate_node(p1));

    process p2;
    p2.pid = getpid(); // Get the current process ID
    p2.state = READY;
    p2.name = "./fib";
    clock_gettime(CLOCK_REALTIME, &p2.start_time);
    sleep(1);
    // Get the end time
    clock_gettime(CLOCK_REALTIME, &p2.end_time);
    start_ms = p2.start_time.tv_sec * 1000LL + p2.start_time.tv_nsec / 1000000LL;
    end_ms = p2.end_time.tv_sec * 1000LL + p2.end_time.tv_nsec / 1000000LL;
    p2.execution_time = end_ms - start_ms;

    all_process = insert_rear(all_process, allocate_node(p2));

    queue* head = all_process;
    while (head != NULL) {
        process p1 = head->p;
        printf("Process ID: %d\n", p1.pid);
        printf("State: %d\n", p1.state);
        printf("Name: %s\n", p1.name);
        printf("Execution Time (ms): %lld\n", p1.execution_time);
        head = head->next;
    }

    return 0;
}