#ifndef INCLUDE_SCHEDULER_EVENT_H
#define INCLUDE_SCHEDULER_EVENT_H

#include <time.h>

enum SchedulerType {
    SCHEDULER_TYPE_PERIODIC,
    SCHEDULER_TYPE_ONESHOT
};

struct SchedulerEvent {
    enum SchedulerType type;

    struct timespec interval;
    void (*function)(void *);
    void * function_args;

    char name[64];
    bool disabled;

    struct timespec next_interval;
};

#endif