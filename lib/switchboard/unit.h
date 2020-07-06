#ifndef _INCLUDE_SWITCHBOARD_UNIT_H
#define _INCLUDE_SWITCHBOARD_UNIT_H

#include "switchboard/declares.h"
#include "config/ModuleConfig.h"
#include "IO.h"

class Unit {
  public:
    uint8_t module;

    uint8_t connections_len;
    Unit ** connection;

    uint8_t IO_Nodes;
    IO_Node ** Node;

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

    bool io_updated;

    uint8_t on_layout:1;

    uint16_t Layout_length;
    char * Layout;

    uint16_t raw_length;
    char * raw;

    Unit(uint16_t M, uint8_t Nodes, char points);
    Unit(ModuleConfig * Config);
    ~Unit();

    void insertBlock(Block * B);
    void insertSwitch(Switch * Sw);
    void insertMSSwitch(MSSwitch * MSSw);
    void insertStation(Station * St);
    void insertSignal(Signal * Sig);

    IO_Port * linkIO(Node_adr adr, void * pntr, enum e_IO_type type);
    IO_Port * linkIO(struct s_IO_port_conf adr, void * pntr, enum e_IO_type type);
    IO_Port * IO(Node_adr adr);
    IO_Port * IO(struct s_IO_port_conf adr);

    void updateIO(int uart_filestream);

    void link_all();
};

extern int unit_len;
extern Unit ** Units;

#endif