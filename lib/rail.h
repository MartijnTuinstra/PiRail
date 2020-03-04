#ifndef _INCLUDE_RAIL_H
#define _INCLUDE_RAIL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "config_data.h"

#define NEXT 0
#define PREV 1
#define SWITCH_CARE 0x80

// rail.h
typedef struct s_Block Block;

typedef struct s_Station Station;
typedef struct s_Switch Switch;
typedef struct s_MSSwitch MSSwitch;
typedef struct s_Signal Signal;

// IO.h
typedef struct s_IO_Port IO_Port;
typedef struct s_node_adr Node_adr;

// Train.h
typedef struct rail_train RailTrain;

#define U_B(U, A) Units[U]->B[A]
#define U_St(U,A) Units[U]->St[A]

#ifndef RAIL_LINK_TYPES
#define RAIL_LINK_TYPES
enum link_types {
  RAIL_LINK_R,
  RAIL_LINK_S,
  RAIL_LINK_s,
  RAIL_LINK_MA,
  RAIL_LINK_MB,
  RAIL_LINK_ma,
  RAIL_LINK_mb
  RAIL_LINK_TT = 0x10, // Turntable
  RAIL_LINK_C  = 0xfe,
  RAIL_LINK_E  = 0xff
};
#endif

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

struct rail_link {
  uint8_t  module;
  uint16_t id;
  enum link_types type;
  void * p;
};

enum Rail_types {
  MAIN,
  STATION,
  NOSTOP,
  TURNTABLE
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

extern char * rail_states_string[8];

typedef struct s_Block {
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

  int msswitch_len;
  MSSwitch ** MSSw;

  //Algorithm selected blocks
  Algor_Blocks Alg;

} Block;

struct block_connect {
  int module;
  int id;
  enum Rail_types type;

  struct rail_link next;
  struct rail_link prev;
};

enum Station_types {
  STATION_PERSON,
  STATION_CARGO,
  STATION_PERSON_YARD,//Yard for Person only
  STATION_CARGO_YARD, //Yard for Cargo only
  STATION_YARD        //Yard for both Person and Cargo trains
};

typedef struct s_Station {
  int module;
  int id;
  int uid;
  char * name;

  uint8_t state;

  Block ** blocks;
  int blocks_len;

  enum Station_types type;

  char switches_len;
  struct switch_link ** switch_link;
} Station;

extern Station ** stations;
extern int stations_len;

#define RESTRICTED_SPEED 40
#define CAUTION_SPEED 90

// void init_rail();

void Create_Block(uint8_t module, struct s_block_conf block);
void * Clear_Block(Block * B);

void Create_Station(int module, int id, char * name, char name_len, enum Station_types type, int len, uint8_t * blocks);
void * Clear_Station(Station * St);

int dircmp(Block *A, Block *B);

// void Connect_Rail_links();

// int block_cmp(Block *A, Block *B);

Block * Next_Switch_Block(Switch * S, enum link_types type, int flags, int level);
Block * Next_MSSwitch_Block(MSSwitch * S, enum link_types type, int flags, int level);

#define Next(B, f, l) _Next(B, (f) | 0b1110, l)
Block * _Next(Block * B, int flags, int level);

// #define _ALGOR_BLOCK_APPLY(_ABl, _c, _A, _B, _C) _A
#define _ALGOR_BLOCK_APPLY(_A) _A

// int Next_check_Switch(void * p, struct rail_link link, int flags);
// int Next_check_Switch_Path(void * p, struct rail_link link, int flags);
// int Next_check_Switch_Path_one_block(Block * B, void * p, struct rail_link link, int flags);
// int Switch_to_rail(Block ** B, void * Sw, char type, uint8_t counter);

struct rail_link * Next_link(Block * B, int flags);
// // struct rail_link Prev_link(Block * B);

void Reserve_To_Next_Switch(Block * B);

#define BLOCK_RESERVE(B) loggerf(DEBUG, "RESERVE BLOCK %02i:%02i", B->module, B->id);\
                          B->reserved++
#define BLOCK_DERESERVE(B) loggerf(INFO, "deRESERVE BLOCK %02i:%02i", B->module, B->id);\
                          B->reserved--

void Block_reserve(Block * B);

void Block_Reverse(Algor_Blocks * AB);
int Block_Reverse_To_Next_Switch(Block * B);

#endif
