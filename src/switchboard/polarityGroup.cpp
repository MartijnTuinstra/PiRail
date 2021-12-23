#include <algorithm>

#include "switchboard/polarityGroup.h"
#include "switchboard/manager.h"
#include "switchboard/unit.h"

#include "rollingstock/train.h"

#include "utils/logger.h"


namespace PolaritySolver {

int solve(Train * T, std::vector<PolarityGroup *> near, PolarityGroup * far){
  loggerf(INFO, "PolaritySolver::solve(id:%i, %x, %x)", T->id, near, far);

  if(far && far->flip()){
    loggerf(INFO, "B/far side fix");
    return true;
  }
  else if(near.size()){
    bool failed = false;
    uint8_t x = 0;
    for (auto i = near.rbegin(); i != near.rend(); ++i ) { 
      PolarityGroup * PG = *i;
      bool test = PG->flippableTest(0);
      loggerf(INFO, " [%i] %i", x++, test);
      for(auto A: PG->blocks)
        printf("%02i ", A->id);
      printf("\n");
      failed |= (test == 0);
    }

    if(failed)
      return false;

    for(auto i: near)
      i->flip();

    return true;
  }
  return 0;
}

};

std::vector<PolarityGroup *> PolarityGroupList;

PolarityGroup::PolarityGroup(uint8_t M, struct configStruct_PolarityGroup * config){
  type = config->type;

  Unit * U = switchboard::Units(M);

  for(uint8_t i = 0; i < config->nr_blocks; i++){
    Block * B = U->B[config->blocks[i]];
    if (!B){
      loggerf(WARNING, " Polarity Group failed to aquire block %i", config->blocks[i]);
      continue;
    }
    blocks.push_back(B);
    B->Polarity = this;
  }

  PolarityGroupList.push_back(this);
}

PolarityGroup::~PolarityGroup(){}

void PolarityGroup::map(){

  if (blocks.size() == 1){
    ends[0] = blocks[0];
    ends[1] = blocks[0];

    direction[0] = NEXT;
    direction[1] = PREV;
  }

  else{
    uint8_t i = 0;

    for(auto b: blocks){
      Block * nB = b->Next_Block(NEXT, 1);
      Block * pB = b->Next_Block(PREV, 1);

      bool n = std::any_of(blocks.begin(), blocks.end(), [nB](Block * b){ return nB == b; });
      bool p = std::any_of(blocks.begin(), blocks.end(), [pB](Block * b){ return pB == b; });

      if(n != p){
        if(i < 2){
          ends[i] = b;

          uint8_t dir = p ? NEXT : PREV;

          direction[i] = dir ^ b->dir;
          i++;
        }
        else
          loggerf(WARNING, "Polarity Group with more than 2 ends!");
      }
    }
  }

  loggerf(INFO, " PolarityGroup ends:");
  loggerf(INFO, "   %02i:%02i   %02i:%02i", ends[0]->module, ends[0]->id, ends[1]->module, ends[1]->id);
  Block * B_next = ends[0]->Next_Block(direction[0] ^ ends[0]->dir, 1);
  Block * B_prev = ends[1]->Next_Block(direction[1] ^ ends[1]->dir, 1);

  if(B_next && B_prev)
    loggerf(INFO, "   %02i:%02i   %02i:%02i", B_next->module, B_next->id, B_prev->module, B_prev->id);
  else
    loggerf(INFO, "   xx:xx   xx:xx");
}

void PolarityGroup::updateDetection(){
  train = std::any_of(blocks.begin(), blocks.end(), [](Block * b){return b->blocked; });
}

bool PolarityGroup::flip(){
  return flip(0);
}

bool PolarityGroup::flip(PolarityGroup * PG){
  Block * B;
  switch(flippable(PG)){
    case 0: // No Problem
      break;
    case 1: // ends[0] is blocked
      B = ends[0]->Next_Block(direction[0] ^ ends[0]->dir, 1);
      loggerf(INFO, "Recursive Search [0]");
      if(!(B && B->Polarity && B->Polarity->flip(this))){
        return false;
      }
      break;
    case 2: // ends[1] is blocked
      B = ends[1]->Next_Block(direction[1] ^ ends[1]->dir, 1);
      loggerf(INFO, "Recursive Search [1]");
      if(!(B && B->Polarity && B->Polarity->flip(this))){
        return false;
      }
      break;
    case 3: // Polarity is not changeable
      return false;
  }

  loggerf(INFO, "Reverse Polarity Group");
  status ^= 1;

  for(Block * b: blocks)
    b->flipPolarity();

  return true;
}

uint8_t PolarityGroup::flippable(){
  return flippable(0);
}

uint8_t PolarityGroup::flippable(PolarityGroup * PG){
  if(type == BLOCK_FL_POLARITY_DISABLED)
    return 3;

  if(ends[0]->blocked){
    Block * B = ends[0]->Next_Block(direction[0] ^ ends[0]->dir, 1);

    if(B->Polarity != PG){

      loggerf(WARNING, "polFlip? Entrance %02i:%02i%cT%i, %02i:%02i%cT%i",
              ends[0]->module, ends[0]->id, ends[0]->blocked ? 'B': ' ', ends[0]->train ? ends[0]->train->id : -1,
              B->module, B->id, B->blocked ? 'B': ' ', B->train ? B->train->id : -1);

      if(B && B->blocked && ends[0]->train == B->train){
        return 1;
      }
    }
  }
  
  if(ends[1]->blocked){
    Block * B = ends[1]->Next_Block(direction[1] ^ ends[1]->dir, 1);

    if(B->Polarity != PG){

      loggerf(WARNING, "polFlip? Exit %02i:%02i%cT%i, %02i:%02i%cT%i",
              ends[1]->module, ends[1]->id, ends[1]->blocked ? 'B': ' ', ends[1]->train ? ends[1]->train->id : -1,
              B->module, B->id, B->blocked ? 'B': ' ', B->train ? B->train->id : -1);

      if(B && B->blocked && ends[1]->train == B->train){
        return 2;
      }
    }
  }
  return 0;
}

bool PolarityGroup::flippableTest(PolarityGroup * PG){
  Block * B;
  switch(flippable(PG)){
    case 0: // No Problem
      break;
    case 1: // ends[0] is blocked
      B = ends[0]->Next_Block(direction[0] ^ ends[0]->dir, 1);
      loggerf(INFO, "Recursive Search [0]");
      if(!(B && B->Polarity))
        return B->Polarity->flippableTest(this);

      break;
    case 2: // ends[1] is blocked
      B = ends[1]->Next_Block(direction[1] ^ ends[1]->dir, 1);
      loggerf(INFO, "Recursive Search [1]");
      if(!(B && B->Polarity))
        return B->Polarity->flippableTest(this);

      break;
    case 3: // Polarity is not changeable
      return false;
  }

  return true;
}