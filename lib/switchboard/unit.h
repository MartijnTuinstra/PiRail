#ifndef _INCLUDE_SWITCHBOARD_UNIT_H
#define _INCLUDE_SWITCHBOARD_UNIT_H

#include "switchboard/declares.h"

typedef struct s_IO_Node IO_Node;

class Unit {
  public:
    uint8_t module;

    uint8_t connections_len;
    Unit ** connection;

    uint8_t IO_Nodes;
    IO_Node * Node;

    int block_len;
    Block ** B;

    int switch_len;
    Switch ** Sw;

    int msswitch_len;
    MSSwitch ** MSSw;

    int signal_len;
    Signal ** Sig;

    int station_len;
    Station ** St;

    uint8_t changed;

    uint8_t block_state_changed:1;
    uint8_t switch_state_changed:1;
    uint8_t msswitch_state_changed:1;
    uint8_t signal_state_changed:1;

    uint8_t io_out_changed:1;

    uint8_t on_layout:1;

    uint16_t Layout_length;
    char * Layout;

    uint16_t raw_length;
    char * raw;

    Unit(uint16_t M, uint8_t Nodes, char points);
    ~Unit();

    void insertBlock(Block * B);
    void insertSwitch(Switch * Sw);
    void insertMSSwitch(MSSwitch * MSSw);
};

extern int unit_len;
extern Unit ** Units;

#endif