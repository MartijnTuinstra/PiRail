#include <stdlib.h>
#include <stdint.h>
#include "module.h"

#ifndef INCLUDE_IO
#define INCLUDE_IO

typedef struct s_unit Unit;

typedef struct s_node_adr {
  uint8_t Node;
  uint16_t io;
} Node_adr;

enum IO_type {
  IO_Undefined,
  IO_Output,
  IO_Input_Block,
  IO_Input_Switch,
  IO_Input_MSSwitch,
  IO_Input
};

enum IO_event {
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
  enum IO_event w_state;
  enum IO_event r_state;
  enum IO_type type;

  void * object;
} IO_Port;

typedef struct s_IO_Node {
  uint8_t id;
  uint16_t io_ports;
  IO_Port ** io;
} IO_Node;

extern const char * IO_type_str[6];
extern const char * IO_event_str[13];

#define U_IO(a, b, c) Units[a]->Node[b].io[c]

void Add_IO_Node(Unit * U, int Node_nr, int IO);

void Init_IO(Unit * U, Node_adr adr, enum IO_type type);

void update_IO();
void IO_set_input(uint8_t module, uint8_t id, uint8_t port, uint8_t state);

#endif
