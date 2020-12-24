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

#define TRAIN_MANUAL 0
#define TRAIN_SEMI_AUTO 1
#define TRAIN_FULL_AUTO 2

#define REVERSE_NO_BLOCKS 1


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
    char type = 0;

    Block * B; // FrontBlock
    std::vector<Block *> blocks; // All blocks that are blocked by the train (detection and virtual)
    std::vector<Block *> reservedBlocks; // Only switch-blocks that are reserved by the train.
    std::vector<Path *>  reservedPaths;  // All paths that are reserved by the train.

    uint8_t id = 0;

    uint16_t speed = 0;        // Real speed
    uint16_t max_speed = 0;    // Real max speed

    // Variables for changing speed along one block
    uint16_t target_speed = 0;
    uint16_t target_distance = 0;
    struct SchedulerEvent * speed_event = 0;
    struct TrainSpeedEventData * speed_event_data = 0;

    uint8_t changing_speed:3;  // RAILTRAIN_SPEED_T_(INIT / CHANGING / UPDATE / DONE / FAIL)
    bool manual = 1;   // TRAIN_MANUAL
    bool fullAuto = 0; // TRAIN_FULL_AUTO
    bool onroute = 0;         // TRAIN_ONROUTE = true
    bool stopped = 1;         // 
    bool dir = 0;             // TRAIN_FORWARD / TRAIN_REVERSE
    bool directionKnown = 0;  //  block direction is matched to the Z21 direction

    PathFinding::Route * route = 0;

    // Only the engine is detectable, cars added virtually
    bool virtualLength = 0;
    uint16_t length = 0;

    bool assigned = 0;

    uint8_t category = 0;

    // struct pathinstruction * instructions;

    RailTrain(Block * B);
    ~RailTrain();

    void setBlock(Block *);
    void releaseBlock(Block *);

    void reserveBlock(Block *);
    void dereserveBlock(Block *);

    void reservePath(Path *);
    void dereservePath(Path * P);

    void dereserveAll();

    void initVirtualBlocks();
    void setVirtualBlocks();

    void moveForward(Block * B);

    void setSpeed(uint16_t _speed);
    void setSpeedZ21(uint16_t);
    void setStopped(bool);
    void changeSpeed(uint16_t target_speed, uint8_t type);

    void reverse();
    void reverse(uint8_t flags);

    int link(int tid, char type);
    void unlink();

    void setRoute(Block * dest);

    bool ContinueCheck();

    inline void setControl(uint8_t control){
      if(control == TRAIN_MANUAL){
        manual = true;
        fullAuto = false;
      }
      else if(control == TRAIN_SEMI_AUTO){
        manual = false;
        fullAuto = false;
      }
      else if(control == TRAIN_FULL_AUTO){
        manual = false;
        fullAuto = true;
      }
    }

};

void RailTrain_ContinueCheck(void * args);

void train_speed_event_create(RailTrain * T, uint16_t targetSpeed, uint16_t distance);
void train_speed_event_calc(struct TrainSpeedEventData * data);
void train_speed_event_init(RailTrain * T);
void train_speed_event_tick(struct TrainSpeedEventData * data);

#endif