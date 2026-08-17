#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        printf("I am child. PID = %d\n", getpid());
    } else {
        printf("I am parent. PID = %d\n", getpid());
        wait(NULL);
    }

    return 0;
}
