
#include "switchboard/manager.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/signals.h"

#include "rollingstock/railtrain.h"

#include "config/LayoutStructure.h"

#include "system.h"
#include "utils/mem.h"
#include "utils/logger.h"
#include "IO.h"

#include "algorithm/core.h"
#include "algorithm/queue.h"

const char * rail_states_string[8] = {
  "BLOCKED",
  "DANGER",
  "RESTRICTED",
  "CAUTION",
  "PROCEED",
  "RESERVED",
  "RESERVED_SWITCH",
  "UNKNOWN" 
};

Block::Block(uint8_t _module, struct configStruct_Block * block){
  loggerf(DEBUG, "Block Constructor %02i:%02i", _module, block->id);
  memset(this, 0, sizeof(Block));
  module = _module;
  id = block->id;
  type = (enum Rail_types)block->type;

  uid = switchboard::SwManager->addBlock(this);

  next.module = block->next.module;
  next.id = block->next.id;
  next.type = (enum link_types)block->next.type;
  prev.module = block->prev.module;
  prev.id = block->prev.id;
  prev.type = (enum link_types)block->prev.type;

  BlockMaxSpeed = block->speed;
  dir = (block->fl & 0x6) >> 1;
  length = block->length;
  oneWay = block->fl & 0x1;

  Alg.B = this;

  IOchanged = 1;
  algorchanged = 1;

  state = PROCEED;
  reverse_state = PROCEED;

  forward_signal = new std::vector<Signal *>();
  reverse_signal = new std::vector<Signal *>();

  U = switchboard::Units(module);

  if(U->IO(block->IO_In))
    In = U->linkIO(block->IO_In, this, IO_Input_Block);

  if(block->fl & 0x8 && U->IO(block->IO_Out)){
    dir_Out = U->linkIO(block->IO_Out, this, IO_Output);
  }

  // Insert block into Unit
  U->insertBlock(this);
}

Block::~Block(){
  loggerf(MEMORY, "Block %i:%i Destructor\n", module, id);
  if(this->Sw)
    _free(this->Sw);

  delete this->forward_signal;
  delete this->reverse_signal;

  this->Sw = 0;
}

void Block::exportConfig(struct configStruct_Block * cfg){
  cfg->id = id;
  cfg->type = type;
  railLinkExport(&cfg->next, next);
  railLinkExport(&cfg->prev, prev);

  cfg->fl = 0;

  if (In)
    In->exportConfig(&cfg->IO_In);
  if (dir_Out){
    dir_Out->exportConfig(&cfg->IO_Out);
    cfg->fl |= 0x8;
  }

  cfg->speed = BlockMaxSpeed;
  cfg->length = length;
  cfg->fl |= oneWay | ((dir & 0x3) << 1);
}

void Block::addSwitch(Switch * Sw){
  if(this->switch_len == 0){
    this->Sw = (Switch **)_calloc(1, Switch *);
    this->switch_len = 1;
  }

  int id = find_free_index(this->Sw, this->switch_len);
  this->Sw[id] = Sw;
}

// Get next block, or i-th next block
struct rail_link * Block::NextLink(int flags){
  // dir: 0 next, 1 prev
  int dir = flags & 0x01;

  struct rail_link * next = 0;

  if((dir == NEXT && (this->dir == 1 || this->dir == 4 || this->dir == 6)) ||
     (dir == PREV && (this->dir == 0 || this->dir == 2 || this->dir == 5))) {
    // If next + reversed direction / flipped normal / flipped switching
    // Or prev + normal direction / switching direction (normal) / flipped reversed direction
    next = &this->prev;
  }
  else{
    // If next + normal direction / switching direction (normal) / flipped reversed
    // or prev + reversed direction / flipped normal / flipped switching
    next = &this->next;
  }

  if(!next)
    loggerf(ERROR, "Empty next Link");
  return next;
}


Block * Block::Next_Block(int flags, int level){
  loggerf(TRACE, "Next(%02i:%02i, %2x, %2x)", this->module, this->id, flags, level);
  // Find next (detection) block in direction dir. Could be used recurse for x-levels
  int dir = flags & 0x0F;
  // dir: 0 next, 1 prev

  struct rail_link * next = 0;

  uint8_t pdir = (dir >> 1);

  if(level <= 0){
    return this;
  }
  
  level--;

  // If not Init
  if(flags & NEXT_FIRST_TIME_SKIP){
    // If toggle request
    if((pdir ^ 0b100) == this->dir){
      dir ^= 0b1;
    }
    // prev -> next
    // reverse(1) -> normal(2)  change direction after next link
    // normal(2) -> reverse(1)  change direction now
    else if(pdir == 2 && this->dir == 1){
      dir ^= 0b1;
    }
    else if(pdir == 1 && this->dir == 2){
      next = this->NextLink(dir & 1);

      dir ^= 0b1;
    }
    // prev -> next
    // reverse(1) -> normal(0)
    // normal(0) -> reverse(1)
    else if((pdir == 1 && this->dir == 0) || (pdir == 0 && this->dir == 1)){
      dir ^= 0b1;

      if(flags & DIRECTION_CARE)
        return this;
    }
  }
  else{
    flags |= NEXT_FIRST_TIME_SKIP;
  }

  if(!next)
    next = this->NextLink(dir & 1);

  // loggerf(TRACE, "dir: %i:%i-%x %x\t%i", this->module, this->id, dir, this->dir, pdir);

  dir = (dir & 1) + (this->dir << 1);

  flags = (flags & 0xF0) + (dir & 0x0F);

  // loggerf(TRACE, "Next     : dir:%i/%x\t%i:%i => %i:%i:%i\t%i", this->dir, dir, this->module, this->id, next->module, next->id, next->type, level);

  if(!next->p.p){
    return 0;
  }

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
  else if(next->type == RAIL_LINK_MA_inside || next->type == RAIL_LINK_MB_inside){
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
    // if(Next_check_Switch(this, *next, flags)){
      // if(level <= 0 && (next->p.MSSw)->Detection != B){
      //   // printf("Detection block\n");
      //   return (next->p.MSSw)->Detection;
      // }
      // else{
        // return Next_MSSwitch_Block(next->p.MSSw, next->type, flags, level);
      // }
    // }
  // }
  else if(next->type == RAIL_LINK_E && next->module == 0 && next->id == 0){
    return 0;
  }

  return 0;
}

uint8_t Block::_NextList(Block * Origin, Block ** blocks, uint8_t block_counter, int flags, int length){
  loggerf(TRACE, "NextList(%02i:%02i, %i, %i)", this->module, this->id, block_counter, length);
  // Find next (detection) block in direction dir. Could be used recurse for x-levels
  int dir = flags & 0x0F;
  // dir: 0 next, 1 prev

  struct rail_link * next;

  uint8_t pdir = (dir >> 1);

  if(length < 0){
    return block_counter;
  }

  length -= this->length;

  // If not Init
  if(flags & NEXT_FIRST_TIME_SKIP){
    // If toggle request
    if((pdir ^ 0b100) == this->dir){
      dir ^= 0b1;
    }
    // prev -> next
    // reverse(1) -> normal(2)
    // normal(2) -> reverse(1)
    else if((pdir == 1 && this->dir == 2) || (pdir == 2 && this->dir == 1)){
      dir ^= 0b1;
    }
    // prev -> next
    // reverse(1) -> normal(0)
    // normal(0) -> reverse(1)
    else if((pdir == 1 && this->dir == 0) || (pdir == 0 && this->dir == 1)){
      dir ^= 0b1;

      if(flags & DIRECTION_CARE)
        return block_counter;
    }
    
    blocks[block_counter++] = this;
  }
  else{
    flags |= NEXT_FIRST_TIME_SKIP;
    length += this->length;
  }

  if(block_counter >= 10)
    return block_counter;

  loggerf(TRACE, "dir: %i:%i-%x %x\t%i", this->module, this->id, dir, this->dir, pdir);
  //NEXT == 0
  //PREV == 1
  next = this->NextLink(dir & 1);

  dir = (dir & 1) + (this->dir << 1);

  flags = (flags & 0xF0) + (dir & 0x0F);

  loggerf(TRACE, "Next     : dir:%i/%x\t%i:%i => %i:%i:%i\t%i", this->dir, dir, this->module, this->id, next->module, next->id, next->type, block_counter);

  if(!next->p.p){
    if(next->type != RAIL_LINK_E && next->type != RAIL_LINK_C)
      loggerf(ERROR, "NO POINTERS %i:%i", this->module, this->id);
    return block_counter;
  }

  if(next->type == RAIL_LINK_R){
    return next->p.B->_NextList(Origin, blocks, block_counter, flags, length);
  }
  else if(next->type == RAIL_LINK_S){
    return next->p.Sw->NextList_Block(Origin, blocks, block_counter, next->type, flags, length);
  }
  else if(next->type == RAIL_LINK_s && next->p.Sw->approachable(this, flags)){
    return next->p.Sw->NextList_Block(Origin, blocks, block_counter, next->type, flags, length);
  }
  else if(next->type == RAIL_LINK_MA && next->p.MSSw->approachableA(this, flags)){
    return next->p.MSSw->NextList_Block(Origin, blocks, block_counter, next->type, flags, length);
  }
  else if(next->type == RAIL_LINK_MB && next->p.MSSw->approachableB(this, flags)){
    return next->p.MSSw->NextList_Block(Origin, blocks, block_counter, next->type, flags, length);
  }
  else if(next->type == RAIL_LINK_MA_inside || next->type == RAIL_LINK_MB_inside){
    return next->p.MSSw->NextList_Block(Origin, blocks, block_counter, next->type, flags, length);
  }
  else if(next->type == RAIL_LINK_TT){
    if(next->p.MSSw->approachableA(this, flags)){
      next->type = RAIL_LINK_MA;

      // If turntable is turned around
      if(next->p.MSSw->NextLink(flags)->p.p != this){
        flags ^= 0b1;
      }
    }
    else if(next->p.MSSw->approachableB(this, flags)){
      next->type = RAIL_LINK_MB;

      // If turntable is turned around
      if(next->p.MSSw->NextLink(flags)->p.p != this){
        flags ^= 0b1;
      }
    }
    return next->p.MSSw->NextList_Block(Origin, blocks, block_counter, next->type, flags, length);
  }
  //   // if(Next_check_Switch(this, *next, flags)){
  //     // if(level <= 0 && (next->p.MSSw)->Detection != B){
  //     //   // printf("Detection block\n");
  //     //   return (next->p.MSSw)->Detection;
  //     // }
  //     // else{
  //       // return Next_MSSwitch_Block(next->p.MSSw, next->type, flags, level);
  //     // }
  //   // }
  // // }
  // else if(next->type == RAIL_LINK_E && next->module == 0 && next->id == 0){
  //   return 0;
  // }

  return block_counter;
}

void Block::reverse(){
  Algor_Blocks * AB = &this->Alg;
  if(!AB)
    return;
  loggerf(WARNING, "Block_Reverse %02i:%02i", AB->B->module, AB->B->id);

  //_ALGOR_BLOCK_APPLY(_ABl, _A, _B, _C) if(_ABl->len == 0){_A}else{_B;for(uint8_t i = 0; i < _ABl->len; i++){_C}}
  int tmp_state;
  AB->B->dir ^= 0b100;
  if(AB->B->state != RESERVED_SWITCH){
    tmp_state = AB->B->state;
    AB->B->state = AB->B->reverse_state;
    AB->B->reverse_state = (enum Rail_states)tmp_state;
  }

  uint8_t len = 0;
  if(AB->next > AB->prev)
    len = AB->next;
  else
    len = AB->prev;

  uint8_t temp;

  temp = AB->next;
  AB->next = AB->prev;
  AB->prev = temp;

  temp = AB->next1;
  AB->next1 = AB->prev1;
  AB->prev1 = temp;

  temp = AB->next2;
  AB->next2 = AB->prev2;
  AB->prev2 = temp;

  temp = AB->next3;
  AB->next3 = AB->prev3;
  AB->prev3 = temp;


  Block * tmp;
  for(uint8_t i = 0; i < len; i++){
    tmp = AB->N[i];
    AB->N[i] = AB->P[i];
    AB->P[i] = tmp;
  }

  Algorithm::print_block_debug(AB->B);
}

void Block::reserve(){
  if(state >= PROCEED)
    state = RESERVED;
  reverse_state = DANGER;

  reserved++;

  statechanged = 1;
  U->block_state_changed = 1;
}

void Block::dereserve(){
  state = PROCEED;
  reverse_state = PROCEED;

  reserved--;

  statechanged = 1;
  U->block_state_changed = 1;
}

void Block::setState(enum Rail_states state){
  this->state = state;

  uint8_t signalsize = this->forward_signal->size();
  for(uint8_t i = 0; i < signalsize; i++){
    this->forward_signal->operator[](i)->set(state);
  }

  this->statechanged = 1;
  U->block_state_changed |= 1;
}

void Block::setReversedState(enum Rail_states state){
  this->reverse_state = state;

  uint8_t signalsize = this->reverse_signal->size();
  for(uint8_t i = 0; i < signalsize; i++){
    this->reverse_signal->operator[](i)->set(state);
  }

  this->statechanged = 1;
  U->block_state_changed |= 1;
}

void Block::setDetection(bool d){
  if(virtualBlocked && d && !detectionBlocked && train){
    train->moveForward(this);
  }

  detectionBlocked = d;
  blocked = (detectionBlocked || virtualBlocked);
  IOchanged = 1;
}

void Block::setVirtualDetection(bool d){
  virtualBlocked = d;
  blocked = (detectionBlocked || virtualBlocked);
  IOchanged = 1;
}

enum Rail_states Block::addSignal(Signal * Sig){
  if(Sig->direction){
    this->forward_signal->push_back(Sig);
    return this->state;
  }
  else{
    this->reverse_signal->push_back(Sig);
    return this->reverse_state;
  }
}



void Block::AlgorClear(){
  loggerf(INFO, "Block %02i:%02i AlgorClear", module, id);
  memset(Alg.P, 0, 10*sizeof(void *));
  memset(Alg.N, 0, 10*sizeof(void *));
  Alg.prev  = 0;
  Alg.prev1 = 0;
  Alg.prev2 = 0;
  Alg.prev3 = 0;

  Alg.next = 0;
  Alg.next1 = 0;
  Alg.next2 = 0;
  Alg.next3 = 0;

  MaxSpeed = BlockMaxSpeed;
}
#define ALGORLENGTH 100

void Block::AlgorSearch(int debug){
  loggerf(TRACE, "Blocks::AlgorSearch - %02i:%02i", module, id);
  Block * next = 0;
  Block * prev = 0;

  AlgorClear();

  loggerf(TRACE, "Search blocks %02i:%02i", module, id);

  next = Next_Block(0 | SWITCH_CARE, 1);
  prev = Next_Block(1 | SWITCH_CARE | DIRECTION_CARE,1);

  if(next){
    Alg.next = _NextList(this, Alg.N, 0, NEXT | SWITCH_CARE, 600);

    Alg.next1 = Alg.next;
    Alg.next2 = Alg.next;
    Alg.next3 = Alg.next;

    uint16_t length = 0;
    uint8_t j = 0;
    uint8_t * n[3] = {&Alg.next1, &Alg.next2, &Alg.next3};

    if(Alg.next > 0)
      length = Alg.N[0]->length;

    for(uint8_t i = 1; i < Alg.next; i++){
      if(length >= ALGORLENGTH && j < 3 && Alg.N[i]->type != NOSTOP &&
         ((!Alg.N[i]->station || (Alg.N[i-1]->station && Alg.N[i]->station != Alg.N[i-1]->station) || Alg.N[i-1]->type == NOSTOP) || !this->station)){

        *(n[j++]) = i;
        length = 0;
      }

      length += Alg.N[i]->length;
    }
  }
  if(prev){
    Alg.prev = _NextList(this, Alg.P, 0, PREV | SWITCH_CARE | DIRECTION_CARE, 600);

    Alg.prev1 = Alg.prev;
    Alg.prev2 = Alg.prev;
    Alg.prev3 = Alg.prev;

    uint16_t length = 0;
    uint8_t j = 0;
    uint8_t * p[3] = {&Alg.prev1, &Alg.prev2, &Alg.prev3};

    if(Alg.prev > 0)
      length = Alg.P[0]->length;

    for(uint8_t i = 1; i < Alg.prev; i++){
      if(length >= ALGORLENGTH && j < 3 && Alg.P[i]->type != NOSTOP &&
         ((!Alg.P[i]->station || (Alg.P[i-1]->station && Alg.P[i]->station != Alg.P[i-1]->station) || Alg.P[i-1]->type == NOSTOP) || !this->station)){

        *(p[j++]) = i;
        length = 0;
      }
      length += Alg.P[i]->length;
    }
  }
}

void Block::checkSwitchFeedback(bool value){
  if(value){
    switchWrongFeedback = true;
  }
  else{
    for(uint8_t i = 0; i < switch_len; i++){
      if(Sw[i]->feedbackWrongState)
        return;
    }

    if(MSSw && MSSw->feedbackWrongState)
      return;

    switchWrongFeedback = false;
  }

  IOchanged = true;
  AlQueue.put(this);
}

/*
void Block::AlgorSearchMSSwitch(int debug){
  loggerf(WARNING, "Algor_turntable_search_Blocks - %02i:%02i", this->module, this->id);
  Block * next = 0;
  Block * prev = 0;

  Algor_Blocks * ABs = &this->Alg;
  Block * tmpB = this;

  AlgorClear();

  if(!this->MSSw){
    loggerf(ERROR, "Turntable has more than no msswitch");
    return;
  }

  next = this->MSSw->Next_Block(RAIL_LINK_TT, NEXT | SWITCH_CARE, 1);
  prev = this->MSSw->Next_Block(RAIL_LINK_TT, PREV | SWITCH_CARE, 1);

  if(next)
    loggerf(WARNING, "%02i:%02i", next->module, next->id);
  if(prev)
    loggerf(WARNING, "%02i:%02i", prev->module, prev->id);

  //Select all surrounding blocks
  uint8_t i = 0;
  uint8_t level = 1;
  uint16_t length = 0;
  if(next){
    do{
      if(i == 0 && ABs->next == 0){
        tmpB = next;
      }
      else{
        tmpB = this->MSSw->Next_Block(RAIL_LINK_TT, NEXT | SWITCH_CARE, level);
      }

      if(!tmpB){
        break;
      }

      ABs->N[i] = tmpB;

      length += tmpB->length;

      ABs->next += 1;

      i++;
      level++;
    }
    while(length < 3*Block_Minimum_Size && level < 10 && i < 10);
  }

  i = 0;
  level = 1;
  length = 0;

  if(prev){
    do{
      if(i == 0 && ABs->prev == 0){
        tmpB = prev;
      }
      else{
        tmpB = this->MSSw->Next_Block(RAIL_LINK_TT, PREV | SWITCH_CARE, level);
      }

      if(!tmpB){
        break;
      }

      ABs->P[i] = tmpB;

      length += tmpB->length;

      ABs->prev += 1;

      i++;
      level++;
    }
    while(length < 3*Block_Minimum_Size && level < 10 && i < 10);
  }
}
*/
// int main(void){
//     C_Block B = C_Block(1, {0, 0, {0, 0, 255}, {0, 0, 255}, {0, 0}, {0, 1}, 90, 100, 0});
  
//     printf("Block %i\n", B.id);

//     // delete B;
//     return 1;
// }


int dircmp(Block *A, Block *B){
  if((A->dir == 2 && (B->dir == 1 || B->dir == 0)) || ((A->dir == 1 || A->dir == 0) && B->dir == 2)){
    return 1;
  }else if(A->dir == B->dir){
    return 1;
  }else if(((A->dir == 0 || A->dir == 2) && B->dir == 0b101) || (A->dir == 1 && B->dir == 0b100)){
    return 1;
  }else if(((B->dir == 0 || B->dir == 2) && A->dir == 0b101) || (B->dir == 1 && A->dir == 0b100)){
    return 1;
  }
  else{
    return 0;
  }
}


void Reserve_To_Next_Switch(Block * B){
  loggerf(WARNING, "Block_Reverse_To_Next_Switch %02i:%02i", B->module, B->id);
  Block * Next_Block = 0;
  Block * Prev_Block = 0;

  if(B->Alg.next > 0 && B->Alg.N[0])
    Next_Block = B->Alg.N[0];

  while(1){
    if(!Next_Block){
      break;
    }
    loggerf(INFO, "Reserve NSw %02i:%02i", Next_Block->module, Next_Block->id);
    Next_Block->reserve();
    // Block_reserve(Next_Block);

    if(Next_Block->Alg.next > 0 && Next_Block->Alg.N[0]){
      if(Next_Block->Alg.N[0] == Prev_Block){
        Next_Block = 0;
      }
      else{
        Prev_Block = Next_Block;
        Next_Block = Next_Block->Alg.N[0];
        if(Next_Block->switch_len || Next_Block->MSSw)
          Next_Block = 0;
      }
    }
    else
      Next_Block = 0;

    usleep(100000);
  }
}

int Block_Reverse_To_Next_Switch(Block * B){
  loggerf(WARNING, "Block_Reverse_To_Next_Switch %02i:%02i", B->module, B->id);
  Block * Next_Block = 0;

  if(B->Alg.next > 0 && B->Alg.N[0])
    Next_Block = B->Alg.N[0];

  while(1){
    if(!Next_Block){
      break;
    }
    loggerf(INFO, "Block reverse NSw %02i:%02i", Next_Block->module, Next_Block->id);
    Next_Block->reverse();
    Next_Block->reserve();
    // Block_Reverse(&Next_Block->Alg);
    // Block_reserve(Next_Block);

    if(Next_Block->Alg.next > 0 && Next_Block->Alg.N[0])
      Next_Block = Next_Block->Alg.N[0];
    else
      Next_Block = 0;
  }
  return 0;
}
