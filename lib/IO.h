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
  IO_Output_Blink,
  IO_Output_Servo,
  IO_Output_PWM,
  IO_Input,
  IO_Input_Block,
  IO_Input_Switch,
  IO_Input_MSSwitch,
};

enum e_IO_output_event {
  IO_event_Low,
  IO_event_High,
  IO_event_Pulse,
  IO_event_Toggle
};

enum e_IO_blink_event {
  IO_event_B_Low,
  IO_event_B_High,
  IO_event_Blink1,
  IO_event_Blink2
};

enum e_IO_servo_event {
  IO_event_Servo1,
  IO_event_Servo2,
  IO_event_Servo3,
  IO_event_Servo4
};

enum e_IO_PWM_event {
  IO_event_PWM1,
  IO_event_PWM2,
  IO_event_PWM3,
  IO_event_PWM4
};

union u_IO_event {
  enum e_IO_output_event output;
  enum e_IO_blink_event blink;
  enum e_IO_servo_event servo;
  enum e_IO_PWM_event pwm;

  uint8_t value;
};

typedef struct s_IO_Port {
  uint8_t id;
  union u_IO_event w_state;
  union u_IO_event r_state;
  enum e_IO_type type;

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


extern const char * IO_enum_type_string[9];
extern const char ** IO_event_string[9];


void Add_IO_Node(Unit * U, int Node_nr, int IO);

void Init_IO_from_conf(Unit * U, struct s_IO_port_conf adr, enum e_IO_type type);
void Init_IO(Unit * U, Node_adr adr, enum e_IO_type type);

void update_IO();
void update_IO_Module(uint8_t module);

void IO_set_input(uint8_t module, uint8_t id, uint8_t port, uint8_t state);

#endif
