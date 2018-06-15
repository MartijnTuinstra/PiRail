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
    int id;
    int MAdr;
    int UAdr;
    enum Rail_states state;
    int type; // 0 = 2-state / 1 = 4-state / 2 = 8-state
    uint8_t length;

    short adr[6];
    char states[4];
    char flash[4];
  } Signal;

  // void create_signal2(struct Seg * B,char adr_nr, uint8_t addresses[adr_nr], char state[BLOCK_STATES], char flash[BLOCK_STATES], char side);

  void set_signal(Signal *Si, enum Rail_states state);
#endif

