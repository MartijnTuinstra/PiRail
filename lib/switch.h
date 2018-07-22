#ifndef _INCLUDE_SWITCH_H
  #define _INCLUDE_SWITCH_H
  
  #include "rail.h"
  #include "train.h"

  struct switch_link {
    char type;
    void * p;
    char states_len;
    int * states;
  };

  struct switch_preference {
    char type;
    char state;
  };

  typedef struct _switch {
    int module;
    int id;

    _Bool hold;

    _Bool feedback;
    char feedback_len;
    IO_Port * feedback_pins;
    char * feedback_states;

    char IO_len;
    IO_Port * IO;
    char * IO_states;

    struct rail_link div;
    struct rail_link str;
    struct rail_link app;

    char state;
    char default_state;

    Block * Detection;

    char links_len;
    struct switch_link * links;

    char pref_len;
    struct switch_preference * preferences;
  } Switch;

  typedef struct _msswitch {
    int module;
    int id;

    _Bool hold;

    _Bool feedback;
    char input_len;
    char * input_pins;
    uint16_t * input_states;

    char output_len;
    char * output_pins;
    uint16_t * output_states;

    struct rail_link * sideA;
    struct rail_link * sideB;

    char state_len;
    char default_state;
    char state;

    Block * Detection;

    char links_len;
    struct switch_link * link;

    char pref_len;
    struct switch_preference * preferences;
  } MSSwitch;

  struct switch_connect {
    int module;
    int id;

    struct rail_link app;
    struct rail_link str;
    struct rail_link div;
  };

  struct msswitch_connect {
    int module;
    int id;

    struct rail_link * sideA;
    struct rail_link * sideB;
  };

  struct switch_list {
    char len;
    char * type;
    void ** p;
  };

  int throw_switch(Switch * S);
  int throw_msswitch(MSSwitch * S);

  int set_switch(Switch * S, char state);
  int set_msswitch(MSSwitch * S, char state);

  int set_multiple_switches(char len, char * data);

  void Create_Switch(struct switch_connect connect, char block_id, char output_len, char * output_pins, char * output_states);
  void Create_MSSwitch(struct msswitch_connect connect, char block_id, char output_len, char * output_pins, uint16_t * output_states);

  int check_Switch(struct rail_link link, _Bool pref);
  int check_Switch_State(struct rail_link adr);

  int free_Switch(Block * B, int dir);

  int free_Route_Switch(Block * B, int dir, Trains * T);
#endif
