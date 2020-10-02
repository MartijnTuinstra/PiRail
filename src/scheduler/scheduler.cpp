#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <string.h>
#include <sys/time.h>
#include <pthread.h>

#include "scheduler/scheduler.h"
#include "utils/logger.h"
#include "utils/mem.h"

bool operator <(const struct timespec lhs, const struct timespec rhs)
{
    if (lhs.tv_sec == rhs.tv_sec)
        return lhs.tv_nsec < rhs.tv_nsec;
    else
        return lhs.tv_sec < rhs.tv_sec;
}

void Scheduler::start(){
    pthread_create(&id, NULL, &thread, this); 
}

void Scheduler::stop(){
    _stop_ = 1;
    interrupt();
    pthread_join(id, NULL);
}

void Scheduler::update(){
    interrupt();
}

void Scheduler::interrupt(){
    pthread_cond_signal(&condition);
}

void * Scheduler::thread(void * args){
    Scheduler * context = (Scheduler *)args;

    while( context->_stop_ == 0 ){
        clock_gettime(CLOCK_REALTIME, &context->scheduler_wait);

        for(auto &e: context->events){
            if(e->next_interval < context->scheduler_wait){
                if(!e->disabled && e->function)
                    e->function(e->function_args);
                context->updateEvent(e);
            }
        }

        bool set = 0;
        if(context->events.size() > 0){
            for(auto &e: context->events){
                if(!e->disabled && (e->next_interval < context->scheduler_wait || !set)){
                    context->scheduler_wait.tv_sec  = e->next_interval.tv_sec;
                    context->scheduler_wait.tv_nsec = e->next_interval.tv_nsec;
                    set = 1;
                }
            }
        }
        else{
            context->scheduler_wait.tv_sec += 1;
        }

        pthread_mutex_lock(&context->mutex);
        pthread_cond_timedwait(&context->condition, &context->mutex, &context->scheduler_wait);
        pthread_mutex_unlock(&context->mutex);
    }

    printf("Scheduler done\n");

    return NULL;
}

void Scheduler::enableEvent(struct SchedulerEvent * event){
    event->disabled = 0;

    struct timespec currenttime;
    clock_gettime(CLOCK_REALTIME, &currenttime);

    if (currenttime.tv_nsec + event->interval.tv_nsec >= 1E9) {
        event->next_interval.tv_sec = currenttime.tv_sec  + event->interval.tv_sec  + 1;
        event->next_interval.tv_nsec = currenttime.tv_nsec + event->interval.tv_nsec - 1E9;
    }
    else{
        event->next_interval.tv_sec = currenttime.tv_sec  + event->interval.tv_sec;
        event->next_interval.tv_nsec = currenttime.tv_nsec + event->interval.tv_nsec;
    }

    update();
}

void Scheduler::disableEvent(struct SchedulerEvent * event){
    event->disabled = 1;

    update();
}

void Scheduler::updateEvent(struct SchedulerEvent * e){
    if (scheduler_wait.tv_nsec + e->interval.tv_nsec >= 1E9) {
        e->next_interval.tv_sec = scheduler_wait.tv_sec  + e->interval.tv_sec  + 1;
        e->next_interval.tv_nsec = scheduler_wait.tv_nsec + e->interval.tv_nsec - 1E9;
    }
    else{
        e->next_interval.tv_sec = scheduler_wait.tv_sec  + e->interval.tv_sec;
        e->next_interval.tv_nsec = scheduler_wait.tv_nsec + e->interval.tv_nsec;
    }
}

struct SchedulerEvent * Scheduler::addEvent(const char * name, struct timespec interval){
    struct SchedulerEvent * e = (struct SchedulerEvent *)_calloc(1, struct SchedulerEvent);
    
    strncpy(e->name, name, 60);
    
    e->interval.tv_sec  = interval.tv_sec;
    e->interval.tv_nsec = interval.tv_nsec;
    e->disabled = 1;

    events.push_back(e);

    if( _stop_ == 0){
        clock_gettime(CLOCK_REALTIME, &e->next_interval);
        e->next_interval.tv_sec  += e->interval.tv_sec;
        e->next_interval.tv_nsec += e->interval.tv_nsec;
    }

    update();

    return e;
}

struct SchedulerEvent * Scheduler::addEvent(struct SchedulerEvent event){
    struct SchedulerEvent * e = (struct SchedulerEvent *)_calloc(1, struct SchedulerEvent);
    memcpy((void *)e, (void *)&event, sizeof(struct SchedulerEvent));

    events.push_back(e);

    if( _stop_ == 0){
        clock_gettime(CLOCK_REALTIME, &e->next_interval);
        e->next_interval.tv_sec += e->interval.tv_sec;
        e->next_interval.tv_nsec += e->interval.tv_nsec;
    }

    update();

    return e;
}

// Private
void Scheduler::removeEvent(uint16_t index){
    _free(events[index]);
    events.erase(events.begin() + index);
}

// Public
void Scheduler::removeEvent(struct SchedulerEvent * event){
    uint16_t index = getEvent(event);

    if(events.size() > index){
        removeEvent(index);
        loggerf(DEBUG, "Removed Scheduler Event %x", (unsigned int)event);
    }
    else
        loggerf(WARNING, "No Scheduler Event %x", (unsigned int)event);
}

// Public
void Scheduler::removeEvent(const char * name){
    uint16_t index = getEvent(name);

    if(events.size() > index){
        removeEvent(index);
        loggerf(DEBUG, "Removed Scheduler Event named %s", name);
    }
    else
        loggerf(WARNING, "No Scheduler Event named %s", name);
}

uint16_t Scheduler::getEvent(struct SchedulerEvent * event){
    uint16_t index = 0;
    for(auto &e: events){
        if(e == event){
            // Equal
            break;
        }
        index++;
    }
    return index;
}

uint16_t Scheduler::getEvent(const char * name){
    uint16_t index = 0;
    for(auto &e: events){
        if(strcmp(e->name, name) == 0){
            // Equal
            break;
        }
        index++;
    }
    return index;
}


void Scheduler::print_events(){
    for(auto &i: events){
        printf("%c next %ld.%ld \t %x %x \t interval %ld.%ld\t %60s\n",
               i->disabled ? 'x':' ', i->next_interval.tv_sec, i->next_interval.tv_nsec,
               (unsigned int)i->function, (unsigned int)i->function_args,
               i->interval.tv_sec, i->interval.tv_nsec, i->name);
    }
}

Scheduler * scheduler = new Scheduler();
