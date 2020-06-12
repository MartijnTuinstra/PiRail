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

  connect.sideB = (struct rail_link *)_calloc(conf.nr_states, struct rail_link);
  
  connect.sideA.module = conf.states[0].sideA.module;
  connect.sideA.id = conf.states[0].sideA.id;
  connect.sideA.type = (enum link_types)conf.states[0].sideA.type;


  for(uint8_t i = 0; i < conf.nr_states; i++){
    connect.sideB[i].module = conf.states[i].sideB.module;
    connect.sideB[i].id = conf.states[i].sideB.id;
    connect.sideB[i].type = (enum link_types)conf.states[i].sideB.type;
  }

  _free(conf.states);

  new MSSwitch(connect, conf.det_block, conf.IO, conf.IO_Ports, 0);
}

MSSwitch::MSSwitch(struct s_msswitch_connect connect, uint8_t block_id, uint8_t output_len, struct s_IO_port_conf * output_pins, uint16_t * output_states){
  loggerf(DEBUG, "Create MSSw %i:%i", connect.module, connect.id);
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

  if(Units[this->module]->block_len > block_id && U_B(this->module, block_id)){
    this->Detection = U_B(this->module, block_id);
    this->Detection->addMSSwitch(this);
  }
  else{
    loggerf(WARNING, "SWITCH %i:%i has no detection block %i", connect.module, connect.id, block_id);
  }
}

MSSwitch::~MSSwitch(){
  _free(this->sideB);
  _free(this->IO);
  _free(this->IO_states);
  // _free(this->links);
  // _free(this->preferences);
}

bool MSSwitch::approachable(void * p, int flags){
  loggerf(TRACE, "MSSwitch::approachable (%x, %x, %x)", (unsigned int)this, (unsigned int)p, flags);
  if((flags & 0x80) == 0){
    //No SWITCH_CARE
    return 1;
  }

  if(this->sideB[this->state].p.p == p){
    return 1;
  }
  return 0;
}

Block * MSSwitch::Next_Block(enum link_types type, int flags, int level){
  struct rail_link * next;

  if(flags & SWITCH_CARE){
    loggerf(CRITICAL, "Fix next_msswitch_block switch state care");
  }

  if(type == RAIL_LINK_TT){
    //turntable
    if(this->Detection){
      if(this->Detection->dir & 0x4){ // reversed
        if(flags & PREV)
          next = &this->sideB[this->state & 0x7F];
        else
          next = &this->sideA;
      }
      else{
        if(flags & PREV)
          next = &this->sideA;
        else
          next = &this->sideB[this->state & 0x7F];
      }
    }
  }
  else if(type == RAIL_LINK_MA){
    next = &this->sideB[this->state & 0x7F];
  }
  else if(type == RAIL_LINK_MB){
    next = &this->sideA;
  }

  if(!next->p.p){
    if(next->type != RAIL_LINK_E)
      loggerf(ERROR, "NO POINTERS");
    return 0;
  }

  if(next->type == RAIL_LINK_R){
    if(this->Detection != next->p.B){
      level--;
    }
    if(level <= 0){
      return next->p.B;
    }
    else{
      return next->p.B->_Next(flags, level);
    }
  }
  else if(next->type == RAIL_LINK_S){
    return next->p.Sw->Next_Block(next->type, flags, level);
  }
  else if(next->type == RAIL_LINK_s && next->p.Sw->approachable(this, flags)){
    return next->p.Sw->Next_Block(next->type, flags, level);
  }
  else if(next->type == RAIL_LINK_MA || next->type == RAIL_LINK_MB){
    // if(Next_check_Switch(S, *next, flags)){
    //   return Next_MSSwitch_Block(next->p.MSSw, next->type, flags, level);
    // }
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

  Algor_search_Blocks(this->Detection, 0);

  Algor_Set_Changed(&this->Detection->Alg);

  this->Detection->algorchanged = 0; // Block is allready search should not be researched

  putList_AlgorQueue(this->Detection->Alg, 0);

  putAlgorQueue(this->Detection, lock);
}
