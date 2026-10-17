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
} SharedData;

static void increment_without_lock(SharedData *sh) {
    for (int i = 0; i < ITERATIONS; ++i) {
        int temp = sh->counter;
        usleep(DELAY_US);  /* Makes interleaving easier to observe */
        sh->counter = temp + 1;
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

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        munmap(sh, sizeof(*sh));
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        increment_without_lock(sh);
        munmap(sh, sizeof(*sh));
        _exit(EXIT_SUCCESS);
    }

    increment_without_lock(sh);

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
