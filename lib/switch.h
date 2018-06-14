#ifndef _INCLUDE_SWITCH_H
  #define _INCLUDE_SWITCH_H
  
  #include "rail.h"
  #include "trains.h"

  struct switch_link {
    char type;
    void * p;
    char states_len;
    int * states;
  };

  struct switch_preference {
    char type;
    char state;
  }

  typedef struct _switch {
    int module;
    int id;

    struct rail_link div;
    struct rail_link str;
    struct rail_link app;

    char state;
    char default_state;

    char UAdr;
    char Out[5];

    Block * Detection;

    char links_len;
    struct switch_link * links;

    char pref_len;
    struct switch_preference * preferences;
  } Switch;

  typedef struct _msswitch {
    int module;
    int id;

    struct rail_link * sideA;
    struct rail_link * sideB;

    char state_len;
    char default_state;
    char state;

    char links_len;
    struct switch_link * link;

    char pref_len;
    struct switch_preference * preferences;
  } MSSwitch;

  struct switch_list {
    char len;
    char * type;
    void ** p;
  }

  int throw_switch(Switch * S);
  int throw_msswitch(MSSwitch * S);

  int set_switch(Switch * S, char state);
  int set_msswitch(MSSwitch * S, char state);

  int set_multiple_switches(struct switch_list list, char * states);

  void Create_Switch();
  void Create_MSSwitch();

  int check_Switch(Block * B, int dir, _Bool pref);

  int free_Switch(Block * B, int dir);

  int free_Route_Switch(Block * B, int dir, Trains * T);
#endif