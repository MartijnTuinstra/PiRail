#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include "system.h"
#include "mem.h"

#include "algorithm.h"
#include "logger.h"

#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/signals.h"
#include "switchboard/station.h"
#include "switchboard/blockconnector.h"
#include "train.h"

#include "modules.h"
#include "com.h"
#include "websocket/stc.h"

#include "sim.h"

#include "submodule.h"
// #include "pathfinding.h"

pthread_mutex_t mutex_lockA;
pthread_mutex_t algor_mutex;

sem_t AlgorQueueNoEmpty;
pthread_mutex_t AlgorQueueMutex;
struct s_AlgorQueue AlgorQueue;

void putAlgorQueue(Block * B, int enableQueue){
  loggerf(TRACE, "putAlgorQueue %x, %i", (unsigned int)B, enableQueue);
  mutex_lock(&AlgorQueueMutex, "AlgorQueueMutex");
  AlgorQueue.B[AlgorQueue.writeIndex++] = B;

  algor_queue_enable(enableQueue);

  if(AlgorQueue.writeIndex == AlgorQueueLength)
    AlgorQueue.writeIndex = 0;
  mutex_unlock(&AlgorQueueMutex, "AlgorQueueMutex");
}

void putList_AlgorQueue(Algor_Blocks ABs, int enable){
  putAlgorQueue(ABs.B, enable);

  for(int i = 0; i < ABs.prev; i++){
    putAlgorQueue(ABs.P[i], enable);
  }
  for(int i = 0; i < ABs.next; i++){
    putAlgorQueue(ABs.N[i], enable);
  }
}

Block * getAlgorQueue(){
  Block * result;
  mutex_lock(&AlgorQueueMutex, "AlgorQueueMutex");
  if(AlgorQueue.writeIndex == AlgorQueue.readIndex)
    result = 0;
  else
    result = AlgorQueue.B[AlgorQueue.readIndex++];

  if(AlgorQueue.readIndex == AlgorQueueLength)
    AlgorQueue.readIndex = 0;

  mutex_unlock(&AlgorQueueMutex, "AlgorQueueMutex");

  return result;
}

void processAlgorQueue(){
  Block * B = getAlgorQueue();
  while(B != 0){
    loggerf(TRACE, "Process %i:%i, %x, %x", B->module, B->id, B->IOchanged + (B->statechanged << 1) + (B->algorchanged << 2), B->state);
    Algor_process(B, 0);
     if(B->IOchanged){
      loggerf(INFO, "ReProcess");
      Algor_process(B, 0);
    }
    B = getAlgorQueue();
  }
}

void * Algor_Run(void * args){
  loggerf(INFO, "Algor_run started");

  while(SYS->UART.state != Module_Run && SYS->UART.state != Module_SIM_State){
    if(SYS->UART.state == Module_Fail || SYS->UART.state == Module_STOP){
      loggerf(ERROR, "Cannot run Algor when UART FAIL or STOP %x", SYS->UART.state);
      return 0;
    }
  }

  //UART_Send_Search();
  
  usleep(1000000);
  SYS_set_state(&SYS->LC.state, Module_LC_Searching);
  if(SYS->UART.state == Module_SIM_State){
    SIM_JoinModules();
    usleep(1000000);
    SYS_set_state(&SYS->LC.state, Module_LC_Connecting);
    usleep(1000000);
  }
  else{
    if(!SYS_wait_for_state(&SYS->UART.state, Module_Run)){
      SYS_set_state(&SYS->LC.state, Module_Fail);
      return 0;
    }
    COM_DevReset();
    while(!SYS->UART.modules_found){
      usleep(1000);
    }
    SYS_set_state(&SYS->LC.state, Module_LC_Connecting);
    Algor_Connect_Rails();
  }
  SIM_Connect_Rail_links();
  WS_stc_Track_Layout(0);
  // scan_All();
  // Scan All Blocks
  for(int i = 0;i<unit_len;i++){
    if(Units[i]){
      for(int j = 0; j < Units[i]->block_len; j++){
        if(U_B(i, j)){
          Algor_process(Units[i]->B[j], _FORCE);
        }
      }
      for(int j = 0; j < Units[i]->switch_len; j++){
        if(U_Sw(i, j)){
          U_Sw(i, j)->updatedState = true;
        }
      }
      for(int j = 0; j < Units[i]->msswitch_len; j++){
        if(U_MSSw(i, j)){
          U_MSSw(i, j)->updatedState = true;
        }
      }
    }
  }
  SYS_set_state(&SYS->LC.state, Module_Run);
  
  //Notify clients
  WS_stc_trackUpdate(0);
  WS_stc_SwitchesUpdate(0);

  // throw_switch(U_Sw(20, 5), 1, 0);
  // throw_switch(U_Sw(20, 6), 1, 1);

  // processAlgorQueue();

  // Block_Reverse(&Units[20]->B[10]->Alg);
  // Block_reserve(Units[20]->B[10]);
  // //void Block_Reverse(B);
  // Block_Reverse_To_Next_Switch(Units[20]->B[10]);

  // Block_Reverse(&Units[20]->B[11]->Alg);

  // processAlgorQueue();
  // WS_trackUpdate(0);
  // WS_SwitchesUpdate(0);

  // Units[20]->B[10]->blocked = 1;
  // Units[20]->B[10]->IOchanged = 1;
  // Units[20]->B[10]->statechanged = 1;
  // Units[Units[20]->B[10]->module]->block_state_changed = 1;
  // Algor_process(U_B(20, 10), 1);

  // throw_switch(U_Sw(20, 0), 1, 1);
  // throw_switch(U_Sw(20, 1), 1, 1);
  usleep(10000);

  loggerf(INFO, "Algor Ready");

  while(SYS->LC.state == Module_Run && SYS->stop == 0){
    sem_wait(&AlgorQueueNoEmpty);
    processAlgorQueue();

    mutex_lock(&algor_mutex, "Algor Mutex");
    //Notify clients
    WS_stc_trackUpdate(0);
    WS_stc_SwitchesUpdate(0);

    update_IO();

    mutex_unlock(&algor_mutex, "Algor Mutex");

    usleep(1000);
  }

  SYS_set_state(&SYS->LC.state, Module_STOP);
  loggerf(INFO, "Algor_run done");
  return 0;
}

void Algor_process(Block * B, int flags){
  loggerf(TRACE, "process %02i:%02i, flags %x", B->module, B->id, flags);

  if((B->IOchanged == 0 && B->algorchanged == 0) && (flags & _FORCE) == 0){
    loggerf(TRACE, "No changes");
    return;
  }

  if(flags & _LOCK){
    loggerf(WARNING, "LOCK");
    lock_Algor_process();
  }

  flags |= _DEBUG;

  // Find all surrounding blocks only if direction has changed or nearby switches
  if(B->IOchanged && B->algorchanged){
    B->AlgorSearch(flags);
  }

  B->IOchanged = 0;
  B->algorchanged = 0;
  // B->statechanged = 1;

  if(!B->blocked && B->state == BLOCKED){
    if(B->Alg.next > 0 && B->Alg.N[0]->blocked){
      B->Alg.N[0]->algorchanged = 1;
      putAlgorQueue(B->Alg.N[0], 1);
    }
    else if(B->Alg.prev > 0 && B->Alg.P[0]->blocked){
      B->Alg.P[0]->algorchanged = 1;
      putAlgorQueue(B->Alg.P[0], 1);
    }
  }

  // Set AllBlocks Blocked
  if(!B->Alg.B){
    loggerf(ERROR, "BLOCK %02i:%02i has no algo", B->module, B->id);
    B->Alg.B = B;
  }


  // if(flags & _DEBUG){
    Algor_print_block_debug(B);
  // }

  //Follow the train arround the layout
  Algor_train_following(&B->Alg, flags);
  if (B->IOchanged){
    loggerf(INFO, "Block Train ReProcess");
    if(flags & _LOCK){
      loggerf(WARNING, "UNLOCK");
      unlock_Algor_process();
    }
    return;
  }

  //Set oncomming switch to correct state
  // Algor_Switch_Checker(&B->Alg, flags);
  if (B->IOchanged){
    loggerf(DEBUG, "Block Switch ReProcess");
    B->AlgorClear();
    if(flags & _LOCK)
      unlock_Algor_process();
    return;
  }
  
  // Print all found blocks
  // if(flags & _DEBUG)

  //Apply block stating
  Algor_rail_state(&B->Alg, flags);

  //Train Control
  // Apply train algorithm only if there is a train on the block and is the front of the train
  if(B->train){
    if(B->Alg.N[0]){
      if(!B->Alg.N[0]->train || B->Alg.N[0]->train != B->train)
        Algor_train_control(&B->Alg, flags);
    }
    else{
      // Stop train no next block!!
      loggerf(INFO, "EMEG BRAKE, NO B_LOCK");
      B->train->changeSpeed(0, IMMEDIATE_SPEED);
    }
  }

  if(flags & _LOCK){
    loggerf(WARNING, "UNLOCK");
    unlock_Algor_process();
  }
  loggerf(TRACE, "Done");
}

// void Algor_Unset_Special_Block(Algor_Blocks * ABs, Block * B){
  // Algor_free_Blocks(ABs);
// }

// void Algor_free_Blocks(Algor_Blocks * ABs){
  
// }

// void Algor_Join_ABlocks(Algor_Block * A, Algor_Block * B){
//   if(!A || !B){
//     loggerf(ERROR, "No algor blocks");
//     return;
//   }
//   else if(A == B){
//     loggerf(ERROR, "Same algor block");
//     return;
//   }

//   for(uint8_t i = 0; i < A->len; i++){
//     B->p.SB[B->len++] = A->p.SB[i];
//   }

//   _free(A);
// }



void Algor_Set_Changed(Algor_Blocks * ABs){
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

/* void Algor_special_search_Blocks(Block * B, int flags){
  loggerf(TRACE, "Algor_special_search_Blocks %02i:%02i", B->module, B->id);
  struct next_prev_Block {
    Block * prev;
    uint8_t prev_l;
    Block * next;
    uint8_t next_l;
  };

  struct next_prev_Block pairs[10];
  memset(pairs, 0, 10*sizeof(struct next_prev_Block));
  uint8_t pair_counter = 0;

  for(int i = 0; i < B->switch_len; i++){
    Block * BlA = B;
    Block * BlB = B;
    int a = Switch_to_rail(&BlA, B->Sw[i], RAIL_LINK_S, 0);
    int b = Switch_to_rail(&BlB, B->Sw[i], RAIL_LINK_s, 0);

    if(a == 0 || b == 0){
      continue;
    }

    // if(BlA && BlB){
    //   printf("%02i:%02i == %02i:%02i\n", BlA->module, BlA->id, BlB->module, BlB->id);
    // }
    // else if(BlA){
    //   printf("%02i:%02i == %02i:%02i\n", BlA->module, BlA->id, 0, 0);
    // }
    // else if(BlB){
    //   printf("%02i:%02i == %02i:%02i\n", 0, 0, BlB->module, BlB->id);
    // }

    for(int j =0; j<10; j++){
      if((pairs[j].next == BlA && pairs[j].prev == BlB) ||
         (pairs[j].prev == BlA && pairs[j].next == BlB)){
         break;
      }
      if(j == 9){
        pairs[pair_counter].next = BlA;
        pairs[pair_counter].next_l = a;
        pairs[pair_counter].prev = BlB;
        pairs[pair_counter].prev_l = b;
        pair_counter++;
      }
    }
  }


  Block * Aside[15];
  Block * Bside[15];
  memset(Aside, 0, 15*sizeof(void *) );
  memset(Bside, 0, 15*sizeof(void *) );

  if(pair_counter == 1){
    uint8_t a_dir = SWITCH_CARE;
    uint8_t b_dir = SWITCH_CARE;
    // Get direction away from the block
    if(Next(pairs[0].next, NEXT | SWITCH_CARE, pairs[0].next_l) == B)
      a_dir |= PREV;
    else if(Next(pairs[0].next, PREV | SWITCH_CARE, pairs[0].next_l) == B)
      a_dir |= NEXT;

    if(Next(pairs[0].prev, NEXT | SWITCH_CARE, pairs[0].prev_l) == B)
      b_dir |= PREV;
    else if(Next(pairs[0].prev, PREV | SWITCH_CARE, pairs[0].prev_l) == B)
      b_dir |= NEXT;

    // Get all blocks to and away from selected blocks
    int16_t lengthA = 0;
    if(pairs[0].next_l > 1){
      // reverse
      for(int i = 1; i < pairs[0].next_l; i++){
        Aside[i-1] = Next(pairs[0].next, (a_dir ^ PREV), pairs[0].next_l - i);
        lengthA += Aside[i-1]->length;
      }
    }
    if(pairs[0].next){
      Aside[pairs[0].next_l - 1] = pairs[0].next;
      lengthA += Aside[pairs[0].next_l - 1]->length;
      for(int i = pairs[0].next_l + 1; i<15; i++){
        Aside[i-1] = Next(pairs[0].next, a_dir, i - pairs[0].next_l);
        if(!Aside[i-1])
          break;
        lengthA += Aside[i-1]->length;
        if(lengthA > 400){
          break;
        }
      }
    }

    uint16_t lengthB = 0;
    if(pairs[0].prev_l > 1){
      // reverse
      for(int i = 1; i < pairs[0].prev_l; i++){
        Bside[i-1] = Next(pairs[0].prev, (b_dir ^ PREV), pairs[0].prev_l - i);
        lengthB += Bside[i-1]->length;
      }
    }
    if(pairs[0].prev){
      Bside[pairs[0].prev_l - 1] = pairs[0].prev;
      lengthB += Bside[pairs[0].prev_l - 1]->length;
      for(int i = pairs[0].prev_l + 1; i<15; i++){
        Bside[i-1] = Next(pairs[0].prev, b_dir, i - pairs[0].prev_l);
        if(!Bside[i-1])
          break;
        lengthB += Bside[i-1]->length;
        if(lengthB > 400){
          break;
        }
      }
    }

    // char debug_output[200];
    // sprintf(debug_output, "A%i    ", a_dir);
    // for(int ab = 0; ab < 15; ab++){
    //   if(Aside[ab])
    //     sprintf(debug_output, "%s%2i:%2i\t", debug_output, Aside[ab]->module, Aside[ab]->id);
    // }
    // loggerf(TRACE, "%s", debug_output);

    // sprintf(debug_output, "B%i    ", a_dir);
    // for(int ab = 0; ab < 15; ab++){
    //   if(Bside[ab])
    //     sprintf(debug_output, "%s%2i:%2i\t", debug_output, Bside[ab]->module, Bside[ab]->id);
    // }
    // loggerf(TRACE, "%s", debug_output);

    // Put all blocks into Algor blocks
    int8_t dir = -1;

    //Determine direction for A side and B side
    if(Aside[0] && Bside[0]){
      // If A and B have the same direction
      if((Aside[0]->dir == Bside[0]->dir || (Aside[0]->dir ^ 0b101) == Bside[0]->dir)){
        // As the block itself
        if(Aside[0]->dir == B->dir || Bside[0]->dir == B->dir){
          dir = (b_dir & 1) ^ 1;
        }
        // Or as reversed blocks
        else if((Aside[0]->dir ^ 0b1) == B->dir || (Bside[0]->dir ^ 0b1) == B->dir){
          dir = (b_dir & 1);
        }
        // if different but neighbours are normal blocks, reverse B
        else if(Aside[0]->dir == B->dir ^ 0b100 && Aside[0]->Alg.B->len == 0 && Bside[0]->Alg.B->len == 0){
          B->dir ^= 0b100;
          dir = (b_dir & 1) ^ 1;
        }
      }

      // If A has the same direction as block
      // If A is reversed to block
      else if(Aside[0]->dir == B->dir || (Aside[0]->dir ^ 0b101) == B->dir){
        dir = (a_dir & 1);
      }

      // If B has the same direction as block
      // If B is reversed to block
      else if(Bside[0]->dir == B->dir || (Bside[0]->dir ^ 0b101) == B->dir){
        dir = (b_dir & 1) ^ 1;
      }
    }
    else if(Bside[0]){
      if(Bside[0]->dir == B->dir || (B->dir ^ 0b101) == Bside[0]->dir){
        dir = (b_dir & 1) ^ 1;
      }
      else{
        B->dir ^= 0b100;
        dir = (b_dir & 1) ^ 1;
      }
    }
    else if(Aside[0]){
      if(Aside[0]->dir == B->dir || (B->dir ^ 0b101) == Aside[0]->dir){
        dir = (a_dir & 1);
      }
      else{
        B->dir ^= 0b100;
        dir = (a_dir & 1);
      }
    }

    // Debug stuf
    if(dir == -1){
      loggerf(ERROR, "No direction found");
      return;
    }

    // Check for neighbouring SPECIAL blocks
    uint8_t a = 0, b = 0;
    if(Aside[0] && Aside[0]->type == SPECIAL){
      if(B->Alg.B && Aside[0]->Alg.B){
        loggerf(INFO, "Join Algor_Blocks");
        Algor_Join_ABlocks(B->Alg.B, Aside[0]->Alg.B);
        B->Alg.B = Aside[0]->Alg.B;
      }
      else if(Aside[0]->Alg.B){
        Aside[0]->Alg.B->p.SB[Aside[0]->Alg.B->len++] = B;
        B->Alg.B = Aside[0]->Alg.B;
      }
      else if(B->Alg.B){
        B->Alg.B->p.SB[B->Alg.B->len++] = Aside[0];
        Aside[0]->Alg.B = B->Alg.B;
      }
      a += B->Alg.B->len - 1;

      if(!dircmp(Aside[0], B)){
        B->dir ^= 0b100;
        dir ^= 1; // Toggle PREV <-> NEXT
      }
    }
    
    if(Bside[0] && Bside[0]->type == SPECIAL){
      if(B->Alg.B){
        loggerf(INFO, "Join Algor_Blocks");
        Algor_Join_ABlocks(B->Alg.B, Bside[0]->Alg.B);
        B->Alg.B = Bside[0]->Alg.B;
      }
      else if(Bside[0]->Alg.B){
        Bside[0]->Alg.B->p.SB[Bside[0]->Alg.B->len++] = B;
        B->Alg.B = Bside[0]->Alg.B;
      }
      else if(B->Alg.B){
        B->Alg.B->p.SB[B->Alg.B->len++] = Bside[0];
        Bside[0]->Alg.B = B->Alg.B;
      }
      b += B->Alg.B->len - 1;

      if(!dircmp(Bside[0], B)){
        B->dir ^= 0b100;
        dir ^= 1; // Toggle PREV <-> NEXT
      }
    }

    if(!B->Alg.B){
      B->Alg->B = B;
    }


    Algor_Block ** Aside_P;
    Algor_Block ** Bside_P;
    uint8_t * Alength;
    uint8_t * Blength;

    if(dir == PREV){
      Bside_P = B->Alg.N;
      Aside_P = B->Alg.P;
      Alength = &B->Alg.prev;
      Blength = &B->Alg.next;
    }
    else if(dir == NEXT){
      Bside_P = B->Alg.P;
      Aside_P = B->Alg.N;
      Alength = &B->Alg.next;
      Blength = &B->Alg.prev;
    }
    else{
      loggerf(WARNING, "No DIR");
    }

    *Alength = 0;
    *Blength = 0;

    uint16_t BlockLength = 0;

    do{
      if(!Bside[b] || !Bside_P)
        break;
      Bside_P[(*Blength)] = Bside[b]->Alg.B;
      BlockLength += Bside[b]->length;
      *Blength = *Blength + 1;
      b++;
    }
    while(BlockLength < Block_Minimum_Size*3 && *Blength < 10);

    BlockLength = 0;
    
    do{
      if(!Aside[a] || !Aside_P)
        break;
      Aside_P[(*Alength)] = Aside[a]->Alg.B;
      BlockLength += Aside[a]->length;
      *Alength = *Alength + 1;
      a++;
    }
    while(BlockLength < Block_Minimum_Size*3 && *Alength < 10);
  }
  else{
    loggerf(ERROR, "Zero or 1+ pairs");
  }
}*/

void Algor_print_block_debug(Block * B){
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
  if(B->train){
    ptr += sprintf(ptr, "T");
  }
  else{
    ptr += sprintf(ptr, " ");
  }
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

void Algor_Switch_Checker(Algor_Blocks * ABs, int debug){
  //Unpack AllBlocks
  //Algor_Block BPPP = *AllBlocks.BPPP;
  //Algor_Block BPP  = *AllBlocks.BPP;
  //Algor_Block BP   = *AllBlocks.BP;
  Block * B = ABs->B;
  Block **N = ABs->N;
  uint8_t next = ABs->next;
  //Block BNNN = *AllBlocks.BNNN;

  if(!B->blocked)
    return;

  Block * tB;

  for(uint8_t i = 0; i < 4; i++){
    if(i > next)
      break;

    if(i == 0){
      tB = B;
    }
    else{
      tB = N[i-1];
    }

    if(tB->blocked)
      continue;

    struct rail_link * link = tB->NextLink(NEXT);
    if (link->type != RAIL_LINK_R && link->type != RAIL_LINK_E && link->type != RAIL_LINK_C) {
      loggerf(INFO, "Switch_Checker scan block (%i,%i)", tB->module, tB->id);
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
      if (!Switch_Check_Path(tB, *link, NEXT | SWITCH_CARE)) {
        loggerf(INFO, "Switch next path!! %02i:%02i", tB->module, tB->id);

        if(Switch_Set_Path(tB, *link, NEXT | SWITCH_CARE)) {
          loggerf(INFO, "Switch set path!! %02i:%02i", tB->module, tB->id);
          tB->IOchanged = 1; // Recalculate
          tB->algorchanged = 1; // Recalculate
          Switch_Reserve_Path(tB, *link, NEXT | SWITCH_CARE);
          return;
        }
        else{
          loggerf(WARNING, "Failed switch set path");
        }
      }
      else if(((link->type == RAIL_LINK_S  || link->type == RAIL_LINK_s ) &&   (link->p.Sw)->Detection &&   (link->p.Sw)->Detection->state != RESERVED_SWITCH) || 
              ((link->type == RAIL_LINK_MA || link->type == RAIL_LINK_MB) && (link->p.MSSw)->Detection && (link->p.MSSw)->Detection->state != RESERVED_SWITCH)){
        loggerf(WARNING, "reserve_switch_path");
        Switch_Reserve_Path(tB, *link, NEXT | SWITCH_CARE);
      }
    }
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

void Algor_rail_state(Algor_Blocks * ABs, int debug){
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
  else if(ABs->next == 0){
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

void Algor_train_following(Algor_Blocks * ABs, int debug){
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
      loggerf(WARNING, "LOST Train block %x", (unsigned int)B);
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
    char debugmsg[1000];
    char * ptr = debugmsg;
    ptr += sprintf(ptr, "Blocked Block without train");
    if(prev > 0)
      ptr += sprintf(ptr, "\nP: %02i:%02i %c%c%c %6x", BP[0]->module, BP[0]->id, BP[0]->blocked ? 'B':' ', BP[0]->detectionblocked ? 'D':' ', BP[0]->virtualblocked ? 'V':' ', BP[0]->train);
    else
      ptr += sprintf(ptr, "\n                   ");
    if(next > 0)
      ptr += sprintf(ptr, "\tN: %02i:%02i %c%c%c %6x", BN[0]->module, BN[0]->id, BN[0]->blocked ? 'B':' ', BN[0]->detectionblocked ? 'D':' ', BN[0]->virtualblocked ? 'V':' ', BN[0]->train);
    else
      ptr += sprintf(ptr, "\t                   ");

    loggerf(INFO, "%s", debugmsg);
    // Train moved forward
    if(prev > 0 && BP[0]->blocked && BP[0]->train){
      // Copy train id from previous block
      B->train = BP[0]->train;
      B->train->moveForward(B);

      if(B->reserved){
        B->reserved--;
        if(B->reserved == 0){
          B->state = PROCEED;
          B->reverse_state = PROCEED;

          B->statechanged = 1;
          Units[B->module]->block_state_changed = 1;
        }
      }
      // if(train_link[B->train])
      //   train_link[B->train]->Block = B;
      loggerf(INFO, "COPY_TRAIN from %02i:%02i to %02i:%02i", BP[0]->module, BP[0]->id, B->module, B->id);
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
          B->state = PROCEED;
          B->reverse_state = PROCEED;

          B->statechanged = 1;
          Units[B->module]->block_state_changed = 1;
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

void Algor_train_control(Algor_Blocks * ABs, int debug){
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
  else if(T->control != TRAIN_MANUAL && (T->target_speed > N[0]->max_speed || T->speed > N[0]->max_speed)){
    loggerf(DEBUG, "%sNext block speed limit", Debug);
    T->changeSpeed(N[0]->max_speed, GRADUAL_SLOW_SPEED);
  }
  else if(T->control != TRAIN_MANUAL && N[0]->max_speed > T->speed && ABs->next > 1 && N[1]->max_speed >= N[0]->max_speed) {
    loggerf(DEBUG, "%sTrain Speed Up", Debug);
    if(N[0]->max_speed <= T->max_speed)
      T->changeSpeed(N[0]->max_speed, GRADUAL_FAST_SPEED);
    else if(T->speed != T->max_speed)
      T->changeSpeed(T->max_speed, GRADUAL_FAST_SPEED);
  }
}

void Algor_Connect_Rails(){
  
  auto connectors = Algorithm_find_connectors();
  uint16_t maxConnectors = connectors.size();

  loggerf(INFO, "Have %i connectors", connectors.size());

  uint16_t msgID = -1;

  msgID = WS_stc_ScanStatus(msgID, 0, maxConnectors);

  while(SYS->LC.state == Module_LC_Connecting && !SYS->stop && SYS->modules_linked == 0){
    sem_wait(&AlgorQueueNoEmpty);
    
    if(uint8_t * findResult = Algorithm_find_connectable(&connectors)){
      Algorithm_connect_connectors(&connectors, findResult);

      WS_stc_ScanStatus(msgID, maxConnectors - connectors.size(), maxConnectors);
    }

    if(connectors.size() == 0)
      break;

    //IF ALL JOINED
    //BREAK

    for(uint8_t j = 0; j < unit_len; j++){
      if(!Units[j])
        continue;
      Unit * U = Units[j];

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

    mutex_lock(&algor_mutex, "Algor Mutex");
    //Notify clients
    WS_stc_trackUpdate(0);

    mutex_unlock(&algor_mutex, "Algor Mutex");

    usleep(100);
  }

  Algor_save_setup();

  SYS->modules_linked = 1;
}

void Algor_save_setup(){
  char data[1024];
  char * dataptr = data;

  for(uint8_t i = 0; i < unit_len; i++){
    if(!Units[i])
      continue;

    dataptr[0] = i;
    dataptr[1] = Units[i]->connections_len;
    dataptr += 2;

    for(uint8_t j = 0; j < Units[i]->connections_len; j++){
      dataptr[0] = Units[i]->connection[j].unit;
      dataptr[1] = Units[i]->connection[j].connector;
      dataptr[1] = Units[i]->connection[j].crossover;
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
