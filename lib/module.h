#ifndef _INCLUDE_MODULE_H
  #define _INCLUDE_MODULE_H

  #include "switch.h"
  #include "rail.h"
  #include "signals.h"
  #include "IO.h"

  typedef struct s_unit {
    uint8_t module;

    uint8_t connections_len;
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

    uint8_t changed;

    uint8_t block_state_changed:1;
    uint8_t switch_state_changed:1;
    uint8_t msswitch_state_changed:1;
    uint8_t signal_state_changed:1;

    uint8_t on_layout:1;
  } Unit;

  #define ModuleConfigBasePath "./configs/units/"

  struct rail_link CAdr(int module, int id, char type);

  #define Unit_Blocks_changed 0x1
  #define Unit_Switch_changed 0x2
  #define Unit_MSSwitch_changed 0x4
  #define Unit_Signal_changed 0x8

  extern int unit_len;
  extern Unit ** Units;

  void init_modules();

  void clear_Modules();

  void Unit_expand_IO(_Bool type, Unit * U);
  
  void LoadModuleFromConfig(int M);
  void ReadAllModuleConfigs();

  void JoinModules();

  void setup_JSON(int arr[], int arr2[], int size, int size2);
#endif
