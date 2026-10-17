#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "event_shared.h"

int main(void)
{
    key_t key;
    int shmid;
    SharedEvent *shared;
    int event_id = 1;

    /* Create the file required by ftok() */
    FILE *fp = fopen(KEY_FILE, "a");

    if (fp == NULL)
    {
        perror("fopen");
        return 1;
    }

    fclose(fp);

    /* Generate System V IPC key */
    key = ftok(KEY_FILE, PROJECT_ID);

    if (key == -1)
    {
        perror("ftok");
        return 1;
    }

    /* Create/access shared memory */
    shmid = shmget(key, sizeof(SharedEvent), IPC_CREAT | 0666);

    if (shmid == -1)
    {
        perror("shmget");
        return 1;
    }

    /* Attach shared memory */
    shared = (SharedEvent *)shmat(shmid, NULL, 0);

    if (shared == (void *)-1)
    {
        perror("shmat");
        return 1;
    }

    /*
     * Generator is active.
     */
    shared->generator_terminated = 0;

    printf("\n============================================\n");
    printf("          EVENT GENERATOR STARTED\n");
    printf("============================================\n");

    printf("Enter EXIT as event type to terminate.\n");

    while (1)
    {
        /*
         * IMPORTANT:
         * Wait until the monitor processes the
         * previous event.
         *
         * event_available = 1 means the previous
         * event is still waiting to be processed.
         */
        while (shared->event_available == 1)
        {
            usleep(100000);
        }

        printf("\nEnter Event Type: ");
        fflush(stdout);

        if (fgets(shared->event_type,
                  MAX_EVENT_TYPE,
                  stdin) == NULL)
        {
            break;
        }

        /* Remove newline */
        shared->event_type[
            strcspn(shared->event_type, "\n")
        ] = '\0';

        /*
         * Check whether the user wants to terminate.
         */
        if (strcmp(shared->event_type, "EXIT") == 0)
        {
            shared->generator_terminated = 1;

            printf("\nTermination requested.\n");

            break;
        }

        printf("Enter Event Message: ");
        fflush(stdout);

        if (fgets(shared->event_message,
                  MAX_EVENT_MESSAGE,
                  stdin) == NULL)
        {
            break;
        }

        /* Remove newline */
        shared->event_message[
            strcspn(shared->event_message, "\n")
        ] = '\0';

        /* Assign event ID */
        shared->event_id = event_id++;

        /*
         * Inform the monitor that a new event
         * is available.
         */
        shared->event_available = 1;

        printf("\nEvent %d sent successfully.\n",
               shared->event_id);
    }

    /* Detach shared memory */
    if (shmdt(shared) == -1)
    {
        perror("shmdt");
    }
    else
    {
        printf("shmdt() successful.\n");
    }

    printf("Event Generator terminated.\n\n");

    return 0;
}
