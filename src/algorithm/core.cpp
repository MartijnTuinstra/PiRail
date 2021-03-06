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
#include "switchboard/msswitch.h"
#include "switchboard/signals.h"
#include "switchboard/station.h"
#include "switchboard/blockconnector.h"
#include "train.h"
#include "flags.h"

#include "uart/uart.h"
#include "websocket/stc.h"

#include "sim.h"

#include "submodule.h"


namespace Algorithm {

std::mutex processMutex;

using namespace switchboard;

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

  if(!B->blocked && B->state == BLOCKED){
    if(B->Alg.next > 0 && B->Alg.N[0]->blocked){
      B->Alg.N[0]->algorchanged = 1;
      AlQueue.put(B->Alg.N[0]);
    }
    else if(B->Alg.prev > 0 && B->Alg.P[0]->blocked){
      B->Alg.P[0]->algorchanged = 1;
      AlQueue.put(B->Alg.P[0]);
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


void Set_Changed(Algor_Blocks * ABs){
  loggerf(TRACE, "Algor_Set_Changed");
  //Scroll through all the pointers of allblocks
  uint8_t prev = ABs->prev;
  uint8_t next = ABs->next;

  for(int i = 0; i < prev; i++){
    if(!ABs->P[i])
      continue;

    if(ABs->P[i] == ABs->B)
      continue;

    ABs->P[i]->algorchanged = 1;
    ABs->P[i]->IOchanged = 1;
    ABs->P[i]->AlgorClear();

    if(!ABs->P[i]->blocked)
      ABs->P[i]->setState(PROCEED);
  }
  for(int i = 0; i < next; i++){
    if(!ABs->N[i])
      continue;

    if(ABs->N[i] == ABs->B)
      continue;

    ABs->N[i]->algorchanged = 1;
    ABs->N[i]->IOchanged = 1;
    ABs->N[i]->AlgorClear();

    if(!ABs->N[i]->blocked)
      ABs->N[i]->setState(PROCEED);
  }

  if(ABs->B){
    ABs->B->algorchanged = 1;
    ABs->B->IOchanged = 1;
    ABs->B->setState(PROCEED);
  }
}

int Switch_to_rail(Block ** B, void * Sw, enum link_types type, uint8_t counter){
  struct rail_link next;

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

  Algor_Blocks * ABs = &B->Alg;

  char blockstates[10] = "BDRC rsU";

  for(int i = 7; i >= 0; i--){
    if(ABs->prev <= i){
      ptr += sprintf(ptr, "       ");
      continue;
    }

    if(ABs->prev > 8 && i == 7){
      ptr += sprintf(ptr, "<<<-%2d ", ABs->prev);
      continue;
    }

    if(ABs->P[i]){
      char state = blockstates[ABs->P[i]->state];
      if(ABs->P[i]->virtualBlocked && !ABs->P[i]->detectionBlocked)
        state = 'V';

      ptr += sprintf(ptr, "%02i:%02i%c%c", ABs->P[i]->module, ABs->P[i]->id, state,
                                           (i == ABs->prev1 || i == ABs->prev2 || i == ABs->prev3) ? '|' : ' ');
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
    if(ABs->next <= i){
      break;
    }

    if(ABs->next > 8 && i == 7){
      ptr += sprintf(ptr, "%-2d->>>", ABs->next);
      break;
    }

    if(ABs->N[i]){
      char state = blockstates[ABs->N[i]->state];
      if(ABs->N[i]->virtualBlocked && !ABs->N[i]->detectionBlocked)
        state = 'V';

      ptr += sprintf(ptr, "%02i:%02i%c ", ABs->N[i]->module, ABs->N[i]->id, state);
    }
    else
      ptr += sprintf(ptr, "------ ");
  }
  ptr[0] = 0;

  loggerf((enum logging_levels)debug, "%s", output);
}

void Switch_Checker(Algor_Blocks * ABs, int debug){
  //Unpack AllBlocks
  //Algor_Block BPPP = *AllBlocks.BPPP;
  //Algor_Block BPP  = *AllBlocks.BPP;
  //Algor_Block BP   = *AllBlocks.BP;
  Block * B = ABs->B;
  Block **N = ABs->N;
  uint8_t next = ABs->next;
  //Block BNNN = *AllBlocks.BNNN;

  if(!B->blocked || (B->train && (B->train->stopped || B->train->B != B)))
    return;

  RailTrain * T = B->train;
  Block * tB;
  uint8_t routeStatus = T->routeStatus;
  int16_t distance = SpeedToDistance_A(T->speed, -10) + B->length;

  if(routeStatus == RAILTRAIN_ROUTE_AT_DESTINATION)
    return;

  for(uint8_t i = 0; i < 5; i++){
    if(i > next)
      break;

    if(i == 0) tB = B;
    else tB = N[i-1];

    if(tB->type != NOSTOP)
      distance -= tB->length;

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

      if(routeStatus == RAILTRAIN_ROUTE_AT_DESTINATION){
        loggerf(DEBUG, "routeBreak");
        break;
      }
    }

    uint8_t flags = 0;
    if     (i == 0) flags = NEXT;
    else if(i == 1) flags = dircmp(B, tB)      ? NEXT : PREV;
    else            flags = dircmp(N[i-2], tB) ? NEXT : PREV;

    struct rail_link * link = tB->NextLink(flags);
    loggerf(DEBUG, "Switch_Checker scan block (%2i:%2i) f:%x, d:%i, %i - %i", tB->module, tB->id, flags, tB->dir, link->type, routeStatus);

    if(tB->type == NOSTOP){
      tB = (i > 2) ? N[i-2] : B;
    }

    if((i >  0 && link->type == RAIL_LINK_S) || (link->type >= RAIL_LINK_s && link->type <= RAIL_LINK_MB_inside)){
      if(SwitchSolver::solve(T, B, tB, *link, NEXT | FL_SWITCH_CARE)){ // Returns true if path is changed
        return;
      }
    }
  }
}


void rail_state(Algor_Blocks * ABs, int debug){
  loggerf(TRACE, "Algor_rail_state %02d:%02d", ABs->B->module, ABs->B->id);
  //Unpack ABs
  // uint8_t prev  = ABs->prev;
  Block ** BP   = ABs->P;
  Block *  B    = ABs->B;
  Block ** BN   = ABs->N;
  // uint8_t next  = ABs->next;
  
  bool Dir = 0; // 0 = setState
                // 1 = setReversedState
  
  enum Rail_states state[3]     = {PROCEED, PROCEED, PROCEED};
  enum Rail_states Rev_state[3] = {PROCEED, PROCEED, PROCEED};
  uint8_t prevGroup[3] = {ABs->prev1, ABs->prev2, ABs->prev3};
  uint8_t nextGroup[3] = {ABs->next1, ABs->next2, ABs->next3};
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
  else if(B->type == NOSTOP && ABs->prev
           && ((ABs->next && B->Alg.N[0]->type != NOSTOP) || ABs->next == 0)
           && (B->getNextState() == DANGER || B->switchWrongState || B->switchWrongFeedback)
          ){
    B->setState(DANGER);
    state[0] = DANGER;
    state[1] = CAUTION;

    uint8_t i = 0;
    while(BP[i]->type == NOSTOP && i++ < ABs->prev);

    if(i == 0)
      j++;
    else if(i != ABs->prev){
      i = prevGroup[0] - i;
      prevGroup[0] -= i;
      prevGroup[1] -= i;
      prevGroup[2] -= i;
    }
  }
  else if(ABs->next == 0){
    B->setState(CAUTION);

    state[0] = CAUTION;
  }

  if(B->type == NOSTOP && ABs->next && ABs->prev && B->Alg.P[0]->type != NOSTOP && B->getPrevState() == DANGER){
    B->setReversedState(DANGER);
    Rev_state[0] = DANGER;
    Rev_state[1] = CAUTION;

    uint8_t i = 0;
    while(BN[i]->type == NOSTOP && i++ < ABs->next);

    if(i == 0)
      Rev_j++;
    else if(i != ABs->next){
      i = nextGroup[0] - i;
      nextGroup[0] -= i;
      nextGroup[1] -= i;
      nextGroup[2] -= i;
    }
  }
  else if(!B->blocked){
    if(!B->reserved && !B->switchReserved)
      B->setReversedState(PROCEED);
    else
      B->setReversedState(DANGER);
  }

  if(state[0] != PROCEED){
    ApplyRailState(ABs->prev, B, BP, state, prevGroup, j, Dir);
  }

  if(Rev_state[0] != PROCEED){
    ApplyRailState(ABs->next, B, BN, Rev_state, nextGroup, Rev_j, Dir^1);
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
  for(uint8_t i = 0; i < blocks; i++){
    if(!BL[i])
      break;

    // Set Dir for state/reversed_state
    if(i > 0)
      Dir ^= !dircmp(BL[i-1], BL[i]);
    else
      Dir ^= !dircmp(B, BL[i]);
    
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
      if(BL[i]->type == STATION && B->type == STATION &&
          ((i  > 0 && BL[i]->station != BL[i-1]->station) || 
           (i == 0 && BL[i]->station != B->station))
        ){ // Other station

        state[1] = RESTRICTED;
        state[2] = CAUTION;
        j++;
      }
      else if(BL[i]->type != NOSTOP)
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

void train_following(Algor_Blocks * ABs, int debug){
  loggerf(TRACE, "Algor_train_following");
  //Unpack AllBlocks
  uint8_t prev = ABs->prev;
  Block ** BP = ABs->P;
  Block *  B  = ABs->B;
  Block ** BN = ABs->N;
  uint8_t next = ABs->next;

  // If block is not blocked but still containing a train
  if(B->train){
    if(!B->blocked){
      // Train is lost
      //  If blocks around are also not blocked

      if(prev > 0 && next > 0 && !BN[0]->blocked && !BP[0]->blocked){
        B->setState(UNKNOWN);
        loggerf(WARNING, "%02i%02i LOST Train block %x", B->module, B->id, (unsigned int)B);
      }
      // Detection is lost when the train should be in the block
      else if(B->train == B->expectedTrain && B->train->assigned){
        B->setState(UNKNOWN);
      }
      else{ // Release the block
        B->train->moveForwardFree(B);
        B->train = 0;

        loggerf(DEBUG, "RESET Train block %x", (unsigned int)B);
        // Units[B->module]->changed |= Unit_Blocks_changed;
      }
    }
    else if(!B->detectionBlocked && B->expectedTrain == 0 && B->train->assigned){
      B->train->moveForwardFree(B);
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

    if(B->reserved || B->switchReserved){
      B->dereserve(B->reservedBy[0]);
    }

    if(B->expectedTrain){
      // loggerf(INFO, "Copy expectedTrain");

      B->train = B->expectedTrain;
      B->train->moveForward(B);

      if(next > 0)
        BN[0]->expectedTrain = B->train;
    }
    // Train moved forward
    else if(prev > 0 && BP[0]->blocked && BP[0]->train){
      loggerf(INFO, "Copy train from previous block");
      // Copy train id from previous block
      RailTrain * T = BP[0]->train;

      if(T->stopped)
        T->setBlock(B);
      else{
        T->dir = 0;

        T->initMoveForward(B);
      }
      // if(train_link[B->train])
      //   train_link[B->train]->Block = B;
      loggerf(TRACE, "COPY_TRAIN from %02i:%02i to %02i:%02i", BP[0]->module, BP[0]->id, B->module, B->id);
    }
    // Train moved backwards
    else if(next > 0 && BN[0]->blocked && BN[0]->train){
      loggerf(INFO, "Copy train from next block");
      // Copy train id from next block
      RailTrain * T = BN[0]->train;

      if(T->stopped)
        T->setBlock(B);
      // else if(T->directionKnown)
      //   T->moveForward(B);
      else{
        T->dir = 1;

        T->initMoveForward(B);
        //FIXME T->reverseZ21(); // Set Train in right direction
      }
    }
    else if( ((prev > 0 && !BP[0]->blocked) || prev == 0) && ((next > 0 && !BN[0]->blocked) || next == 0) ){
      //NEW TRAIN
      B->train = new RailTrain(B);

      //Create a message for WebSocket
      WS_stc_NewTrain(B->train, B->module, B->id);
    }
  }
}

void train_control(RailTrain * T){
  char Debug[100];
  sprintf(Debug, "Algor_train_control RT%2i\n", T->id);

  Block * B = T->B;
  Block ** N = B->Alg.N;

  if(!T->assigned)
    return;

  if(B->Alg.next == 0){
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
  
  if(T->manual || T->changing_speed == RAILTRAIN_SPEED_T_CHANGING || T->changing_speed == RAILTRAIN_SPEED_T_UPDATE)
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


  if(T->routeStatus == RAILTRAIN_ROUTE_AT_DESTINATION && 
      T->route->routeType == PATHFINDING_ROUTE_STATION){
    speeds[i].speed = 0;
    speeds[i].BrakingDistance = 0.173625 * (T->speed * T->speed) / 20;
    speeds[i].BrakingOffset = length - speeds[i].BrakingDistance;
    speeds[i].reason = RAILTRAIN_SPEED_R_ROUTE;
      
    i++;

    RouteBrake = 2;
  }

  // Fill in all brake points
  while(B->Alg.next > i){
    addI = 1;
    
    if(i > 0)
      Dir ^= !dircmp(N[i-1], N[i]);
    else
      Dir ^= !dircmp(B, N[i]);

    if(N[i]->blocked){
      speeds[i].speed = 0;
      speeds[i].BrakingDistance = length - N[i]->length;
      speeds[i].BrakingOffset = 0;
      speeds[i].reason = RAILTRAIN_SPEED_R_SIGNAL;
      accelerate = false;
      break;
    }
    else{
      uint16_t BlockSpeed = N[i]->getSpeed(Dir);
      speeds[i].speed = BlockSpeed;
      speeds[i].BrakingDistance = 0.173625 * ( (BlockSpeed * BlockSpeed) - (T->speed * T->speed) ) / (2 * -10);
      speeds[i].BrakingOffset = length - speeds[i].BrakingDistance;

      if(N[i]->state < PROCEED)
        speeds[i].reason = RAILTRAIN_SPEED_R_SIGNAL;
      else
        speeds[i].reason = RAILTRAIN_SPEED_R_MAXSPEED;
    }

    if(T->routeStatus != RAILTRAIN_ROUTE_DISABLED && RouteBrake < 2){
      if(T->routeStatus == RAILTRAIN_ROUTE_ENTERED_DESTINATION)
        RouteBrake = 1;

      if(T->route->routeType == PATHFINDING_ROUTE_STATION){
        if(N[i] == T->route->destinationBlocks[0] || N[i] == T->route->destinationBlocks[1]){
          if(RouteBrake){
            speeds[i+1].speed = 0;
            speeds[i+1].BrakingDistance = 0.173625 * (T->speed * T->speed) / 20;
            speeds[i+1].BrakingOffset = N[i]->length + length - speeds[i+1].BrakingDistance;
            speeds[i+1].reason = RAILTRAIN_SPEED_R_ROUTE;
            
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
        if(Request.reason == RAILTRAIN_SPEED_R_SIGNAL)
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
    Request.reason = RAILTRAIN_SPEED_R_MAXSPEED;
  }

  T->changeSpeed(Request);
}


void Connect_Rails(){
  
  auto connectors = Algorithm::find_connectors();
  uint16_t maxConnectors = connectors.size();

  loggerf(INFO, "Have %i connectors", connectors.size());

  uint16_t msgID = -1;

  msgID = WS_stc_ScanStatus(msgID, 0, maxConnectors);

  while(SYS->LC.state == Module_LC_Connecting && !SYS->stop && SYS->modules_linked == 0){
  	// continue;
    // FIXME sem_wait(&AlgorQueueNoEmpty); 
    Block * B = AlQueue.getWait();
    while(B != 0){
      B = AlQueue.get();
    }
    
    if(uint8_t * findResult = Algorithm::find_connectable(&connectors)){
      Algorithm::connect_connectors(&connectors, findResult);

      WS_stc_ScanStatus(msgID, maxConnectors - connectors.size(), maxConnectors);
    }

    if(connectors.size() == 0)
      break;

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

    // FIXME mutex_lock(&algor_mutex, "Algor Mutex");
    //Notify clients
    WS_stc_trackUpdate(0);

    // FIXME mutex_unlock(&algor_mutex, "Algor Mutex");

    usleep(100);
  }

  auto s = BlockConnectorSetup();
  s.save();

  SYS->modules_linked = 1;
}


}; // namespace Algorithm
