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

  RailLink * sideA;
  RailLink * sideB;
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

    bool feedback_en = 0;
    uint8_t feedback_len = 0;
    IO_Port ** feedback = 0;
    union u_IO_event ** feedback_events = 0;
    char * feedback_states = 0;

    uint8_t IO_len;
    IO_Port ** IO;

    std::vector<Signal *> Signals;

    uint8_t state_len = 0; // Total number of states
    uint8_t defaultState;  // Default state
    uint8_t state;         // Current state
    uint16_t maxSpeed;     // Current maximum Speed

    BlockLink * sideA = 0;      // Rail Link for each state
    BlockLink * sideB = 0;      // Rail Link for each state
    uint8_t * state_direction = 0;     // Direction for each state
    union u_IO_event ** IO_states = 0; // [State][IO_Port]
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

    RailLink * NextLink(int flags);

    bool approachableA(void * p, int flags);
    bool approachableB(void * p, int flags);
    Block * Next_Block(enum link_types type, int flags, int level);
    uint NextList_Block(Block * Origin, Block ** blocks, uint8_t block_counter, enum link_types type, int flags, int length);

    void setState(uint8_t _state);
    void setState(uint8_t _state, uint8_t lock);
    void updateState(uint8_t _state);

    bool checkPolarity(Block * B);
    bool checkPolarity(Block * B, uint8_t state);
    bool cmpPolarity(Block * B);
    bool cmpPolarity(Block * B, uint8_t state);
};

#endif
