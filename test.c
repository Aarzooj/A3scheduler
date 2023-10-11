// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <sys/wait.h>
// #include <time.h>
// #include <stdbool.h>

// void timer_handler(int signum) {
//     printf("Timer expired!\n");
// }

// void create_timer(){
//     struct sigevent se;
//     timer_t timer;
//     struct itimerspec its;
//     long long milliseconds = 3000;

//     // Create a timer
//     se.sigev_notify = SIGEV_SIGNAL;
//     se.sigev_signo = SIGALRM;
//     // sev.sigev_notify_function = timer_handler;
//     // sev.sigev_value.sival_ptr = &timerid;
//     se.sigev_value.sival_ptr = &timer;
//     se.sigev_value.sival_int = 0;
//     if (timer_create(CLOCK_REALTIME, &se, &timer) == -1) {
//         perror("timer_create");
//         exit(1);
//     }

//     // Set the timer to expire in milliseconds
//     its.it_value.tv_sec = milliseconds / 1000; // Whole seconds
//     its.it_value.tv_nsec = (milliseconds % 1000) * 1000000; // Nanoseconds for the remaining milliseconds
//     its.it_interval.tv_sec = 0; // Non-repeating timer
//     its.it_interval.tv_nsec = 0;

//     if (timer_settime(timer, TIMER_ABSTIME, &its,NULL) == -1) {
//         perror("timer_settime");
//         exit(1);
//     }
//     printf("Timer created");
// }

// int main() {

//     return 0
// }

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

#include <unistd.h>
#include <signal.h>
#include <semaphore.h>

// #include "try.c"
// #include "scheduler.h"

sem_t exit_mutex; /* Semaphore mutex used to signal between main() and sigint_handler(). */

/*!
 * \brief SIGINT interrupt handler.
 * \details sigint_handler() interacts with the program main() function via
 *          a semaphore and the global mutex, exit_mutex. main() calls
 *          sem_wait(&exit_mutex) to essentially put the process in a sleeping
 *          state. sigint_handler() will sem_post(&exit_mutex) causing the
 *          sem_wait() in main() to return and effectively causing the program
 *          to exit.
 * \param sig Signal number (i.e., SIGINT).
 */
static void sigint_handler(int sig);

/*!
 * \brief Write "hello world" to STDOUT.
 * \param sig Signal number (i.e., SIGALRM).
 */
static void sigalrm_handler(int sig);

/*!
 * \brief Call perror() with argument message \a msg and exit with EXIT_FAILURE.
 * \param msg Error message to be output by perror().
 */
static void print_error_and_exit(const char *msg);

/*!
 * \brief Define this process' action on the signal \a sig.
 * \param sig     The signal number of the signal for which an action is being
 *                registered.
 * \param flags   sigaction() flags (see man sigaction).
 * \param handler Signal handler function.
 * \return 0 on success, -1 on failure. Call perror() for failure details.
 */
static int init_action(int sig, int flags, void (*handler)(int));

/*!
 * \brief Create an interval timer that triggers every \a frequency_nsec nsec.
 * \details init_interval_timer() constructs an interval timer that triggers
 *          at the parameter frequency. At each timer expiration, the SIGALRM
 *          will be sent causing the sigalrm_handler() handler to be triggered
 *          if sigalrm_handler() was previously registered via a call to
 *          init_action() (or directly through sigaction()).
 * \param frequency_nsec The frequency at which the timer will trigger in
 *                       nanoseconds.
 * \return 0 on success, -1 on failure. Call perror() for failure details.
 */
static int init_interval_timer(long long frequency_nsec);

// static void sigint_handler(int sig)
// {
//     (void)sig; /* Avoid compiler warning -Werror-unused-parameter. */

//     /* User hit CTRL-c meaning they want to exit the program. Post so that
//        main() becomes unblocked and can exit neatly. */
//     sem_post(&exit_mutex);
//     if (-1 == sem_post(&exit_mutex))
//     {
//         write(STDERR_FILENO, "sem_post() failed\n", 18);
//         _exit(EXIT_FAILURE);
//     }
// }

// void scheduler(int ncpu, int tslice);

static void sigalrm_handler(int sig)
{
    (void)sig; /* Avoid compiler warning -Werror-unused-parameter. */

    /* Do interesting thing here... */
    int start = front_r;
    int end = rear_r + 1;
    for (int i = start; i < end; i++)
    {
        if (running_queue[i]->pid != -1)
        {
            printf("RUNNING QUEUE --->\n");
            for (int j = front_r; j < rear_r + 1; j++)
            {
                printf("%s %d\n", running_queue[j]->name,running_queue[j]->state);
            }
            printf("Stopped process: %s\n", running_queue[i]->name);
            kill(running_queue[i]->pid, SIGSTOP);
            process *p = remove_process_r(running_queue[i]);
            p->state = READY;
            add_process(p);
        }
    }
    // write(STDOUT_FILENO, "Timer expired\n", 15);
}

static void print_error_and_exit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

static int init_action(int sig, int flags, void (*handler)(int))
{
    struct sigaction action;
    action.sa_flags = flags;
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);

    return sigaction(sig, &action, NULL);
}

static int init_interval_timer(long long frequency_nsec)
{
    timer_t timerid;
    struct sigevent evp;
    evp.sigev_notify = SIGEV_SIGNAL;
    evp.sigev_signo = SIGALRM;
    evp.sigev_value.sival_ptr = &timerid;
    if (-1 == timer_create(CLOCK_REALTIME, &evp, &timerid))
        return -1;

    static const long long SEC_TO_NSEC = 1000000000LL;
    struct itimerspec alarm;
    alarm.it_value.tv_sec = frequency_nsec / SEC_TO_NSEC;
    alarm.it_value.tv_nsec = frequency_nsec % SEC_TO_NSEC;
    alarm.it_interval.tv_sec = frequency_nsec / SEC_TO_NSEC;
    alarm.it_interval.tv_nsec = frequency_nsec % SEC_TO_NSEC;
    if (-1 == timer_settime(timerid, 0, &alarm, NULL))
        return -1;

    printf("Timer created\n");
    return 0;
}