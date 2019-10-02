#ifndef _INCLUDE_SWITCH_H
#define _INCLUDE_SWITCH_H

#include "rail.h"
// #include "train.h"
// #include "pathfinding.h"

#define U_Sw(U, A) Units[U]->Sw[A]
#define U_MSSw(U, A) Units[U]->MSSw[A]

struct switch_link {
  char type;
  void * p;
  uint8_t states_len;
  uint8_t * states;
};

struct switch_preference {
  char type;
  uint8_t state;
};

typedef struct s_Switch {
  uint8_t  module;
  uint16_t id;

  _Bool hold;

  _Bool feedback_en;
  uint8_t feedback_len;
  IO_Port ** feedback;
  char * feedback_states;

  uint8_t IO_len;
  IO_Port ** IO;
  uint8_t * IO_states;

  struct rail_link div;
  struct rail_link str;
  struct rail_link app;

  uint8_t state;
  uint8_t default_state;

  Block * Detection;

  uint8_t links_len;
  struct switch_link * links;

  uint8_t pref_len;
  struct switch_preference * preferences;
} Switch;

typedef struct s_MSSwitch {
  uint8_t  module;
  uint16_t id;

  _Bool hold;

  _Bool feedback_en;
  uint8_t feedback_len;
  IO_Port ** feedback;
  char * feedback_states;

  uint8_t IO_len;
  IO_Port ** IO;
  uint16_t * IO_states;

  struct rail_link * sideA;
  struct rail_link * sideB;

  uint8_t state_len;
  uint8_t default_state;
  uint8_t state;

  Block * Detection;

  uint8_t links_len;
  struct switch_link * links;

  uint8_t pref_len;
  struct switch_preference * preferences;
} MSSwitch;

struct s_switch_connect {
  uint8_t  module;
  uint16_t id;

  struct rail_link app;
  struct rail_link str;
  struct rail_link div;
};

struct s_msswitch_connect {
  uint8_t  module;
  uint16_t id;

  uint8_t states;

  struct rail_link * sideA;
  struct rail_link * sideB;
};

struct switch_list {
  uint8_t len;
  char * type;
  void ** p;
};

// int set_switch(Switch * S, uint8_t state);
// int set_msswitch(MSSwitch * S, uint8_t state);


// int Next_check_Switch_Route(void * p, struct rail_link link, int flags, struct pathinstruction * route, struct pathinstruction * instr);
// int set_switch_route(void * p, struct rail_link link, int flags, struct pathinstruction * instr);

// int set_switch_path(void * p, struct rail_link link, int flags);
// int reserve_switch_path(void * p, struct rail_link link, int flags);

void Create_Switch(struct s_switch_connect connect, uint8_t block_id, uint8_t output_len, Node_adr * output_pins, uint8_t * output_states);
void Create_MSSwitch(struct s_msswitch_connect connect, uint8_t block_id, uint8_t output_len, Node_adr * output_pins, uint16_t * output_states);
void * Clear_Switch(Switch * Sw);
void * Clear_MSSwitch(MSSwitch * Sw);

void throw_switch(Switch * S, uint8_t state, uint8_t lock);
void throw_msswitch(MSSwitch * S, uint8_t state, uint8_t lock);
int throw_multiple_switches(uint8_t len, char * data);

int Next_check_Switch(void * p, struct rail_link link, int flags);
int Switch_Check_Path(void * p, struct rail_link link, int flags);
int Switch_Reserve_Path(void * p, struct rail_link link, int flags);
int Switch_Set_Path(void * p, struct rail_link link, int flags);

// int check_Switch(struct rail_link link, _Bool pref);
// int check_Switch_State(struct rail_link adr);

// int free_Switch(Block * B, int dir);

// int free_Route_Switch(Block * B, int dir, Trains * T);
#endif
