#ifndef _INCLUDE_SIGNALS_H
  #define _INCLUDE_SIGNALS_H

  #include "IO.h"

  /**///Signal passing speed
  //Flashing Red speed
  #define RED_F_SPEED 20
  //Flashing Amber speed
  #define AMBER_F_SPEED 30
  //Amber speed
  #define AMBER_SPEED 40

  #include "rail.h"

  struct _signal_stating {
    enum IO_event state[8];
  };
  
  typedef struct _signal{
    uint16_t id;
    uint8_t module;
    Block * B;
    enum Rail_states state;

    uint8_t output_len;

    IO_Port ** output;
    struct _signal_stating * output_stating;

  } Signal;


  
  #define create_signal_from_conf(module, data) create_signal(module, data.blockId, data.id, data.side, data.output_len, data.output, data.stating)
  void create_signal(uint8_t module, uint8_t blockId, uint16_t signalId, _Bool side, char output_len, struct s_IO_port_conf * output, struct s_IO_signal_event_conf * stating);
  
  void * clear_Signal(Signal * Sig);

  void set_signal(Signal *Si, enum Rail_states state);
#endif

