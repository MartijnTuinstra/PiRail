#include <stdio.h>
#include <stdlib.h>
#include "com.h"
#include "logger.h"
#include "system.h"
#include "mem.h"
#include "IO.h"

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

void Init_IO(Unit * U, Node_adr adr, enum IO_type type){
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
    loggerf(ERROR, "Init_IO Error");
  }
}

void update_IO(){
  for(int u = 0; u < unit_len; u++){
    if(!Units[u] || Units[u]->io_out_changed == 0)
      continue;

    for(int n = 0; n < Units[u]->IO_Nodes; n++){
      for(int io = 0; io < Units[u]->Node[n].io_ports; io++){
        if(U_IO(u, n, io)->type == IO_Output && U_IO(u, n, io)->w_state != U_IO(u, n, io)->r_state){
          loggerf(WARNING, "Update io %02i:%02i:%02i %s", u, n, io, IO_event_str[U_IO(u, n, io)->w_state]);
          U_IO(u, n, io)->r_state = U_IO(u, n, io)->w_state;
        }
      }
    }
  }
}

void IO_set_input(uint8_t module, uint8_t id, uint8_t port, uint8_t state){
  if(!Units[module] || !Units[module]->Node || !U_IO(module,id,port))
    return;

  if(state){ // High
    U_IO(module, id, port)->w_state = IO_event_High;
    U_IO(module, id, port)->r_state = IO_event_High;
  }
  else{ // Low
    U_IO(module, id, port)->w_state = IO_event_Low;
    U_IO(module, id, port)->r_state = IO_event_Low;
  }

  if(U_IO(module, id, port)->type == IO_Input_Block){
    ((Block *)U_IO(module, id, port)->object)->changed |= IO_Changed;
    putAlgorQueue((Block *)U_IO(module, id, port)->object, 1);
  }
  else{
    loggerf(CRITICAL, "Unknown io type %i", U_IO(module, id, port)->type);
  }

}

const char * IO_type_str[6] = {
  "IO_Undefined", "IO_Output", "IO_Input_Block", "IO_Input_Switch", "IO_Input_MSSwitch", "IO_Input"
};
const char * IO_event_str[13] = {
  "IO_event_High", "IO_event_Low", "IO_event_Pulse", "IO_event_Blink1", "IO_event_Blink2",
  "IO_event_Servo1", "IO_event_Servo2", "IO_event_Servo3", "IO_event_Servo4",
  "IO_event_PWM1", "IO_event_PWM2", "IO_event_PWM3", "IO_event_PWM4"
};

void str_IO_type(enum IO_type type, char * str){

}
void str_IO_event(enum IO_event event, char * str){
}