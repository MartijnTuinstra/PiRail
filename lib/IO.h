#ifndef INCLUDE_IO
#define INCLUDE_IO

#include <stdlib.h>
#include <stdint.h>
#include "modules.h"
#include "config_data.h"

typedef struct s_unit Unit;

typedef struct s_node_adr {
  uint8_t Node;
  uint16_t io;
} Node_adr;

enum e_IO_type {
  IO_Undefined,
  IO_Output,
  IO_Input_Block,
  IO_Input_Switch,
  IO_Input_MSSwitch,
  IO_Input
};

enum e_IO_event {
  IO_event_High,
  IO_event_Low,
  IO_event_Pulse,
  IO_event_Blink1,
  IO_event_Blink2,

  IO_event_Servo1,
  IO_event_Servo2,
  IO_event_Servo3,
  IO_event_Servo4,
  IO_event_PWM1,
  IO_event_PWM2,
  IO_event_PWM3,
  IO_event_PWM4
};

typedef struct s_IO_Port {
  uint8_t id;
  enum e_IO_event w_state;
  enum e_IO_event r_state;
  enum e_IO_type type;

  void * object;
} IO_Port;

typedef struct s_IO_Node {
  uint8_t id;
  uint16_t io_ports;
  IO_Port ** io;
} IO_Node;

#define U_IO(a, b, c) Units[a]->Node[b].io[c]

void Add_IO_Node(Unit * U, int Node_nr, int IO);

void Init_IO_from_conf(Unit * U, struct s_IO_port_conf adr, enum e_IO_type type);
void Init_IO(Unit * U, Node_adr adr, enum e_IO_type type);

void update_IO();

void str_IO_type(enum e_IO_type type, char * str);
void str_IO_event(enum e_IO_event event, char * str);

#endif
