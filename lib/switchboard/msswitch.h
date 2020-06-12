#ifndef _INCLUDE_SWICHBOARD_MSSWITCH_H
#define _INCLUDE_SWICHBOARD_MSSWITCH_H


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "switch.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "IO.h"


#define U_MSSw(U, A) Units[U]->MSSw[A]

void create_msswitch_from_conf(uint8_t module, struct ms_switch_conf conf);

struct s_msswitch_connect {
  uint8_t  module;
  uint16_t id;

  uint8_t states;

  struct rail_link sideA;
  struct rail_link * sideB;
};


class MSSwitch {
  public:

    /*
     B -.
     B --`,-- A
     B --'
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
    uint16_t * IO_states;

    struct rail_link sideA;
    struct rail_link * sideB;

    uint8_t state_len;
    uint8_t default_state;
    uint8_t state;

    Block * Detection;

    uint8_t links_len;
    struct CoupledSwitch * links;

    uint8_t pref_len;
    struct switch_preference * preferences;

    // Switch(uint8_t module, struct s_switch_conf config);
    MSSwitch(struct s_msswitch_connect connect, uint8_t block_id, uint8_t output_len, struct s_IO_port_conf * output_pins, uint16_t * output_states);
    ~MSSwitch();

    bool approachable(void * p, int flags);
    Block * Next_Block(enum link_types type, int flags, int level);

    void setState(uint8_t state, uint8_t lock);
};

#endif
