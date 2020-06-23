#ifndef _INCLUDE_ROLLINGSTOCK_RAILTRAIN_H
#define _INCLUDE_ROLLINGSTOCK_RAILTRAIN_H

#include <time.h>

#include "rollingstock/declares.h"
#include "rollingstock/engine.h"
#include "rollingstock/train.h"

#define RAILTRAIN_ENGINE_TYPE 0
#define RAILTRAIN_TRAIN_TYPE 1


struct train_speed_timer {
  RailTrain * T;
  uint16_t target_speed;
  uint16_t length;
};

struct TrainSpeedEventData {
  RailTrain * T;

  uint16_t startSpeed;
  float acceleration;
  float time;
  uint16_t steps;
  uint16_t stepCounter;
  uint32_t stepTime;

  struct timespec starttime;
};


class RailTrain {
  public:
    union {
      Engine * E;
      Train * T;
      void * p;
    } p;
    char type;

    Block * B;

    uint8_t link_id;

    uint16_t speed;        // Real speed
    uint16_t max_speed;    // Real max speed

    uint16_t target_speed;
    uint16_t target_distance;

    uint8_t changing_speed:3;  // RAILTRAIN_SPEED_T_(INIT / CHANGING / UPDATE / DONE / FAIL)
    uint8_t control:2;         // TRAIN_(MANUAL / SEMI_AUTO / FULL_AUTO)
    uint8_t route:1;           //
    uint8_t stop:1;            // 
    uint8_t dir:1;             // TRAIN_FORWARD / TRAIN_REVERSE

    struct SchedulerEvent * speed_event;
    struct TrainSpeedEventData * speed_event_data;

    uint8_t category;

    // struct pathinstruction * instructions;

    RailTrain();
    ~RailTrain();

    void inline setSpeed(uint16_t speed){
      this->speed = speed;
      if(this->type == RAILTRAIN_ENGINE_TYPE) this->p.E->setSpeed(this->speed);
      else this->p.T->setSpeed(this->speed);
    }
    void setSpeedZ21(uint16_t speed);
    void changeSpeed(uint16_t target_speed, uint8_t type);

    int link(int tid, char type);
    void unlink();

    void setRoute(Block * dest);

};

void RailTrain_ContinueCheck(void * args);

extern RailTrain ** train_link;
extern int train_link_len;

RailTrain * new_railTrain();

void train_speed_event_create(RailTrain * T, uint16_t targetSpeed, uint16_t distance);
void train_speed_event_calc(struct TrainSpeedEventData * data);
void train_speed_event_init(RailTrain * T);
void train_speed_event_tick(struct TrainSpeedEventData * data);

#endif