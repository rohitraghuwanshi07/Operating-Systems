#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

volatile sig_atomic_t process_state = 1;
volatile sig_atomic_t running = 1;

void handle_sigusr1(int sig) {
    (void)sig;
    process_state = 0;
}

void handle_sigusr2(int sig) {
    (void)sig;
    process_state = 1;
}

void handle_sigint(int sig) {
    (void)sig;
    const char msg[] = "\nSIGINT received. Process will NOT terminate.\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
}

void handle_sigterm(int sig) {
    (void)sig;
    running = 0;
}

int main(void) {
    if (signal(SIGUSR1, handle_sigusr1) == SIG_ERR ||
        signal(SIGUSR2, handle_sigusr2) == SIG_ERR ||
        signal(SIGINT,  handle_sigint)  == SIG_ERR ||
        signal(SIGTERM, handle_sigterm) == SIG_ERR) {
        perror("signal");
        return 1;
    }

    printf("========================================\n");
    printf("     SIGNAL-BASED PROCESS CONTROL\n");
    printf("========================================\n");
    printf("Process started successfully.\n");
    printf("PID: %d\n", getpid());
    printf("\nSignal controls:\n");
    printf("SIGUSR1 -> Pause process\n");
    printf("SIGUSR2 -> Resume process\n");
    printf("SIGINT  -> Display warning, do not terminate\n");
    printf("SIGTERM -> Terminate gracefully\n");
    printf("\nCurrent state: RUNNING\n");
    printf("========================================\n");
    fflush(stdout);

    while (running) {
        printf("PID: %d | State: %s | Process is %s.\n",
               getpid(),
               process_state ? "RUNNING" : "PAUSED",
               process_state ? "active" : "paused");
        fflush(stdout);
        sleep(1);
    }

    printf("\nSIGTERM received. Terminating gracefully.\n");
    printf("PID: %d | Final state: %s\n",
           getpid(), process_state ? "RUNNING" : "PAUSED");
    fflush(stdout);
    return 0;
}
