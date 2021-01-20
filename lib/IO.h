#ifndef INCLUDE_IO
#define INCLUDE_IO

#include <stdlib.h>
#include <stdint.h>

#include "switchboard/declares.h"
#include "config_data.h"


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

class IO_Node;

struct configStruct_IOport;
struct configStruct_Node;

class IO_Port {
  public:
    uint8_t id;
    union u_IO_event w_state;
    union u_IO_event r_state;
    enum e_IO_type type;

    union {
      Block * B;
      Switch * Sw;
      MSSwitch * MSSw;
      void * p;
    } p;

    IO_Node * Node;

    IO_Port(IO_Node * Node, uint8_t id, enum e_IO_type type);

    void exportConfig(struct configStruct_IOport *);

    void link(void * pntr, enum e_IO_type type);

    void setInput(uint8_t state);
    void setOutput(uint8_t state);
    void setOutput(union u_IO_event state);
};

class IO_Node {
  public:
    uint8_t id;
    uint16_t io_ports;
    IO_Port ** io;

    Unit * U;

    bool updated;

    IO_Node(Unit * U, struct node_conf conf);
    IO_Node(Unit * U, struct configStruct_Node * conf);
    ~IO_Node();

    inline void update();
};

extern const char * IO_type_str[6];
extern const char * IO_event_str[13];
extern const char * IO_enum_type_string[9];
extern const char ** IO_event_string[9];

#define U_IO(a, b, c) Units[a]->Node[b].io[c]




// void Add_IO_Node(Unit * U, struct node_conf node);

// void Init_IO_from_conf(Unit * U, struct s_IO_port_conf adr, void * pntr);
// void Init_IO(Unit * U, Node_adr adr, void * pntr);

void update_IO();
void update_IO_Module(uint8_t module);

// void IO_set_input(IO_Port * port, uint8_t state);

#endif
