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

#define RAILTRAIN_ROUTE_DISABLED 0
#define RAILTRAIN_ROUTE_RUNNING 1
#define RAILTRAIN_ROUTE_ENTERED_DESTINATION 2
#define RAILTRAIN_ROUTE_AT_DESTINATION 3

#define RAILTRAIN_FIFO_SIZE 64 // Blocks


struct train_speed_timer {
  RailTrain * T;
  uint16_t target_speed;
  uint16_t length;
};

struct TrainSpeedEventRequest {
  uint16_t targetSpeed;
  uint16_t distance;
  uint8_t reason;
  void * ptr;
};

struct TrainSpeedEventData {
  RailTrain * T;

  uint8_t reason;     // RAILTRAIN_SPEED_R_(NONE / SIGNAL / MAXSPEED / ROUTE)
  Block * signalBlock;       //  If reason is Signal, the block that has a different state
  
  uint16_t target_speed = 0;     // Speed that should be reached at target_distance
  uint16_t target_distance = 0;

  uint16_t startSpeed;       // The speed the train was going before the SpeedEvent
  float displacement;        // The displacement the train traveled from the start of the SpeedEvent
  float startDisplacement;   // The displacement the train traveld before the SpeedEvent

  float acceleration;

  float time;
  uint16_t steps;
  uint16_t stepCounter;
  uint32_t stepTime;

  struct timespec updateTime;
  struct timespec starttime;
};

struct RailTrainBlocksFifo {
  Block * B[RAILTRAIN_FIFO_SIZE];
  uint8_t Front = 0;
  uint8_t End = 0;
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
    std::vector<Block *> blocks;         // All blocks that are blocked by the train (detection and virtual)
    std::vector<Block *> reservedBlocks; // All blocks with switches that are reserved by the train.

    uint8_t id = 0;

    uint16_t speed = 0;        // Real speed
    uint16_t MaxSpeed = 0;     // Real max speed

    // Variables for changing speed along one block
    struct SchedulerEvent * speed_event = 0;
    struct TrainSpeedEventData * speed_event_data = 0;
    uint8_t changing_speed;  // RAILTRAIN_SPEED_T_(INIT / CHANGING / UPDATE / DONE / FAIL)

    bool manual = 1;   // TRAIN_MANUAL
    bool fullAuto = 0; // TRAIN_FULL_AUTO
    bool stopped = 1;         // 
    bool dir = 0;             // TRAIN_FORWARD / TRAIN_REVERSE
    bool directionKnown = 0;  //  block direction is matched to the Z21 direction
    bool reverseDirection = 0;  // Train is reversed with respect to the front block
    uint8_t routeStatus = RAILTRAIN_ROUTE_DISABLED;         // RAILTRAIN_ROUTE - DISABLED / RUNNING / ENTERED_DESTINATION / AT_DESTINATION

    PathFinding::Route * route = 0;

    // Only the engine is detectable, cars added virtually
    bool virtualLength = 0;
    uint16_t length = 0;

    bool assigned = 0;

    uint8_t category = 0;

    uint8_t Detectables;
    struct RailTrainBlocksFifo * DetectedBlocks;

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

    void initMoveForward(Block *);
    void moveForward(Block *);
    void moveFrontForward(Block *);

    void setSpeed(uint16_t _speed);
    void setStopped(bool);

    void changeSpeed(uint16_t, uint16_t);
    void changeSpeed(struct TrainSpeedEventRequest);

    void reverse();    // Reverse all
    void reverseFromPath(Path * P);
    void reverseBlocks();
    void Z21_reverse(); // Reverse simple

    int link(int tid, char type);
    int link(int tid, char type, uint8_t, RailTrain **);
    void unlink();

    void setRoute(Block * dest);
    void setRoute(Station * dest);

    bool ContinueCheck();
    uint16_t checkMaxSpeed();

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

void train_speed_event_create(RailTrain *, struct TrainSpeedEventRequest);
void train_speed_event_calc(struct TrainSpeedEventData * data);
void train_speed_event_init(RailTrain * T);
void train_speed_event_tick(struct TrainSpeedEventData * data);

#endif