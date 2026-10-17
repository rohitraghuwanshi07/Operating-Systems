#ifndef SCHEDULER_SHARED_H
#define SCHEDULER_SHARED_H

#define MAX_JOBS 5

typedef struct {
    int job_id;
    int priority;
    int cpu_burst;
    int active;
} Job;

typedef struct {
    Job jobs[MAX_JOBS];
    int job_count;
    int generator_done;
} SharedData;

#endif
