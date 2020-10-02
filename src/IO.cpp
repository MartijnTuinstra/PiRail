#include <stdio.h>
#include <stdlib.h>
#include "IO.h"

#include "logger.h"
#include "system.h"
#include "mem.h"
#include "com.h"
#include "algorithm/queue.h"

#include "switchboard/manager.h"
#include "switchboard/unit.h"

using namespace switchboard;

IO_Node::IO_Node(Unit * U, struct node_conf conf){
  loggerf(DEBUG, "  Node %02i (0-%3i)", conf.Node, conf.size);
  memset(this, 0 , sizeof(IO_Node));

  this->id = conf.Node;
  this->io_ports = conf.size;
  
  this->io = (IO_Port **)_calloc(conf.size, IO_Port*);
  // this->io = new IO_Port[conf.size];

  // loggerf(CRITICAL, "TODO IMPLEMENT node data"); // TODO: What is this?

  for(int i = 0; i < conf.size; i++){
    this->io[i] = new IO_Port(this, i, (enum e_IO_type)((conf.data[i/2] >> (4 * (i % 2))) & 0xF));
    // this->io[i]->type = (enum e_IO_type);
  }

  if(U->IO_Nodes <= conf.Node){
    U->Node = (IO_Node **)_realloc(U->Node, U->IO_Nodes + 1, IO_Node *);
  }


  this->U = U;
  U->Node[this->id] = this;
}

IO_Node::~IO_Node(){
  for(int i = 0; i < this->io_ports; i++){
    delete this->io[i];
  }
  _free(this->io);
}

inline void IO_Node::update(){
  this->updated = true;
  this->U->io_updated = true;
};

// IO_Port * IO_Node::linkPort(uint8_t port, void * pntr){
//   this->io[port]->link(pntr);

//   return this->io[port];
// }






IO_Port::IO_Port(IO_Node * Node, uint8_t id, enum e_IO_type type){
  loggerf(TRACE, "    IO %i - %s", id, IO_enum_type_string[type]);
  memset(this, 0, sizeof(IO_Port));

  this->Node = Node;
  this->id = id;
  this->type = type;
}

void IO_Port::link(void * pntr, enum e_IO_type type){
  if(this->type == IO_Undefined)
    loggerf(WARNING, "IO %i:%i:%i is not configured correctly", this->Node->U->module, this->Node->id, this->id);
  else if(type == IO_Output && this->type >= IO_Input)
    loggerf(WARNING, "IO %i:%i:%i set as %s but is linked to an Output", this->Node->U->module, this->Node->id, this->id, IO_enum_type_string[this->type]);
  else if(type >= IO_Input && this->type < IO_Input)
    loggerf(WARNING, "IO %i:%i:%i set as %s but is linked to an Input", this->Node->U->module, this->Node->id, this->id, IO_enum_type_string[this->type]);

  this->p.p = pntr;
}

void IO_Port::setOutput(uint8_t state){
  if(this->type == IO_Undefined || this->type >= IO_Input){
    loggerf(WARNING, "setOutput on input port");
    return;
  }

  this->w_state.value = state;
  this->Node->update();
}

void IO_Port::setOutput(union u_IO_event state){
  this->setOutput(state.value);
}

void IO_Port::setInput(uint8_t state){
  if(this->type == IO_Undefined)
    return;

  if(this->type < IO_Input){
    loggerf(WARNING, "setInput on output port");
    return;
  }

  this->w_state.value = state;

  if(!this->p.p && this->type >= IO_Input_Block){
    loggerf(WARNING, "IO %02d:%02d:%02d without any link", this->Node->U->module, this->Node->id, this->id);
    return;
  }

  if(w_state.value != r_state.value){
    if(type == IO_Input_Block){
      if(state == IO_event_High)
        p.B->detectionblocked = 1;
      else
        p.B->detectionblocked = 0;

      loggerf(INFO, "IO updated %02i:%02i:%02i\t%s", Node->U->module, Node->id, id, IO_event_string[1][w_state.value]);

      p.B->IOchanged = 1;
      AlQueue.put(p.B);
    }
    else if(type == IO_Input_Switch){
      p.Sw->updateFeedback();
    }
    else{
      loggerf(CRITICAL, "Unknown io type %i", this->type);
    }
  }

  this->r_state.value = this->w_state.value;
}

// IO_Port::update(){

// }


// void Init_IO_from_conf(Unit * U, struct s_IO_port_conf adr, void * pntr){
//   Node_adr new_adr = {adr.Node, adr.Adr};
//   Init_IO(U, new_adr, pntr);
// }

// void Init_IO(Unit * U, Node_adr adr, void * pntr){
//   if((adr.Node < U->IO_Nodes) && 
//      (adr.io < U->Node[adr.Node].io_ports) &&
//      U->Node[adr.Node].io[adr.io]){

//     IO_Port * A = U->Node[adr.Node].io[adr.io];

//     // if(A->type != IO_Undefined)
//     //   loggerf(WARNING, "IO %i:%i:%i already in use", U->module, adr.Node, adr.io);
//     if(A->type == IO_Undefined)
//       loggerf(WARNING, "IO %i:%i:%i is not configured correctly", U->module, adr.Node, adr.io);

//     A->object = pntr;
//     A->id = adr.io;
//   }
//   else{
//     loggerf(ERROR, "Init_IO Error, addr %02x:%02x is not in range", adr.Node, adr.io);
//   }
// }

void update_IO(){
  for(int u = 0; u < SwManager->Units.size; u++){
    if(!Units(u) || !Units(u)->io_updated)
      continue;

    Units(u)->updateIO(0);
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
  "IO_event_Low",
  "IO_event_High",
  "IO_event_Pulse",
  "IO_event_Toggle"
};
const char * IO_blink_string[4] = {
  "IO_event_B_Low",
  "IO_event_B_High",
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
