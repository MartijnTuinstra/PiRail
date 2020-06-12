#ifndef _INCLUDE_SWICHBOARD_SWITCH_H
#define _INCLUDE_SWICHBOARD_SWITCH_H


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "switch.h"
#include "switchboard/rail.h"
#include "IO.h"

#define U_Sw(U, A) Units[U]->Sw[A]


struct switch_list {
  uint8_t len;
  char * type;
  void ** p;
};

struct CoupledSwitch {
  char type;
  union p {
    Switch * Sw;
    MSSwitch * MSSw;
    void * p;
  };
  uint8_t states_len;
  uint8_t * states;
};

struct switch_preference {
  char type;
  uint8_t state;
};

struct s_switch_connect {
  uint8_t  module;
  uint16_t id;

  struct rail_link app;
  struct rail_link str;
  struct rail_link div;
};


class Switch {
  public:

    /*
     s -.
     s --`--- S
    */

    uint8_t  module;
    uint16_t id;

    bool hold;

    bool feedback_en;
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
    struct CoupledSwitch * coupled;

    uint8_t pref_len;
    struct switch_preference * preferences;

    // Switch(uint8_t module, struct s_switch_conf config);
    Switch(struct s_switch_connect connect, uint8_t block_id, uint8_t output_len, Node_adr * output_pins, uint8_t * output_states);
    ~Switch();

    bool approachable(void * p, int flags);
    Block * Next_Block(enum link_types type, int flags, int level);

    void setState(uint8_t state, uint8_t lock);
};


int throw_multiple_switches(uint8_t len, char * data);

int Switch_Check_Path(void * p, struct rail_link link, int flags);
int Switch_Reserve_Path(void * p, struct rail_link link, int flags);
int Switch_Set_Path(void * p, struct rail_link link, int flags);


#endif
