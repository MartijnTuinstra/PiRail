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

struct s_signal_stating {
  enum e_IO_event state[8];
};

typedef struct s_Signal{
  uint16_t id;
  uint8_t module;
  Block * B;
  enum Rail_states state;

  uint8_t output_len;

  IO_Port ** output;
  struct s_signal_stating * output_stating;

} Signal;

#define U_Sig(M, S) Units[M]->Sig[S]

#define create_signal_from_conf(module, data) Create_Signal(module, data.blockId, data.id, data.side, data.output_len, data.output, data.stating)
void Create_Signal(uint8_t module, uint8_t blockId, uint16_t signalId, _Bool side, char output_len, struct s_IO_port_conf * output, struct s_IO_signal_event_conf * stating);
void * Clear_Signal(Signal * Sig);

void check_Signal(Signal * Si);
void set_signal(Signal *Si, enum Rail_states state);
#endif

