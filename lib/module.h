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
