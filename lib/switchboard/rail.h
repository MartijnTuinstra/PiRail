#ifndef _INCLUDE_SWITCHBOARD_RAIL_H
#define _INCLUDE_SWITCHBOARD_RAIL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "switchboard/declares.h"
#include "path.h"
#include "config_data.h"

#define NEXT 0
#define PREV 1

#define SWITCH_CARE 0x80
#define DIRECTION_CARE 0x40
#define NEXT_FIRST_TIME_SKIP 0x20

// IO.h
typedef struct s_IO_Port IO_Port;
typedef struct s_node_adr Node_adr;

// Train.h
class RailTrain;

#define U_B(U, A) Units[U]->B[A]
#define RESTRICTED_SPEED 40
#define CAUTION_SPEED 90

#include "switchboard/links.h"

// typedef struct s_algor_block {
//   union {
//     Block * B;
//     Block * SB[5];
//   } p;
//   uint8_t len; // 0 = one block, 1 or more is Switch Blocks

//   RailTrain * train;

//   uint8_t blocked;
//   uint8_t reserved;
// } Algor_Block;

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

class Block {
  public:

    /*
     R ---- R
    */

    uint8_t  module;
    uint16_t id;

    //Input
    IO_Port * In;

    enum Rail_types type;
    uint8_t dir;
    IO_Port * dir_Out;
    int length;

    struct rail_link next;
    struct rail_link prev;

    Station * station;

    Path * path;

    uint16_t max_speed;

    enum Rail_states state;
    enum Rail_states reverse_state;

    uint8_t reserved:4;
    uint8_t blocked:1;

    RailTrain * train; //Follow id
    uint8_t IOchanged:1;
    uint8_t statechanged:1;
    uint8_t algorchanged:1;
    uint8_t oneWay:1;

    Signal * NextSignal;
    Signal * PrevSignal;

    int switch_len;
    Switch ** Sw;

    MSSwitch * MSSw;

    //Algorithm selected blocks
    Algor_Blocks Alg;

    Block(uint8_t module, struct s_block_conf block);  // Constructor
    ~Block(); // Destructor

    void addSwitch(Switch * Sw);

    struct rail_link * NextLink(int flags);
    Block * _Next(int flags, int level);

    uint8_t _NextList(Block ** blocks, uint8_t block_counter, int flags, int length);

    void reverse();
    void reserve();

    void setState(enum Rail_states state);
    void setReversedState(enum Rail_states state);

    void AlgorClear();
    void AlgorSearch(int debug);
    void AlgorSearchMSSwitch(int debug);
};


int dircmp(Block *A, Block *B);
void Reserve_To_Next_Switch(Block * B);
int Block_Reverse_To_Next_Switch(Block * B);

#endif
