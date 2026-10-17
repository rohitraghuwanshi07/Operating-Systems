#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct
{
    int num1;
    int num2;
} Numbers;

typedef struct
{
    int addition;
    int subtraction;
    int multiplication;
} Results;

int main(void)
{
    int pipe_parent_to_child[2];
    int pipe_child_to_parent[2];

    Numbers numbers;
    Results results;

    if (pipe(pipe_parent_to_child) == -1)
    {
        perror("pipe");
        return 1;
    }

    if (pipe(pipe_child_to_parent) == -1)
    {
        perror("pipe");
        return 1;
    }

    printf("========================================\n");
    printf("       PARENT-CHILD PIPE PROGRAM\n");
    printf("========================================\n\n");

    printf("Enter first number: ");
    scanf("%d", &numbers.num1);

    printf("Enter second number: ");
    scanf("%d", &numbers.num2);

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid > 0)
    {
        /* Parent process */

        close(pipe_parent_to_child[0]);
        close(pipe_child_to_parent[1]);

        printf("\n[Parent] Sending numbers to child...\n");

        ssize_t bytes_written = write(
            pipe_parent_to_child[1],
            &numbers,
            sizeof(numbers)
        );

        if (bytes_written != sizeof(numbers))
        {
            perror("write");
            close(pipe_parent_to_child[1]);
            close(pipe_child_to_parent[0]);
            wait(NULL);
            return 1;
        }

        close(pipe_parent_to_child[1]);

        printf("[Parent] Numbers sent successfully.\n");
        printf("[Parent] Waiting for results from child...\n");

        ssize_t bytes_read = read(
            pipe_child_to_parent[0],
            &results,
            sizeof(results)
        );

        if (bytes_read != sizeof(results))
        {
            perror("read");
            close(pipe_child_to_parent[0]);
            wait(NULL);
            return 1;
        }

        close(pipe_child_to_parent[0]);

        wait(NULL);

        printf("\n========================================\n");
        printf("              RESULTS\n");
        printf("========================================\n");
        printf("First Number  : %d\n", numbers.num1);
        printf("Second Number : %d\n", numbers.num2);
        printf("----------------------------------------\n");
        printf("Addition       : %d\n", results.addition);
        printf("Subtraction    : %d\n", results.subtraction);
        printf("Multiplication : %d\n", results.multiplication);
        printf("========================================\n");

        printf("\n[Parent] Results received successfully.\n");
        printf("[Parent] Program completed.\n");
    }
    else
    {
        /* Child process */

        close(pipe_parent_to_child[1]);
        close(pipe_child_to_parent[0]);

        printf("\n[Child] Waiting for numbers from parent...\n");

        ssize_t bytes_read = read(
            pipe_parent_to_child[0],
            &numbers,
            sizeof(numbers)
        );

        if (bytes_read != sizeof(numbers))
        {
            perror("read");
            close(pipe_parent_to_child[0]);
            close(pipe_child_to_parent[1]);
            exit(1);
        }

        close(pipe_parent_to_child[0]);

        printf("[Child] Numbers received: %d and %d\n",
               numbers.num1,
               numbers.num2);

        printf("[Child] Performing arithmetic operations...\n");

        results.addition = numbers.num1 + numbers.num2;
        results.subtraction = numbers.num1 - numbers.num2;
        results.multiplication = numbers.num1 * numbers.num2;

        printf("[Child] Calculations completed.\n");
        printf("[Child] Sending results to parent...\n");

        ssize_t bytes_written = write(
            pipe_child_to_parent[1],
            &results,
            sizeof(results)
        );

        if (bytes_written != sizeof(results))
        {
            perror("write");
            close(pipe_child_to_parent[1]);
            exit(1);
        }

        close(pipe_child_to_parent[1]);

        printf("[Child] Results sent successfully.\n");
        printf("[Child] Terminating.\n");

        exit(0);
    }

    return 0;
}
