#ifndef INCLUDE_SCHEDULER_EVENT_H
#define INCLUDE_SCHEDULER_EVENT_H

#include <time.h>

struct SchedulerEvent {
    struct timespec interval;
    void (*function)(void *);
    void * function_args;

    char name[64];
    bool disabled;

    struct timespec next_interval;
};

#endif