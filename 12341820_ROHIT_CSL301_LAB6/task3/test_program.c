#include <stdio.h>

int main(void) {
    printf("Hello from the remote C program!\n");
    printf("This program was compiled on the server.\n");
    printf("Execution was performed using fork() and exec().\n");
    printf("Output was captured using a pipe and sent back to the client.\n");
    return 0;
}
