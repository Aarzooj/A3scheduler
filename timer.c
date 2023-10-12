// Reference: https://gist.github.com/ivan-guerra/8366162f90970490d866617021867ae1

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

/*!
 * \brief Write "hello world" to STDOUT.
 * \param sig Signal number (i.e., SIGALRM).
 */


/*!
 * \brief Call perror() with argument message \a msg and exit with EXIT_FAILURE.
 * \param msg Error message to be output by perror().
 */

/*!
 * \brief Define this process' action on the signal \a sig.
 * \param sig     The signal number of the signal for which an action is being
 *                registered.
 * \param flags   sigaction() flags (see man sigaction).
 * \param handler Signal handler function.
 * \return 0 on success, -1 on failure. Call perror() for failure details.
 */


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


timer_t timer;

static void sigalrm_handler(int sig)
{
    (void)sig; /* Avoid compiler warning -Werror-unused-parameter. */

    /* Do interesting thing here... */
    int start = front_r;
    int end = rear_r + 1;
    for (int i = start; i < end; i++)
    {
        printf("RUNNING QUEUE --->\n");
        for (int j = front_r; j < rear_r + 1; j++)
        {
            printf("%s %d\n", running_queue[j]->name, running_queue[j]->state);
        }
        printf("Stopped process: %s\n", running_queue[i]->name);
        running_queue[i]->execution_time += TSLICE;
        kill(running_queue[i]->pid, SIGSTOP);
        process *p = remove_process_r(running_queue[i]);
        p->state = READY;
        add_process(p);
    }
    if (is_empty()){
        printf("CPU CYCLE: %d\n",++CPU_CYCLES);
        CPU_CYCLES = 0;
        displayProcesses();
        shell_loop();
    }
    else{
        printf("CPU CYCLE: %d\n",++CPU_CYCLES);
        timer_delete(timer);
        scheduler(NCPU,TSLICE);
    }
}

static int set_sigalrm(int sig, int flags, void (*handler)(int))
{
    struct sigaction action;
    action.sa_flags = flags;
    action.sa_handler = handler;
    return sigaction(sig, &action, NULL);
}

static int timer_handler(long long frequency_nsec)
{
    struct sigevent notif;
    struct itimerspec alarm;
    notif.sigev_notify = SIGEV_SIGNAL;
    notif.sigev_signo = SIGALRM;
    notif.sigev_value.sival_ptr = &timer;
    if (timer_create(CLOCK_REALTIME, &notif, &timer)==-1)
        return -1;

    static const long long SEC_TO_NSEC = 1000000000LL;
    alarm.it_value.tv_sec = frequency_nsec / SEC_TO_NSEC;
    alarm.it_value.tv_nsec = frequency_nsec % SEC_TO_NSEC;
    alarm.it_interval.tv_sec = 0;
    alarm.it_interval.tv_nsec = 0;
    if (timer_settime(timer, 0, &alarm, NULL)==-1)
        return -1;
    return 0;
}