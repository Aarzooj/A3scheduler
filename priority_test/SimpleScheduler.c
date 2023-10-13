void scheduler()
{
    /*
    test1: count_vowels.c
    test2: fib.c
    test3: factorial.c
    test4: helloworld.c
    */
    if (timer_handler(TSLICE * 1e6) == -1)
    {
        perror("Timer handler");
    }
    if (ready_queue1[front1] != NULL)
    {
        move_ready_to_running(front1, rear1, ready_queue1);
    }
    else if (ready_queue2[front2] != NULL)
    {
        move_ready_to_running(front2, rear2, ready_queue2);
    }
    else if (ready_queue3[front3] != NULL)
    {
        move_ready_to_running(front3, rear3, ready_queue3);
    }
    else if (ready_queue4[front4] != NULL)
    {
        move_ready_to_running(front4, rear4, ready_queue4);
    }
    else
    {
        printf("All ready queues are empty\n");
    }
}

void displayProcesses()
{
    printf("--------------------------------\n");
    for (int i = 0; i < total_processes; i++)
    {
        printf("submit %s\nPID: %d\nExecution time: %f ms\nWait time: %lld ms\n", process_table[i]->name, process_table[i]->pid, process_table[i]->execution_time, process_table[i]->wait_time);
        printf("--------------------------------\n");
    }
}

void move_ready_to_running(int front, int rear, process **ready_queue)
{
    int end = front + NCPU;
    int start = front;
    for (int i = start; i < end; i++)
    {
        if (ready_queue[i] != NULL)
        {
            printf("READY QUEUE --->\n");
            for (int j = front; j < rear + 1; j++)
            {
                printf("%s %d\n", ready_queue[j]->name, ready_queue[j]->pid);
            }
            printf("Runned process: %s   %d\n", ready_queue[i]->name, ready_queue[i]->priority);
            if (clock_gettime(CLOCK_MONOTONIC, &ready_queue[i]->start_time) == -1)
            {
                perror("clock_gettime");
                exit(EXIT_FAILURE);
            }
            kill(ready_queue[i]->pid, SIGCONT);
            process *p = remove_process(ready_queue[i]);
            p->current_cycle = CPU_CYCLES + 1;
            p->wait_time += (p->current_cycle - p->prev_cycle - 1) * TSLICE;
            p->prev_cycle = p->current_cycle;
            p->state = RUNNING;
            add_process_r(p);
        }
        else
        {
            printf("Ready queue is empty\n");
            break;
        }
    }
}