
#include "switchboard/manager.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/signals.h"

#include "rollingstock/train.h"

#include "config/LayoutStructure.h"

#include "system.h"
#include "utils/mem.h"
#include "utils/logger.h"
#include "IO.h"
#include "path.h"

#include <algorithm>

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

Block::Block(uint8_t _module, struct configStruct_Block * block):
  next(block->next), prev(block->prev)
{
  loggerf(DEBUG, "Block Constructor %02i:%02i", _module, block->id);
  module = _module;
  id = block->id;
  type = (enum Rail_types)block->type;

  uid = switchboard::SwManager->addBlock(this);

  // Algor Init
  Alg.N = &Alg.AlgorBlocks[0];
  Alg.P = &Alg.AlgorBlocks[1];
  Alg.trainFollowingChecked = true;
  Alg.switchChecked = true;
  Alg.polarityChecked = true;
  Alg.statesChecked = false;

  AlgorClear();
  
  statechanged = 0;
  recalculate = 0;
  switchWrongState = 0;
  switchWrongFeedback = 0;

  BlockMaxSpeed = block->speed;
  // dir = (block->fl & 0x6) >> 1;
  dir = 0;
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

  if(U->IO(block->IOdetection))
    In_detection = U->linkIO(block->IOdetection, this, IO_Input_Block);

  // if(block->fl & 0x8 && U->IO(block->IOpolarity)){
  //   Out_polarity.push_back(U->linkIO(block->IOpolarity, this, IO_Output));
  // }

  // Polarity
  polarity_type   = block->Polarity;
  polarity_status = POLARITY_NORMAL;

  if(polarity_type && polarity_type < BLOCK_FL_POLARITY_LINKED_BLOCK)
    new Path(this);

  switch(polarity_type){
    case BLOCK_FL_POLARITY_SINGLE_IO:
      Out_polarity.push_back(U->linkIO(block->Polarity_IO[0], this, IO_Output));
      break;
    case BLOCK_FL_POLARITY_DOUBLE_IO:
      Out_polarity.push_back(U->linkIO(block->Polarity_IO[0], this, IO_Output));
      Out_polarity.push_back(U->linkIO(block->Polarity_IO[1], this, IO_Output));
      break;
    case BLOCK_FL_POLARITY_LINKED_BLOCK:
      if(block->Polarity_IO[0].Port >= U->block_len){
        loggerf(WARNING, "Link polarity block out of bounds");
        break;
      }

      polarity_link = U->B[block->Polarity_IO[0].Port];
      break;
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

  if (In_detection)
    In_detection->exportConfig(&cfg->IOdetection);

  if(polarity_type > 1)
    Out_polarity[0]->exportConfig(&cfg->Polarity_IO[0]);
  if(polarity_type == 3)
    Out_polarity[1]->exportConfig(&cfg->Polarity_IO[1]);

  cfg->Polarity = polarity_type;

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
RailLink * Block::NextLink(int flags){
  int LinkDir = flags & FL_DIRECTION_MASK;

  RailLink * nextLink = 0;

  if(dir == NEXT)
    nextLink = (LinkDir == NEXT) ? &this->next : &this->prev;
  else
    nextLink = (LinkDir == NEXT) ? &this->prev : &this->next;

  if(!nextLink)
    loggerf(ERROR, "Empty next Link");

  return nextLink;
}

Block * Block::Next_Block(int flags, int level){
  // Find next (detection) block in direction dir. Could be used recurse for x-levels
  loggerf(TRACE, "Next Block(%02i:%02i, %x, %i)", module, id, flags, length);
  Block * B[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  uint8_t currentLevel, blocks;
  do{
    currentLevel = level >= 10 ? 10 : level;
    blocks = _NextList(this, (Block **)B, 0, flags | FL_BLOCKS_COUNT, currentLevel);
    level -= 10;
  }
  while(level > 0);

  if(!blocks)
    return 0;
  else
    return B[currentLevel-1];
}

uint8_t Block::_NextList(Block * Origin, Block ** blocks, uint8_t block_counter, uint32_t flags, int length){
  // Find next (detection) blocks in direction dir.
  loggerf(TRACE, "NextList(%02i:%02i, %i, %4x, %i)", module, id, block_counter, flags, length);

  RailLink * nextLink;

  if(length < 0){
    return block_counter;
  }

  if(flags & FL_BLOCKS_COUNT)
    length--;
  else
    length -= this->length;

  // If not Init
  if(flags & FL_NEXT_FIRST_TIME_SKIP){
    // If toggle request
    Block * prevBlock;

    if(block_counter > 0)
      prevBlock = blocks[block_counter-1];
    else
      prevBlock = Origin;

    char buffer[100];

    sprintf(buffer, " %c (%i %i)? =>", (flags & FL_DIRECTION_MASK) ? 'P' : 'N', dircmp(prevBlock->dir, dir), cmpPolarity(prevBlock));

    if(!dircmp(prevBlock->dir, dir)){
      flags ^= FL_DIRECTION_MASK;

      if(flags & FL_DIRECTION_CARE)
        return block_counter;
    }

    if(!cmpPolarity(prevBlock)){
      flags ^= FL_DIRECTION_MASK;
    }

    loggerf(TRACE, "%s %c", buffer, (flags & FL_DIRECTION_MASK) ? 'P' : 'N');
    
    blocks[block_counter++] = this;
  }
  else{
    flags |= FL_NEXT_FIRST_TIME_SKIP;
    if((flags & FL_BLOCKS_COUNT) == 0)
      length += this->length;
  }

  if(block_counter >= 10)
    return block_counter;

  nextLink = this->NextLink(flags & FL_DIRECTION_MASK);

  if(!nextLink->p.p){
    if(nextLink->type != RAIL_LINK_E && nextLink->type != RAIL_LINK_C)
      loggerf(ERROR, "NO POINTERS %i:%i", this->module, this->id);
    return block_counter;
  }

  return _NextList_NextIteration(nextLink, this, Origin, blocks, block_counter, flags, length);
}

uint8_t _NextList_NextIteration(RailLink * nextLink, void * p, Block * Origin, Block ** blocks, uint8_t block_counter, uint64_t flags, int length){
  if(nextLink->type == RAIL_LINK_R){
    return nextLink->p.B->_NextList(Origin, blocks, block_counter, flags, length);
  }
  else if(nextLink->type == RAIL_LINK_S){
    return nextLink->p.Sw->NextList_Block(Origin, blocks, block_counter, nextLink->type, flags, length);
  }
  else if(nextLink->type == RAIL_LINK_s && nextLink->p.Sw->approachable(p, flags)){
    return nextLink->p.Sw->NextList_Block(Origin, blocks, block_counter, nextLink->type, flags, length);
  }
  else if(nextLink->type == RAIL_LINK_MA && nextLink->p.MSSw->approachableA(p, flags)){
    return nextLink->p.MSSw->NextList_Block(Origin, blocks, block_counter, nextLink->type, flags, length);
  }
  else if(nextLink->type == RAIL_LINK_MB && nextLink->p.MSSw->approachableB(p, flags)){
    return nextLink->p.MSSw->NextList_Block(Origin, blocks, block_counter, nextLink->type, flags, length);
  }
  else if(nextLink->type == RAIL_LINK_MA_inside || nextLink->type == RAIL_LINK_MB_inside){
    flags &= ~FL_NEXT_FIRST_TIME_SKIP;
    return nextLink->p.MSSw->NextList_Block(Origin, blocks, block_counter, nextLink->type, flags, length);
  }
  else if(nextLink->type == RAIL_LINK_TT){
    if(nextLink->p.MSSw->approachableA(p, flags)){
      nextLink->type = RAIL_LINK_MA;

      // If turntable is turned around
      if(nextLink->p.MSSw->NextLink(flags)->p.p != p){
        flags ^= 0b1;
      }
    }
    else if(nextLink->p.MSSw->approachableB(p, flags)){
      nextLink->type = RAIL_LINK_MB;

      // If turntable is turned around
      if(nextLink->p.MSSw->NextLink(flags)->p.p != p){
        flags ^= 0b1;
      }
    }
    return nextLink->p.MSSw->NextList_Block(Origin, blocks, block_counter, nextLink->type, flags, length);
  }

  return block_counter;
}

void Block::reverse(){
  loggerf(INFO, "Block_Reverse %02i:%02i %i -> %i", module, id, dir, dir ^ 1);

  //_ALGOR_BLOCK_APPLY(_ABl, _A, _B, _C) if(_ABl->len == 0){_A}else{_B;for(uint8_t i = 0; i < _ABl->len; i++){_C}}
  dir ^= 1;

  // Swap states
  if(state != RESERVED_SWITCH)
    std::swap(state, reverse_state);

  // Swap block lists
  std::swap(Alg.N, Alg.P);
  Algorithm::print_block_debug(this);

  // Swap Signals
  std::swap(forward_signal, reverse_signal);
}

void Block::reserve(Train * T){
  loggerf(INFO, "Reserve Block %2i:%2i for train %i (%i)", module, id, T->id, reserved);
  reserved = true;

  if(!blocked && state >= PROCEED)
    setState(RESERVED);

  setReversedState(DANGER);

  reservedBy.push_back(T);
}

void Block::dereserve(Train * T){

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
  }
}

bool Block::isReservedBy(Train * T){
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
  loggerf(DEBUG, "Block %2i:%2i setState %s", module, id, rail_states_string[_state]);
  if(_state == PROCEED && reserved)
    _state = RESERVED;

  setState(&state, _state, forward_signal);
}

void Block::setReversedState(enum Rail_states _state){
  loggerf(DEBUG, "Block %2i:%2i setReversedState %s", module, id, rail_states_string[_state]);

  setState(&reverse_state, _state, reverse_signal);
}

void Block::setState(enum Rail_states * statePtr, enum Rail_states _state, std::vector<Signal *> * Signals){
  *statePtr = _state;

  uint8_t signalsize = Signals->size();
  for(uint8_t i = 0; i < signalsize; i++){
    Signals->operator[](i)->set(_state);
  }

  statechanged = 1;
  U->block_state_changed |= 1;
}

enum Rail_states Block::getNextState(){
  Block * Next = 0;
  if(Alg.N->group[3] > 0)
    Next = Alg.N->B[0];
  else
    return DANGER;

  if(dircmp(this, Next))
    return Next->state;
  else
    return Next->reverse_state;
}

enum Rail_states Block::getPrevState(){
  Block * Prev = 0;
  if(Alg.P->group[3] > 0)
    Prev = Alg.P->B[0];
  else
    return DANGER;

  if(dircmp(this, Prev))
    return Prev->reverse_state;
  else
    return Prev->state;
}

void Block::setDetection(bool detected){
  Alg.trainFollowingChecked = false;
  Alg.doneAll = false;

  if(virtualBlocked && detected && !detectionBlocked && expectedDetectable && train){
    expectedDetectable->stepForward(this);
  }

  bool prevBlocked = detectionBlocked;

  detectionBlocked = detected;
  blocked = (detectionBlocked || virtualBlocked);

  if(prevBlocked != detectionBlocked)
    IOchanged = 1;
}

void Block::setVirtualDetection(bool d){
  Alg.trainFollowingChecked = false;
  Alg.doneAll = false;

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
  memset(Alg.P, 0, sizeof(struct algor_blocks));
  memset(Alg.N, 0, sizeof(struct algor_blocks));

  Alg.B = this;
  Alg.algorBlockSearched = 0;
  Alg.doneAll = 0;

  MaxSpeed = BlockMaxSpeed;
}
#define ALGORLENGTH 100

void Block::AlgorSearch(int debug){
  loggerf(TRACE, "Blocks::AlgorSearch - %02i:%02i", module, id);
  // Block * next = 0;
  // Block * prev = 0;

  AlgorClear();

  loggerf(TRACE, "Search blocks %02i:%02i", module, id);

  // next = Next_Block(NEXT | FL_SWITCH_CARE, 1);
  // prev = Next_Block(PREV | FL_SWITCH_CARE, 1);

  Alg.N->group[3] = _NextList(this, Alg.N->B, 0, NEXT | FL_SWITCH_CARE, 600);
  AlgorSetDepths(NEXT);

  Alg.P->group[3] = _NextList(this, Alg.P->B, 0, PREV | FL_SWITCH_CARE, 600);
  AlgorSetDepths(PREV);

  Alg.algorBlockSearched = true;
}

void Block::AlgorSetDepths(bool Side){
  struct {
    uint8_t * n;
    uint8_t * nx;// = {D1, D2, D3};
    Block ** B;
  } data;

  switch(Side){
    case NEXT:
      data.nx = &Alg.N->group[0];
      data.n  = &Alg.N->group[3];
      data.B  = (Block **)&Alg.N->B;
      break;
    case PREV:
      data.nx = &Alg.P->group[0];
      data.n  = &Alg.P->group[3];
      data.B  = (Block **)&Alg.P->B;
      break;
  }

  data.nx[0] = *data.n;
  data.nx[1] = *data.n;
  data.nx[2] = *data.n;

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
      data.nx[j++] = i;
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

// Polarity
bool Block::checkPolarity(Block * B){
  if(B == this)
    return 1;
  // if(Alg.next == 0 || Alg.prev == 0 || B->Alg.next == 0 || B->Alg.prev == 0)
  //   return 0;
  // loggerf(WARNING, "%x == %x\t%i %i, %i %i\t%x %x, %x %x", (unsigned int) this, (unsigned int) B, Alg.next, Alg.prev, B->Alg.next, B->Alg.prev, (unsigned int) Alg.N[0], (unsigned int) Alg.P[0], (unsigned int) B->Alg.N[0], (unsigned int) B->Alg.P[0]);
  if(MSSw && B->MSSw){
    loggerf(ERROR, "FIXME");
  }
  else if(MSSw)
    return MSSw->checkPolarity(B);
  else if(B->MSSw)
    return B->MSSw->checkPolarity(this);

  if(std::any_of(next.Polarity.begin(), next.Polarity.end(), [this, B](auto i){ return i.first == B && (i.second ^ this->polarity_status ^ B->polarity_status); } ) ||
     std::any_of(prev.Polarity.begin(), prev.Polarity.end(), [this, B](auto i){ return i.first == B && (i.second ^ this->polarity_status ^ B->polarity_status); } )    ){
       return 1;
  }
  return 0;
}

bool Block::cmpPolarity(Block * B){
  if(B == this)
    return 1;
  // if(Alg.next == 0 || Alg.prev == 0 || B->Alg.next == 0 || B->Alg.prev == 0)
  //   return 0;
  // loggerf(WARNING, "%x == %x\t%i %i, %i %i\t%x %x, %x %x", (unsigned int) this, (unsigned int) B, Alg.next, Alg.prev, B->Alg.next, B->Alg.prev, (unsigned int) Alg.N[0], (unsigned int) Alg.P[0], (unsigned int) B->Alg.N[0], (unsigned int) B->Alg.P[0]);
  if(MSSw && B->MSSw){
    loggerf(ERROR, "FIXME");
  }
  else if(MSSw)
    return MSSw->cmpPolarity(B);
  else if(B->MSSw)
    return B->MSSw->cmpPolarity(this);

  if(std::any_of(next.Polarity.begin(), next.Polarity.end(), [this, B](auto i){ return i.first == B && i.second; } ) ||
     std::any_of(prev.Polarity.begin(), prev.Polarity.end(), [this, B](auto i){ return i.first == B && i.second; } )    ){
       return 1;
  }
  return 0;
}

void Block::flipPolarity(){
  flipPolarity(0);
}
void Block::flipPolarity(bool _reverse){
  loggerf(WARNING, "flipPolarity %2i:%2i  %i", module, id, polarity_type);

  if(polarity_type == BLOCK_FL_POLARITY_DISABLED)
    return;

  polarity_status ^= 1;

  switch(polarity_type){
    case BLOCK_FL_POLARITY_SINGLE_IO:
      Out_polarity[0]->setOutput(polarity_status);
      break;
    case BLOCK_FL_POLARITY_DOUBLE_IO:
      Out_polarity[0]->setOutput(polarity_status ? IO_event_Low   : IO_event_Pulse);
      Out_polarity[1]->setOutput(polarity_status ? IO_event_Pulse : IO_event_Low);
      break;
    default:
      break;
  }

  if(_reverse)
    reverse();
}


int dircmp(Block *A, Block *B){
  return dircmp(A->dir, B->dir);
}

int dircmp(uint8_t A, uint8_t B){
  return A == B;
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
