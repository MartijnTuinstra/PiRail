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
  union u_IO_event state[8];
};

typedef struct s_Signal{
  uint16_t id;            // Signal ID
  uint8_t module;         // Module number
  Block * B;              // Parent block
  enum Rail_states state; // State of the signal

  uint8_t output_len;     // Number of IO outputs

  IO_Port ** output;      // List of IO_port pointers
  struct s_signal_stating * output_stating;

} Signal;

#define U_Sig(M, S) Units[M]->Sig[S]

#define create_signal_from_conf(module, data) Create_Signal(module, data.blockId, data.id, data.side, data.output_len, data.output, data.stating)
void Create_Signal(uint8_t module, uint8_t blockId, uint16_t signalId, bool side, char output_len, struct s_IO_port_conf * output, struct s_IO_signal_event_conf * stating);
Signal * Clear_Signal(Signal * Sig);

void check_Signal(Signal * Si);
void set_signal(Signal *Si, enum Rail_states state);
#endif

