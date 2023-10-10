#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <stdbool.h>
// #include <librt.h>

#include "dummy_main.h"
#include "try.c"

// defining sizes for data structures allocated
#define INPUT_SIZE 256
#define HISTORY_SIZE 100
#define MAX_PROGRAM_NAME 50

int NCPU;
int TSLICE;

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
    return args;
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

void sigchld_handler(int signum){

}

void timer_handler(int signum) {
    for (int i = 0; i < rear_r-front_r+1; i++){
        kill(running_queue[i]->pid,SIGSTOP);
        process* p = remove_process_r(running_queue[i]);
        p->state =  READY;
        add_process(p);
    }
}

void create_timer(){
    struct sigevent se;
    timer_t timer;
    struct itimerspec its;
    long long milliseconds = TSLICE;

    // Create a timer
    se.sigev_notify = SIGEV_SIGNAL;
    se.sigev_signo = SIGALRM;
    se.sigev_value.sival_ptr = &timer;
    se.sigev_value.sival_int = 0;
    if (timer_create(CLOCK_REALTIME, &se, &timer) == -1) {
        perror("timer_create");
        exit(1);
    }

    // Get the current time
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);

    // Calculate the absolute time at which the timer should expire
    its.it_value.tv_sec = now.tv_sec + (milliseconds / 1000);
    its.it_value.tv_nsec = (now.tv_nsec + (milliseconds % 1000) * 1000000) % 1000000000;

    its.it_interval.tv_sec = 0; // Non-repeating timer
    its.it_interval.tv_nsec = 0;

    if (timer_settime(timer, TIMER_ABSTIME, &its, NULL) == -1) {
        perror("timer_settime");
        exit(1);
    }

    // Print the message "Timer created"
    printf("Timer created\n");
}

void scheduler(int ncpu, int tslice)
{
    // create_timer();
    // printf("In scheduler\n");
    // for (int i = front; i < num_processes; i++)
    // {
    //     sleep(1);
    //     printf("%d %s %d %f\n", ready_queue[i]->pid, ready_queue[i]->name, ready_queue[i]->state,ready_queue[i]->start_time.tv_nsec/1e9);
    // }
    for (int i = front; i < front + ncpu; i++){
        create_timer();
        kill(ready_queue[i]->pid,SIGCONT);
        process* p = remove_process(ready_queue[i]);
        p->state = RUNNING;
        add_process_r(p);
    }
}

// main shell loop
void shell_loop()
{
    // Setting the function for SIGINT (Ctrl + C)
    if (signal(SIGINT, my_handler) == SIG_ERR)
    {
        perror("SIGINT handling failed");
    }
    if (signal(SIGALRM, timer_handler) == SIG_ERR)
    {
        perror("SIGALRM handling failed");
    }
    if (signal(SIGCHLD, sigchld_handler) == SIG_ERR)
    {
        perror("SIGCHLD handling failed");
    }
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
    int status;
    do
    {
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
        else if (strstr(command, "run"))
        {
            scheduler(NCPU, TSLICE);
        }
        else
        {
            char **args = tokenize(command, " ");
            strcpy(history.record[history.historyCount].command, tmp);
            history.record[history.historyCount].start_time = time(NULL);
            if (strcmp(args[0], "submit") == 0)
            {
                char program[MAX_PROGRAM_NAME];
                sscanf(command, "submit %s", program);

                pid_t pid = fork();
                if (pid == 0)
                { // Child process
                    // Wait for the scheduler signal before starting execution
                    raise(SIGSTOP);
                    // execl(program, program, (char *)NULL);
                    char* temp[1] = {args[1]};
                    execvp(args[1],temp);
                    fprintf(stderr, "Error executing %s\n", program);
                    exit(1);
                }
                else if (pid > 0)
                { // Parent process
                    // Add the process to the list
                    // strcpy(ready_queue[num_processes].name, program);
                    // ready_queue[num_processes].pid = pid;
                    // ready_queue[num_processes].start_time = time(NULL);
                    process *p = create_process(args[1]);
                    p->pid = pid;
                    // p->start_time = time(NULL);
                    if (clock_gettime(CLOCK_MONOTONIC, &p->start_time) == -1)
                    {
                        perror("clock_gettime");
                        exit(EXIT_FAILURE);
                    }
                    add_process(p);
                    num_processes++;
                }
                else
                {
                    fprintf(stderr, "Fork failed\n");
                    exit(1);
                }
                status = pid;
                if (status > 0)
                {
                    history.record[history.historyCount].process_pid = status;
                }
                else
                {
                    history.record[history.historyCount].process_pid = 0;
                }
            }
            else
            {
                status = launch(args);
            }
            history.record[history.historyCount].end_time = time(NULL);
            history.record[history.historyCount].duration = difftime(
                history.record[history.historyCount].end_time,
                history.record[history.historyCount].start_time);
            history.historyCount++;
        }
    } while (status);
}

// Main function
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <NCPU> <TSLICE>\n", argv[0]);
        exit(1);
    }
    NCPU = atoi(argv[1]);
    TSLICE = atoi(argv[2]);
    // printf("%d\n", NCPU);
    // printf("%d\n", TSLICE);
    // initializing count for elements in history
    history.historyCount = 0;
    shell_loop();
    return 0;
}