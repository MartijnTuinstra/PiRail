#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

#include "algorithm/core.h"
#include "algorithm/component.h"
#include "algorithm/queue.h"
#include "algorithm/blockconnector.h"

#include "system.h"
#include "utils/mem.h"
#include "utils/logger.h"


#include "switchboard/manager.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/switchsolver.h"
#include "switchboard/polaritysolver.h"
#include "switchboard/msswitch.h"
#include "switchboard/signals.h"
#include "switchboard/station.h"
#include "switchboard/blockconnector.h"
#include "train.h"
#include "flags.h"
#include "pathfinding.h"

#include "uart/uart.h"
#include "websocket/stc.h"

#include "sim.h"

#include "submodule.h"


namespace Algorithm {

std::mutex processMutex;

using namespace switchboard;

#define ALGORITHM_SUCESS_CODE 1
#define ALGORITHM_FAILED_CODE 2

void processBlock(blockAlgorithm * BA, int flags){
  loggerf(DEBUG, "processBA %02i:%02i, flags %x", BA->B->module, BA->B->id, flags);
  print_block_debug(BA->B);

  if(BA->doneAll && !(flags & _FORCE)){
    loggerf(WARNING, "  allreadyDone");
    return;
  }

  // Lock the processMutex.
  //   now clients cannot manipulate switches
  const std::lock_guard<std::mutex> lock(processMutex);

  // set scope variables
  Block * B = BA->B;
  BA->doneAll = 0;

  if(!B){
    loggerf(WARNING, "process Failed. Cannot find block (blockAlgorithm 0x%x)", (unsigned long)BA);
    BA->doneAll = ALGORITHM_FAILED_CODE;
    return;
  }

  if(!BA->algorBlockSearched){
    loggerf(DEBUG, "  AlgorSearch");
    B->AlgorSearch(flags);
  }

  // Add neighbouring blocks for state
  uint8_t prev = BA->P->group[3];
  uint8_t next = BA->N->group[3];
  Block ** PB = BA->P->B;
  Block ** NB = BA->N->B;

  if(!B->blocked && B->state == BLOCKED){
    if(next > 0 && NB[0]->blocked){
      // NB[0]->algorchanged = 1;
      loggerf(DEBUG, "  add block for state check %2i:%2i", NB[0]->module, NB[0]->id);
      NB[0]->Alg.statesChecked = false;
      NB[0]->Alg.doneAll = false;
      AlQueue.put(NB[0]);
    }
    else if(prev > 0 && PB[0]->blocked){
      // PB[0]->algorchanged = 1;
      PB[0]->Alg.statesChecked = false;
      PB[0]->Alg.doneAll = false;
      loggerf(DEBUG, "  add block for state check %2i:%2i", PB[0]->module, PB[0]->id);
      AlQueue.put(PB[0]);
    }
  }

  //Follow the train arround the layout
  if(!BA->trainFollowingChecked){
    loggerf(DEBUG, "  TrainFollowing");
    if(train_following(BA, flags)){
      BA->switchChecked   = false;
      BA->polarityChecked = false;
      BA->doneAll = false;
    };
    BA->trainFollowingChecked = true;
  }

  //Set oncomming switch to correct state
  if(!BA->switchChecked){
    loggerf(DEBUG, "  SwitchChecked");
    if(Switch_Checker(BA, flags))
      return;
    BA->switchChecked = true;
  }

  // Set paths to right direction
  if(!BA->polarityChecked){
    loggerf(DEBUG, "  PolarityChecked");
    if(Polarity_Checker(BA, flags))
      return;
    BA->polarityChecked = true;
  }
  
  // Print all found blocks
  // if(flags & _DEBUG)
    print_block_debug(B);

  // Station Stating
  if(B->station){
    if(B->blocked && B->state != BLOCKED)
      B->station->occupy(B->train);
    else if(!B->blocked && B->state == BLOCKED)
      B->station->release();
  }

  //Apply block stating
  if(!BA->statesChecked){
    loggerf(DEBUG, "  StatesChecked");
    rail_state(&B->Alg, flags);
  }

  BA->doneAll = ALGORITHM_SUCESS_CODE;

  loggerf(TRACE, "Done");
}

void process(Block * B, int flags){
  loggerf(TRACE, "process %02i:%02i, flags %x", B->module, B->id, flags);

  if((B->IOchanged == 0 && B->algorchanged == 0) && (flags & _FORCE) == 0){
    loggerf(TRACE, "No changes");
    return;
  }

  const std::lock_guard<std::mutex> lock(processMutex);

  // flags |= _DEBUG;

  // Find all surrounding blocks only if direction has changed or nearby switches
  if(B->IOchanged && B->algorchanged){
    B->AlgorSearch(flags);
  }

  B->IOchanged = 0;
  B->algorchanged = 0;

  uint8_t prev = B->Alg.P->group[3];
  uint8_t next = B->Alg.N->group[3];
  Block ** PB = B->Alg.P->B;
  Block ** NB = B->Alg.N->B;


  if(!B->blocked && B->state == BLOCKED){
    if(next > 0 && NB[0]->blocked){
      NB[0]->algorchanged = 1;
      AlQueue.put(NB[0]);
    }
    else if(prev > 0 && PB[0]->blocked){
      PB[0]->algorchanged = 1;
      AlQueue.put(PB[0]);
    }
  }

  // Set AllBlocks Blocked
  if(!B->Alg.B){
    loggerf(ERROR, "BLOCK %02i:%02i has no algo", B->module, B->id);
    B->Alg.B = B;
    B->AlgorClear();
  }

  // if(flags & _DEBUG){
  //   print_block_debug(B);
  // }

  //Follow the train arround the layout
  train_following(&B->Alg, flags);
  if (B->recalculate){
    loggerf(INFO, "Block Train ReProcess");
  }

  //Set oncomming switch to correct state
  Switch_Checker(&B->Alg, flags);
  if (B->recalculate){
    loggerf(DEBUG, "Block Switch ReProcess");
    B->algorchanged = true;
  }
  
  // Print all found blocks
  // if(flags & _DEBUG)
  print_block_debug(B);

  // Station Stating
  if(B->station){
    if(B->blocked && B->state != BLOCKED)
      B->station->occupy(B->train);
    else if(!B->blocked && B->state == BLOCKED)
      B->station->release();
  }

  //Apply block stating
  rail_state(&B->Alg, flags);
  loggerf(TRACE, "Done");
}


void Set_Changed(struct blockAlgorithm * ABs){
  loggerf(TRACE, "Algor_Set_Changed");
  //Scroll through all the pointers of allblocks
  uint8_t prev = ABs->P->group[3];
  uint8_t next = ABs->N->group[3];
  Block ** PB = ABs->P->B;
  Block ** NB = ABs->N->B;

  for(int i = 0; i < prev; i++){
    if(!PB[i])
      continue;

    if(PB[i] == ABs->B)
      continue;

    PB[i]->algorchanged = 1;
    PB[i]->IOchanged = 1;
    PB[i]->AlgorClear();

    if(!PB[i]->blocked)
      PB[i]->setState(PROCEED);
  }
  for(int i = 0; i < next; i++){
    if(!NB[i])
      continue;

    if(NB[i] == ABs->B)
      continue;

    NB[i]->algorchanged = 1;
    NB[i]->IOchanged = 1;
    NB[i]->AlgorClear();

    if(!NB[i]->blocked)
      NB[i]->setState(PROCEED);
  }

  if(ABs->B){
    ABs->B->algorchanged = 1;
    ABs->B->IOchanged = 1;
    ABs->B->setState(PROCEED);
  }
}

int Switch_to_rail(Block ** B, void * Sw, enum link_types type, uint8_t counter){
  RailLink next;

  //if(type == RAIL_LINK_S || type == RAIL_LINK_s){
  //  printf("Sw %i:%i\t%x\n", ((Switch *)Sw)->module, ((Switch *)Sw)->id, type);
  //}

  if(type == RAIL_LINK_S){
    if(( ((Switch *)Sw)->state & 0x7f) == 0)
      next = ((Switch *)Sw)->str;
    else
      next = ((Switch *)Sw)->div;
  }
  else if(type == RAIL_LINK_s){
    next = ((Switch *)Sw)->app;
  }

  if(next.type == RAIL_LINK_S){
    Switch * NSw = next.p.Sw;
    if(NSw->Detection && NSw->Detection != *B){
      counter++;
      *B = NSw->Detection;
    }
    return Switch_to_rail(B, next.p.Sw, RAIL_LINK_S, counter);
  }
  else if(next.type == RAIL_LINK_s){
    Switch * NSw = next.p.Sw;
    if(NSw->Detection && NSw->Detection != *B){
      counter++;
      *B = NSw->Detection;
      //printf("-%i:%i\n", (*B)->module, (*B)->id);
    }
    if((NSw->state & 0x7f) == 0 && NSw->str.p.Sw == Sw){
      return Switch_to_rail(B, NSw, RAIL_LINK_s, counter);
    }
    else if((NSw->state & 0x7f) == 1 && NSw->div.p.Sw == Sw){
      return Switch_to_rail(B, NSw, RAIL_LINK_s, counter);
    }
    else{
      *B = 0;
      return counter;
    }
  }
  else if(next.type == RAIL_LINK_R){
    Block * tmp_B = next.p.B;
    if(tmp_B != *B){
      counter++;
      *B = tmp_B;
      return counter;
    }
  }
  return 0;
}


void print_block_debug(Block * B){
  int debug = INFO;

  char output[300] = "";
  char * ptr = output;

  struct blockAlgorithm * ABs = &B->Alg;
  uint8_t prev = ABs->P->group[3];
  Block ** BP = ABs->P->B;
  Block ** BN = ABs->N->B;
  uint8_t next = ABs->N->group[3];

  char blockstates[10] = "BDRC rsU";

  for(int i = 7; i >= 0; i--){
    if(prev <= i){
      ptr += sprintf(ptr, "       ");
      continue;
    }

    if(prev > 8 && i == 7){
      ptr += sprintf(ptr, "<<<-%2d ", prev);
      continue;
    }

    if(BP[i]){
      char state = blockstates[BP[i]->state];
      if(BP[i]->virtualBlocked && !BP[i]->detectionBlocked)
        state = 'V';

      ptr += sprintf(ptr, "%02i:%02i%c%c", BP[i]->module, BP[i]->id, state,
                                           (i == ABs->P->group[0] || i == ABs->P->group[1] || i == ABs->P->group[2]) ? '|' : ' ');
    }
    else
      ptr += sprintf(ptr, "------ ");
  }

  ptr += sprintf(ptr, " A%3i %2x%02i:%02i;", B->length, B->type, B->module, B->id);
  ptr += sprintf(ptr, "%c", B->train ? 'T' : ' ');
  ptr += sprintf(ptr, "D%-2iS%x/%x", B->dir,B->state,B->reverse_state);
  if(B->detectionBlocked)
    ptr += sprintf(ptr, "b");
  else if(B->virtualBlocked)
    ptr += sprintf(ptr, "v");
  else
    ptr += sprintf(ptr, " ");

  ptr += sprintf(ptr, "  ");


  for(uint8_t i = 0; i < 8; i++){
    if(next <= i){
      break;
    }

    if(next > 8 && i == 7){
      ptr += sprintf(ptr, "%-2d->>>", next);
      break;
    }

    if(BN[i]){
      char state = blockstates[BN[i]->state];
      if(BN[i]->virtualBlocked && !BN[i]->detectionBlocked)
        state = 'V';

      ptr += sprintf(ptr, "%02i:%02i%c ", BN[i]->module, BN[i]->id, state);
    }
    else
      ptr += sprintf(ptr, "------ ");
  }
  ptr[0] = 0;

  loggerf((enum logging_levels)debug, "%s", output);
}

bool Switch_Checker(struct blockAlgorithm * ABs, int debug){
  //Unpack AllBlocks
  Block *  B  = ABs->B;
  Block ** BN = ABs->N->B;
  uint8_t next = ABs->N->group[3];
  //Block BNNN = *AllBlocks.BNNN;

  if(!B->blocked || (B->train && (B->train->stopped || B->train->B != B)))
    return false;

  Train * T = B->train;
  Block * tB;
  uint8_t routeStatus = T->routeStatus;
  int16_t distance = SpeedToDistance_A(T->speed, -10) + B->length;
  bool nostopDone = (B->type != NOSTOP);
  bool dir = NEXT;

  if(routeStatus == TRAIN_ROUTE_AT_DESTINATION)
    return false;

  // Search a maximum of five blocks even if the breaking distance is larger
  for(uint8_t i = 0; i < 5; i++){
    if(i > next)
      break;

    // Get block
    if(i == 0) tB = B;
    else tB = BN[i-1];

    if(tB->type != NOSTOP){
      distance -= tB->length;
      nostopDone = true;
    }

    if(distance < 0){
      loggerf(DEBUG, "distanceBreak");
      break;
    }

    if(tB->type == NOSTOP)
      distance -= tB->length;

    if(tB->blocked && tB != B){
      loggerf(DEBUG, "blockedBreak");
      break;
    }

    if(routeStatus && (tB == T->route->destinationBlocks[0] || tB == T->route->destinationBlocks[1])){
      routeStatus++;

      if(routeStatus == TRAIN_ROUTE_AT_DESTINATION){
        loggerf(DEBUG, "routeBreak");
        break;
      }
    }

    if(i == 1)     dir ^= (dircmp(B, tB)       != B->cmpPolarity(tB));
    else if(i > 1) dir ^= (dircmp(BN[i-2], tB) != BN[i-2]->cmpPolarity(tB));

    RailLink * link = tB->NextLink(dir);
    loggerf(DEBUG, "Switch_Checker scan block (%2i:%2i) f:%x, d:%i, %i - %i", tB->module, tB->id, dir, tB->dir, link->type, routeStatus);

    if(tB->type == NOSTOP){
      tB = (i > 1) ? BN[i-1] : B;
    }

    if(nostopDone && ((i >  0 && link->type == RAIL_LINK_S) || (link->type >= RAIL_LINK_s && link->type <= RAIL_LINK_MB_inside))){
      if(SwitchSolver::solve(T, B, tB, *link, NEXT | FL_SWITCH_CARE)){ // Returns true if path is changed
        return true; // Should reprocess
      }
      nostopDone = false;
    }
  }

  return false;
}

bool Polarity_Checker(struct blockAlgorithm * ABs, int debug){
  //Unpack Algorithm Blocks,
  Block *  B  = ABs->B;
  Block ** BN = ABs->N->B;
  uint8_t next = ABs->N->group[3];

  if(!B->blocked || (B->train && (B->train->stopped || B->train->B != B)))
    return false;

  Train * T = B->train;
  Block * tB;
  // Path * P = B->path;
  uint8_t routeStatus = T->routeStatus;
  int16_t distance = SpeedToDistance_A(T->speed, -10) + B->length;
  bool nostopDone = (B->type != NOSTOP);
  bool dir = NEXT;
  // bool polarity = 0;

  // Search a maximum of five blocks even if the breaking distance is larger
  for(uint8_t i = 0; i < 5; i++){
    if(i >= next){
      loggerf(DEBUG, "Next break");
      break;
    }

    // Get block
    tB = BN[i];

    if(tB->type != NOSTOP){
      distance -= tB->length;
      nostopDone = true;
    }

    // if(distance < 0){
    //   loggerf(DEBUG, "distanceBreak");
    //   break;
    // }

  //   if(tB->type == NOSTOP)
  //     distance -= tB->length;

    if(tB->blocked && tB != B){
      loggerf(DEBUG, "blockedBreak");
      break;
    }

    if(routeStatus && (tB == T->route->destinationBlocks[0] || tB == T->route->destinationBlocks[1])){
      routeStatus++;

      if(routeStatus == TRAIN_ROUTE_AT_DESTINATION){
        loggerf(DEBUG, "routeBreak");
        break;
      }
    }

    Block * prevBlock = (i == 0) ? B : BN[i-1];
    dir ^= (dircmp(prevBlock, tB) != prevBlock->cmpPolarity(tB));

    loggerf(INFO, "Polarity Search %02i:%02i -> %02i:%02i, dir:%i, polarity:%i", prevBlock->module, prevBlock->id, tB->module, tB->id, dir, prevBlock->checkPolarity(tB));

    if(!prevBlock->checkPolarity(tB)){
      loggerf(WARNING, "POLARITY FLIP NEEDED %02i:%02i", tB->module, tB->id);
      if(PolaritySolver::solve(T, prevBlock->Polarity, tB->Polarity))
        return true;
    }

  //   RailLink * link = tB->NextLink(dir);
  //   loggerf(DEBUG, "Switch_Checker scan block (%2i:%2i) f:%x, d:%i, %i - %i", tB->module, tB->id, dir, tB->dir, link->type, routeStatus);

  //   if(tB->type == NOSTOP){
  //     tB = (i > 1) ? BN[i-1] : B;
  //   }

  //   if(nostopDone && ((i >  0 && link->type == RAIL_LINK_S) || (link->type >= RAIL_LINK_s && link->type <= RAIL_LINK_MB_inside))){
  //     if(SwitchSolver::solve(T, B, tB, *link, NEXT | FL_SWITCH_CARE)){ // Returns true if path is changed
  //       return true; // Should reprocess
  //     }
  //     nostopDone = false;
  //   }
  }

  return false;
}

void rail_state(struct blockAlgorithm * ABs, int debug){
  loggerf(TRACE, "Algor_rail_state %02d:%02d", ABs->B->module, ABs->B->id);
  //Unpack ABs
  uint8_t prev  = ABs->P->group[3];
  Block ** BP   = ABs->P->B;
  Block *  B    = ABs->B;
  Block ** BN   = ABs->N->B;
  uint8_t next  = ABs->N->group[3];
  
  bool Dir = 0; // 0 = setState
                // 1 = setReversedState
  
  enum Rail_states state[3]     = {PROCEED, PROCEED, PROCEED};
  enum Rail_states Rev_state[3] = {PROCEED, PROCEED, PROCEED};
  uint8_t prevGroup[3] = {ABs->P->group[0], ABs->P->group[1], ABs->P->group[2]};
  uint8_t nextGroup[3] = {ABs->N->group[0], ABs->N->group[1], ABs->N->group[2]};
  uint8_t j = 0;
  uint8_t Rev_j = 0;

  if(!B->blocked && B->state == BLOCKED)
    B->setState(DANGER);;

  if(B->blocked){
    B->setState(BLOCKED);
    // Set states
    state[0] = DANGER;
    state[1] = CAUTION;

    if(B->type == STATION && B->station->type >= STATION_YARD)
      state[0] = RESTRICTED;
  }
  else if(B->type == NOSTOP && prev
           && ((next && BN[0]->type != NOSTOP) || next == 0)
           && (B->getNextState() == DANGER || B->switchWrongState || B->switchWrongFeedback)
          ){
    B->setState(DANGER);
    state[0] = DANGER;
    state[1] = CAUTION;

    uint8_t i = 0;
    while(BP[i]->type == NOSTOP && i++ < prev);

    if(i == 0)
      j++;
    else if(i != prev){
      i = prevGroup[0] - i;
      prevGroup[0] -= i;
      prevGroup[1] -= i;
      prevGroup[2] -= i;
    }
  }
  else if(next == 0){
    B->setState(CAUTION);

    state[0] = CAUTION;
  }

  if(B->type == NOSTOP && next && prev && BP[0]->type != NOSTOP && B->getPrevState() == DANGER){
    B->setReversedState(DANGER);
    Rev_state[0] = DANGER;
    Rev_state[1] = CAUTION;

    uint8_t i = 0;
    while(BN[i]->type == NOSTOP && i++ < next);

    if(i == 0)
      Rev_j++;
    else if(i != next){
      i = nextGroup[0] - i;
      nextGroup[0] -= i;
      nextGroup[1] -= i;
      nextGroup[2] -= i;
    }
  }
  else if(!B->blocked){
    if(!B->reserved)
      B->setReversedState(PROCEED);
    else
      B->setReversedState(DANGER);
  }

  if(state[0] != PROCEED){
    ApplyRailState(prev, B, BP, state, prevGroup, j, Dir);
  }

  if(Rev_state[0] != PROCEED){
    ApplyRailState(next, B, BN, Rev_state, nextGroup, Rev_j, Dir^1);
  }
  // else{
  //   bool blocked = false;
  //   for(uint8_t i = 0; i < ABs->next3; i++){
  //     if(BN[i]->blocked){
  //       blocked = true;
  //       break;
  //     }
  //   }

  //   if(!blocked)
  //     B->setState(PROCEED);
  // }
}

void ApplyRailState(uint8_t blocks, Block * B, Block * BL[10], enum Rail_states state[3], uint8_t prevGroup[3], uint8_t j, bool Dir){
  loggerf(INFO, "ApplyRailState -- %02i:%02i  d%i", B->module, B->id, Dir);
  for(uint8_t i = 0; i < blocks; i++){
    if(!BL[i])
      break;
    // Set Dir for state/reversed_state
    bool a, b;
    if(i > 0){
      a = dircmp(BL[i-1], BL[i]);
      b = BL[i-1]->cmpPolarity(BL[i]);
      Dir ^= (a != b);
    }
    else{
      a = dircmp(B, BL[i]);
      b = B->cmpPolarity(BL[i]);
      Dir ^= (a != b);
    }

    loggerf(INFO, "           %2i: %02i:%02i  d%i, dir %i, pol %i", i, BL[i]->module, BL[i]->id, Dir, a, b);

    
    // Apply railstate
    if(BL[i]->blocked)
      break;
    else if(j > 2)
      BL[i]->setState(PROCEED, Dir);
    else if(i < prevGroup[j])
      BL[i]->setState(state[j], Dir);
  
    else if(i >= prevGroup[j]){ // Ready to go to the next group
      // Should not go to next group if
      //  - NOSTOP group
      //  - Station Group
      //  - Same Polarity Group
      loggerf(INFO, " i%i>=pG[j%i] %x %x",i,j, (unsigned long)B->Polarity, (unsigned long)BL[i]->Polarity);
      if(BL[i]->type == STATION && B->type == STATION &&
          ((i  > 0 && BL[i]->station != BL[i-1]->station) || 
           (i == 0 && BL[i]->station != B->station))
        ){ // Other station

        state[1] = RESTRICTED;
        state[2] = CAUTION;
        j++;
      }
      else if(BL[i]->type != NOSTOP && B->Polarity && B->Polarity != BL[i]->Polarity || !B->Polarity)
        j++;

      if(j > 2)
        BL[i]->setState(PROCEED, Dir);
      else
        BL[i]->setState(state[j], Dir);
    }
    else
      BL[i]->setState(PROCEED, Dir);

  }
}

bool train_following(struct blockAlgorithm * ABs, int debug){
  loggerf(TRACE, "Algor_train_following");
  //Unpack AllBlocks
  uint8_t prev = ABs->P->group[3];
  Block ** BP = ABs->P->B;
  Block *  B  = ABs->B;
  Block ** BN = ABs->N->B;
  uint8_t next = ABs->N->group[3];

  // If block is not blocked but still containing a train
  if(B->train){
    if(!B->blocked){
      // Train is lost
      //  If blocks around are also not blocked

      if(prev > 0 && next > 0 && !BN[0]->blocked && !BP[0]->blocked){
        B->setState(UNKNOWN);
        loggerf(WARNING, "%02i%02i LOST Train block %x", B->module, B->id, (unsigned long)B);
      }
      // Detection is lost when the train should be in the block
      else if(B->train == B->expectedTrain && B->train->assigned){
        B->setState(UNKNOWN);
      }
      else{ // Release the block
        Train * T = B->train;
        
        T->move(B);

        if(B->path){
          if(T->assigned && B->path->Exit == B){
            B->path->trainExit(T);
            T->exitPath(B->path);
          }
          else{
            B->path->analyzeTrains();
            T->analyzePaths();
          }
        }

        loggerf(DEBUG, "RESET Train block %x", (unsigned long)B);
        // Units[B->module]->changed |= Unit_Blocks_changed;
      }
    }
    else if(!B->detectionBlocked && B->expectedTrain == 0 && B->train->assigned){
      B->train->move(B);
    }
  }

  // If block has no train but is blocked
  if(B->blocked && B->train == 0){
    // char debugmsg[1000];
    // char * ptr = debugmsg;
    // ptr += sprintf(ptr, "Blocked Block without train");
    // if(prev > 0)
    //   ptr += sprintf(ptr, "\nP: %02i:%02i %c%c%c %6x", BP[0]->module, BP[0]->id, BP[0]->blocked ? 'B':' ', BP[0]->detectionBlocked ? 'D':' ', BP[0]->virtualBlocked ? 'V':' ', BP[0]->train);
    // else
    //   ptr += sprintf(ptr, "\n                   ");
    // if(next > 0)
    //   ptr += sprintf(ptr, "\tN: %02i:%02i %c%c%c %6x", BN[0]->module, BN[0]->id, BN[0]->blocked ? 'B':' ', BN[0]->detectionBlocked ? 'D':' ', BN[0]->virtualBlocked ? 'V':' ', BN[0]->train);
    // else
    //   ptr += sprintf(ptr, "\t                   ");

    // loggerf(INFO, "%s", debugmsg);

    if(B->reserved){
      B->dereserve(B->reservedBy[0]);
    }

    if(B->expectedDetectable){
      loggerf(INFO, "Copy expectedDetectable");

      B->expectedDetectable->stepForward(B);

      // If it is the front of the train
      if(B->train->B == B && B->path && B->path->Entrance == B)
        B->path->trainEnter(B->train);

      // B->train = B->expectedTrain;
      // B->train->moveForward(B);

      // if(next > 0)
      //   BN[0]->expectedTrain = B->train;
      return true;
    }
    else if(B->expectedTrain){
      loggerf(WARNING, "Copy expectedTrain");

      B->train = B->expectedTrain;
      B->train->move(B);
      B->train->moveForward(B);

      // If it is the front of the train
      if(B->train->B == B && B->path && B->path->Entrance == B)
        B->path->trainEnter(B->train);

      if(next > 0)
        BN[0]->expectedTrain = B->train;

      return true;
    }
    // Train moved forward
    else if(prev > 0 && BP[0]->blocked && BP[0]->train){
      loggerf(INFO, "Copy train from previous block");
      // Copy train id from previous block
      Train * T = BP[0]->train;

      if(T->stopped){
        T->setBlock(B);
        
        if(B->path){
          B->path->analyzeTrains();
          T->analyzePaths();
        }
      }
      else{
        T->dir = 0;

        T->initMove(B);

        return true;
      }
      // if(train_link[B->train])
      //   train_link[B->train]->Block = B;
      loggerf(TRACE, "COPY_TRAIN from %02i:%02i to %02i:%02i", BP[0]->module, BP[0]->id, B->module, B->id);
    }
    // Train moved backwards / or sporadic detection at the rear
    else if(next > 0 && BN[0]->blocked && BN[0]->train){
      loggerf(INFO, "Copy train from next block");
      // Copy train id from next block
      Train * T = BN[0]->train;

      // FIXME
      if(T->initialized){
        T->move(B);
        if(B->path)
          B->path->analyzeTrains();
        return true;
      }
      else if(T->stopped){
        T->setBlock(B);
        
        if(B->path){
          B->path->analyzeTrains();
          T->analyzePaths();
        }
      }
      else{
        T->dir = 1;

        T->initMove(B);
        
        if(B->path)
          B->path->analyzeTrains();
        return true;
      }
    }
    else if( ((prev > 0 && (!BP[0]->blocked || (BP[0]->blocked && !BP[0]->train))) || prev == 0) && 
             ((next > 0 && (!BN[0]->blocked || (BN[0]->blocked && !BN[0]->train))) || next == 0) ){
      //NEW TRAIN
      B->train = new Train(B);

      //Create a message for WebSocket
      WS_stc_NewTrain(B->train, B->module, B->id);
    }
  }

  if(B->state == UNKNOWN && B->blocked && B->train){
    loggerf(INFO, "Restoring ghosting train");

    // Remove all ghosted blocks around restored block.
    for(uint8_t i = 0; i < next; i++){
      if(BN[i]->blocked || BN[i]->state != UNKNOWN || BN[i]->train == 0)
        break;
      
      BN[i]->train = 0;
      BN[i]->setState(PROCEED);
    }
    for(uint8_t i = 0; i < prev; i++){
      if(BP[i]->blocked || BP[i]->state != UNKNOWN || BP[i]->train == 0)
        break;
      
      BP[i]->train = 0;
      BP[i]->setState(PROCEED);
    }
  }
  return false;
}

void train_control(Train * T){
  char Debug[100];
  sprintf(Debug, "Algor_train_control RT%2i\n", T->id);

  Block * B = T->B;
  Block ** N = B->Alg.N->B;

  if(!T->assigned)
    return;

  if(!B){
    loggerf(ERROR, "Train %i, %x has no block????", T->id, (unsigned long)T);
    return;
  }

  if(B->Alg.N->group[3] == 0){
    T->changeSpeed(0, B->length);
    return;
  }

  // if(T->B)
  //   loggerf(DEBUG, "%i (%02i:%02i) -> %s (%02i:%02i)", T->id, T->B->module, T->B->id, rail_states_string[N[0]->state], N[0]->module, N[0]->id);
  // else
  //   loggerf(DEBUG, "%i (xx:xx) -> %s (%02i:%02i)", T->id, rail_states_string[N[0]->state], N[0]->module, N[0]->id);

  // Calculate the speed that is obtainable in the current block
  uint16_t AcceleratedSpeed = sqrt((20/0.173625) * B->length + T->speed * T->speed);
  { // Speed cannot exceed maximum speed of the train
  uint16_t tmpSpeed = T->checkMaxSpeed();
  if(AcceleratedSpeed > tmpSpeed)
    AcceleratedSpeed = tmpSpeed;
  }
  bool accelerate = true;
  
  if(T->manual || T->SpeedState == TRAIN_SPEED_CHANGING || T->SpeedState == TRAIN_SPEED_UPDATE)
    accelerate = false;

  // Calculate the distances needed to make a full stop
  uint16_t maxDistance          = SpeedToDistance_A(AcceleratedSpeed, -10);  // Max Deceleration of 10km/h/s = 2.78m/s^2.. km2/h2 / km-1h-1s-1
  uint16_t extraBrakingDistance = maxDistance - SpeedToDistance_A(T->speed, -10);

  // Counter and distance for loop
  uint8_t i = 0;
  uint8_t addI = 1;
  uint16_t length = B->length;

  bool Dir = 0; // 0 = setState
                // 1 = setReversedState

  // Array of brake points
  struct {
    uint16_t speed;
    int16_t BrakingDistance;
    int16_t BrakingOffset;
    uint8_t reason;
  } speeds[12] = {{0,0,0,0}};

  memset(speeds, 0, 12 * (sizeof(uint16_t)*3 + sizeof(uint8_t)) );

  uint8_t RouteBrake = 0;


  if(T->routeStatus == TRAIN_ROUTE_AT_DESTINATION && 
      T->route->routeType == PATHFINDING_ROUTE_STATION){
    speeds[i].speed = 0;
    speeds[i].BrakingDistance = 0.173625 * (T->speed * T->speed) / 20;
    speeds[i].BrakingOffset = length - speeds[i].BrakingDistance;
    speeds[i].reason = TRAIN_SPEED_R_ROUTE;
      
    i++;

    RouteBrake = 2;
  }

  // Fill in all brake points
  while(B->Alg.N->group[3] > i){
    addI = 1;
    
    if(i > 0)
      Dir ^= (dircmp(N[i-1], N[i]) != N[i-1]->cmpPolarity(N[i]));
    else
      Dir ^= (dircmp(B, N[i]) != B->cmpPolarity(N[i]));

    if(N[i]->blocked){
      speeds[i].speed = 0;
      speeds[i].BrakingDistance = length - N[i]->length;
      speeds[i].BrakingOffset = 0;
      speeds[i].reason = TRAIN_SPEED_R_SIGNAL;
      accelerate = false;
      break;
    }
    else{
      uint16_t BlockSpeed = N[i]->getSpeed(Dir);
      speeds[i].speed = BlockSpeed;
      speeds[i].BrakingDistance = 0.173625 * ( (BlockSpeed * BlockSpeed) - (T->speed * T->speed) ) / (2 * -10);
      speeds[i].BrakingOffset = length - speeds[i].BrakingDistance;

      if(N[i]->state < PROCEED)
        speeds[i].reason = TRAIN_SPEED_R_SIGNAL;
      else
        speeds[i].reason = TRAIN_SPEED_R_MAXSPEED;
    }

    if(T->routeStatus != TRAIN_ROUTE_DISABLED && RouteBrake < 2){
      if(T->routeStatus == TRAIN_ROUTE_ENTERED_DESTINATION)
        RouteBrake = 1;

      if(T->route->routeType == PATHFINDING_ROUTE_STATION){
        if(N[i] == T->route->destinationBlocks[0] || N[i] == T->route->destinationBlocks[1]){
          if(RouteBrake){
            speeds[i+1].speed = 0;
            speeds[i+1].BrakingDistance = 0.173625 * (T->speed * T->speed) / 20;
            speeds[i+1].BrakingOffset = N[i]->length + length - speeds[i+1].BrakingDistance;
            speeds[i+1].reason = TRAIN_SPEED_R_ROUTE;
            
            addI = 2;
          }

          RouteBrake++;
        }
      }
    }

    if(length > maxDistance){
      i += addI;
      break;
    }

    length += N[i]->length;
    i += addI;
  }

  // Parameters for acceleration/deceleration:
  struct TrainSpeedEventRequest Request = {.targetSpeed = T->speed, .distance = 0, .reason = 0, .ptr = 0};

  // For each brake point check if it must brake now or if it could be done later.
  for(int8_t j = i - 1; j >= 0; j--){
    // loggerf(INFO, " %i   %3ikm/h %3icm %3icm %1i %c %3icm", j, speeds[j].speed, speeds[j].BrakingOffset, speeds[j].BrakingDistance, speeds[j].reason, accelerate ? '+' : '-', speeds[j].BrakingOffset - extraBrakingDistance);
    if(speeds[j].speed <= Request.targetSpeed){
      if(speeds[j].BrakingOffset < (B->length + 10)){
        accelerate = false;
        Request.targetSpeed = speeds[j].speed;
        Request.distance = speeds[j].BrakingDistance + speeds[j].BrakingOffset;
        Request.reason = speeds[j].reason;
        if(Request.reason == TRAIN_SPEED_R_SIGNAL)
          Request.ptr = (void *)N[j];
      }
      else if(accelerate){
        if(speeds[j].BrakingOffset - extraBrakingDistance < (B->length + 10))
          accelerate = false;
      }
    }
  }

  // If no deceleration is necessary and acceleration is allowed
  if(accelerate){
    Request.targetSpeed = AcceleratedSpeed;
    Request.distance = B->length;
    Request.reason = TRAIN_SPEED_R_MAXSPEED;
  }

  T->changeSpeed(Request);
}


void Connect_Rails(){
  
  SYS->LC.connectors = Algorithm::find_connectors();
  BlockConnectors * connectors = &SYS->LC.connectors;
  uint16_t maxConnectors = connectors->size();

  loggerf(INFO, "Have %i connectors", connectors->size());

  uint16_t msgID = -1;

  msgID = WS_stc_ScanStatus(msgID, 0, maxConnectors);

  while(SYS->LC.state == Module_LC_Connecting && !SYS->stop && SYS->modules_linked == 0){
  	// continue;
    // FIXME sem_wait(&AlgorQueueNoEmpty); 
    Block * B = AlQueue.getWait();
    while(B != 0){
      B = AlQueue.get();
    }

    processMutex.lock();
    
    if(uint8_t * findResult = Algorithm::find_connectable(connectors)){
      Algorithm::connect_connectors(connectors, findResult);

      WS_stc_ScanStatus(msgID, maxConnectors - connectors->size(), maxConnectors);
    }

    if(connectors->size() == 0){
      processMutex.unlock();
      break;
    }

    //IF ALL JOINED
    //BREAK

    for(uint8_t j = 0; j < SwManager->Units.size; j++){
      Unit * U = Units(j);
      if(!U)
        continue;

      U->block_state_changed = 1;

      for(uint8_t k = 0; k < U->block_len; k++){
        if(!U->B[k])
          continue;

        if(U->B[k]->blocked)
          U->B[k]->state = BLOCKED;
        else
          U->B[k]->state = PROCEED;
      }
    }

    processMutex.unlock();

    //Notify clients
    WS_stc_trackUpdate(0);

    usleep(100);
  }

  if(SYS->LC.state == Module_LC_Connecting){
    auto s = BlockConnectorSetup();
    s.save();
  }

  WS_stc_Track_Layout(0);
  SYS->modules_linked = 1;
}


}; // namespace Algorithm
