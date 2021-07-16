#ifndef _INCLUDE_SWICHBOARD_SWITCH_H
#define _INCLUDE_SWICHBOARD_SWITCH_H


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <vector>

#include "switchboard/declares.h"
#include "switchboard/links.h"
#include "IO.h"

#include "config/LayoutStructure.h"

#define U_Sw(U, A) Units(U)->Sw[A]

#define STRAIGHT_SWITCH 0
#define DIVERGING_SWITCH 1


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

  RailLink app;
  RailLink str;
  RailLink div;
};

struct configStruct_Switch;

class Switch {
  public:

    /*
     s -.
     s --`--- S
    */

    uint8_t  module;
    uint16_t id;
    uint16_t uid;
    Unit * U;

    bool lockout = 0; // Controller has locked the switch

    bool updatedState = 0;
    bool feedbackWrongState = 0;

    bool feedback_en = 0;
    uint8_t feedback_len = 0;
    IO_Port ** feedback = 0;
    union u_IO_event * feedback_events[2] = {0, 0};

    uint8_t IO_len = 0;
    IO_Port ** IO = 0;
    union u_IO_event * IO_events[2] = {0, 0};

    std::vector<Signal *> Signals;

    RailLink div;
    RailLink str;
    RailLink app;

    uint8_t state = 0;
    uint8_t default_state = 0;

    uint16_t MaxSpeed[2] = {0, 0};

    Block * Detection = 0;

    uint8_t links_len = 0;
    struct CoupledSwitch * coupled = 0;

    uint8_t pref_len = 0;
    struct switch_preference * preferences = 0;

    Switch(uint8_t, struct configStruct_Switch *);
    ~Switch();

    void exportConfig(struct configStruct_Switch *);

    void addSignal(Signal * Sig);

    bool approachable(void * p, int flags);
    Block * Next_Block(enum link_types type, int flags, int level);
    uint NextList_Block(Block * Origin, Block ** blocks, uint8_t block_counter, enum link_types type, int flags, int length);

    void setState(uint8_t _state);
    void setState(uint8_t _state, bool overrideLockout);
    void setState(uint8_t _state, bool overrideLockout, bool mutexLock);
    void updateState(uint8_t _state);

    void updateFeedback();
};


int throw_multiple_switches(uint8_t len, char * data);

// int Switch_Check_Path(void * p, RailLink link, int flags);
// int Switch_Check_Path(PathFinding::Route * r, void * p, RailLink link, int flags);

// int Switch_Reserve_Path(Train * T, void * p, RailLink link, int flags);
// int Switch_Set_Free_Path(void * p, RailLink link, int flags);
// int Switch_Set_Free_Path(PathFinding::Route * r, void * p, RailLink link, int flags);

// int Switch_Set_Wrong_Route(PathFinding::Route * r, void * p, RailLink link, int flags);


#endif
