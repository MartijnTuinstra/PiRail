#ifndef _INCLUDE_SIGNALS_H
#define _INCLUDE_SIGNALS_H

#include "IO.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"

#include "config/LayoutStructure.h"

/**///Signal passing speed
//Flashing Red speed
#define RED_F_SPEED 20
//Flashing Amber speed
#define AMBER_F_SPEED 30
//Amber speed
#define AMBER_SPEED 40

struct s_signal_stating {
  union u_IO_event state[8];
};

struct SignalSwitchLink {
  bool MSSw;
  union {
    Switch * Sw;
    MSSwitch * MSSw;
    void * p;
  } p;
  uint8_t state;
};

class Signal {
  public:
    uint8_t module;         // Module number
    uint16_t id;            // Signal ID
    uint16_t uid;
    Unit * U;

    bool direction;         // Forward?

    RailLink block_link;
    Block * B = 0;              // Parent block
    void * tmpP = 0;           // Block where the signal is standing
    enum Rail_states state = BLOCKED; // State of the signal

    uint8_t output_len = 0;     // Number of IO outputs

    IO_Port ** output = 0;      // List of IO_port pointers
    struct s_signal_stating * output_stating = 0;

    bool switchDanger = 0;
    std::vector<struct SignalSwitchLink> Switches;

    Signal(uint8_t, struct configStruct_Signal *);
    ~Signal();

    void map();

    void exportConfig(struct configStruct_Signal *);

    void check();
    void set(enum Rail_states state);
    void setIO();
    void switchUpdate();
};

#define U_Sig(M, S) Units[M]->Sig[S]


// void Create_Signal(uint8_t module, uint8_t blockId, uint16_t signalId, bool side, char output_len, struct s_IO_port_conf * output, struct s_IO_signal_event_conf * stating);
// Signal * Clear_Signal(Signal * Sig);

// void check_Signal(Signal * Si);
// void set_signal(Signal *Si, enum Rail_states state);
#endif

