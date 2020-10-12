#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

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
#include "switchboard/msswitch.h"
#include "switchboard/signals.h"
#include "switchboard/station.h"
#include "switchboard/blockconnector.h"
#include "train.h"
#include "train.h"

#include "modules.h"
#include "com.h"
#include "websocket/stc.h"

#include "sim.h"

#include "submodule.h"


namespace Algorithm {

using namespace switchboard;

void process(Block * B, int flags){
  loggerf(TRACE, "process %02i:%02i, flags %x", B->module, B->id, flags);

  if((B->IOchanged == 0 && B->algorchanged == 0) && (flags & _FORCE) == 0){
    loggerf(TRACE, "No changes");
    return;
  }

  if(flags & _LOCK){
    loggerf(WARNING, "LOCK");
    // lock_Algor_process(); // FIXME
  }

  flags |= _DEBUG;

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
    print_block_debug(B);
  // }

  //Follow the train arround the layout
  train_following(&B->Alg, flags);
  if (B->recalculate){
    loggerf(INFO, "Block Train ReProcess");
    if(flags & _LOCK){
      loggerf(WARNING, "UNLOCK");
      // unlock_Algor_process(); // FIXME
    }
    return;
  }

  //Set oncomming switch to correct state
  Switch_Checker(&B->Alg, flags);
  if (B->recalculate){
    loggerf(DEBUG, "Block Switch ReProcess");
    // if(flags & _LOCK)
      // unlock_Algor_process(); // FIXME
    return;
  }
  
  // Print all found blocks
  // if(flags & _DEBUG)

  // Station Stating
  if(B->station){
    if(B->blocked && B->state != BLOCKED)
      B->station->occupy(B->train);
    else if(!B->blocked && B->state == BLOCKED)
      B->station->release();
  }

  //Apply block stating
  rail_state(&B->Alg, flags);

  //Train Control
  // Apply train algorithm only if there is a train on the block and is the front of the train
  if(B->train && B == B->train->B){
    if(B->Alg.N[0]){
      train_control(&B->Alg, flags);
    }
    else{
      // Stop train no next block!!
      loggerf(INFO, "EMEG BRAKE, NO B_LOCK");
      B->train->changeSpeed(0, IMMEDIATE_SPEED);
    }
  }

  if(flags & _LOCK){
    loggerf(WARNING, "UNLOCK");
    // unlock_Algor_process(); // FIXME
  }
  loggerf(TRACE, "Done");
}


void Set_Changed(Algor_Blocks * ABs){
  loggerf(TRACE, "Algor_Set_Changed");
  //Scroll through all the pointers of allblocks
  for(int i = 0; i < ABs->prev; i++){
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
  for(int i = 0; i < ABs->next; i++){
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
      if(ABs->P[i]->virtualblocked && !ABs->P[i]->detectionblocked)
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
  if(B->detectionblocked)
    ptr += sprintf(ptr, "b");
  else if(B->virtualblocked)
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
      if(ABs->N[i]->virtualblocked && !ABs->N[i]->detectionblocked)
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

  if(!B->blocked || (B->train && B->train->stopped))
    return;

  RailTrain * T = B->train;
  Block * tB;

  for(uint8_t i = 0; i < 4; i++){
    if(i > next)
      break;

    if(i == 0) tB = B;
    else tB = N[i-1];

    if(tB->blocked && tB != B)
      break;

    struct rail_link * link = tB->NextLink(NEXT);
    loggerf(DEBUG, "Switch_Checker scan block (%i,%i) - %i", tB->module, tB->id, link->type);

    if((i >  0 && link->type == RAIL_LINK_S) || (link->type >= RAIL_LINK_s && link->type <= RAIL_LINK_MB_inside)){
      SwitchSolver::solve(T, B, tB, *link, NEXT | SWITCH_CARE);

      return;
    }
      // if(B->train && B->train->route == 1){
        // struct pathinstruction temp;
        // if(!Next_check_Switch_Route(tmp, *link, NEXT, B->train->instructions, &temp)){
        //   if(set_switch_route(tmp, *link, NEXT | SWITCH_CARE, &temp)){
        //     B->changed |= IO_Changed; // Recalculate
        //     free_pathinstruction(&temp);
        //     return;
        //   }
        //   else{
        //     loggerf(WARNING, "Stop Train on Route");
        //   }
        // }
        // else{
        //   loggerf(INFO, "Path applied");
        //   if(i == 0){
        //     //clear instructions from train
        //     remove_pathinstructions(*link, B->train->instructions);
        //   }
        // }
        // free_pathinstructions(&temp);
      // }
      // else
      // if (!Switch_Check_Path(tB, *link, NEXT | SWITCH_CARE)) {
      //   loggerf(INFO, "Switch next path!! %02i:%02i", tB->module, tB->id);

      //   if(Switch_Set_Path(tB, *link, NEXT | SWITCH_CARE)) {
      //     loggerf(INFO, "Switch set path!! %02i:%02i", tB->module, tB->id);
      //     tB->IOchanged = 1; // Recalculate
      //     tB->algorchanged = 1; // Recalculate
      //     Switch_Reserve_Path(tB, *link, NEXT | SWITCH_CARE);
      //     return;
      //   }
      //   else{
      //     loggerf(WARNING, "Failed switch set path");
      //   }
      // }
      // else if(((link->type == RAIL_LINK_S  || link->type == RAIL_LINK_s ) &&   (link->p.Sw)->Detection &&   (link->p.Sw)->Detection->state != RESERVED_SWITCH) || 
      //         ((link->type == RAIL_LINK_MA || link->type == RAIL_LINK_MB) && (link->p.MSSw)->Detection && (link->p.MSSw)->Detection->state != RESERVED_SWITCH)){
      //   loggerf(WARNING, "reserve_switch_path");
      //   Switch_Reserve_Path(tB, *link, NEXT | SWITCH_CARE);
      // }
  }

  // //Check Next 1
  // if(B->blocked && (((BN.switches || BNN.switches) && BN.blocks > 0) || (BN.blocks == 0 || BNN.blocks == 0))){
  //   Block * tmp;
  //   for(int i = 0; i <= BN.blocks; i++){
  //     if(i == 0){
  //       tmp = B;
  //     }
  //     else{
  //       tmp = BN.B[i - 1];
  //     }

  //     // loggerf(DEBUG, "checking block next link %i:%i", tmp->module, tmp->id);
  //     if (tmp->type == SPECIAL) {
  //       continue;
  //     }
  //     if (tmp->blocked){
  //       continue;
  //     }

      
  //   }
  // }
}


void rail_state(Algor_Blocks * ABs, int debug){
  loggerf(TRACE, "Algor_rail_state %02d:%02d", ABs->B->module, ABs->B->id);
  //Unpack ABs
  // uint8_t prev  = ABs->prev;
  Block ** BP   = ABs->P;
  Block *  B    = ABs->B;
  // Block ** BN   = ABs->N;
  // uint8_t next  = ABs->next;

  if(!B->blocked){
    if(!B->reserved){
      B->setReversedState(PROCEED);
    }
    else{
      B->setReversedState(DANGER);
    }
  }
  else{
    B->setState(BLOCKED);
  }

  if(B->blocked){
    // Reset Reserved
    B->reservedBy = 0;

    // Set states
    enum Rail_states prev1state = DANGER;
    enum Rail_states prev2state = CAUTION;
    enum Rail_states prev3state = PROCEED;

    if(B->type == STATION && B->station->type >= STATION_YARD)
      prev1state = RESTRICTED;

    for(uint8_t i = 0; i < ABs->prev; i++){
      if(!BP[i])
        break;
      else if(BP[i]->blocked)
        break;
      else if(i < ABs->prev1)
        BP[i]->setState(prev1state);
    
      else if(i == ABs->prev1){
        if(BP[i]->type == STATION && B->type == STATION){
          if(BP[i]->station == B->station){
            prev2state = DANGER;
            prev3state = CAUTION;
          }
          else if(i > 0 && BP[i-1]->type == NOSTOP){
            prev2state = RESTRICTED;
            prev3state = CAUTION;
          }
          else{
            prev2state = DANGER;
            prev3state = CAUTION;
          }
        }

        BP[i]->setState(prev2state);
      }
      else if(i < ABs->prev2){
        BP[i]->setState(prev2state);
      }
    
      else if(i < ABs->prev3)
        BP[i]->setState(prev3state);

      else
        BP[i]->setState(PROCEED);

    }
  }
  else if(ABs->next == 0 || B->switchWrongState || B->switchWrongFeedback){
    if(B->type != NOSTOP){
      B->setState(CAUTION);


      if(B->type == STATION){
        for(uint8_t i = 0; i < ABs->prev1; i++){
          if(BP[i]->blocked)
            break;

          BP[i]->setState(CAUTION);
        }
      }
    }
    else{
      B->setState(DANGER);

      uint8_t maxblocks = ABs->prev2;
      enum Rail_states nostopper = DANGER;

      for(uint8_t i = 0; i < maxblocks; i++){
        if(BP[i]->blocked)
          break;

        if(BP[i]->type != NOSTOP){
          BP[i]->setState(CAUTION);

          nostopper = CAUTION;

          if(i == 0)
            maxblocks = ABs->prev1;
        }
        else
          BP[i]->setState(nostopper);
      }
    }
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

void train_following(Algor_Blocks * ABs, int debug){
  loggerf(TRACE, "Algor_train_following");
  //Unpack AllBlocks
  uint8_t prev = ABs->prev;
  Block ** BP = ABs->P;
  Block *  B  = ABs->B;
  Block ** BN = ABs->N;
  uint8_t next = ABs->next;

  // If block is not blocked but still containing a train
  if(!B->blocked && B->train != 0){
    // Train is lost
    //  If blocks around are also not blocked
    if(prev > 0 && next > 0 && !BN[0]->blocked && !BP[0]->blocked){
      B->setState(UNKNOWN);
      loggerf(WARNING, "%02i%02i LOST Train block %x", B->module, B->id, (unsigned int)B);
    }
    else{ // Release the block
      B->train->releaseBlock(B);
      B->train = 0;

      loggerf(DEBUG, "RESET Train block %x", (unsigned int)B);
      // Units[B->module]->changed |= Unit_Blocks_changed;
    }
  }

  // If block has no train but is blocked
  if(B->blocked && B->train == 0){
    // char debugmsg[1000];
    // char * ptr = debugmsg;
    // ptr += sprintf(ptr, "Blocked Block without train");
    // if(prev > 0)
    //   ptr += sprintf(ptr, "\nP: %02i:%02i %c%c%c %6x", BP[0]->module, BP[0]->id, BP[0]->blocked ? 'B':' ', BP[0]->detectionblocked ? 'D':' ', BP[0]->virtualblocked ? 'V':' ', BP[0]->train);
    // else
    //   ptr += sprintf(ptr, "\n                   ");
    // if(next > 0)
    //   ptr += sprintf(ptr, "\tN: %02i:%02i %c%c%c %6x", BN[0]->module, BN[0]->id, BN[0]->blocked ? 'B':' ', BN[0]->detectionblocked ? 'D':' ', BN[0]->virtualblocked ? 'V':' ', BN[0]->train);
    // else
    //   ptr += sprintf(ptr, "\t                   ");

    // loggerf(INFO, "%s", debugmsg);
    // Train moved forward
    if(prev > 0 && BP[0]->blocked && BP[0]->train){
      // Copy train id from previous block
      B->train = BP[0]->train;
      B->train->moveForward(B);

      if(B->reserved){
        B->reserved--;
        if(B->reserved == 0){
          B->setState(PROCEED);
          B->setReversedState(PROCEED);
        }
      }
      // if(train_link[B->train])
      //   train_link[B->train]->Block = B;
      loggerf(TRACE, "COPY_TRAIN from %02i:%02i to %02i:%02i", BP[0]->module, BP[0]->id, B->module, B->id);
    }
    // Train moved backwards
    else if(false){}
    else if( ((prev > 0 && !BP[0]->blocked) || prev == 0) && ((next > 0 && !BN[0]->blocked) || next == 0) ){
      //NEW TRAIN
      // find a new follow id
      // loggerf(ERROR, "FOLLOW ID INCREMENT, bTrain");
      B->train = new RailTrain(B);

      if(B->reserved){
        B->reserved--;
        if(B->reserved == 0){
          B->setState(PROCEED);
          B->setReversedState(PROCEED);
        }
      }

      //Create a message for WebSocket
      WS_stc_NewTrain(B->train, B->module, B->id);

      loggerf(INFO, "NEW_TRAIN %x", (unsigned int)B->train);
    }
  }

  // if(!B->blocked && B->train != 0){

  // }
  // // else if(B->blocked && B->train == 0){
  // //   Units[B->module]->changed |= Unit_Blocks_changed;
  // // }
  // // else if(B->blocked && B->train != 0 && train_link[B->train] && !train_link[B->train]->Block){
  // //   // Set block of train
  // //   train_link[B->train]->Block = B;
  // //   if(debug) printf("SET_BLOCK");
  // // }

  // // else if(B->blocked && BNN.blocks > 0 && !BN->blocked && !BNN.blocked){

  // // }

  // // Reverse track if block ahead is allready blocked but current is not blocked
  // if(B->blocked && next > 0){
  //   //If only current and next blocks are occupied
  //   // Reverse immediate block
  //   if(((prev > 0 && !BP[0]->blocked) || prev == 0) && BN[0]->blocked && BN[0]->train && !B->train){
  //     //REVERSED
  //     loggerf(WARNING, "REVERSE BLOCK %02i:%02i", B->module, B->id);
  //     // Block_Reverse(ABs);
  //     B->reverse();

  //     if(!dircmp(B, BP[0])){
  //       // B->IOchanged = 1;
  //     // }
  //     // else{
  //       for(uint8_t i = 0; i < prev; i++){
  //         if(!BP[i])
  //           continue;
  //         if(BP[i]->blocked){
  //           loggerf(INFO, "%02i:%02i", BP[i]->module, BP[i]->id);
  //         }
  //         else
  //           continue;


  //         // Block_Reverse(&BP[i]->Alg);
  //         BP[i]->reverse();
  //         // BN[i]->IOchanged;
  //       }
  //     }

  //     Block_Reverse_To_Next_Switch(B);
  //     loggerf(INFO, "Done");
  //   }

  //   for(uint8_t i = 0; i < 4; i++){
  //     if(next > i+1 && BN[i]->state == RESERVED_SWITCH){
  //       loggerf(ERROR, "Blocked and next is switch lane %x", (unsigned int)B);
  //       Block * tB = BN[i+1];

  //       if(!dircmp(B, BN[i])){
  //         loggerf(WARNING, "REVERSE NEXT SWITCH BLOCK %02i:%02i", BN[i]->module, BN[i]->id);
  //         BN[i]->reverse();
  //       }

  //       if(tB->state != RESERVED_SWITCH){
  //         if(!dircmp(B, tB)){
  //           loggerf(WARNING, "REVERSE BLOCK %02i:%02i after switchlane", tB->module, tB->id);
  //           tB->reverse();
  //           tB->reserve();
  //           // Block_Reverse(&tB->Alg);
  //           // Block_reserve(tB);
  //           //void Block_Reverse(B);
  //           Block_Reverse_To_Next_Switch(tB);
  //         }
  //         else if(tB->state != RESERVED){
  //           loggerf(WARNING, "RESERVE BLOCK %02i:%02i until switchlane", tB->module, tB->id);
  //           //reserve untill next switchlane
  //           tB->reserve();
  //           // Block_reserve(tB);
  //           Reserve_To_Next_Switch(tB);
  //         }
  //       }
  //       else if(!dircmp(B, tB)){
  //         loggerf(WARNING, "REVERSE SWITCH BLOCK %02i:%02i", tB->module, tB->id);
  //         tB->reverse();
  //         // Block_Reverse(&tB->Alg);
  //         continue;
  //       }

	 //      break;
  //     }
  //   }
  // }

  // // Split train: If current block is unoccupied and surrounding are occupied and have the same train pointer
  // else if(next > 0 && prev > 0 && BN[0]->blocked && BP[0]->blocked && !B->blocked && BN[0]->train == BP[0]->train){
  //   //A train has split
  //   Block * tN = BN[0];
  //   Block * tP = BP[0];
  //   WS_stc_TrainSplit(BN[0]->train, tP->module,tP->id,tN->module,tN->id);

  //   loggerf(INFO, "SPLIT_TRAIN");
  // }

  // // If only current and prev blocks are occupied
  // // and if next block is reversed
  // //int dircmp_algor(Algor_Block * A, Algor_Block * B)
  // if(prev > 0 && next > 0 && B->blocked && BP[0]->blocked && !BN[0]->blocked && !dircmp(B, BN[0])) {
  //   //Reversed ahead
  //   loggerf(INFO, "%x Reversed ahead (%02i:%02i)", (unsigned int)B, BN[0]->module, BN[0]->id);
  //   // Block_Reverse(&BN[0]->Alg);
  //   BN[0]->reverse();
  //   // Block_Reverse_To_Next_Switch(BN.B[0]);
  // }

}

void train_control(Algor_Blocks * ABs, int debug){
  char Debug[100];
  sprintf(Debug, "Algor_train_control %2i:%2i\n", ABs->B->module, ABs->B->id);
  //Unpack AllBlocks
  Block *  B = ABs->B;
  Block ** N = ABs->N;

  RailTrain * T = B->train;

  if(ABs->next == 0){
    loggerf(WARNING, "Stop train, end of line");
    return;
  }

  if(!T){
    loggerf(ERROR, "No train");
    return;
  }

  if(T->B)
    loggerf(DEBUG, "%i (%02i:%02i) -> %s (%02i:%02i)", T->link_id, T->B->module, T->B->id, rail_states_string[N[0]->state], N[0]->module, N[0]->id);
  else
    loggerf(DEBUG, "%i (xx:xx) -> %s (%02i:%02i)", T->link_id, rail_states_string[N[0]->state], N[0]->module, N[0]->id);

  if(N[0]->blocked){
    loggerf(WARNING, "%sTrain Next block Blocked", Debug);
    T->changeSpeed(0, IMMEDIATE_SPEED);
  }
  else if(N[0]->state == DANGER){
    loggerf(WARNING, "%sTrain Next block Blocked %02i:%02i", Debug, N[0]->module, N[0]->id);
    T->changeSpeed(0, GRADUAL_FAST_SPEED);
  }
  else if(N[0]->state == RESTRICTED){
    loggerf(WARNING, "%sTrain Next block Restricted %02i:%02i", Debug, N[0]->module, N[0]->id);
    T->changeSpeed(10, GRADUAL_FAST_SPEED);
  }
  else if(N[0]->state == CAUTION){
    if(T->speed > CAUTION_SPEED){
      loggerf(WARNING, "%sTrain Next block Caution %02i:%02i", Debug, N[0]->module, N[0]->id);
      T->changeSpeed(CAUTION_SPEED, GRADUAL_FAST_SPEED);
    }
  }
  else if(!T->manual && (T->target_speed > N[0]->max_speed || T->speed > N[0]->max_speed)){
    loggerf(INFO, "%sNext block speed limit", Debug);
    T->changeSpeed(N[0]->max_speed, GRADUAL_SLOW_SPEED);
  }
  else if(!T->manual && N[0]->max_speed > T->speed && ABs->next > 1 && N[1]->max_speed >= N[0]->max_speed) {
    loggerf(INFO, "%sTrain Speed Up", Debug);
    if(N[0]->max_speed <= T->max_speed)
      T->changeSpeed(N[0]->max_speed, GRADUAL_FAST_SPEED);
    else if(T->speed != T->max_speed)
      T->changeSpeed(T->max_speed, GRADUAL_FAST_SPEED);
  }
}


void Connect_Rails(){
  
  auto connectors = Algorithm::find_connectors();
  uint16_t maxConnectors = connectors.size();

  loggerf(INFO, "Have %i connectors", connectors.size());

  uint16_t msgID = -1;

  msgID = WS_stc_ScanStatus(msgID, 0, maxConnectors);

  while(SYS->LC.state == Module_LC_Connecting && !SYS->stop && SYS->modules_linked == 0){
  	// continue;
    // sem_wait(&AlgorQueueNoEmpty); // FIXME
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

    // mutex_lock(&algor_mutex, "Algor Mutex"); // FIXME
    //Notify clients
    WS_stc_trackUpdate(0);

    // mutex_unlock(&algor_mutex, "Algor Mutex"); // FIXME

    usleep(100);
  }

  save_setup();

  SYS->modules_linked = 1;
}

void save_setup(){
  char data[1024];
  char * dataptr = data;

  for(uint8_t i = 0; i < SwManager->Units.size; i++){
    Unit * U = Units(i);
    if(!U)
      continue;

    dataptr[0] = i;
    dataptr[1] = U->connections_len;
    dataptr += 2;

    for(uint8_t j = 0; j < U->connections_len; j++){
      dataptr[0] = U->connection[j].unit;
      dataptr[1] = U->connection[j].connector;
      dataptr[1] = U->connection[j].crossover;
      dataptr += 3;
    }
  }

  char filename[40];
  time_t t = time(NULL);

  struct tm tm = *localtime(&t);
  sprintf(filename, "configs/setup-%04d%02d%02d-%02d%02d%02d.bin", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);


  FILE * fp = fopen(filename, "wb");

  if(!fp){
    loggerf(ERROR, "Failed to open file for setup saving");
    return;
  }

  fwrite(data, (int)dataptr - (int)data, 1, fp);

  fclose(fp);
}


};