#include <stdio.h>
#include <setjmp.h>

#define MAX_PROCESSES 2

typedef struct {
    jmp_buf context;
    int state;
} Process;

Process processes[MAX_PROCESSES];
int current_process = 0;

void scheduler() {
    int next_process;

    while (1) {
        next_process = (current_process + 1) % MAX_PROCESSES;

        if (processes[next_process].state == 0) {
            // Save the context of the current process
            if (setjmp(processes[current_process].context) == 0) {
                processes[current_process].state = 1;

                // Restore the context of the next process
                longjmp(processes[next_process].context, 1);
            }
        }

        // Switch to the next process
        current_process = next_process;
    }
}

void example_task(int id) {
    while (1) {
        printf("Process %d is running\n", id);
    }
}

int main() {
    // Initialize the processes
    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i].state = 0;
    }

    // Start the scheduler
    scheduler();

    return 0;
}
