#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256

int main(void)
{
    int parent_to_child[2];
    int child_to_parent[2];

    pid_t pid;

    char message[BUFFER_SIZE];
    char acknowledgement[BUFFER_SIZE];

    /*
     * Create first pipe:
     * Parent -> Child
     */
    if (pipe(parent_to_child) == -1)
    {
        perror("pipe");
        return 1;
    }

    /*
     * Create second pipe:
     * Child -> Parent
     */
    if (pipe(child_to_parent) == -1)
    {
        perror("pipe");
        return 1;
    }

    /*
     * Create child process.
     */
    pid = fork();

    if (pid == -1)
    {
        perror("fork");
        return 1;
    }

    /*
     * ==============================
     * CHILD PROCESS
     * ==============================
     */
    if (pid == 0)
    {
        /*
         * Child only reads from
         * parent_to_child.
         */
        close(parent_to_child[1]);

        /*
         * Child only writes to
         * child_to_parent.
         */
        close(child_to_parent[0]);

        printf("\n============================================\n");
        printf("              CHILD PROCESS\n");
        printf("============================================\n");

        while (1)
        {
            ssize_t bytes_read;

            /*
             * Read message from parent.
             */
            bytes_read = read(
                parent_to_child[0],
                message,
                BUFFER_SIZE - 1
            );

            if (bytes_read <= 0)
            {
                break;
            }

            message[bytes_read] = '\0';

            /*
             * Check termination message.
             */
            if (strcmp(message, "exit") == 0)
            {
                printf("\nChild received termination request.\n");
                break;
            }

            printf("\nMessage received from Parent:\n");
            printf(">>> %s\n", message);

            /*
             * Send acknowledgement.
             */
            strcpy(
                acknowledgement,
                "ACK: Message received successfully."
            );

            write(
                child_to_parent[1],
                acknowledgement,
                strlen(acknowledgement) + 1
            );

            printf("Acknowledgement sent to Parent.\n");
        }

        /*
         * Close child pipe ends.
         */
        close(parent_to_child[0]);
        close(child_to_parent[1]);

        printf("\nChild process terminated.\n");

        exit(0);
    }

    /*
     * ==============================
     * PARENT PROCESS
     * ==============================
     */

    /*
     * Parent only writes to
     * parent_to_child.
     */
    close(parent_to_child[0]);

    /*
     * Parent only reads from
     * child_to_parent.
     */
    close(child_to_parent[1]);

    printf("\n============================================\n");
    printf("              PARENT PROCESS\n");
    printf("============================================\n");

    printf("Parent PID: %d\n", getpid());
    printf("Child PID : %d\n", pid);

    while (1)
    {
        printf("\nEnter message (type 'exit' to terminate): ");
        fflush(stdout);

        if (fgets(
                message,
                BUFFER_SIZE,
                stdin
            ) == NULL)
        {
            break;
        }

        /*
         * Remove newline.
         */
        message[strcspn(message, "\n")] = '\0';

        /*
         * Send message to child.
         */
        write(
            parent_to_child[1],
            message,
            strlen(message) + 1
        );

        /*
         * Termination message.
         */
        if (strcmp(message, "exit") == 0)
        {
            printf("\nTermination message sent to Child.\n");
            break;
        }

        /*
         * Wait for acknowledgement.
         */
        if (read(
                child_to_parent[0],
                acknowledgement,
                BUFFER_SIZE - 1
            ) > 0)
        {
            acknowledgement[BUFFER_SIZE - 1] = '\0';

            printf("\nAcknowledgement from Child:\n");
            printf(">>> %s\n", acknowledgement);
        }
    }

    /*
     * Close parent pipe ends.
     */
    close(parent_to_child[1]);
    close(child_to_parent[0]);

    /*
     * Wait for child to terminate.
     */
    waitpid(pid, NULL, 0);

    printf("\n============================================\n");
    printf("Parent process terminated.\n");
    printf("Communication completed successfully.\n");
    printf("============================================\n\n");

    return 0;
}

