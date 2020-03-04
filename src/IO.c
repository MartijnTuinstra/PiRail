#include <stdio.h>
#include <stdlib.h>
#include "logger.h"
#include "system.h"
#include "mem.h"
#include "IO.h"
#include "com.h"

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
        if(U_IO(u, n, io)->type == IO_Output && U_IO(u, n, io)->w_state != U_IO(u, n, io)->r_state){
          char output[200];
          sprintf(output, "Update io %02i:%02i:%02i", u, n, io);
          str_IO_event(U_IO(u, n, io)->w_state, output);
          loggerf(DEBUG, "%s", output);

	  if(io % 2 == 0){
            buf[io/2 + 2] = U_IO(u, n, io)->w_state;
          }
	  else{ // i% 2 == 1
            buf[io/2 + 2] |= U_IO(u, n, io)->w_state << 4;
          }
	  if(U_IO(u, n, io)->w_state == IO_event_Pulse) // Reset When pulsing output
            U_IO(u, n, io)->w_state = IO_event_Low;
          U_IO(u, n, io)->r_state = U_IO(u, n, io)->w_state;
        }
      }
    }
  }
}



void str_IO_type(enum e_IO_type type, char * str){
  if(type == IO_Undefined){
    sprintf(str, "%s IO_Undefined", str);
  }
  else if(type == IO_Output){
    sprintf(str, "%s IO_Output", str);
  }
  else if(type == IO_Input_Block){
    sprintf(str, "%s IO_Input_Block", str);
  }
  else if(type == IO_Input_Switch){
    sprintf(str, "%s IO_Input_Switch", str);
  }
  else if(type == IO_Input_MSSwitch){
    sprintf(str, "%s IO_Input_MSSwitch", str);
  }
  else if(type == IO_Input){
    sprintf(str, "%s IO_Input", str);
  }
}
void str_IO_event(enum e_IO_event event, char * str){
  if(event == IO_event_High){
    sprintf(str, "%s IO_event_High", str);
  }
  else if(event == IO_event_Low){
    sprintf(str, "%s IO_event_Low", str);
  }
  else if(event == IO_event_Pulse){
    sprintf(str, "%s IO_event_Pulse", str);
  }
  else if(event == IO_event_Blink1){
    sprintf(str, "%s IO_event_Blink1", str);
  }
  else if(event == IO_event_Blink2){
    sprintf(str, "%s IO_event_Blink2", str);
  }

  else if(event == IO_event_Servo1){
    sprintf(str, "%s IO_event_Servo1", str);
  }
  else if(event == IO_event_Servo2){
    sprintf(str, "%s IO_event_Servo2", str);
  }
  else if(event == IO_event_Servo3){
    sprintf(str, "%s IO_event_Servo3", str);
  }
  else if(event == IO_event_Servo4){
    sprintf(str, "%s IO_event_Servo4", str);
  }
  else if(event == IO_event_PWM1){
    sprintf(str, "%s IO_event_PWM1", str);
  }
  else if(event == IO_event_PWM2){
    sprintf(str, "%s IO_event_PWM2", str);
  }
  else if(event == IO_event_PWM3){
    sprintf(str, "%s IO_event_PWM3", str);
  }
  else if(event == IO_event_PWM4){
    sprintf(str, "%s IO_event_PWM4", str);
  }
}
