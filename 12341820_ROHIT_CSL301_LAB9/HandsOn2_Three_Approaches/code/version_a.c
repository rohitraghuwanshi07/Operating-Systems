#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#define ITERATIONS 1000
#define DELAY_US 10

typedef struct {
    volatile int counter;
    volatile int turn;
    volatile int flag[2];
} SharedData;

static void worker(SharedData *sh, int me) {
    int other = 1 - me;
    for (int i = 0; i < ITERATIONS; ++i) {
        /* ===== ENTRY SECTION ===== */
        while (sh->turn != me) { }

        /* ===== CRITICAL SECTION ===== */
        int temp = sh->counter;
        usleep(DELAY_US);
        sh->counter = temp + 1;

        /* ===== EXIT SECTION ===== */
        sh->turn = other;
    }
}

int main(void) {
    SharedData *sh = mmap(NULL, sizeof(*sh),
                          PROT_READ | PROT_WRITE,
                          MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sh == MAP_FAILED) { perror("mmap"); return EXIT_FAILURE; }

    /* Initialize BEFORE fork() */
    sh->counter = 0;
    sh->turn = 0;
    sh->flag[0] = 0;
    sh->flag[1] = 0;

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); munmap(sh, sizeof(*sh)); return EXIT_FAILURE; }

    if (pid == 0) {
        worker(sh, 1);
        munmap(sh, sizeof(*sh));
        _exit(EXIT_SUCCESS);
    }

    worker(sh, 0);

    if (waitpid(pid, NULL, 0) < 0) { perror("waitpid"); return EXIT_FAILURE; }

    printf("Expected counter: %d\n", 2 * ITERATIONS);
    printf("Observed counter: %d\n", sh->counter);

    munmap(sh, sizeof(*sh));
    return EXIT_SUCCESS;
}
