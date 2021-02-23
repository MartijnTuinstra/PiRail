#ifndef INCLUDE_SCHEDULER_H
#define INCLUDE_SCHEDULER_H

#include <pthread.h>
#include <vector>
#include "scheduler/event.h"

bool operator <(const struct timespec lhs, const struct timespec rhs);
struct timespec operator -(const struct timespec lhs, const struct timespec rhs);

class Scheduler{
    private:
        uint8_t _stop_ = 0;
        struct timespec scheduler_wait = {0, 0};
        pthread_t id;
        pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
        pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

        std::vector<struct SchedulerEvent *> events = {};

        static void * thread(void * args);
        void interrupt();

        void removeEvent(uint16_t index);
    public:
        ~Scheduler();
        void start();
        void stop();
        void update();
        void updateClock();

        void enableEvent(struct SchedulerEvent * event);
        void disableEvent(struct SchedulerEvent * event);

        bool hasPassed(struct SchedulerEvent * event);

        struct SchedulerEvent * addEvent(const char * name, struct timespec interval);
        struct SchedulerEvent * addEvent(struct SchedulerEvent event);
        void updateEvent(struct SchedulerEvent * event);
        void removeEvent(struct SchedulerEvent * event);
        void removeEvent(const char * name);

        uint16_t getEvent(struct SchedulerEvent * event);
        uint16_t getEvent(const char * name);

        void print_events();
};


extern Scheduler * scheduler;

#endif