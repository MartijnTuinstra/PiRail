#include <stdio.h>
#include <stdlib.h>
#include "IO.h"

#include "logger.h"
#include "system.h"
#include "mem.h"
#include "com.h"

void Add_IO_Node(Unit * U, struct node_conf node){
  IO_Node Z;

  Z.id = node.Node;
  Z.io_ports = node.size;
  
  Z.io = _calloc(node.size, IO_Port*);

  for(int i = 0; i<node.size; i++){
    Z.io[i] = _calloc(1, IO_Port);
    Z.io[i]->type = (node.data[i/2] >> (4 * (i % 2))) & 0xF;
    loggerf(INFO, "IO Port %i:%i -- %s", Z.id, i, IO_enum_type_string[Z.io[i]->type]);
  }

  if(U->IO_Nodes <= node.Node){
    U->Node = _realloc(U->Node, U->IO_Nodes + 1, IO_Node);
  }

  loggerf(DEBUG, "Node %02i:%02i created (0-%3i)", U->module, node.Node, node.size);

  U->Node[Z.id] = Z;
  return;
}

void Init_IO_from_conf(Unit * U, struct s_IO_port_conf adr, void * pntr){
  Node_adr new_adr = {adr.Node, adr.Adr};
  Init_IO(U, new_adr, pntr);
}

void Init_IO(Unit * U, Node_adr adr, void * pntr){
  if((adr.Node < U->IO_Nodes) && 
     (adr.io < U->Node[adr.Node].io_ports) &&
     U->Node[adr.Node].io[adr.io]){

    IO_Port * A = U->Node[adr.Node].io[adr.io];

    // if(A->type != IO_Undefined)
    //   loggerf(WARNING, "IO %i:%i:%i already in use", U->module, adr.Node, adr.io);
    if(A->type == IO_Undefined)
      loggerf(WARNING, "IO %i:%i:%i is not configured correctly", U->module, adr.Node, adr.io);

    A->object = pntr;
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
        if(U_IO(u, n, io)->type == IO_Output && U_IO(u, n, io)->w_state.value != U_IO(u, n, io)->r_state.value){;
          loggerf(DEBUG, "Update io %02i:%02i:%02i %s", u, n, io,
                  IO_event_string[U_IO(u, n, io)->type][U_IO(u, n, io)->w_state.value]);

          buf[io/4 + 2] = U_IO(u, n, io)->w_state.value << ((io % 4) * 2);

          if(U_IO(u, n, io)->w_state.output == IO_event_Pulse) // Reset When pulsing output
            U_IO(u, n, io)->w_state.output = IO_event_Low;
            U_IO(u, n, io)->r_state.value = U_IO(u, n, io)->w_state.value;
        }
      }
    }
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
