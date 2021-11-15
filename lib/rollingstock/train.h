#ifndef _INCLUDE_ROLLINGSTOCK_TRAIN_H
#define _INCLUDE_ROLLINGSTOCK_TRAIN_H

#include <time.h>
#include <algorithm>
#include <vector>

#include "switchboard/declares.h"
#include "rollingstock/declares.h"
#include "rollingstock/engine.h"
#include "rollingstock/trainset.h"
#include "rollingstock/traindetection.h"

// Pathfinding declares
namespace PathFinding {class Route; };

#define TRAIN_ENGINE_TYPE 0
#define TRAIN_TRAIN_TYPE 1

#define TRAIN_MANUAL 0
#define TRAIN_SEMI_AUTO 1
#define TRAIN_FULL_AUTO 2

#define TRAIN_ROUTE_DISABLED 0
#define TRAIN_ROUTE_RUNNING 1
#define TRAIN_ROUTE_ENTERED_DESTINATION 2
#define TRAIN_ROUTE_AT_DESTINATION 3

// #define TRAIN_SPEED_T_INIT 0
// #define TRAIN_SPEED_T_CHANGING 1
// #define TRAIN_SPEED_T_UPDATE 2
// #define TRAIN_SPEED_T_DONE 3
// #define TRAIN_SPEED_T_FAIL 4

#define TRAIN_SPEED_R_NONE     0
#define TRAIN_SPEED_R_SIGNAL   1
#define TRAIN_SPEED_R_MAXSPEED 2
#define TRAIN_SPEED_R_ROUTE    3

#define TRAIN_FORWARD 0
#define TRAIN_REVERSE 1

struct train_speed_timer {
  Train * T;
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
  Train * T;

  uint8_t reason;     // TRAIN_SPEED_R_(NONE / SIGNAL / MAXSPEED / ROUTE)
  Block * signalBlock;       //  If reason is Signal, the block that has a different state
  
  uint16_t target_speed = 0;     // Speed that should be reached at target_distance
  uint16_t target_distance = 0;

  uint16_t startSpeed;       // The speed the train was going before the SpeedEvent
  float displacement;        // The displacement the train traveled from the start of the SpeedEvent
  float startDisplacement;   // The displacement the train traveled before the SpeedEvent

  float acceleration;

  float time;
  uint16_t steps;
  uint16_t stepCounter;
  uint32_t stepTime;

  struct timespec updateTime;
  struct timespec starttime;
};

extern char TrainStatesStrings[40][20];

// Train States
enum _TrainSpeedStates {
    TRAIN_SPEED_IDLE,                // Parked
    TRAIN_SPEED_RESUMING,            // Stopped but ready to roll
    TRAIN_SPEED_STOPPING,            // Stopped but ready to park
    TRAIN_SPEED_STOPPING_REVERSE,    // Stopped but ready to reverse

    // Stopped train
    TRAIN_SPEED_STOPPING_WAIT,       // Stopped but ready to wait
    TRAIN_SPEED_WAITING,             // Train waiting

    TRAIN_SPEED_WAITING_DESTINATION, // Train waiting at destination

    // Moving train
    TRAIN_SPEED_DRIVING,             
    TRAIN_SPEED_CHANGING,            
    TRAIN_SPEED_UPDATE,              

    // Auxiliary states
    TRAIN_SPEED_INITIALIZING        
};

Block * FindFront(Train *, Block *, int16_t);

class Train {
  // private:
  public:
    union {
      Engine   * E;
      TrainSet * T;
      void * p;
    } p;
    char type = 0;

  // public:
    Block * B; // FrontBlock
    std::vector<Block *> blocks;         // All blocks that are blocked by the train (detection and virtual)
    std::vector<Block *> reservedBlocks; // All blocks with switches that are reserved by the train.
    std::vector<Path *>  paths;          // All the paths the train is in.

    uint8_t id = 0;

    uint16_t speed = 0;        // Real speed
    uint16_t MaxSpeed = 0;     // Real max speed

    // Event Scheduler for state changes and accelerate/decelerate
    struct SchedulerEvent * speed_event = 0;

    // Variables for changing speed along one block
    struct TrainSpeedEventData * speed_event_data = 0;
    enum _TrainSpeedStates SpeedState = TRAIN_SPEED_INITIALIZING;

    bool manual = 1;   // TRAIN_MANUAL
    bool fullAuto = 0; // TRAIN_FULL_AUTO
    bool stopped = 1;         // 
    bool dir = 0;             // TRAIN_FORWARD / TRAIN_REVERSE
    bool directionKnown = 0;  //  block direction is matched to the Z21 direction
    bool reverseDirection = 0;  // Train is reversed with respect to the front block
    bool initialized = 0;       // Virtual blocks are initialized
    uint8_t routeStatus = TRAIN_ROUTE_DISABLED;         // TRAIN_ROUTE - DISABLED / RUNNING / ENTERED_DESTINATION / AT_DESTINATION

    PathFinding::Route * route = 0;

    // Only the engine is detectable, cars added virtually
    bool virtualLength = 0;
    uint16_t length = 0;

    uint16_t virtualLengthBefore = 0;
    uint16_t virtualLengthAfter  = 0;

    bool assigned = 0;

    uint8_t category = 0;

    std::vector<TrainDetectable *> Detectables;
    // uint8_t Detectables;
    // struct TrainDetectables * DetectedBlocks;

    // struct pathinstruction * instructions;

    Train(Block * B);
    ~Train();

    void setBlock(Block *);
    void setBlock(std::vector<Block *>::iterator I, Block * sB);
    void releaseBlock(Block *);

    void reserveBlock(Block *);
    void dereserveBlock(Block *);
    void dereserveAll();

    void initMove(Block *);     // First time train moved while linked
    void move(Block *);         //  train moves, or any intermediate detection
    void moveForward(Block *);  //  train moves a step forward

    void initDetectables();

    void initVirtualBlocks();
    void VirtualBlocks();

    uint8_t setVirtualBlocks(algor_blocks * blockGroup, uint16_t length);
    uint8_t releaseVirtualBlocks(Algor_Blocks * blockGroup, uint8_t offset);

    // void initMoveForward(Block *);
    // void moveForwardFree(Block * tB);
    // void moveForward(Block *);
    // void moveFrontForward(Block *);

    // train/speed.cpp ----
    public:
    void _setSpeed(uint16_t _speed);
    void setSpeed(uint16_t _speed);

    void changeSpeed(uint16_t, uint16_t);             // Change speed gradually
    void changeSpeed(struct TrainSpeedEventRequest);  // Change speed gradually
    private:
    void setStopped(bool);
    void setStationStopped(bool);
    void applySpeed(uint16_t);
    // ---------------------
    public:

    void reverse();    // Reverse all
    void reverseFromPath(Path * P);
    void reverseBlocks();
    void Z21_reverse(); // Reverse simple

    int link(int tid, char type);
    int link(int tid, char type, uint8_t, Train **);
    void unlink();

    // Paths
    void enterPath(Path *);
    void exitPath(Path *);
    void analyzePaths();

    // train/route.cpp ----
    void setRoute(Block * dest);
    void setRoute(Station * dest);
    void clearRoute();
    // ---------------------

    bool ContinueCheck(); // Function to check if the train is allowed or able to continue
    void Continue();      // Function to set switches when granted by ContinueCheck

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

void Train_ContinueCheck(void * args);

void train_speed_event_create(Train *, struct TrainSpeedEventRequest);
void train_speed_event_calc(struct TrainSpeedEventData * data);
void train_speed_event_init(Train * T);
void train_speed_event_tick(struct TrainSpeedEventData * data);

#endif