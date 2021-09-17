#include "switchboard/manager.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/signals.h"

#include "utils/mem.h"
#include "utils/logger.h"
#include "IO.h"
#include "system.h"
#include "flags.h"

#include "algorithm/core.h"
#include "algorithm/queue.h"

using namespace switchboard;


MSSwitch::MSSwitch(uint8_t _module, struct configStruct_MSSwitch * conf){

  loggerf(TRACE, "Create MSSw %i:%i", _module, conf->id);
  memset(this, 0, sizeof(MSSwitch));

  module = _module;
  id = conf->id;

  uid = SwManager->addMSSwitch(this);

  sideA = (BlockLink *)_calloc(conf->nr_states, BlockLink);
  sideB = (BlockLink *)_calloc(conf->nr_states, BlockLink);
  stateMaxSpeed = (uint16_t *)_calloc(conf->nr_states, uint16_t);
  state_direction = (uint8_t *)_calloc(conf->nr_states, uint8_t);
  IO_states = (union u_IO_event **)_calloc(conf->nr_states, union u_IO_event *);

  for(uint8_t i = 0; i < conf->nr_states; i++){
    sideA[i] = BlockLink(conf->states[i].sideA);
    sideB[i] = BlockLink(conf->states[i].sideB);

    stateMaxSpeed[i]   = conf->states[i].speed;
    state_direction[i] = conf->states[i].dir;

    IO_states[i] = (union u_IO_event *)_calloc(conf->IO, union u_IO_event);
    for(uint8_t j = 0; j < conf->IO; j++){
      if((conf->states[i].output_sequence >> j) & 0b1)
        IO_states[i][j].output = IO_event_High;
      else
        IO_states[i][j].output = IO_event_Low;
    }
  }

  maxSpeed = stateMaxSpeed[defaultState];

  state_len = conf->nr_states;
  type = conf->type;

  IO_len = conf->IO;
  IO = (IO_Port **)_calloc(IO_len, IO_Port *);

  U = Units(module);

  for(int i = 0; i < IO_len; i++){
    if(!U->IO(conf->IO_Ports[i]))
      continue;

    IO[i] = U->linkIO(conf->IO_Ports[i], this, IO_Output);
  }

  U->insertMSSwitch(this);

  Detection = U->registerDetection(this, conf->det_block);
}

MSSwitch::~MSSwitch(){
  _free(sideA);
  _free(sideB);
  _free(stateMaxSpeed);
  _free(state_direction);

  for(uint8_t i = 0; i < state_len; i++){
    _free(IO_states[i]);
  }
  _free(IO_states);
  _free(IO);
}

void MSSwitch::exportConfig(struct configStruct_MSSwitch * obj){
  // uint8_t id;
  // uint8_t det_block;
  // uint8_t type;
  // uint8_t nr_states;
  // uint8_t IO;
  // struct configStruct_MSSwitchState * states;
  // struct configStruct_IOport * IO_Ports;

  // struct configStruct_RailLink sideA;
  // struct configStruct_RailLink sideB;
  // uint16_t speed;
  // uint8_t dir;
  // uint8_t output_sequence;

  obj->id = id;
  obj->det_block = Detection->id;
  obj->type = type;

  if (IO){
    obj->IO = IO_len;
    obj->IO_Ports = (struct configStruct_IOport *)_calloc(IO_len, struct configStruct_IOport);
    for(uint8_t i = 0; i < IO_len; i++){
      if (IO[i])
        IO[i]->exportConfig(&obj->IO_Ports[i]);
    }
  }

  if (sideA && sideB){
    obj->nr_states = state_len;

    obj->states = (struct configStruct_MSSwitchState *)_calloc(IO_len, struct configStruct_MSSwitchState);
    for(uint8_t i = 0; i < IO_len; i++){
      railLinkExport(&obj->states[i].sideA, sideA[i]);
      railLinkExport(&obj->states[i].sideB, sideB[i]);

      obj->states[i].speed = stateMaxSpeed[i];
      obj->states[i].dir   = state_direction[i];

      obj->states[i].output_sequence = 0;
      for(int8_t j = IO_len - 1; j >= 0; j--){
        obj->states[i].output_sequence <<= 1;
        obj->states[i].output_sequence |= (IO_states[i][j].output == IO_event_High) ? 1 : 0;
      }
    }
  }
}

void MSSwitch::addSignal(Signal * Sig){
  loggerf(INFO, "AddSignal %i to switch %i", Sig->id, this->id);
  this->Signals.push_back(Sig);
}

bool MSSwitch::approachableA(void * p, int flags){
  loggerf(TRACE, "MSSwitch::approachableA (%x, %x, %x)", (unsigned long)this, (unsigned long)p, flags);
  if((flags & FL_SWITCH_CARE) == 0){
    return 1;
  }

  if(this->sideA[this->state].p.p == p){
    return 1;
  }
  return 0;
}

bool MSSwitch::approachableB(void * p, int flags){
  loggerf(TRACE, "MSSwitch::approachableB (%x, %x, %x)", (unsigned long)this, (unsigned long)p, flags);
  if((flags & FL_SWITCH_CARE) == 0){
    return 1;
  }

  if(this->sideB[this->state].p.p == p){
    return 1;
  }
  return 0;
}

RailLink * MSSwitch::NextLink(int flags){
  RailLink * next = 0;

  Block * B = this->Detection;

  next = B->NextLink(flags);

  if(next->type == RAIL_LINK_MA)
    next = &this->sideA[this->state];
  else
    next = &this->sideB[this->state];

  return next;
}

Block * MSSwitch::Next_Block(enum link_types type, int flags, int level){
  RailLink * next;

  if(type == RAIL_LINK_TT){
    return 0;
  }
  
  if(level <= 0 && !(type == RAIL_LINK_MB_inside || type == RAIL_LINK_MA_inside)){
    return this->Detection;
  }
  
  if(type == RAIL_LINK_MA){
    return this->Detection->Next_Block(flags, level);
    // next = &this->sideB[this->state];
    // level--;
  }else if(type == RAIL_LINK_MB_inside){
    next = &this->sideB[this->state];
  }
  else if(type == RAIL_LINK_MB){
    return this->Detection->Next_Block(flags, level);
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
    return next->p.B->Next_Block(flags, level);
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

uint MSSwitch::NextList_Block(Block * Origin, Block ** blocks, uint8_t block_counter, enum link_types type, int flags, int length){
  loggerf(TRACE, "MSSwitch::NextList_Block(%02i:%02i, %i, %2x)", module, id, block_counter, flags);
  RailLink * nextLink;

  if(type == RAIL_LINK_TT){
    return block_counter;
  }

  if(flags & FL_BLOCKS_COUNT)
    length--;
  else
    length -= Detection->length;

  if(flags & FL_NEXT_FIRST_TIME_SKIP){
    // If toggle request
    Block * prevBlock;

    if(block_counter > 0)
      prevBlock = blocks[block_counter-1];
    else
      prevBlock = Origin;

    char buffer[100];

    sprintf(buffer, " %c (%i %i)? =>", (flags & FL_DIRECTION_MASK) ? 'P' : 'N', dircmp(prevBlock->dir, Detection->dir), cmpPolarity(prevBlock));

    if(!dircmp(prevBlock->dir, Detection->dir)){
      loggerf(INFO, "flip Direction");
      flags ^= FL_DIRECTION_MASK;

      if(flags & FL_DIRECTION_CARE)
        return block_counter;
    }

    if(!cmpPolarity(prevBlock)){
      loggerf(INFO, "flip Diretion polarity");
      flags ^= FL_DIRECTION_MASK;
    }

    loggerf(TRACE, "%s %c", buffer, (flags & FL_DIRECTION_MASK) ? 'P' : 'N');
    
    blocks[block_counter++] = Detection;
  }
  else{
    flags |= FL_NEXT_FIRST_TIME_SKIP;
    if((flags & FL_BLOCKS_COUNT) == 0)
      length += Detection->length;
  }
  
  
  if(type == RAIL_LINK_MA || type == RAIL_LINK_MB_inside)
    nextLink = &sideB[state];
  else if(type == RAIL_LINK_MB || type == RAIL_LINK_MA_inside)
    nextLink = &sideA[state];

  if(Detection == Origin){
    uint16_t speed = stateMaxSpeed[state];
    if (speed != 0 && Origin->MaxSpeed > speed)
      Origin->MaxSpeed = speed;
  }

  if(!nextLink->p.p){
    if(nextLink->type != RAIL_LINK_E)
      loggerf(ERROR, "NO POINTERS");
    return block_counter;
  }

  if(block_counter >= 10)
    return block_counter;

  loggerf(TRACE, "Next     :        \t%i:%i => %i:%i:%i\t%i", module, id, nextLink->module, nextLink->id, nextLink->type, block_counter);

  return _NextList_NextIteration(nextLink, this, Origin, blocks, block_counter, flags, length);
}

void MSSwitch::setState(uint8_t _state){
  setState(_state, 1);
}

void MSSwitch::setState(uint8_t _state, uint8_t _mutexLock){
  bool overrideLockout = false;
  loggerf(WARNING, "MSSwitch::setState (%2i:%2i, %i->%i, %i)", module, id, state, _state, overrideLockout, _mutexLock);

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

void MSSwitch::updateState(uint8_t _state){
  // FIXME unused statement
  if (state_direction[state] != state_direction[state])
    Detection->dir ^= 0b100;

  state = _state;
  updatedState = true;

  // Update IO
  for(uint8_t i = 0; i < IO_len; i++){
    if(!IO[i])
      continue;

    IO[i]->setOutput(IO_states[state][i]);
  }

  if(feedback_en){
    feedbackWrongState = true;
    Detection->checkSwitchFeedback(true);
  }

  // Update linked Signals
  for(auto Sig: Signals){
    Sig->switchUpdate();
  }

  U->switch_state_changed |= 1;
}

bool MSSwitch::checkPolarity(Block * B){
  return checkPolarity(B, state);
}

bool MSSwitch::checkPolarity(Block * B, uint8_t _state){
  // if(Alg.next == 0 || Alg.prev == 0 || B->Alg.next == 0 || B->Alg.prev == 0)
  //   return 0;
  // loggerf(WARNING, "%x == %x\t%i %i, %i %i\t%x %x, %x %x", (unsigned int) this, (unsigned int) B, Alg.next, Alg.prev, B->Alg.next, B->Alg.prev, (unsigned int) Alg.N[0], (unsigned int) Alg.P[0], (unsigned int) B->Alg.N[0], (unsigned int) B->Alg.P[0]);

  if(std::any_of(sideA[_state].Polarity.begin(), sideA[_state].Polarity.end(), [this, B](auto i){ return i.first == B && (i.second ^ this->Detection->polarity_status ^ B->polarity_status); } ) ||
     std::any_of(sideB[_state].Polarity.begin(), sideB[_state].Polarity.end(), [this, B](auto i){ return i.first == B && (i.second ^ this->Detection->polarity_status ^ B->polarity_status); } )    ){
       return 1;
  }
  return 0;
}

bool MSSwitch::cmpPolarity(Block * B){
  return cmpPolarity(B, state);
}

bool MSSwitch::cmpPolarity(Block * B, uint8_t _state){
  // if(Alg.next == 0 || Alg.prev == 0 || B->Alg.next == 0 || B->Alg.prev == 0)
  //   return 0;
  // uint8_t i = 0;
  // loggerf(TRACE, "%x", B);
  // for(auto P: sideA[state].Polarity){
  //   loggerf(TRACE, "A%i  %x %i", i++, P.first, P.second);
  // }
  // i = 0;
  // loggerf(TRACE, "%x", B);
  // for(auto P: sideB[state].Polarity){
  //   loggerf(TRACE, "B%i  %x %i", i++, P.first, P.second);
  // }

  if(std::any_of(sideA[_state].Polarity.begin(), sideA[_state].Polarity.end(), [B](auto i){ return i.first == B && i.second; } ) ||
     std::any_of(sideB[_state].Polarity.begin(), sideB[_state].Polarity.end(), [B](auto i){ return i.first == B && i.second; } )    ){
       return 1;
  }
  return 0;
}
