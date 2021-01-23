#ifndef _INCLUDE_SWICHBOARD_MSSWITCH_H
#define _INCLUDE_SWICHBOARD_MSSWITCH_H


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "switch.h"
#include "switchboard/unit.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "IO.h"

#include "config/LayoutStructure.h"


#define U_MSSw(U, A) Units(U)->MSSw[A]

void create_msswitch_from_conf(uint8_t module, struct ms_switch_conf conf);

struct s_msswitch_connect {
  uint8_t  module;
  uint16_t id;

  uint8_t states;

  struct rail_link * sideA;
  struct rail_link * sideB;
  uint8_t * dir;
};

#define MSSW_TYPE_CROSSING 0
#define MSSW_TYPE_TURNTABLE 1
#define MSSW_TYPE_TRAVERSETABLE 2

class MSSwitch {
  public:

    /*
     B -.
     B --`,-- A
     B --'
    */

    uint8_t  module;
    uint16_t id;
    uint16_t uid;
    Unit * U;

    uint8_t type;

    bool hold;
    bool updatedState;
    bool feedbackWrongState;

    bool feedback_en;
    uint8_t feedback_len;
    IO_Port ** feedback;
    union u_IO_event ** feedback_events;
    char * feedback_states;

    uint8_t IO_len;
    IO_Port ** IO;

    std::vector<Signal *> Signals;

    uint8_t state_len;      // Total number of states
    uint8_t defaultState;  // Default state
    uint8_t state;          // Current state
    uint16_t maxSpeed;      // Current maximum Speed

    struct rail_link * sideA;      // Rail Link for each state
    struct rail_link * sideB;      // Rail Link for each state
    uint8_t * state_direction;     // Direction for each state
    union u_IO_event ** IO_states; // [State][IO_Port]
    uint16_t * stateMaxSpeed;      // Maximum Speed for each state

    Block * Detection;

    uint8_t links_len;
    struct CoupledSwitch * links;

    uint8_t pref_len;
    struct switch_preference * preferences;

    MSSwitch(uint8_t, struct configStruct_MSSwitch *);
    ~MSSwitch();

    void exportConfig(struct configStruct_MSSwitch *);

    void addSignal(Signal * Sig);

    struct rail_link * NextLink(int flags);

    bool approachableA(void * p, int flags);
    bool approachableB(void * p, int flags);
    Block * Next_Block(enum link_types type, int flags, int level);
    uint NextList_Block(Block * Origin, Block ** blocks, uint8_t block_counter, enum link_types type, int flags, int length);

    void setState(uint8_t _state);
    void setState(uint8_t _state, uint8_t lock);
    void updateState(uint8_t _state);
};

#endif
