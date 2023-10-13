// #include "scheduler.h"

process *create_process(char *name)
{
    process *p = (process *)malloc(sizeof(process));
    p->pid = 0;
    p->state = READY;
    p->name = name;
    p->execution_time = 0;
    p->wait_time = 0;
    p->prev_cycle = 0;
    return p;
}

void empty_process_table()
{
    for (int i = 0; i < total_processes; i++)
    {
        free(process_table[i]);
        process_table[i] = NULL;
    }
    total_processes = 0;
}

void add_process_r(process *p)
{
    rear_r++;
    running_queue[rear_r] = p;
}

void add_process(process *p)
{
    rear++;
    ready_queue[rear] = p;
}

void add_process_table(process *p)
{
    process_table[total_processes] = p;
    total_processes++;
}

bool is_empty()
{

    return (front > rear && front_r > rear_r);
}

process *remove_process(process *p)
{
    if (front > rear)
    {
        process *dummy = create_process("None");
        dummy->pid = -1;
        return dummy;
    }
    p = ready_queue[front];
    front++;
    return p;
}

process *remove_process_r(process *p)
{
    if (front_r > rear_r)
    {
        process *dummy = create_process("None");
        dummy->pid = -1;
        return dummy;
    }
    p = running_queue[front_r];
    front_r++;
    return p;
}

// int main(){

// }

// int main() {
//     // Get the start time
//     queue* all_process = NULL;
//     process p1;
//     p1.pid = getpid(); // Get the current process ID
//     p1.state = READY;
//     p1.name = "./a.out";
//     clock_gettime(CLOCK_REALTIME, &p1.start_time);
//     sleep(2);
//     // Get the end time
//     clock_gettime(CLOCK_REALTIME, &p1.end_time);

//     // Calculate execution time in milliseconds
//     start_ms = p1.start_time.tv_sec * 1000LL + p1.start_time.tv_nsec / 1000000LL;
//     end_ms = p1.end_time.tv_sec * 1000LL + p1.end_time.tv_nsec / 1000000LL;
//     p1.execution_time = end_ms - start_ms;

//     all_process = insert_rear(all_process, allocate_node(p1));

//     process p2;
//     p2.pid = getpid(); // Get the current process ID
//     p2.state = READY;
//     p2.name = "./fib";
//     clock_gettime(CLOCK_REALTIME, &p2.start_time);
//     sleep(1);
//     // Get the end time
//     clock_gettime(CLOCK_REALTIME, &p2.end_time);
//     start_ms = p2.start_time.tv_sec * 1000LL + p2.start_time.tv_nsec / 1000000LL;
//     end_ms = p2.end_time.tv_sec * 1000LL + p2.end_time.tv_nsec / 1000000LL;
//     p2.execution_time = end_ms - start_ms;

//     all_process = insert_rear(all_process, allocate_node(p2));

//     queue* head = all_process;
//     while (head != NULL) {
//         process p1 = head->p;
//         printf("Process ID: %d\n", p1.pid);
//         printf("State: %d\n", p1.state);
//         printf("Name: %s\n", p1.name);
//         printf("Execution Time (ms): %lld\n", p1.execution_time);
//         head = head->next;
//     }

//     return 0;
// }
