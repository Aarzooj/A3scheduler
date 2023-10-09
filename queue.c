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