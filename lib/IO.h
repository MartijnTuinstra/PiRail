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
	IO_Input,
	IO_Output
};

typedef struct s_IO_Port {
	uint8_t id;
	uint8_t state;
	enum IO_type type;
} IO_Port;

typedef struct s_IO_Node {
	uint8_t id;
	uint16_t io_ports;
	IO_Port ** io;
} IO_Node;

enum IO_event {
  IO_event_Pulse,
  IO_event_High,
  IO_event_Low,
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

void Add_IO_Node(Unit * U, int Node_nr, int IO);

#endif
