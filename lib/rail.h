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

typedef struct algor_blocks {
  uint8_t prev;
  Block ** P;
  Block * B;
  Block ** N;
  uint8_t next;
} Algor_Blocks;

struct rail_link {
  uint8_t  module;
  uint16_t id;
  uint8_t type;
  void * p;
};

enum Rail_types {
  MAIN,
  STATION,
  SPECIAL
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

#ifndef RAIL_LINK_TYPES
#define RAIL_LINK_TYPES
enum link_types {
  RAIL_LINK_R,
  RAIL_LINK_S,
  RAIL_LINK_s,
  RAIL_LINK_M,
  RAIL_LINK_m,
  RAIL_LINK_C = 0xfe,
  RAIL_LINK_E = 0xff
};
#endif

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

// void init_rail();

void Create_Block(uint8_t module, struct s_block_conf block);
void * Clear_Block(Block * B);

void Create_Station(int module, int id, char * name, char name_len, enum Station_types type, int len, uint8_t * blocks);
void * Clear_Station(Station * St);

// void Connect_Rail_links();

// int dircmp(Block *A, Block *B);
// int block_cmp(Block *A, Block *B);

// Block * Next_Switch_Block(Switch * S, char type, int flags, int level);
// Block * Next_MSSwitch_Block(MSSwitch * S, char type, int flags, int level);
// Block * Next_Special_Block(Block * B, int flags, int level);

// #define Next(B, f, l) _Next(B, (f) | 0b1110, l)
// Block * _Next(Block * B, int flags, int level);

// int Next_check_Switch(void * p, struct rail_link link, int flags);
// int Next_check_Switch_Path(void * p, struct rail_link link, int flags);
// int Next_check_Switch_Path_one_block(Block * B, void * p, struct rail_link link, int flags);
// int Switch_to_rail(Block ** B, void * Sw, char type, uint8_t counter);

// struct rail_link * Next_link(Block * B, int flags);
// // struct rail_link Prev_link(Block * B);

// void Reserve_To_Next_Switch(Block * B);

// #define Block_reserve(B) loggerf(DEBUG, "RESERVE BLOCK %02i:%02i", B->module, B->id);\
//                           B->reserved++
// #define Block_dereserve(B) loggerf(DEBUG, "deRESERVE BLOCK %02i:%02i", B->module, B->id);\
//                           B->reserved--

// void Block_Reverse(Block * B);
// int Block_Reverse_To_Next_Switch(Block * B);

#endif
