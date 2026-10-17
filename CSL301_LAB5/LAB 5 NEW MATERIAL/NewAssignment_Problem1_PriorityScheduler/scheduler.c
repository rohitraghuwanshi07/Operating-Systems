#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#include "scheduler_shared.h"

#define KEY_FILE "scheduler_key"
#define PROJECT_ID 'J'

int main(void)
{
    /*
     * Generate the same key used by the Job Generator.
     */
    key_t key = ftok(KEY_FILE, PROJECT_ID);

    if (key == -1)
    {
        perror("ftok");
        return 1;
    }

    /*
     * Access the existing shared memory segment.
     */
    int shmid = shmget(key, sizeof(SharedData), 0666);

    if (shmid == -1)
    {
        perror("shmget");
        return 1;
    }

    /*
     * Attach shared memory.
     */
    SharedData *shared = (SharedData *)shmat(shmid, NULL, 0);

    if (shared == (void *)-1)
    {
        perror("shmat");
        return 1;
    }

    printf("========================================\n");
    printf("            JOB SCHEDULER\n");
    printf("========================================\n");

    /*
     * Wait until the Job Generator has finished
     * writing all jobs into shared memory.
     */
    printf("Waiting for Job Generator...\n");

    while (shared->generator_done == 0)
    {
        sleep(1);
    }

    printf("Job Generator has completed job creation.\n");
    printf("Total jobs received: %d\n\n", shared->job_count);

    printf("========================================\n");
    printf("       PRIORITY SCHEDULING\n");
    printf("========================================\n");

    /*
     * Process jobs according to priority.
     *
     * Higher priority number = higher priority.
     */
    int completed_jobs = 0;

    while (completed_jobs < shared->job_count)
    {
        int selected = -1;

        /*
         * Find the active job with the highest priority.
         */
        for (int i = 0; i < shared->job_count; i++)
        {
            if (shared->jobs[i].active == 1)
            {
                if (selected == -1 ||
                    shared->jobs[i].priority >
                    shared->jobs[selected].priority)
                {
                    selected = i;
                }
            }
        }

        /*
         * No active job found.
         */
        if (selected == -1)
        {
            break;
        }

        Job *job = &shared->jobs[selected];

        printf("\n----------------------------------------\n");
        printf("Executing Job\n");
        printf("Job ID       : %d\n", job->job_id);
        printf("Priority     : %d\n", job->priority);
        printf("CPU Burst    : %d\n", job->cpu_burst);
        printf("----------------------------------------\n");

        /*
         * Simulate CPU execution.
         * Each burst unit is represented by 0.2 seconds.
         */
        printf("CPU executing");

        for (int i = 0; i < job->cpu_burst; i++)
        {
            printf(".");
            fflush(stdout);
            usleep(200000);
        }

        printf(" Done!\n");

        /*
         * Mark the job as completed.
         */
        job->active = 0;
        completed_jobs++;

        printf("Job %d completed.\n", job->job_id);
        printf("Jobs completed: %d/%d\n",
               completed_jobs,
               shared->job_count);
    }

    printf("\n========================================\n");
    printf("All jobs have been completed.\n");
    printf("Priority Scheduler terminating.\n");
    printf("========================================\n");

    /*
     * Detach shared memory.
     */
    if (shmdt(shared) == -1)
    {
        perror("shmdt");
        return 1;
    }

    /*
     * Remove the shared memory segment.
     */
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
    {
        perror("shmctl");
        return 1;
    }

    printf("Shared memory removed successfully.\n");

    return 0;
}
