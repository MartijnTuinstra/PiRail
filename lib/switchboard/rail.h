#ifndef _INCLUDE_SWITCHBOARD_RAIL_H
#define _INCLUDE_SWITCHBOARD_RAIL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <vector>

#include "switchboard/declares.h"
#include "switchboard/links.h"

#include "rollingstock/declares.h"
// #include "path.h"
#include "flags.h"

#include "config/LayoutStructure.h"

// IO.h
class IO_Port;
typedef struct s_node_adr Node_adr;

#define U_B(U, A) Units(U)->B[A]
#define RESTRICTED_SPEED 40
#define CAUTION_SPEED 90

typedef struct algor_blocks {
  Block * B[10];
  uint8_t dir[10];

  uint8_t group[4];
} Algor_Blocks;

struct blockAlgorithm {
  struct algor_blocks * N;
  struct algor_blocks * P;

  struct algor_blocks AlgorBlocks[2];
  
  Block * B;

  bool algorBlockSearched;
  bool trainFollowingChecked;
  bool switchChecked;
  bool polarityChecked;
  bool statesChecked;
  uint8_t doneAll;
};

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

  RailLink next;
  RailLink prev;
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
    // std::vector<IO_Port *> In_detection; //   Input
    IO_Port * In_detection;
    std::vector<IO_Port *> Out_polarity; //   Output Direction

    // -- Links --
    struct BlockLink next;
    struct BlockLink prev;

    // -- Pointers --
    Station * station = 0;     // The station that
    Path * path = 0;           // The path this block is part off

    Train * train = 0;         // The train that is in this block
    Train * expectedTrain = 0; // The train that is expected to be in this block
    TrainDetectable * expectedDetectable = 0;

    std::vector<Train *> reservedBy;    // The train that has reserved this block in a whole path 
                                        //  A block with switches can be SWITCH_RESERVED. 

    std::vector<Signal *> * forward_signal;
    std::vector<Signal *> * reverse_signal;

    int switch_len = 0;
    Switch ** Sw = 0;
    MSSwitch * MSSw = 0;

    bool reserved = 0;       // If the block is reserved, if reserved no switches can be thrown

    bool blocked = 0;           // If either virtual or detection blocked
    bool virtualBlocked = 0;    // if virtual blocked
    bool detectionBlocked = 0;  // if blocked by detection

    uint8_t polarity_status:4;
    uint8_t polarity_type:4;
    void * polarity_link = 0;

    uint8_t IOchanged:1;
    uint8_t statechanged:1;
    uint8_t algorchanged:1;
    uint8_t recalculate:1;
    uint8_t oneWay:1;

    uint8_t switchWrongState:1;    // Set block to DANGER/CAUTION if switch cannot be aligned properly
    uint8_t switchWrongFeedback:1; // Set block to DANGER/CAUTION if switch is still moving

    //Algorithm selected blocks
    struct blockAlgorithm Alg;

    Block(uint8_t, struct configStruct_Block *);
    ~Block(); // Destructor

    void exportConfig(struct configStruct_Block *);

    void addSwitch(Switch * Sw);

    RailLink * NextLink(int flags);
    Block * Next_Block(int flags, int level);

    uint8_t _NextList(Block * Origin, Block ** blocks, uint8_t block_counter, uint32_t flags, int length);

    inline Block * getBlock(uint8_t side, uint8_t i){
      struct algor_blocks * A = (&Alg.N)[side];
      if(i > A->group[3])
        return 0;
      else
        return A->B[i];
    };

    void reverse();

    void reserve(Train *);
    void dereserve(Train *);
    bool isReservedBy(Train *);

    private:
    void setState(enum Rail_states *, enum Rail_states, std::vector<Signal *> *);
    public:
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

    bool checkPolarity(Block * B); // Check if there is a continuous path of the same polarity
    bool cmpPolarity(Block * B);   // Check if block has same default polarity
    void flipPolarity();
    void flipPolarity(bool reverse);
};

uint8_t _NextList_NextIteration(RailLink * nextLink, void * p, Block * Origin, Block ** blocks, uint8_t block_counter, uint64_t flags, int length);

int dircmp(Block *A, Block *B);
int dircmp(uint8_t A, uint8_t B);

void Reserve_To_Next_Switch(Block * B);
int Block_Reverse_To_Next_Switch(Block * B);

#endif
