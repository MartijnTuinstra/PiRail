#include <stdio.h>
#include <stdlib.h>
#include "com.h"
#include "logger.h"
#include "system.h"
#include "mem.h"
#include "IO.h"
#include "com.h"
#include "algorithm.h"

void Add_IO_Node(Unit * U, int Node_nr, int IO){
  IO_Node Z;

  Z.id = Node_nr;
  Z.io_ports = IO;
  
  Z.io = _calloc(IO, IO_Port*);

  for(int i = 0; i<IO; i++){
    Z.io[i] = _calloc(1, IO_Port);
  }

  if(U->IO_Nodes <= Node_nr){
    U->Node = _realloc(U->Node, U->IO_Nodes + 1, IO_Node);
  }

  loggerf(DEBUG, "Node %02i:%02i created (0-%3i)", U->module, Node_nr, IO);

  U->Node[Node_nr] = Z;
  return;
}

void Init_IO_from_conf(Unit * U, struct s_IO_port_conf adr, enum e_IO_type type){
  Node_adr new_adr = {adr.Node, adr.Adr};
  Init_IO(U, new_adr, type);
}

void Init_IO(Unit * U, Node_adr adr, enum e_IO_type type){
  if((adr.Node < U->IO_Nodes) && 
     (adr.io < U->Node[adr.Node].io_ports) &&
     U->Node[adr.Node].io[adr.io]){

    IO_Port * A = U->Node[adr.Node].io[adr.io];

    if(A->type != IO_Undefined)
      loggerf(WARNING, "IO %i:%i:%i already in use", U->module, adr.Node, adr.io);

    A->type = type;
    A->id = adr.io;
  }
  else{
    loggerf(ERROR, "Init_IO Error, addr %02x:%02x is not in range", adr.Node, adr.io);
  }
}

void update_IO(){
  for(int u = 0; u < unit_len; u++){
    if(!Units[u] || Units[u]->io_out_changed == 0)
      continue;

    for(int n = 0; n < Units[u]->IO_Nodes; n++){
      char buf[100];
      memset(buf, 0, 100);
      buf[0] = n;
      buf[1] = COMopc_SetAllOut;

      for(int io = 0; io < Units[u]->Node[n].io_ports; io++){
        if(U_IO(u, n, io)->type == IO_Output && U_IO(u, n, io)->w_state.value != U_IO(u, n, io)->r_state.value){
          loggerf(WARNING, "Update io %02i:%02i:%02i %s", u, n, io, IO_event_string[U_IO(u, n, io)->w_state.value]);
          U_IO(u, n, io)->r_state.value = U_IO(u, n, io)->w_state.value;

          buf[io/4 + 2] = U_IO(u, n, io)->w_state.value << ((io % 4) * 2);

          if(U_IO(u, n, io)->w_state.output == IO_event_Pulse) // Reset When pulsing output
            U_IO(u, n, io)->w_state.output = IO_event_Low;
            U_IO(u, n, io)->r_state.value = U_IO(u, n, io)->w_state.value;
        }
      }
    }
  }
}

void IO_set_input(uint8_t module, uint8_t id, uint8_t port, uint8_t state){
  if(!Units[module] || !Units[module]->Node || !U_IO(module,id,port))
    return;

  if(state){ // High
    U_IO(module, id, port)->w_state.value = IO_event_High;
    U_IO(module, id, port)->r_state.value = IO_event_High;
  }
  else{ // Low
    U_IO(module, id, port)->w_state.value = IO_event_Low;
    U_IO(module, id, port)->r_state.value = IO_event_Low;
  }

  if(U_IO(module, id, port)->type == IO_Input_Block){
    ((Block *)U_IO(module, id, port)->object)->IOchanged = 1;
    putAlgorQueue((Block *)U_IO(module, id, port)->object, 1);
  }
  else{
    loggerf(CRITICAL, "Unknown io type %i", U_IO(module, id, port)->type);
  }

}

const char * IO_enum_type_string[9] = {
  "IO_Undefined",
  "IO_Output",
  "IO_Output_Blink",
  "IO_Output_Servo",
  "IO_Output_PWM",
  "IO_Input_Block",
  "IO_Input_Switch",
  "IO_Input_MSSwitch",
  "IO_Input"
};

const char * IO_undefined_string[4] = {
  "IO_event_undefined",
  "IO_event_undefined",
  "IO_event_undefined",
  "IO_event_undefined",
};
const char * IO_output_string[4] = {
  "IO_event_High",
  "IO_event_Low",
  "IO_event_Pulse",
  "IO_event_Toggle"
};
const char * IO_blink_string[4] = {
  "IO_event_B_High",
  "IO_event_B_Low",
  "IO_event_Blink1",
  "IO_event_Blink2"
};
const char * IO_servo_string[4] = {
  "IO_event_Servo1",
  "IO_event_Servo2",
  "IO_event_Servo3",
  "IO_event_Servo4"
};
const char * IO_pwm_string[4] = {
  "IO_event_PWM1",
  "IO_event_PWM2",
  "IO_event_PWM3",
  "IO_event_PWM4"
};
const char ** IO_event_string[9] = {
  IO_undefined_string,
  IO_output_string,
  IO_blink_string,
  IO_servo_string,
  IO_pwm_string,
  &IO_enum_type_string[5],
  &IO_enum_type_string[6],
  &IO_enum_type_string[7],
  &IO_enum_type_string[8]
};
