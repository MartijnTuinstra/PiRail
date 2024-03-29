#include <stdio.h>
#include <stdlib.h>
#include "IO.h"

#include "utils/logger.h"
#include "utils/mem.h"
#include "utils/strings.h"
#include "system.h"
#include "algorithm/queue.h"

#include "config/LayoutStructure.h"

#include "switchboard/manager.h"
#include "switchboard/unit.h"
#include "switchboard/switch.h"

using namespace switchboard;

IO_Node::IO_Node(Unit * U, struct configStruct_Node * conf){
  loggerf(DEBUG, "  Node %02i (0-%3i)", conf->Node, conf->ports);
  memset(this, 0 , sizeof(IO_Node));

  this->id = conf->Node;
  this->io_ports = conf->ports;
  
  this->io = (IO_Port **)_calloc(conf->ports, IO_Port*);
  // this->io = new IO_Port[conf->ports];

  // loggerf(CRITICAL, "TODO IMPLEMENT node data"); // TODO: What is this?

  for(int i = 0; i < conf->ports; i++){
    this->io[i] = new IO_Port(this, i, (enum e_IO_type)(conf->config[i].type));
    // this->io[i]->type = (enum e_IO_type);
  }

  if(U->IO_Nodes <= conf->Node){
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

void IO_Port::exportConfig(struct configStruct_IOport * cfg){
  printf("Exporting IO: %i:%i\n", Node->id, id);

  cfg->Node = Node->id;
  cfg->Port = id;
}

void IO_Port::link(void * pntr, enum e_IO_type type){
  if(this->type == IO_Undefined)
    loggerf(WARNING, "IO %i:%i:%i is not configured correctly", Node->U->module, Node->id, id);
  else if(type == IO_Output && type >= IO_Input)
    loggerf(WARNING, "IO %i:%i:%i set as %s but is linked to an Output", Node->U->module, Node->id, id, IO_enum_type_string[type]);
  else if(type >= IO_Input && type < IO_Input)
    loggerf(WARNING, "IO %i:%i:%i set as %s but is linked to an Input", Node->U->module, Node->id, id, IO_enum_type_string[type]);

  p.p = pntr;
}

void IO_Port::setOutput(uint8_t state){
  loggerf(TRACE, "IO_Port::setOutput %2i:%2i -> %s", Node->id, id, IO_event_string[type][state]);
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
      p.B->setDetection(state == IO_event_High);

      loggerf(TRACE, "IO updated %02i:%02i:%02i\t%s", Node->U->module, Node->id, id, IO_event_string[1][w_state.value]);

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

    Units(u)->updateIO();
  }
}

