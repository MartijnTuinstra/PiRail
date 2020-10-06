#ifndef _INCLUDE_SWICHBOARD_SWITCH_H
#define _INCLUDE_SWICHBOARD_SWITCH_H


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "config/ModuleConfig.h"
#include "switch.h"
#include "switchboard/rail.h"
#include "IO.h"
#include "pathfinding.h"

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
    uint16_t uid;
    Unit * U;

    bool hold;
    bool updatedState;
    bool feedbackWrongState;

    bool feedback_en;
    uint8_t feedback_len;
    IO_Port ** feedback;
    union u_IO_event * feedback_events[2];

    uint8_t IO_len;
    IO_Port ** IO;
    union u_IO_event * IO_events[2];

    std::vector<Signal *> Signals;

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
    Switch(uint8_t Module, struct switch_conf s);
    // Switch(struct s_switch_connect connect, uint8_t block_id, uint8_t output_len, Node_adr * output_pins, uint8_t * output_states);
    ~Switch();

    void addSignal(Signal * Sig);

    bool approachable(void * p, int flags);
    Block * Next_Block(enum link_types type, int flags, int level);
    uint NextList_Block(Block ** blocks, uint8_t block_counter, enum link_types type, int flags, int length);

    void setState(uint8_t _state);
    void setState(uint8_t state, uint8_t lock);
    void updateState(uint8_t state);

    void updateFeedback();
};


int throw_multiple_switches(uint8_t len, char * data);


namespace SwitchSolver {

int solve(RailTrain *, Block *, Block *, struct rail_link, int);

struct find {
  int possible;
  int allreadyCorrect;
};

struct find findPath(RailTrain *, PathFinding::Route *, void *, struct rail_link, int);
int setPath(RailTrain *, PathFinding::Route *, void *, struct rail_link, int);

int setWrong(PathFinding::Route *, void *, struct rail_link, int);

void dereservePath(RailTrain * T, PathFinding::Route * r, void * p, struct rail_link link, int flags);
int reservePath(RailTrain *, PathFinding::Route *, void *, struct rail_link, int);

};


// int Switch_Check_Path(void * p, struct rail_link link, int flags);
// int Switch_Check_Path(PathFinding::Route * r, void * p, struct rail_link link, int flags);

// int Switch_Reserve_Path(RailTrain * T, void * p, struct rail_link link, int flags);
// int Switch_Set_Free_Path(void * p, struct rail_link link, int flags);
// int Switch_Set_Free_Path(PathFinding::Route * r, void * p, struct rail_link link, int flags);

// int Switch_Set_Wrong_Route(PathFinding::Route * r, void * p, struct rail_link link, int flags);


#endif
