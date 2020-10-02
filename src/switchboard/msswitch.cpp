#include "switchboard/manager.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/signals.h"

#include "utils/mem.h"
#include "utils/logger.h"
#include "IO.h"
#include "modules.h"
#include "system.h"

#include "algorithm/core.h"
#include "algorithm/queue.h"

using namespace switchboard;


MSSwitch::MSSwitch(uint8_t module, struct ms_switch_conf conf){

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
  connect.dir = (uint8_t *)_calloc(conf.nr_states, uint8_t);

  for(uint8_t i = 0; i < conf.nr_states; i++){
    connect.sideA[i].module = conf.states[i].sideA.module;
    connect.sideA[i].id = conf.states[i].sideA.id;
    connect.sideA[i].type = (enum link_types)conf.states[i].sideA.type;
    
    connect.sideB[i].module = conf.states[i].sideB.module;
    connect.sideB[i].id = conf.states[i].sideB.id;
    connect.sideB[i].type = (enum link_types)conf.states[i].sideB.type;

    connect.dir[i] = conf.states[i].dir;
  }

  // new MSSwitch(connect, conf.type, conf.det_block, conf.IO, conf.IO_Ports, 0);
// }

// MSSwitch::MSSwitch(struct s_msswitch_connect connect, uint8_t type, uint8_t block_id, uint8_t output_len, struct s_IO_port_conf * output_pins, uint16_t * output_states){
  loggerf(INFO, "Create MSSw %i:%i", connect.module, connect.id);
  memset(this, 0, sizeof(MSSwitch));

  this->module = connect.module;
  this->id = connect.id;

  uid = SwManager->addMSSwitch(this);

  this->sideA = connect.sideA;
  this->sideB = connect.sideB;
  this->state_direction = connect.dir;

  this->state_len = connect.states;
  this->type = type;

  this->IO_len = conf.IO;
  this->IO = (IO_Port **)_calloc(this->IO_len, IO_Port *);

  U = Units(this->module);

  for(int i = 0; i < this->IO_len; i++){
    if(!U->IO(conf.IO_Ports[i]))
      continue;

    this->IO[i] = U->linkIO(conf.IO_Ports[i], this, IO_Output);
  }

  loggerf(ERROR, "TODO: implement outputstates");
  // this->IO_states = (uint16_t *)_calloc(this->IO_len, uint16_t);
  // memcpy(this->IO_states, conf.IO_Ports, this->IO_len * sizeof(uint16_t));

  U->insertMSSwitch(this);

  // Add msswitch to detection block 
  if(U->block_len > conf.det_block && U->B[conf.det_block]){
    this->Detection = U->B[conf.det_block];

    if(this->Detection->MSSw)
      loggerf(WARNING, "Block %02i:%02i has duplicate msswitch, overwritting ...", this->Detection->module, this->Detection->id);

    this->Detection->MSSw = this;
  }
  else{
    loggerf(WARNING, "MSSWITCH %i:%i has no detection block %i", connect.module, connect.id, conf.det_block);
  }
}

MSSwitch::~MSSwitch(){
  _free(this->sideA);
  _free(this->sideB);
  _free(this->state_direction);
  _free(this->IO);
  _free(this->IO_states);
  // _free(this->links);
  // _free(this->preferences);
}

void MSSwitch::addSignal(Signal * Sig){
  loggerf(INFO, "AddSignal %i to switch %i", Sig->id, this->id);
  this->Signals.push_back(Sig);
}

bool MSSwitch::approachableA(void * p, int flags){
  loggerf(TRACE, "MSSwitch::approachableA (%x, %x, %x)", (unsigned int)this, (unsigned int)p, flags);
  if((flags & SWITCH_CARE) == 0){
    return 1;
  }

  if(this->sideA[this->state].p.p == p){
    return 1;
  }
  return 0;
}

bool MSSwitch::approachableB(void * p, int flags){
  loggerf(TRACE, "MSSwitch::approachableB (%x, %x, %x)", (unsigned int)this, (unsigned int)p, flags);
  if((flags & SWITCH_CARE) == 0){
    return 1;
  }

  if(this->sideB[this->state].p.p == p){
    return 1;
  }
  return 0;
}

struct rail_link * MSSwitch::NextLink(int flags){
  struct rail_link * next = 0;

  Block * B = this->Detection;

  next = B->NextLink(flags);

  if(next->type == RAIL_LINK_MA)
    next = &this->sideA[this->state];
  else
    next = &this->sideB[this->state];

  return next;
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
    // next = &this->sideB[this->state];
    // level--;
  }else if(type == RAIL_LINK_MB_inside){
    next = &this->sideB[this->state];
  }
  else if(type == RAIL_LINK_MB){
    return this->Detection->_Next(flags, level);
    // next = &this->sideA[this->state];
    // level--;
  }else if(type == RAIL_LINK_MA_inside){
    next = &this->sideA[this->state];
  }

  if(!next->p.p){
    if(next->type != RAIL_LINK_E)
      loggerf(ERROR, "NO POINTERS");
    return 0;
  }

  loggerf(TRACE, "Next     :        \t%i:%i => %i:%i:%i\t%i", this->module, this->id, next->module, next->id, next->type, level);

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

uint MSSwitch::NextList_Block(Block ** blocks, uint8_t block_counter, enum link_types type, int flags, int length){
  loggerf(TRACE, "MSSwitch::NextList_Block(%02i:%02i, %i, %2x)", this->module, this->id, block_counter, flags);
  struct rail_link * next;

  if(type == RAIL_LINK_TT){
    return block_counter;
  }
  
  
  if(type == RAIL_LINK_MA){
    blocks[block_counter++] = this->Detection;
    next = &this->sideB[this->state];
    // return this->Detection->_NextList(blocks, block_counter, flags, length);
  }else if(type == RAIL_LINK_MB_inside){
    next = &this->sideB[this->state];
    flags |= NEXT_FIRST_TIME_SKIP;

    uint8_t dir = (flags & 1) + (this->Detection->dir << 1);
    flags = (flags & 0xF0) + (dir & 0x0F);
  }
  else if(type == RAIL_LINK_MB){
    blocks[block_counter++] = this->Detection;
    next = &this->sideA[this->state];
    // return this->Detection->_NextList(blocks, block_counter, flags, length);
  }else if(type == RAIL_LINK_MA_inside){
    next = &this->sideA[this->state];
    flags |= NEXT_FIRST_TIME_SKIP;

    uint8_t dir = (flags & 1) + (this->Detection->dir << 1);
    flags = (flags & 0xF0) + (dir & 0x0F);
  }

  if(!next->p.p){
    if(next->type != RAIL_LINK_E)
      loggerf(ERROR, "NO POINTERS");
    return block_counter;
  }

  if(block_counter >= 10)
    return block_counter;

  loggerf(TRACE, "Next     :        \t%i:%i => %i:%i:%i\t%i", this->module, this->id, next->module, next->id, next->type, block_counter);

  if(next->type == RAIL_LINK_R){
    return next->p.B->_NextList(blocks, block_counter, flags, length);
  }
  else if(next->type == RAIL_LINK_S){
    return next->p.Sw->NextList_Block(blocks, block_counter, next->type, flags, length);
  }
  else if(next->type == RAIL_LINK_s && next->p.Sw->approachable(this, flags)){
    return next->p.Sw->NextList_Block(blocks, block_counter, next->type, flags, length);
  }
  else if(next->type == RAIL_LINK_MA && next->p.MSSw->approachableA(this, flags)){
    return next->p.MSSw->NextList_Block(blocks, block_counter, next->type, flags, length);
  }
  else if(next->type == RAIL_LINK_MB && next->p.MSSw->approachableB(this, flags)){
    return next->p.MSSw->NextList_Block(blocks, block_counter, next->type, flags, length);
  }
  else if(next->type == RAIL_LINK_MA_inside || next->type == RAIL_LINK_MB_inside){
    return next->p.MSSw->NextList_Block(blocks, block_counter, next->type, flags, length);
  }
  else if(next->type == RAIL_LINK_TT){
    if(next->p.MSSw->approachableA(this, flags)){
      next->type = RAIL_LINK_MA;

      // If turntable is turned around
      if(next->p.MSSw->NextLink(flags)->p.p != this){
        flags ^= 0b10;
      }
    }
    else if(next->p.MSSw->approachableB(this, flags)){
      next->type = RAIL_LINK_MB;

      // If turntable is turned around
      if(next->p.MSSw->NextLink(flags)->p.p != this){
        flags ^= 0b10;
      }
    }
    return next->p.MSSw->NextList_Block(blocks, block_counter, next->type, flags, length);
  }
  // else if(next->type == RAIL_LINK_E){
  //   return 0;
  // }

  return block_counter;
}

void MSSwitch::setState(uint8_t _state, uint8_t lock){
  loggerf(TRACE, "throw_msswitch");

  if(Detection && (Detection->state == BLOCKED || Detection->state == RESERVED_SWITCH))
    return; // Switch is blocked

  Algorithm::Set_Changed(&Detection->Alg);
  AlQueue.puttemp(&Detection->Alg);

  updateState(_state);

  Detection->AlgorSearch(0);

  Algorithm::Set_Changed(&Detection->Alg);

  Detection->algorchanged = 0; // Block is allready search should not be researched

  AlQueue.puttemp(&Detection->Alg);
  AlQueue.puttemp(Detection);
  AlQueue.cpytmp();
}

void MSSwitch::updateState(uint8_t state){
  if (this->state_direction[this->state] != this->state_direction[state])
    this->Detection->dir ^= 0b100;

  this->state = state;
  this->updatedState = true;

  for(auto Sig: this->Signals){
    Sig->switchUpdate();
  }

  U->switch_state_changed |= 1;
}

