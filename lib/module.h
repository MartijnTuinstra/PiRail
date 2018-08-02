#ifndef _INCLUDE_MODULE_H
  #define _INCLUDE_MODULE_H

  #include "switch.h"
  #include "rail.h"
  #include "signals.h"
  #include "IO.h"

  typedef struct s_unit {
    char module;

    char connections_len;
    struct s_unit ** connection;

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
  } Unit;

  struct s_unit_conf {
    char module;
    char connections;
    char IO_Nodes;
    uint16_t Blcoks;
    uint16_t Switches;
    uint16_t MSSwitches;
    uint16_t Signals;
    uint16_t Stations;
  };

  struct s_link_conf {
    char type;
    char module;
    char id;
  };

  struct s_IO_port_conf {
    char Node;
    char Adr;
  };

  struct s_block_conf {
    char id;
    enum Rail_types type;
    struct s_link_conf next;
    struct s_link_conf prev;
    struct s_IO_port_conf IO;
    char speed;
    uint16_t length;
    char fl; //FLAGS, 0x1 = OneWay, 0x6 = dir
  };

  struct s_switch_conf {
    char id;
    char det_block;
    struct s_link_conf App;
    struct s_link_conf Str;
    struct s_link_conf Div;
  };

  struct s_switch_state_conf {
    char Node;
    char Adr;
    enum IO_event on_state_set;
    char speed;
  };

  struct rail_link CAdr(int module, int id, char type);

  extern int unit_len;
  extern Unit ** Units;

  void init_modules();

  void free_modules();

  void clear_modules();

  void Unit_expand_IO(_Bool type, Unit * U);

  void LoadModules(int M);

  void JoinModules();

  void setup_JSON(int arr[], int arr2[], int size, int size2);
#endif
