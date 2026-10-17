#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#include "scheduler_shared.h"

#define KEY_FILE "scheduler_key"
#define PROJECT_ID 'J'

int main(void)
{
    int fd = open(KEY_FILE, O_CREAT | O_RDWR, 0666);
    if (fd == -1)
    {
        perror("open");
        return 1;
    }
    close(fd);

    key_t key = ftok(KEY_FILE, PROJECT_ID);
    if (key == -1)
    {
        perror("ftok");
        return 1;
    }

    int shmid = shmget(key, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid == -1)
    {
        perror("shmget");
        return 1;
    }

    SharedData *shared = (SharedData *)shmat(shmid, NULL, 0);

    if (shared == (void *)-1)
    {
        perror("shmat");
        return 1;
    }

    shared->job_count = 0;
    shared->generator_done = 0;

    printf("========================================\n");
    printf("        JOB GENERATOR\n");
    printf("========================================\n");
    printf("Shared memory capacity: %d jobs\n\n", MAX_JOBS);

    int total_jobs;

    printf("Enter total number of jobs (1-%d): ", MAX_JOBS);
    scanf("%d", &total_jobs);

    if (total_jobs < 1 || total_jobs > MAX_JOBS)
    {
        printf("Invalid number of jobs.\n");
        printf("The shared memory can hold a maximum of %d jobs.\n", MAX_JOBS);

        shmdt(shared);
        shmctl(shmid, IPC_RMID, NULL);

        return 1;
    }

    printf("\nEnter job details:\n");
    printf("(Higher priority number = higher priority)\n\n");

    for (int i = 0; i < total_jobs; i++)
    {
        printf("Job %d\n", i + 1);

        printf("  Job ID       : ");
        scanf("%d", &shared->jobs[i].job_id);

        printf("  Priority     : ");
        scanf("%d", &shared->jobs[i].priority);

        printf("  CPU Burst    : ");
        scanf("%d", &shared->jobs[i].cpu_burst);

        shared->jobs[i].active = 1;

        shared->job_count++;

        printf("\n");
    }

    shared->generator_done = 1;

    printf("========================================\n");
    printf("All %d jobs stored in shared memory.\n", total_jobs);
    printf("Waiting for Scheduler to process jobs...\n");
    printf("========================================\n");

    /*
     * Wait until the Scheduler marks all jobs inactive.
     */
    while (1)
    {
        int remaining = 0;

        for (int i = 0; i < shared->job_count; i++)
        {
            if (shared->jobs[i].active == 1)
            {
                remaining++;
            }
        }

        if (remaining == 0)
        {
            break;
        }

        sleep(1);
    }

    printf("\nScheduler has completed all jobs.\n");
    printf("Job Generator terminating.\n");

    shmdt(shared);

    return 0;
}
