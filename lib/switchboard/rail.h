#ifndef _INCLUDE_SWITCHBOARD_RAIL_H
#define _INCLUDE_SWITCHBOARD_RAIL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "switchboard/declares.h"
#include "rollingstock/declares.h"
#include "path.h"
#include "flags.h"

#include "config/LayoutStructure.h"

// IO.h
class IO_Port;
typedef struct s_node_adr Node_adr;

#define U_B(U, A) Units(U)->B[A]
#define RESTRICTED_SPEED 40
#define CAUTION_SPEED 90

#include "switchboard/links.h"

typedef struct algor_blocks {
  uint8_t prev;
  uint8_t prev3;
  uint8_t prev2;
  uint8_t prev1;
  Block * P[10];
  Block * B;
  Block * N[10];
  uint8_t next1;
  uint8_t next2;
  uint8_t next3;
  uint8_t next;
} Algor_Blocks;

enum Rail_types {
  MAIN,
  STATION,
  NOSTOP,
  TURNTABLE,
  CROSSING
};

enum Rail_states {
  BLOCKED,          // 0
  DANGER,           // 1
  RESTRICTED,       // 2
  CAUTION,          // 3
  PROCEED,          // 4
  RESERVED,         // 5
  RESERVED_SWITCH,  // 6
  UNKNOWN           // 7
};

extern const char * rail_states_string[8];

struct block_connect {
  int module;
  int id;
  enum Rail_types type;

  struct rail_link next;
  struct rail_link prev;
};

struct configStruct_Block;

class Block {
  public:

    /*
     R ---- R
    */

    uint8_t  module;
    uint16_t id;
    uint16_t uid;
    Unit * U;

    enum Rail_types type;
    uint8_t dir;
    int length;

    uint16_t BlockMaxSpeed; // Maximum allow speed in this block (if no speed restrictions)
    uint16_t MaxSpeed;      // Current Maximum Speed

    enum Rail_states state;
    enum Rail_states reverse_state;

    // -- IO --
    //   Input
    IO_Port * In;

    //   Output Direction
    IO_Port * dir_Out;

    // -- Links --
    struct rail_link next;
    struct rail_link prev;

    // -- Pointers --
    Station * station = 0;     // The station that
    Path * path = 0;           // The path this block is part off

    Train * train;         // The train that is in this block
    Train * expectedTrain; // The train that is expected to be in this block
    TrainDetectable * expectedDetectable;

    std::vector<Train *> reservedBy;    // The train that has reserved this block in a whole path 
                                        //  A block with switches can be SWITCH_RESERVED. 

    std::vector<Signal *> * forward_signal;
    std::vector<Signal *> * reverse_signal;

    int switch_len;
    Switch ** Sw;
    MSSwitch * MSSw;

    bool reserved;       // If the block is reserved
    bool switchReserved; // If the block and switches are reserved

    bool blocked;           // If either virtual or detection blocked
    bool virtualBlocked;    // if virtual blocked
    bool detectionBlocked;  // if blocked by detection


    uint8_t IOchanged:1;
    uint8_t statechanged:1;
    uint8_t algorchanged:1;
    uint8_t recalculate:1;
    uint8_t oneWay:1;

    uint8_t switchWrongState:1;    // Set block to DANGER/CAUTION if switch cannot be aligned properly
    uint8_t switchWrongFeedback:1; // Set block to DANGER/CAUTION if switch is still moving

    //Algorithm selected blocks
    Algor_Blocks Alg;

    Block(uint8_t, struct configStruct_Block *);
    ~Block(); // Destructor

    void exportConfig(struct configStruct_Block *);

    void addSwitch(Switch * Sw);

    struct rail_link * NextLink(int flags);
    Block * Next_Block(int flags, int level);

    uint8_t _NextList(Block * Origin, Block ** blocks, uint8_t block_counter, uint32_t flags, int length);

    void reverse();

    void reserve(Train *);
    void dereserve(Train *);
    bool isReservedBy(Train *);

    void setState(enum Rail_states, bool);
    void setState(enum Rail_states);
    void setReversedState(enum Rail_states);
    enum Rail_states getNextState();
    enum Rail_states getPrevState();

    void setDetection(bool d);
    void setVirtualDetection(bool d);

    enum Rail_states addSignal(Signal * Sig);

    void setSpeed();                  // Function to recalculate max speed
    uint16_t getSpeed();              // Function to get current allowed max speed
    uint16_t getSpeed(uint8_t Dir);   // Function to get current allowed max speed

    void AlgorClear();
    void AlgorSearch(int debug);
    void AlgorSearchMSSwitch(int debug);
    void AlgorSetDepths(bool Side);

    void checkSwitchFeedback(bool);
};


int dircmp(Block *A, Block *B);
int dircmp(uint8_t A, uint8_t B);

void Reserve_To_Next_Switch(Block * B);
int Block_Reverse_To_Next_Switch(Block * B);

#endif
