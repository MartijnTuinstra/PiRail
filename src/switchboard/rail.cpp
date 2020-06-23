

#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"

#include "system.h"
#include "mem.h"
#include "modules.h"
#include "logger.h"
#include "IO.h"
#include "algorithm.h"


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

Block::Block(uint8_t module, struct s_block_conf block){
  loggerf(MEMORY, "Block Constructor %02i:%02i", module, block.id);
  memset(this, 0, sizeof(Block));
  this->module = module;
  this->id = block.id;
  this->type = (enum Rail_types)block.type;

  //Unit * U = Units[this->module]; Never used

  this->next.module = block.next.module;
  this->next.id = block.next.id;
  this->next.type = (enum link_types)block.next.type;
  this->prev.module = block.prev.module;
  this->prev.id = block.prev.id;
  this->prev.type = (enum link_types)block.prev.type;

  this->max_speed = block.speed;
  this->dir = (block.fl & 0x6) >> 1;
  this->length = block.length;
  this->oneWay = block.fl & 0x1;

  this->Alg.B = this;

  this->IOchanged = 1;
  this->algorchanged = 1;

  this->state = PROCEED;
  this->reverse_state = PROCEED;

  // struct s_node_adr in;
  // in.Node = block.IO_In.Node;
  // in.io = block.IO_In.Adr;

  // // Init_IO(Units[this->module], in, this);

  // if(block.fl & 0x8){
  //     struct s_node_adr out;
  //     out.Node = block.IO_Out.Node;
  //     out.io = block.IO_Out.Adr;

  //     // Init_IO(Units[this->module], out, this);
  // }

  // Insert block into Unit
  Units[this->module]->insertBlock(this);
}

Block::~Block(){
  loggerf(MEMORY, "Block %i:%i Destructor\n", module, id);
  if(this->Sw)
    _free(this->Sw);

  this->Sw = 0;
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

  if(this->dir == 2){
    dir = !dir;
  }

  if(!next)
    loggerf(ERROR, "Empty next Link");
  return next;
}


Block * Block::_Next(int flags, int level){
  loggerf(TRACE, "Next(%02i:%02i, %2x, %2x)", this->module, this->id, flags, level);
  // Find next (detection) block in direction dir. Could be used recurse for x-levels
  int dir = flags & 0x0F;
  // dir: 0 next, 1 prev

  struct rail_link * next;

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
    // reverse(1) -> normal(2)
    // normal(2) -> reverse(1)
    // reverse(1) -> normal(0)
    // normal(0) -> reverse(1)
    else if((pdir == 1 && this->dir == 2) || (pdir == 2 && this->dir == 1) ||
            (pdir == 1 && this->dir == 0) || (pdir == 0 && this->dir == 1)){
      dir ^= 0b1;
    }
  }
  else
    flags |= NEXT_FIRST_TIME_SKIP;

  // loggerf(TRACE, "dir: %i:%i-%x %x\t%i", this->module, this->id, dir, this->dir, pdir);
  //NEXT == 0
  //PREV == 1
  next = this->NextLink(dir & 1);

  dir = (dir & 1) + (this->dir << 1);

  flags = (flags & 0xF0) + (dir & 0x0F);

  // loggerf(TRACE, "Next     : dir:%i/%x\t%i:%i => %i:%i:%i\t%i", this->dir, dir, this->module, this->id, next->module, next->id, next->type, level);

  if(!next->p.p){
    if(next->type != RAIL_LINK_E && next->type != RAIL_LINK_C)
      loggerf(ERROR, "NO POINTERS %i:%i", this->module, this->id);
    return 0;
  }

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

  Algor_print_block_debug(AB->B);
}

void Block::reserve(){
  if(this->state >= PROCEED)
    this->state = RESERVED;
  this->reverse_state = DANGER;

  this->reserved++;

  this->statechanged = 1;
  Units[this->module]->block_state_changed = 1;
}



void Block::AlgorClear(){
  loggerf(INFO, "Block %02i:%02i AlgorClear", this->module, this->id);
  memset(this->Alg.P, 0, 10*sizeof(void *));
  memset(this->Alg.N, 0, 10*sizeof(void *));
  this->Alg.prev = 0;
  this->Alg.next = 0;
}


void Block::AlgorSearch(int debug){
  loggerf(TRACE, "Blocks::AlgorSearch - %02i:%02i", this->module, this->id);
  Block * next = 0;
  Block * prev = 0;

  // if(this->type == TURNTABLE || this->type == CROSSING){
  //   loggerf(ERROR, "Block is a turntable/crossover");
  //   AlgorSearchMSSwitch(debug);
  //   Algor_print_block_debug(this);
  //   return;
  // }

  Algor_Blocks * ABs = &this->Alg;
  Block * tmpB = this;

  AlgorClear();

  loggerf(TRACE, "Search blocks %02i:%02i", this->module, this->id);

  next = this->_Next(0 | SWITCH_CARE,1);
  prev = this->_Next(1 | SWITCH_CARE,1);

  //Select all surrounding blocks
  uint8_t i = 0;
  uint8_t j = 0;
  uint8_t level = 1;
  uint16_t length = 0;
  if(next){
    do{
      if(i == 0 && ABs->next == 0){
        tmpB = next;
      }
      else{
        tmpB = this->_Next(NEXT | SWITCH_CARE, level);
      }

      if(!tmpB){
        break;
      }

      ABs->N[i] = tmpB;

      length += tmpB->length;

      ABs->next += 1;

      i++;
      level++;

      if(length > Block_Minimum_Size){
        if(j == 0)
          ABs->next1 = ABs->next;
        else if(j == 1)
          ABs->next2 = ABs->next;
        else if(j == 2)
          ABs->next3 = ABs->next;
        length = 0;
        j++;
      }
    }
    while(j < 3 && i < 10);

    // If not all blocks were available
    // Not 3 times Minimum Size
    if (j == 2)
      ABs->next3 = ABs->next;
    else if (j == 1)
      ABs->next2 = ABs->next;
    else if (j == 0)
      ABs->next1 = ABs->next;
  }

  i = 0;
  j = 0;
  level = 1;
  length = 0;

  if(prev){
    do{
      if(i == 0 && ABs->prev == 0){
        tmpB = prev;
      }
      else{
        tmpB = this->_Next(PREV | SWITCH_CARE, level);
      }

      if(!tmpB){
        break;
      }

      ABs->P[i] = tmpB;

      length += tmpB->length;

      ABs->prev += 1;

      i++;
      level++;

      if(length > Block_Minimum_Size){
        if(j == 0)
          ABs->prev1 = ABs->prev;
        else if(j == 1)
          ABs->prev2 = ABs->prev;
        else if(j == 2)
          ABs->prev3 = ABs->prev;
        length = 0;
        j++;
      }
    }
    while(j < 3 && i < 10);

    // If not all blocks were available
    // Not 3 times Minimum Size
    if (j == 2)
      ABs->prev3 = ABs->prev;
    else if (j == 1)
      ABs->prev2 = ABs->prev;
    else if (j == 0)
      ABs->prev1 = ABs->prev;
  }
}


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
