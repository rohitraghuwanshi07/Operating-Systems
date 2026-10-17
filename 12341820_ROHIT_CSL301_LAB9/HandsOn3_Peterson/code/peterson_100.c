#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#define ITERATIONS 100

/*
 * Educational illustration of classic Peterson's Algorithm.
 * IMPORTANT: volatile alone is not a portable synchronization
 * mechanism in modern C. Use a process-shared POSIX mutex or a
 * properly designed platform-supported atomic implementation in
 * production code.
 */
typedef struct {
    volatile int counter;
    volatile int flag[2];
    volatile int turn;
} SharedData;

static void worker(SharedData *sh, int me) {
    int other = 1 - me;

    for (int i = 0; i < ITERATIONS; ++i) {
        /* Entry section */
        sh->flag[me] = 1;
        sh->turn = other;

        while (sh->flag[other] == 1 && sh->turn == other) {
            /* Busy waiting */
        }

        /* Critical section */
        int temp = sh->counter;
        sh->counter = temp + 1;

        /* Exit section */
        sh->flag[me] = 0;
    }
}

int main(void) {
    SharedData *sh = mmap(NULL, sizeof(*sh),
                          PROT_READ | PROT_WRITE,
                          MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sh == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }

    sh->counter = 0;
    sh->flag[0] = 0;
    sh->flag[1] = 0;
    sh->turn = 0;

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        munmap(sh, sizeof(*sh));
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        worker(sh, 1);
        munmap(sh, sizeof(*sh));
        _exit(EXIT_SUCCESS);
    }

    worker(sh, 0);

    if (waitpid(pid, NULL, 0) < 0) {
        perror("waitpid");
        munmap(sh, sizeof(*sh));
        return EXIT_FAILURE;
    }

    printf("Expected counter: %d\n", 2 * ITERATIONS);
    printf("Observed counter: %d\n", sh->counter);

    munmap(sh, sizeof(*sh));
    return EXIT_SUCCESS;
}
