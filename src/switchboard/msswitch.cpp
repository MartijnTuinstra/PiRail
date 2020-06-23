#include "switchboard/switch.h"
#include "switchboard/msswitch.h"

#include "mem.h"
#include "logger.h"
#include "IO.h"
#include "modules.h"
#include "system.h"
#include "algorithm.h"


void create_msswitch_from_conf(uint8_t module, struct ms_switch_conf conf){

  // uint8_t id;
  // uint8_t det_block;

  // uint8_t nr_states;
  // uint8_t IO;

  // struct s_ms_switch_state_conf * states;
  // struct s_IO_port_conf * IO_Ports;  

  struct s_msswitch_connect connect;
  connect.module = module;
  connect.id = conf.id;
  connect.states = conf.nr_states;

  connect.sideA = (struct rail_link *)_calloc(conf.nr_states, struct rail_link);
  connect.sideB = (struct rail_link *)_calloc(conf.nr_states, struct rail_link);

  for(uint8_t i = 0; i < conf.nr_states; i++){
    connect.sideA[i].module = conf.states[i].sideA.module;
    connect.sideA[i].id = conf.states[i].sideA.id;
    connect.sideA[i].type = (enum link_types)conf.states[i].sideA.type;
    
    connect.sideB[i].module = conf.states[i].sideB.module;
    connect.sideB[i].id = conf.states[i].sideB.id;
    connect.sideB[i].type = (enum link_types)conf.states[i].sideB.type;
  }

  _free(conf.states);

  new MSSwitch(connect, conf.det_block, conf.IO, conf.IO_Ports, 0);
}

MSSwitch::MSSwitch(struct s_msswitch_connect connect, uint8_t block_id, uint8_t output_len, struct s_IO_port_conf * output_pins, uint16_t * output_states){
  loggerf(INFO, "Create MSSw %i:%i", connect.module, connect.id);
  memset(this, 0, sizeof(MSSwitch));

  this->module = connect.module;
  this->id = connect.id;

  this->sideA = connect.sideA;
  this->sideB = connect.sideB;

  this->state_len = connect.states;

  this->IO = (IO_Port **)_calloc(output_len, IO_Port *);

  for(int i = 0; i < output_len; i++){
    Init_IO_from_conf(Units[connect.module], output_pins[i], this);

    this->IO[i] = Units[connect.module]->Node[output_pins[i].Node].io[output_pins[i].Adr];
  }
  if(output_pins)
    _free(output_pins);

  this->IO_len = output_len;
  this->IO_states = output_states;

  Units[this->module]->insertMSSwitch(this);

  // Add msswitch to detection block 
  if(Units[this->module]->block_len > block_id && U_B(this->module, block_id)){
    this->Detection = U_B(this->module, block_id);

    if(this->Detection->MSSw)
      loggerf(WARNING, "Block %02i:%02i has duplicate msswitch, overwritting ...", this->Detection->module, this->Detection->id);

    this->Detection->MSSw = this;
  }
  else{
    loggerf(WARNING, "MSSWITCH %i:%i has no detection block %i", connect.module, connect.id, block_id);
  }
}

MSSwitch::~MSSwitch(){
  _free(this->sideB);
  _free(this->IO);
  _free(this->IO_states);
  // _free(this->links);
  // _free(this->preferences);
}

bool MSSwitch::approachableA(void * p, int flags){
  loggerf(TRACE, "MSSwitch::approachableA (%x, %x, %x)", (unsigned int)this, (unsigned int)p, flags);
  if((flags & 0x80) == 0){
    //No SWITCH_CARE
    return 1;
  }

  if(this->sideA[this->state & 0x7F].p.p == p){
    return 1;
  }
  return 0;
}

bool MSSwitch::approachableB(void * p, int flags){
  loggerf(TRACE, "MSSwitch::approachableB (%x, %x, %x)", (unsigned int)this, (unsigned int)p, flags);
  if((flags & 0x80) == 0){
    //No SWITCH_CARE
    return 1;
  }

  if(this->sideB[this->state & 0x7F].p.p == p){
    return 1;
  }
  return 0;
}

Block * MSSwitch::Next_Block(enum link_types type, int flags, int level){
  struct rail_link * next;

  if(type == RAIL_LINK_TT){
    return 0;
  }
  
  if(level <= 0 && !(type == RAIL_LINK_MB_inside || type == RAIL_LINK_MA_inside)){
    return this->Detection;
  }
  
  if(type == RAIL_LINK_MA){
    return this->Detection->_Next(flags, level);
    // next = &this->sideB[this->state & 0x7F];
    // level--;
  }else if(type == RAIL_LINK_MB_inside){
    next = &this->sideB[this->state & 0x7F];
  }
  else if(type == RAIL_LINK_MB){
    return this->Detection->_Next(flags, level);
    // next = &this->sideA[this->state & 0x7F];
    // level--;
  }else if(type == RAIL_LINK_MA_inside){
    next = &this->sideA[this->state & 0x7F];
  }

  if(!next->p.p){
    if(next->type != RAIL_LINK_E)
      loggerf(ERROR, "NO POINTERS");
    return 0;
  }

  loggerf(INFO, "Next     :        \t%i:%i => %i:%i:%i\t%i", this->module, this->id, next->module, next->id, next->type, level);

  if(next->type == RAIL_LINK_R){
    return next->p.B->_Next(flags, level);
  }
  else if(next->type == RAIL_LINK_S){
    return next->p.Sw->Next_Block(next->type, flags, level);
  }
  else if(next->type == RAIL_LINK_s && next->p.Sw->approachable(this, flags)){
    return next->p.Sw->Next_Block(next->type, flags, level);
  }
  else if(next->type == RAIL_LINK_MA && next->p.MSSw->approachableA(this, flags)){
    return next->p.MSSw->Next_Block(next->type, flags, level);
  }
  else if(next->type == RAIL_LINK_MB && next->p.MSSw->approachableB(this, flags)){
    return next->p.MSSw->Next_Block(next->type, flags, level);
  }
  else if(next->type == RAIL_LINK_TT){
    if(next->p.MSSw->approachableA(this, flags)){
      next->type = RAIL_LINK_MA;
    }
    else if(next->p.MSSw->approachableB(this, flags)){
      next->type = RAIL_LINK_MB;
    }
    return next->p.MSSw->Next_Block(next->type, flags, level);
  }
  else if(next->type == RAIL_LINK_E){
    return 0;
  }

  return 0;
}

void MSSwitch::setState(uint8_t state, uint8_t lock){
  loggerf(TRACE, "throw_msswitch");

  if(this->Detection && (this->Detection->state == BLOCKED || this->Detection->state == RESERVED_SWITCH))
    return; // Switch is blocked

  Algor_Set_Changed(&this->Detection->Alg);
  putList_AlgorQueue(this->Detection->Alg, 0);

  this->state = (state & 0x0f) | 0x80;

  Units[this->module]->msswitch_state_changed |= 1;

  this->Detection->AlgorSearch(0);

  Algor_Set_Changed(&this->Detection->Alg);

  this->Detection->algorchanged = 0; // Block is allready search should not be researched

  putList_AlgorQueue(this->Detection->Alg, 0);

  putAlgorQueue(this->Detection, lock);
}
