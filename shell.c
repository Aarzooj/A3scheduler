#include "scheduler.h"
#include "SimpleScheduler.c"
#include "timer.c"
#include "queue.c"

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

void sigchld_handler(int signum)
{
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        for (int i = front_r; i < rear_r + 1; i++)
        {
            if (running_queue[i]->pid == pid)
            {
                if (clock_gettime(CLOCK_MONOTONIC, &running_queue[i]->end_time) == -1)
                {
                    perror("clock_gettime");
                    exit(EXIT_FAILURE);
                }
                printf("Process terminated: %s\n", running_queue[i]->name);
                running_queue[i]->execution_time += ((running_queue[i]->end_time.tv_sec - running_queue[i]->start_time.tv_sec) * 1000) + (running_queue[i]->end_time.tv_nsec + running_queue[i]->start_time.tv_nsec) / 1e6;
                for (int j = i; j < rear_r; j++)
                {
                    running_queue[j] = running_queue[j + 1];
                }
                running_queue[rear_r] = NULL;
                rear_r--;
                break;
            }
        }
    }
}

void sigusr_handler(int signum)
{
    scheduler(NCPU, TSLICE);
}

// main shell loop
void shell_loop()
{
    // Setting the function for SIGINT (Ctrl + C)
    if (signal(SIGINT, my_handler) == SIG_ERR)
    {
        perror("SIGINT handling failed");
    }
    if (signal(SIGUSR1, sigusr_handler) == SIG_ERR)
    {
        perror("SIGUSR1 handling failed");
    }
    if (set_sigalrm(SIGALRM, SA_RESTART, sigalrm_handler) == -1)
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
            kill(getpid(), SIGUSR1);
        }
        else
        {
            char **args = tokenize(command, " ");
            strcpy(history.record[history.historyCount].command, tmp);
            history.record[history.historyCount].start_time = time(NULL);
            if (strcmp(args[0], "submit") == 0)
            {

                pid_t pid = fork();
                if (pid == 0)
                { // Child process
                    // Wait for the scheduler signal before starting execution
                    printf("Process continued: %s\n", args[1]);
                    char *temp[2] = {args[1], NULL};
                    execvp(args[1], temp);
                    fprintf(stderr, "Error executing %s\n", args[1]);
                    exit(1);
                }
                else if (pid > 0)
                { // Parent process
                    // Add the process to the list
                    kill(pid, SIGSTOP);
                    process *p = create_process(args[1]);
                    p->pid = pid;
                    add_process(p);
                    add_process_table(p);
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
    // initializing count for elements in history
    history.historyCount = 0;
    shell_loop();
    return 0;
}