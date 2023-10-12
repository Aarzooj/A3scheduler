void scheduler(int ncpu, int tslice)
{
    /*
    test1: count_vowels.c
    test2: fib.c
    test3: factorial.c
    test4: helloworld.c
    */
    if (-1 == init_interval_timer(tslice * 1e6))
    {
        print_error_and_exit("init_interval_timer");
    }
    int end = front + ncpu;
    int start = front;
    for (int i = start; i < end; i++)
    {
        if (ready_queue[i] != NULL)
        {
            printf("READY QUEUE --->\n");
            for (int j = front; j < rear + 1; j++)
            {
                printf("%s %d\n", ready_queue[j]->name,ready_queue[j]->pid);
            }
            printf("Runned process: %s\n", ready_queue[i]->name);
            kill(ready_queue[i]->pid, SIGCONT);
            process *p = remove_process(ready_queue[i]);
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

void displayProcesses(){
    printf("--------------------------------\n");
    for (int i = 0; i < total_processes; i++){
        printf("submit %s\nPID: %d\nExecution time: %lld\nWait time: %lld\n",process_table[i]->name,process_table[i]->pid,process_table[i]->execution_time,process_table[i]->wait_time);
        printf("--------------------------------\n");
    }
}