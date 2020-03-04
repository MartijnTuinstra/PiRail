#ifndef _INCLUDE_MODULE_H
#define _INCLUDE_MODULE_H

#include <stdint.h>

#ifndef _INCLUDE_RAIL_H
typedef struct s_Block Block;
typedef struct s_Switch Switch;
typedef struct s_MSSwitch MSSwitch;
typedef struct s_Signal Signal;
typedef struct s_Station Station;
#endif
#ifndef _INCLUDE_IO_H
typedef struct s_IO_Node IO_Node;
#endif

struct s_unit;

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

  uint8_t io_out_changed:1;

  uint8_t on_layout:1;

  uint16_t Layout_length;
  char * Layout;

  uint16_t raw_length;
  char * raw;
} Unit;

extern int unit_len;
extern Unit ** Units;

#define ModuleConfigBasePath "./configs/units/"

#define Unit_Blocks_changed 0x1
#define Unit_Switch_changed 0x2
#define Unit_MSSwitch_changed 0x4
#define Unit_Signal_changed 0x8

void Create_Unit(uint16_t M, uint8_t Nodes, char points);
void * Clear_Unit(Unit * U);

void read_module_Config(uint16_t M);
void write_module_Config(uint16_t M);
void load_module_Configs();
void unload_module_Configs();

#endif
