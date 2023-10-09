#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <stdbool.h>

#include "dummy_main.h"

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

queue* all_process = NULL;

// defining sizes for data structures allocated
#define INPUT_SIZE 256
#define HISTORY_SIZE 100
#define MAX_BGPROCESS 5

int NCPU;
int TSLICE;

// PIDs of background process running in the background. Can handle max of 5 processes only
// initialising all values to 0
pid_t running_bg_process[MAX_BGPROCESS] = {0};

// flag if the background process is started
int bgProcess = 0;

// struct for each command stored in history
struct CommandParameter
{
    char command[INPUT_SIZE];
    time_t start_time;
    time_t end_time;
    double duration;
    pid_t process_pid;
};

struct CommandHistory
{
    struct CommandParameter record[HISTORY_SIZE];
    int historyCount;
};

struct CommandHistory history;

// Function to display details of each command when the program is terminated using Ctrl + C
void displayTerminate()
{
    printf("--------------------------------\n");
    for (int i = 0; i < history.historyCount; i++)
    {
        struct CommandParameter record = history.record[i];
        // conversion of start and end time to string structures
        struct tm *start_time_info = localtime(&record.start_time);
        char start_time_buffer[80];
        strftime(start_time_buffer, sizeof(start_time_buffer), "%Y-%m-%d %H:%M:%S", start_time_info);
        struct tm *end_time_info = localtime(&record.end_time);
        char end_time_buffer[80];
        strftime(end_time_buffer, sizeof(end_time_buffer), "%Y-%m-%d %H:%M:%S", end_time_info);
        printf("%s\nProcess PID: %d\n", record.command, record.process_pid);
        printf("Start time: %s\nEnd Time: %s\nProcess Duration: %f\n", start_time_buffer, end_time_buffer, record.duration);
        printf("--------------------------------\n");
    }

    queue* head = all_process;
    while (head != NULL) {
        process p1 = head->p;
        printf("Process ID: %d\n", p1.pid);
        printf("State: %d\n", p1.state);
        printf("Name: %s\n", p1.name);
        printf("Execution Time (ms): %lld\n", p1.execution_time);
        head = head->next;
    }
}

// SIGINT (Ctrl + C) handler
static void my_handler(int signum)
{
    printf("\n");
    displayTerminate();
    exit(0);
}

// displaying command history
void displayHistory()
{
    history.record[history.historyCount].process_pid = getpid();
    for (int i = 0; i < history.historyCount + 1; i++)
    {
        printf("%d  %s\n", i + 1, history.record[i].command);
    }
}

// append background process as the latest value in the array
int append(pid_t pid)
{
    int added = -1;
    // if no background process running then add it as the first value
    if (running_bg_process[0] == 0 && running_bg_process[1] == 0 && running_bg_process[2] == 0 && running_bg_process[3] == 0 && running_bg_process[4] == 0)
    {
        added = 0;
        running_bg_process[0] = pid;
        return added;
    }
    // adding the process only after the last non-zero PID that is a running background process
    for (int i = MAX_BGPROCESS - 2; i >= 0; i--)
    {
        if (running_bg_process[i] != 0)
        {
            running_bg_process[i + 1] = pid;
            added = i + 1;
        }
    }
    return added;
}

int pop(pid_t pid)
{
    // removing the PID of the completed background process and resetting it to 0
    for (int i = 0; i < MAX_BGPROCESS; i++)
    {
        if (running_bg_process[i] == pid)
        {
            running_bg_process[i] = 0;
            return i;
        }
    }
    return -1;
}

// running shell commands
int create_process_and_run(char **args)
{
    int status = fork();
    if (status < 0)
    {
        perror("Fork Failed");
    }
    else if (status == 0)
    {
        int check = execvp(args[0], args);
        if (check == -1)
        {
            printf("%s: command not found\n", args[0]);
            exit(1);
        }
    }
    else
    {
        // Checking for background process
        if (!(bgProcess))
        {
            int child_status;
            // Wait for the child to complete
            wait(&child_status);
            if (WIFEXITED(child_status))
            {
                int exit_code = WEXITSTATUS(child_status);
            }
            else
            {
                printf("Child process did not exit normally.\n");
            }
        }
        else
        {
            // parent doesn't wait for the background child processes to terminate
            int order = append(status);
            if (order != -1)
            {
                history.record[history.historyCount].process_pid = status;
                // start of background process
                printf("[%d] %d\n", order + 1, status);
            }
            else
            {
                history.record[history.historyCount].process_pid = 0;
                printf("No more background processes can be added");
            }
        }
    }
    return status;
}

// launch function
int launch(char **args)
{
    int status;
    status = create_process_and_run(args);
    if (status > 0)
    {
        history.record[history.historyCount].process_pid = status;
    }
    else
    {
        history.record[history.historyCount].process_pid = 0;
    }
    return status;
}

// taking input from the terminal
char *read_user_input()
{
    char *input = (char *)malloc(INPUT_SIZE);
    if (input == NULL)
    {
        perror("Can't allocate memory\n");
        free(input);
        exit(EXIT_FAILURE);
    }
    size_t size = 0;
    int read = getline(&input, &size, stdin);
    if (read != -1)
    {
        return input;
    }
    else
    {
        perror("Error while reading line\n");
        free(input);
    }
}

// function to strip the leading and trailing spaces
char *strip(char *string)
{
    char stripped[strlen(string) + 1];
    int len = 0;
    int flag;
    if (string[0] != ' ')
    {
        flag = 1;
    }
    else
    {
        flag = 0;
    }
    for (int i = 0; string[i] != '\0'; i++)
    {
        if (string[i] != ' ' && flag == 0)
        {
            stripped[len++] = string[i];
            flag = 1;
        }
        else if (flag == 1)
        {
            stripped[len++] = string[i];
        }
        else if (string[i] != ' ')
        {
            flag = 1;
        }
    }
    stripped[len] = '\0';
    char *final_strip = (char *)malloc(INPUT_SIZE);
    if (final_strip == NULL)
    {
        perror("Memory allocation failed\n");
    }
    memcpy(final_strip, stripped, INPUT_SIZE);
    return final_strip;
}

// splitting the command according to specified delimiter
char **tokenize(char *command, const char delim[2])
{
    char **args = (char **)malloc(INPUT_SIZE * sizeof(char *));
    if (args == NULL)
    {
        perror("Memory allocation failed\n");
    }
    int count = 0;
    char *token = strtok(command, delim);
    while (token != NULL)
    {
        args[count++] = strip(token);
        token = strtok(NULL, delim);
    }
    // checking for & as the last argument to check if the process should be in the background
    if (count > 0 && strcmp(args[count - 1], "&") == 0 && strcmp(delim, " ") == 0)
    {
        bgProcess = 1;
        free(args[count - 1]);
        args[count - 1] = NULL;
        count--;
    }
    return args;
}

// Handling SIGCHLD: Passed when a child process terminates (background process in this case)
void handle_sigchld(int signum)
{
    int status;
    pid_t pid;
    // returns the pid of the terminated child
    // WNOHANG: returns 0 if the status information of any process is not available i.e. the process has not terminated
    // In other words, the loop will only run after the background processes running has terminated or changed state
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        // Finding the child process in history according to the PID of the terminated process
        for (int i = 0; i < history.historyCount; i++)
        {
            if (history.record[i].process_pid == pid)
            {
                // finding from background process array
                int order = pop(pid);
                if (order != -1)
                {
                    history.record[i].end_time = time(NULL);
                    history.record[i].duration = difftime(
                        history.record[i].end_time,
                        history.record[i].start_time);
                    // duplicating the command to tmp to avoid corruption of data
                    char *tmp = strdup(history.record[i].command);
                    if (tmp == NULL)
                    {
                        perror("Error in strdup");
                        exit(EXIT_FAILURE);
                    }
                    tmp = strtok(tmp, "&");
                    printf("\n[%d]+ Done                    %s\n", order + 1, tmp);
                    break;
                }
                else
                {
                    // background process not found i.e. it was not added to the array
                    history.record[i].end_time = history.record[i].start_time;
                    history.record[i].duration = difftime(
                        history.record[i].end_time,
                        history.record[i].start_time);
                }
            }
        }
    }
}

// Checking for quotation marks and backslash in the input
bool validate_command(char *command)
{
    if (strchr(command, '\\') || strchr(command, '\"') || strchr(command, '\''))
    {
        return true;
    }
    return false;
}

// main shell loop
void shell_loop()
{
    // Setting the function for SIGINT (Ctrl + C)
    if (signal(SIGINT, my_handler) == SIG_ERR)
    {
        perror("SIGINT handling failed");
    }
    // Setting the function for SIGCHLD
    if (signal(SIGCHLD, handle_sigchld) == SIG_ERR)
    {
        perror("SIGCHLD handling failed");
    }
    int status;
    do
    {
        // Creating the prompt text
        char *user = getenv("USER");
        if (user == NULL)
        {
            perror("USER environment variable not declared");
            exit(1);
        }
        char host[INPUT_SIZE];
        int hostname = gethostname(host, sizeof(host));
        if (hostname == -1)
        {
            perror("HOST not declared");
            exit(1);
        }
        printf("%s@%s~$ ", user, host);

        // taking input
        char *command = read_user_input();
        // handling the case if the input is blank or enter key
        if (strlen(command) == 0 || strcmp(command, "\n") == 0)
        {
            status = 1;
            continue;
        }
        // removing the newline character
        command = strtok(command, "\n");
        bool isInvalidCommand = validate_command(command);
        char *tmp = strdup(command);
        if (tmp == NULL)
        {
            perror("Error in strdup");
            exit(EXIT_FAILURE);
        }
        if (isInvalidCommand)
        {
            status = 1;
            strcpy(history.record[history.historyCount].command, tmp);
            history.record[history.historyCount].start_time = time(NULL);
            history.record[history.historyCount].end_time = time(NULL);
            history.record[history.historyCount].duration = difftime(
                history.record[history.historyCount].end_time,
                history.record[history.historyCount].start_time);
            history.historyCount++;
            printf("Invalid Command : includes quotes/backslash\n");
            continue;
        }
        // checking if the input is "history"
        if (strstr(command, "history"))
        {
            if (history.historyCount > 0)
            {
                strcpy(history.record[history.historyCount].command, tmp);
                history.record[history.historyCount].start_time = time(NULL);
                displayHistory();
                history.record[history.historyCount].end_time = time(NULL);
                history.record[history.historyCount].duration = difftime(
                    history.record[history.historyCount].end_time,
                    history.record[history.historyCount].start_time);
                history.historyCount++;
            }
            else
            {
                status = 1;
                printf("No command in the history\n");
                continue;
            }
        }
        else
        {
            char **args = tokenize(command, " ");
            strcpy(history.record[history.historyCount].command, tmp);
            history.record[history.historyCount].start_time = time(NULL);
            if (strcmp(args[0],"submit")){
                process p;
                p.name = args[1];
                p.state = READY;
                all_process = insert_rear(all_process, allocate_node(p));
            }
            else{
                status = launch(args);
            }         
            history.record[history.historyCount].end_time = time(NULL);
            history.record[history.historyCount].duration = difftime(
                history.record[history.historyCount].end_time,
                history.record[history.historyCount].start_time);
            history.historyCount++;
        }
        // resetting the background process variable
        bgProcess = 0;
    } while (status);
}


// //scheduler
// void scheduler()
// {
//     while (true)
//     {
//         lock(process_table);
//         foreach (Process p
//                  : scheduling_algorithm(process_table))
//         {
//             if (p->state != READY)
//             {
//                 continue;
//             }
//             p->state = RUNNING;
//             unlock(process_table);
//             swtch(scheduler_process, p);
//             // p is done for now..
//             lock(process_table);
//         }
//         unlock(process_table);
//     }
// }



// //scheduling algorithm
// void scheduling_algorithm(){

// }


// //Launch function
// void launch(int c , void scheduler){
        
// }


// Main function
int main(int argc, char *argv[])
{
    NCPU = atoi(argv[1]);
    TSLICE = atof(argv[2]);

    // for (int cpu = 1; cpu < NCPU; cpu++)
    // {
    //     launch(cpu, scheduler);
    // }

    // scheduler();
    // initializing count for elements in history
    history.historyCount = 0;
    shell_loop();
    return 0;
}
