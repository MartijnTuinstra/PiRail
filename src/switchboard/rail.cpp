
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
  // Find next (detection) block in direction dir. Could be used recurse for x-levels
  Block * B[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  uint8_t blocks = _NextList(this, (Block **)B, 0, flags, 1);

  if(!blocks)
    return 0;
  else
    return B[0];
}

uint8_t Block::_NextList(Block * Origin, Block ** blocks, uint8_t block_counter, int flags, int length){
  // Find next (detection) blocks in direction dir.
  loggerf(TRACE, "NextList(%02i:%02i, %i, %i)", this->module, this->id, block_counter, length);
  int dir = flags & 0x0F;
  // dir: 0 next, 1 prev

  struct rail_link * next;

  uint8_t pdir = (dir >> 1);

  if(length < 0){
    return block_counter;
  }

  length -= this->length;

  // If not Init
  if(flags & FL_NEXT_FIRST_TIME_SKIP){
    // If toggle request
    if(!dircmp(pdir, this->dir)){
      dir ^= 0b1;

      if(!((pdir & 0b010) || (this->dir & 0b010)) && flags & FL_DIRECTION_CARE){
        return block_counter;
      }
    }
    else if((pdir == 2 && this->dir == 1) || (pdir == 6 && this->dir == 5) || 
            (pdir == 1 && this->dir == 2) || (pdir == 5 && this->dir == 6)){
      dir ^= 0b1;
    }
    
    blocks[block_counter++] = this;
  }
  else{
    flags |= FL_NEXT_FIRST_TIME_SKIP;
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
  loggerf(INFO, "Block_Reverse %02i:%02i %i -> %i", module, id, dir, dir ^ 0b100);

  //_ALGOR_BLOCK_APPLY(_ABl, _A, _B, _C) if(_ABl->len == 0){_A}else{_B;for(uint8_t i = 0; i < _ABl->len; i++){_C}}
  dir ^= 0b100;

  // Swap states

  if(state != RESERVED_SWITCH)
    std::swap(state, reverse_state);

  // Swap block lists

  uint8_t len = 0;
  if(Alg.next > Alg.prev)
    len = Alg.next;
  else
    len = Alg.prev;

  std::swap(Alg.prev,  Alg.next);
  std::swap(Alg.prev1, Alg.next1);
  std::swap(Alg.prev2, Alg.next2);
  std::swap(Alg.prev3, Alg.next3);

  for(uint8_t i = 0; i < len; i++)
    std::swap(Alg.N[i], Alg.P[i]);

  Algorithm::print_block_debug(this);

  // Swap Signals

  std::swap(forward_signal, reverse_signal);
}

void Block::reserve(RailTrain * T){
  loggerf(INFO, "Reserve Block %2i:%2i for train %i (%i, %i)", module, id, T->id, switchReserved, reserved);
  if(type != NOSTOP)
    reserved = true;

  if(switchReserved || reserved){
    if(!blocked && state >= PROCEED){
      if(switchReserved)
        setState(RESERVED_SWITCH);
      else
        setState(RESERVED);
    }

    setReversedState(DANGER);
  }

  reservedBy.push_back(T);
}

void Block::dereserve(RailTrain * T){

  reservedBy.erase(std::remove_if(reservedBy.begin(),
                                  reservedBy.end(),
                                  [T](const auto & o) { return (o == T); }),
                                  reservedBy.end()
                                 );

  if(reservedBy.size() == 0){
    if(!blocked){
      setState(PROCEED);
      setReversedState(PROCEED);
    }

    reserved = false;
    switchReserved = false;
  }
}

bool Block::isReservedBy(RailTrain * T){
  for(auto t: reservedBy){
    if(t == T)
      return true;
  }

  return false;
}

void Block::setState(enum Rail_states _state, bool reversed){
  switch(reversed){
    case true:
      setReversedState(_state);
      break;
    case false:
      setState(_state);
      break;
  }
}

void Block::setState(enum Rail_states _state){
  loggerf(TRACE, "Block %2i:%2i setState %s", module, id, rail_states_string[_state]);
  if(_state == PROCEED){
    if(reserved)
      _state = RESERVED;
    else if(switchReserved)
      _state = RESERVED_SWITCH;
  }

  state = _state;

  uint8_t signalsize = forward_signal->size();
  for(uint8_t i = 0; i < signalsize; i++){
    forward_signal->operator[](i)->set(state);
  }

  statechanged = 1;
  U->block_state_changed |= 1;
}

void Block::setReversedState(enum Rail_states _state){
  loggerf(TRACE, "Block %2i:%2i setReversedState %s", module, id, rail_states_string[_state]);
  reverse_state = _state;

  uint8_t signalsize = reverse_signal->size();
  for(uint8_t i = 0; i < signalsize; i++){
    reverse_signal->operator[](i)->set(reverse_state);
  }

  statechanged = 1;
  U->block_state_changed |= 1;
}

enum Rail_states Block::getNextState(){
  Block * Next = 0;
  if(Alg.next > 0)
    Next = Alg.N[0];
  else
    return DANGER;

  if(dircmp(this, Next))
    return Next->state;
  else
    return Next->reverse_state;
}

enum Rail_states Block::getPrevState(){
  Block * Prev = 0;
  if(Alg.prev > 0)
    Prev = Alg.P[0];
  else
    return DANGER;

  if(dircmp(this, Prev))
    return Prev->reverse_state;
  else
    return Prev->state;
}

void Block::setDetection(bool d){
  if(virtualBlocked && d && !detectionBlocked && train){
    train->moveForward(this);
  }

  bool prevBlocked = detectionBlocked;

  detectionBlocked = d;
  blocked = (detectionBlocked || virtualBlocked);

  if(prevBlocked != detectionBlocked)
    IOchanged = 1;
}

void Block::setVirtualDetection(bool d){
  bool prevBlocked = virtualBlocked;

  virtualBlocked = d;
  blocked = (detectionBlocked || virtualBlocked);

  if(prevBlocked != virtualBlocked)
    IOchanged = 1;
}

enum Rail_states Block::addSignal(Signal * Sig){
  if(Sig->direction){
    forward_signal->push_back(Sig);
    return state;
  }
  else{
    reverse_signal->push_back(Sig);
    return reverse_state;
  }
}

// void Block::setSpeed(){
//   loggerf(WARNING, "Block reset MaxSpeed %3i", BlockMaxSpeed);
//   MaxSpeed = BlockMaxSpeed;

//   for(uint8_t i = 0; i < switch_len; i++){
//     uint16_t SwSpeed = Sw[i]->MaxSpeed[Sw[i]->state];
//     loggerf(WARNING, "  Got switch %2i -> %3i", Sw[i]->id, SwSpeed);
//     if(SwSpeed && SwSpeed < MaxSpeed)
//       MaxSpeed = SwSpeed;
//   }

//   if(MSSw){
//     uint16_t SwSpeed = MSSw->maxSpeed;
//     loggerf(WARNING, "  Got msswitch %2i -> %3i", MSSw->id, SwSpeed);
//     if(SwSpeed && SwSpeed < MaxSpeed)
//       MaxSpeed = SwSpeed;
//   }

//   loggerf(WARNING, "Block MaxSpeed: %3i", MaxSpeed);
// }

uint16_t Block::getSpeed(){
  return getSpeed(0);
}

uint16_t Block::getSpeed(uint8_t Dir){
  uint16_t speed = MaxSpeed;

  switch(Dir ? reverse_state : state){
    case DANGER:
      speed = 0;
      break;
    case RESTRICTED:
      speed = 15;
      break;
    case CAUTION:
      speed = CAUTION_SPEED;
      break;
    default:
      break;
  }

  return speed;
}

void Block::AlgorClear(){
  loggerf(TRACE, "Block %02i:%02i AlgorClear", module, id);
  memset(&Alg, 0, sizeof(struct algor_blocks));

  Alg.B = this;

  MaxSpeed = BlockMaxSpeed;
}
#define ALGORLENGTH 100

void Block::AlgorSearch(int debug){
  loggerf(TRACE, "Blocks::AlgorSearch - %02i:%02i", module, id);
  Block * next = 0;
  Block * prev = 0;

  AlgorClear();

  loggerf(TRACE, "Search blocks %02i:%02i", module, id);

  next = Next_Block(NEXT | FL_SWITCH_CARE, 1);
  prev = Next_Block(PREV | FL_SWITCH_CARE, 1);

  if(next){
    Alg.next = _NextList(this, Alg.N, 0, NEXT | FL_SWITCH_CARE, 600);

    AlgorSetDepths(NEXT);
  }
  if(prev){
    Alg.prev = _NextList(this, Alg.P, 0, PREV | FL_SWITCH_CARE, 600);

    AlgorSetDepths(PREV);
  }
}

void Block::AlgorSetDepths(bool Side){
  struct {
    uint8_t * n;
    uint8_t * nx[3];// = {D1, D2, D3};
    Block ** B;
  } data;

  switch(Side){
    case NEXT:
      data.nx[0] = &Alg.next1;
      data.nx[1] = &Alg.next2;
      data.nx[2] = &Alg.next3;
      data.n     = &Alg.next;
      data.B    = (Block **)&Alg.N;
      break;
    case PREV:
      data.nx[0] = &Alg.prev1;
      data.nx[1] = &Alg.prev2;
      data.nx[2] = &Alg.prev3;
      data.n     = &Alg.prev;
      data.B    = (Block **)&Alg.P;
      break;
  }

  *data.nx[0] = *data.n;
  *data.nx[1] = *data.n;
  *data.nx[2] = *data.n;

  uint16_t length = 0;
  uint8_t j = 0;

  if(*data.n > 0)
    length = data.B[0]->length;

  for(uint8_t i = 1; i < *data.n; i++){
    bool sameStation  = !this->station;
         sameStation |= !data.B[i]->station;
         sameStation |= (data.B[i-1]->station && data.B[i]->station != data.B[i-1]->station);
         sameStation |= data.B[i-1]->type == NOSTOP;

    if(length >= ALGORLENGTH && j < 3 && data.B[i]->type != NOSTOP && sameStation){
      *(data.nx[j++]) = i;
      length = 0;
    }

    length += data.B[i]->length;
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
  return dircmp(A->dir, B->dir);
}

int dircmp(uint8_t A, uint8_t B){
  uint8_t returnMatrix[8*8] = {
    //    A   / 0  1  2  3  4  5  6  7
    /*    B 0*/ 1, 0, 1, 0, 0, 1, 0, 0,
    /*      1*/ 0, 1, 0, 0, 1, 0, 1, 0,
    /*      2*/ 1, 0, 1, 0, 0, 1, 0, 0,
    /*      3*/ 0, 0, 0, 0, 0, 0, 0, 0,
    /*      4*/ 0, 1, 0, 0, 1, 0, 1, 0,
    /*      5*/ 1, 0, 1, 0, 0, 1, 0, 0,
    /*      6*/ 0, 1, 0, 0, 1, 0, 1, 0,
    /*      7*/ 0, 0, 0, 0, 0, 0, 0, 0
  };
  return returnMatrix[A + (B<<3)];
}


// void Reserve_To_Next_Switch(Block * B){
//   loggerf(WARNING, "Block_Reverse_To_Next_Switch %02i:%02i", B->module, B->id);
//   Block * Next_Block = 0;
//   Block * Prev_Block = 0;

//   if(B->Alg.next > 0 && B->Alg.N[0])
//     Next_Block = B->Alg.N[0];

//   while(1){
//     if(!Next_Block){
//       break;
//     }
//     loggerf(INFO, "Reserve NSw %02i:%02i", Next_Block->module, Next_Block->id);
//     Next_Block->reserve();
//     // Block_reserve(Next_Block);

//     if(Next_Block->Alg.next > 0 && Next_Block->Alg.N[0]){
//       if(Next_Block->Alg.N[0] == Prev_Block){
//         Next_Block = 0;
//       }
//       else{
//         Prev_Block = Next_Block;
//         Next_Block = Next_Block->Alg.N[0];
//         if(Next_Block->switch_len || Next_Block->MSSw)
//           Next_Block = 0;
//       }
//     }
//     else
//       Next_Block = 0;

//     usleep(100000);
//   }
// }

// int Block_Reverse_To_Next_Switch(Block * B){
//   loggerf(WARNING, "Block_Reverse_To_Next_Switch %02i:%02i", B->module, B->id);
//   Block * Next_Block = 0;

//   if(B->Alg.next > 0 && B->Alg.N[0])
//     Next_Block = B->Alg.N[0];

//   while(1){
//     if(!Next_Block){
//       break;
//     }
//     loggerf(INFO, "Block reverse NSw %02i:%02i", Next_Block->module, Next_Block->id);
//     Next_Block->reverse();
//     Next_Block->reserve();
//     // Block_Reverse(&Next_Block->Alg);
//     // Block_reserve(Next_Block);

//     if(Next_Block->Alg.next > 0 && Next_Block->Alg.N[0])
//       Next_Block = Next_Block->Alg.N[0];
//     else
//       Next_Block = 0;
//   }
//   return 0;
// }
