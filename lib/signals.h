#ifndef _INCLUDE_SIGNALS_H
  #define _INCLUDE_SIGNALS_H

  /**///Signal passing speed
  //Flashing Red speed
  #define RED_F_SPEED 20
  //Flashing Amber speed
  #define AMBER_F_SPEED 30
  //Amber speed
  #define AMBER_SPEED 40

  #include "rail.h"

  typedef struct _signal{
    char id;
    char module;
    Block * B;
    enum Rail_states state;
    char io; // 0 = 2-state / 1 = 4-state / 2 = 8-state

    short adr[8];
    char states[4];
    char flash[4];
  } Signal;

  void create_signal(Block * B, _Bool side, char length, char * addresses, char * states, char * flash);

  void signal_create_states(char io, enum Rail_states state, char * list, ...);

  void set_signal(Signal *Si, enum Rail_states state);
#endif

