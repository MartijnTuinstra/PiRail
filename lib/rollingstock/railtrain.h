#ifndef _INCLUDE_ROLLINGSTOCK_RAILTRAIN_H
#define _INCLUDE_ROLLINGSTOCK_RAILTRAIN_H

#include <time.h>
#include <algorithm>
#include <vector>

#include "rollingstock/declares.h"
#include "rollingstock/engine.h"
#include "rollingstock/train.h"
#include "pathfinding.h"

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

    Block * B; // FrontBlock
    std::vector<Block *> blocks = {}; // All blocks that are blocked by the train (detection and virtual)
    std::vector<Block *> reservedBlocks = {}; // All blocks with switches that are reserved by the train.

    uint8_t link_id;

    uint16_t speed;        // Real speed
    uint16_t max_speed;    // Real max speed

    // Variables for changing speed along one block
    uint16_t target_speed;
    uint16_t target_distance;
    struct SchedulerEvent * speed_event;
    struct TrainSpeedEventData * speed_event_data;

    uint8_t changing_speed:3;  // RAILTRAIN_SPEED_T_(INIT / CHANGING / UPDATE / DONE / FAIL)
    uint8_t control:2;         // TRAIN_(MANUAL / SEMI_AUTO / FULL_AUTO)
    uint8_t onroute:1;         // TRAIN_ONROUTE = true
    uint8_t stopped:1;         // 
    uint8_t dir:1;             // TRAIN_FORWARD / TRAIN_REVERSE

    PathFinding::Route * route;

    // Only the engine is detectable, cars added virtually
    bool virtualLength;
    uint16_t length;

    bool assigned;

    uint8_t category;

    // struct pathinstruction * instructions;

    RailTrain(Block * B);
    ~RailTrain();

    void setBlock(Block *);
    void releaseBlock(Block *);

    void reserveBlock(Block *);
    void dereserveBlock(Block *);
    void dereserveAll();

    void initVirtualBlocks();
    void setVirtualBlocks();

    void moveForward(Block * B);

    void setSpeed(uint16_t _speed);
    void setSpeedZ21(uint16_t);
    void setStopped(bool);
    void changeSpeed(uint16_t target_speed, uint8_t type);

    int link(int tid, char type);
    void unlink();

    void setRoute(Block * dest);

    bool ContinueCheck();

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