#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

#include "system.h"
#include "mem.h"

#include "algorithm.h"
#include "logger.h"

#include "switch.h"
#include "train.h"
#include "switch.h"
#include "signals.h"

#include "module.h"
#include "com.h"
#include "websocket_msg.h"

#include "submodule.h"

pthread_mutex_t mutex_lockA;
pthread_mutex_t algor_mutex;

void * scan_All_continiously(){
  while(_SYS->_STATE & STATE_RUN){
  //printf("\n\n\n");
  clock_t t;
  t = clock();
  mutex_lock(&mutex_lockA, "Mutex A");
  #ifdef En_UART
  for(int i = 0;i<strlen(List_of_Modules);i++){
    printf("R%i ",List_of_Modules[i]);
    struct COM_t C;
    memset(C.Data,0,32);
    C.Adr = List_of_Modules[i];
    C.Opcode = 6;
    C.Length = 0;

    mutex_lock(&mutex_UART, "UART Mutex");
    COM_Send(C);
    char COM_data[20];
    memset(COM_data,0,20);
    COM_Recv(COM_data);
    COM_Parse(COM_data);
    mutex_unlock(&mutex_UART, "UART Mutex");
    usleep(10);
  }
  printf("\n");
  #endif
  for(int i = 0;i<unit_len;i++){
    if(Units[i]){
    for(int j = 0;j<=Units[i]->block_len;j++){
      if(Units[i]->B[j]){
      //printf("%i:%i\n",i,j);
      // process(Units[i]->B[j],0);
      }
    }
    }
  }
  WS_trackUpdate(0);
  WS_SwitchesUpdate(0);
  mutex_unlock(&mutex_lockA, "Mutex A");
  t = clock() - t;
  //printf ("It took me %d clicks (%f seconds).\n",t,((float)t)/CLOCKS_PER_SEC);
  //printf("\n\n\n\n\n\n");

  procces_accessoire();

  //FILE *data;
  //data = fopen("data.txt", "a");
  //fprintf(data,"%d\n",t);
  //fclose(data);

  usleep(1000000);

  // printf("\n");
  }
  return 0;
}

void scan_All(){
  mutex_lock(&mutex_lockA, "Mutex A");
  for(int i = 0;i<unit_len;i++){
    if(Units[i]){
      for(int j = 0; j < Units[i]->block_len; j++){
        if(U_B(i, j)){
          //printf("%i:%i\n",i,j);
          process(Units[i]->B[j], 2);
        }
      }
      for(int j = 0; j <= Units[i]->switch_len; j++){
        if(U_Sw(i, j)){
          U_Sw(i, j)->state |= 0x80;
        }
      }
    }
  }

  WS_trackUpdate(0);
  WS_SwitchesUpdate(0);
  mutex_unlock(&mutex_lockA, "Mutex A");
}

sem_t AlgorQueueNoEmpty;
pthread_mutex_t AlgorQueueMutex;
struct s_AlgorQueue AlgorQueue;

void putAlgorQueue(Block * B, int enableQueue){
  loggerf(TRACE, "putAlgorQueue");
  mutex_lock(&AlgorQueueMutex, "AlgorQueueMutex");
  AlgorQueue.B[AlgorQueue.writeIndex++] = B;

  algor_queue_enable(enableQueue);

  if(AlgorQueue.writeIndex == AlgorQueueLength)
    AlgorQueue.writeIndex = 0;
  mutex_unlock(&AlgorQueueMutex, "AlgorQueueMutex");
}

void putList_AlgorQueue(struct algor_blocks AllBlocks, int enable){
  putAlgorQueue(AllBlocks.B, enable);

  Algor_Block * AB;

  for(int i = 0; i < 6; i++){
    if(i == 0)
      AB = AllBlocks.BPPP;
    else if(i == 1)
      AB = AllBlocks.BPP;
    else if(i == 2)
      AB = AllBlocks.BP;
    else if(i == 3)
      AB = AllBlocks.BN;
    else if(i == 4)
      AB = AllBlocks.BNN;
    else if(i == 5)
      AB = AllBlocks.BNNN;

    for(int j = 0; j < AB->blocks; j++){
      putAlgorQueue(AB->B[j], enable);
    }
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
    loggerf(TRACE, "Process %i:%i, %x, %x", B->module, B->id, B->changed, B->state);
    process(B, 2);
     if(B->changed & IO_Changed){
      loggerf(TRACE, "ReProcess");
      process(B, 0);
    }
    B = getAlgorQueue();
  }
}

void * Algor_Run(){
  loggerf(INFO, "Algor_run started");

  while(_SYS->UART_State != _SYS_Module_Run){
    if(_SYS->UART_State == _SYS_Module_Fail || _SYS->UART_State == _SYS_Module_Stop){
      loggerf(ERROR, "Cannot run Algor when UART FAIL or STOP %x", _SYS->UART_State);
      return 0;
    }
  }

  //UART_Send_Search();
  
  usleep(200000);
  _SYS->LC_State = _SYS_LC_Searching;
  WS_stc_SubmoduleState();
  JoinModules();
  usleep(100000);
  _SYS->LC_State = _SYS_LC_Connecting;
  WS_stc_SubmoduleState();
  Connect_Rail_links();
  WS_Track_Layout(0);
  usleep(800000);
  _SYS->LC_State = _SYS_Module_Run;
  WS_stc_SubmoduleState();
  scan_All();
  throw_switch(U_Sw(20, 5), 1, 0);
  throw_switch(U_Sw(20, 6), 1, 1);
  // throw_switch(U_Sw(20, 2), 1);
  usleep(10000);

  while(_SYS->LC_State == _SYS_Module_Run){
    sem_wait(&AlgorQueueNoEmpty);
    processAlgorQueue();

    mutex_lock(&algor_mutex, "Algor Mutex");
    //Notify clients
    WS_trackUpdate(0);
    WS_SwitchesUpdate(0);

    update_IO();

    mutex_unlock(&algor_mutex, "Algor Mutex");

    usleep(1000);
  }

  loggerf(INFO, "Algor_run done");
  return 0;
}

void change_block_state(Algor_Block * A, enum Rail_states state){
  if(!A->blocked){
  for(int i = 0;i<A->length;i++){
    if(A->B[i]->state > state){
    printf("change_block_state: %i:%i    %i => %i\n", A->B[i]->module, A->B[i]->id, A->B[i]->state, state);
    A->B[i]->changed = 1;
    A->B[i]->state = state;
    }
  }
  }else{
  for(int i = 0;i<A->length;i++){
    printf("change_block_state: %i:%i    %i => %i\n", A->B[i]->module, A->B[i]->id, A->B[i]->state, state);
    if(A->B[i]->blocked){
    break;
    }
    if(A->B[i]->state > state){
    A->B[i]->changed = 1;
    A->B[i]->state = state;
    }
  }
  }
}

void Algor_Set_Changed(struct algor_blocks * blocks){
  loggerf(TRACE, "Algor_Set_Changed");
  //Scroll through all the pointers of allblocks
  for(int i = 0; i < 7; i++){
    if(i == 3){
      continue;
    }

    Algor_Block * AB;

    if(i == 0)
      AB = blocks->BPPP;
    else if(i == 1)
      AB = blocks->BPP;
    else if(i == 2)
      AB = blocks->BP;
    else if(i == 4)
      AB = blocks->BN;
    else if(i == 5)
      AB = blocks->BNN;
    else if(i == 6)
      AB = blocks->BNNN;

    for(int j = 0; j < AB->blocks; j++){
      AB->B[j]->changed |= Block_Algor_Changed;
    }
  }
}

void process(Block * B,int flags){
  loggerf(TRACE, "process %02i:%02i, flags %x", B->module, B->id, flags);

  int debug = (flags & 1);
  int force = (flags & 2) >> 1;

  if((flags & NO_LOCK) == 0){
    lock_Algor_process();
  }

  if((B->changed & (IO_Changed | Block_Algor_Changed)) == 0 && !force){
    if((flags & NO_LOCK) == 0)
      unlock_Algor_process();
    return;
  }

  // Find all surrounding blocks only if direction has changed or nearby switches
  if(B->changed & (Block_Algor_Changed | IO_Changed)){
    Algor_search_Blocks(&B->Alg, debug);
    B->changed &= ~(Block_Algor_Changed | IO_Changed);
  }

  B->changed |= State_Changed;

  if(B->train && !B->blocked){
    if(B->Alg.BN->blocks > 0 && B->Alg.BN->B[0]->blocked){
      putAlgorQueue(B->Alg.BN->B[0], 1);
    }
    else if(B->Alg.BP->blocks > 0 && B->Alg.BP->B[0]->blocked){
      putAlgorQueue(B->Alg.BP->B[0], 1);
    }
  }

  Algor_GetBlocked_Blocks(B->Alg);

  if(debug){
    Algor_print_block_debug(B->Alg);
  }

  //Follow the train arround the layout
  Algor_train_following(B->Alg, debug);
  if (B->changed & IO_Changed){
    printf("Block Train ReProcess\n");
    Algor_clear_Blocks(&B->Alg);
    if((flags & NO_LOCK) == 0)
      unlock_Algor_process();
    return;
  }

  //Set oncomming switch to correct state
  Algor_Switch_Checker(B->Alg, debug);
  if (B->changed & IO_Changed){
    printf("Block Switch ReProcess\n");
    Algor_clear_Blocks(&B->Alg);
    if((flags & NO_LOCK) == 0)
      unlock_Algor_process();
    return;
  }

  //Apply block stating
  Algor_rail_state(B->Alg, debug);

  //Check Switch

  // Print all found blocks to stdout
  // Algor_print_block_debug(B->Alg);

  Algor_signal_state(B->Alg, debug);

  // Apply train algorithm only if there is a train on the block and is the front of the train
  if(B->train){
    if(B->Alg.BN->B[0]){
      if(!B->Alg.BN->B[0]->train)
        Algor_train_control(B->Alg, debug);
    }
    else{
      // Stop train no next block!!
      loggerf(INFO, "EMEG BRAKE, NO BLOCK");
      train_change_speed(B->train, 0, IMMEDIATE_SPEED);
    }

  }

  //Train Control

  if((flags & NO_LOCK) == 0)
    unlock_Algor_process();
}


void Algor_init_Blocks(struct algor_blocks * AllBlocks, Block * B){
  //init_Algor_Blocks and clear
  Algor_Block * BPPP = _calloc(1, Algor_Block);
  Algor_Block * BPP  = _calloc(1, Algor_Block);
  Algor_Block * BP   = _calloc(1, Algor_Block);
  Algor_Block * BN   = _calloc(1, Algor_Block);
  Algor_Block * BNN  = _calloc(1, Algor_Block);
  Algor_Block * BNNN = _calloc(1, Algor_Block);

  // Link algor_blocks
  AllBlocks->BPPP = BPPP;
  AllBlocks->BPP  = BPP;
  AllBlocks->BP   = BP;
  AllBlocks->B    = B;
  AllBlocks->BN   = BN;
  AllBlocks->BNN  = BNN;
  AllBlocks->BNNN = BNNN;

  Algor_clear_Blocks(AllBlocks);
}

void Algor_clear_Blocks(struct algor_blocks * AllBlocks){
  memset(AllBlocks->BPPP->B, 0, 5*sizeof(void *));
  memset(AllBlocks->BPP->B, 0, 5*sizeof(void *));
  memset(AllBlocks->BP->B, 0, 5*sizeof(void *));
  memset(AllBlocks->BN->B, 0, 5*sizeof(void *));
  memset(AllBlocks->BNN->B, 0, 5*sizeof(void *));
  memset(AllBlocks->BNNN->B, 0, 5*sizeof(void *));
  AllBlocks->BPPP->blocks = 0;
  AllBlocks->BPP->blocks = 0;
  AllBlocks->BP->blocks = 0;
  AllBlocks->BN->blocks = 0;
  AllBlocks->BNN->blocks = 0;
  AllBlocks->BNNN->blocks = 0;

  AllBlocks->BPPP->signal = 0;
  AllBlocks->BPP->signal = 0;
  AllBlocks->BP->signal = 0;
  AllBlocks->BN->signal = 0;
  AllBlocks->BNN->signal = 0;
  AllBlocks->BNNN->signal = 0;

  AllBlocks->BPPP->switches = 0;
  AllBlocks->BPP->switches = 0;
  AllBlocks->BP->switches = 0;
  AllBlocks->BN->switches = 0;
  AllBlocks->BNN->switches = 0;
  AllBlocks->BNNN->switches = 0;

  AllBlocks->BPPP->length = 0;
  AllBlocks->BPP->length = 0;
  AllBlocks->BP->length = 0;
  AllBlocks->BN->length = 0;
  AllBlocks->BNN->length = 0;
  AllBlocks->BNNN->length = 0;
}

void Algor_free_Blocks(struct algor_blocks * AllBlocks){
  _free(AllBlocks->BPPP);
  _free(AllBlocks->BPP);
  _free(AllBlocks->BP);
  _free(AllBlocks->BN);
  _free(AllBlocks->BNN);
  _free(AllBlocks->BNNN);
}

void Algor_print_block_debug(struct algor_blocks AllBlocks){

  //Unpack AllBlocks
  Algor_Block BPPP = *AllBlocks.BPPP;
  Algor_Block BPP  = *AllBlocks.BPP;
  Algor_Block BP   = *AllBlocks.BP;
  Block * B        =  AllBlocks.B;
  Algor_Block BN   = *AllBlocks.BN;
  Algor_Block BNN  = *AllBlocks.BNN;
  Algor_Block BNNN = *AllBlocks.BNNN;

  int debug = INFO;

  char output[200] = "";

  if(BPPP.blocks > 0){
    sprintf(output, "%sPPP%i ", output, BPP.blocks);
    if(BPPP.blocked)
      sprintf(output, "%sB", output);
    else
      sprintf(output, "%s ", output);

    for(int i = 1;i>=0;i--){
      if(BPPP.B[i]){
        sprintf(output, "%s%02i:%02i", output,BPPP.B[i]->module,BPPP.B[i]->id);
        if(BPPP.B[i]->blocked){
          sprintf(output, "%sB  ", output);
        }else{
          sprintf(output, "%s   ", output);
        }
      }
      else if(i < BPPP.blocks){
        debug = ERROR;
        sprintf(output, "%s !!!!!! ", output);
      }
      else{
        sprintf(output, "%s        ", output);
      }
    }
  }else{
    sprintf(output, "%s                      ", output);
  }
  if(BPP.blocks > 0){
    sprintf(output, "%sPP%i ", output,BPP.blocks);
    if(BPP.blocked)
    sprintf(output, "%sB", output);
    else
    sprintf(output, "%s ", output);
    for(int i = 1;i>=0;i--){
      if(BPP.B[i]){
        sprintf(output, "%s%02i:%02i", output,BPP.B[i]->module,BPP.B[i]->id);
        if(BPP.B[i]->blocked){
        sprintf(output, "%sB  ", output);
        }else{
        sprintf(output, "%s   ", output);
        }
      }
      else if(i < BPP.blocks){
        debug = ERROR;
        sprintf(output, "%s !!!!!! ", output);
      }else{
        sprintf(output, "%s        ", output);
      }
    }
  }else{
    sprintf(output, "%s                     ", output);
  }
  if(BP.blocks > 0){
    sprintf(output, "%sP%i ", output,BP.blocks);
    if(BP.blocked)
    sprintf(output, "%sB", output);
    else
    sprintf(output, "%s ", output);
    for(int i = 2;i>=0;i--){
      if(BP.B[i]){
        sprintf(output, "%s%02i:%02i", output,BP.B[i]->module,BP.B[i]->id);
        if(BP.B[i]->blocked){
        sprintf(output, "%sB  ", output);
        }else{
        sprintf(output, "%s   ", output);
        }
      }
      else if(i < BP.blocks){
        debug = ERROR;
        sprintf(output, "%s !!!!!! ", output);
      }else{
        sprintf(output, "%s        ", output);
      }
    }
  }else{
    sprintf(output, "%s                            ", output);
  }
  sprintf(output, "%sA%3i %2x%02i:%02i;",output,B->length,B->type,B->module,B->id);
  if(B->train){
    sprintf(output, "T");
  }
  else{
    sprintf(output, " ");
  }
  sprintf(output, "D%-2iS%-2i", B->dir,B->state);

  if(B->blocked){
    sprintf(output, "%sB", output);
  }
  sprintf(output, "%s\t", output);
  if(BN.blocks > 0){
    sprintf(output, "%sN%i ", output,BN.blocks);
    if(BN.blocked)
      sprintf(output, "%sB", output);
    else
      sprintf(output, "%s ", output);

    for(int i = 0;i<3;i++){
      if(BN.B[i]){
        sprintf(output, "%s%02i:%02i", output,BN.B[i]->module,BN.B[i]->id);
        if(BN.B[i]->blocked){
        sprintf(output, "%sB  ", output);
        }else{
        sprintf(output, "%s   ", output);
        }
      }
      else if(i < BN.blocks){
        debug = ERROR;
        sprintf(output, "%s !!!!!! ", output);
      }else{
        sprintf(output, "%s        ", output);
      }
    }
  }
  if(BNN.blocks > 0){
      sprintf(output, "%sNN%i ", output,BNN.blocks);
    if(BNN.blocked)
      sprintf(output, "%sB", output);
    else
      sprintf(output, "%s ", output);

    for(int i = 0;i<2;i++){
      if(BNN.B[i]){
        sprintf(output, "%s%02i:%02i", output,BNN.B[i]->module,BNN.B[i]->id);
        if(BNN.B[i]->blocked){
        sprintf(output, "%sB  ", output);
        }else{
        sprintf(output, "%s   ", output);
        }
      }
      else if(i < BNN.blocks){
        debug = ERROR;
        sprintf(output, "%s !!!!!! ", output);
      }else{
      sprintf(output, "%s        ", output);
     }
    }
  }
  if(BNNN.blocks > 0){
    sprintf(output, "%sNNN%i ", output,BNNN.blocks);
    if(BNNN.blocked)
      sprintf(output, "%sB", output);
    else
      sprintf(output, "%s ", output);

    for(int i = 0;i<2;i++){
      if(BNNN.B[i]){
        sprintf(output, "%s%02i:%02i", output,BNNN.B[i]->module,BNNN.B[i]->id);
        if(BNNN.B[i]->blocked){
          sprintf(output, "%sB  ", output);
        }else{
          sprintf(output, "%s   ", output);
        }
      }
      else if(i < BNNN.blocks){
        debug = ERROR;
        sprintf(output, "%s !!!!!! ", output);
      }else{
      sprintf(output, "%s        ", output);
      }
    }
  }

  loggerf(debug, "%s", output);
}

void Algor_Switch_Checker(struct algor_blocks AllBlocks, int debug){
  //Unpack AllBlocks
  //Algor_Block BPPP = *AllBlocks.BPPP;
  //Algor_Block BPP  = *AllBlocks.BPP;
  //Algor_Block BP   = *AllBlocks.BP;
  Block * B        =  AllBlocks.B;
  Algor_Block BN   = *AllBlocks.BN;
  Algor_Block BNN  = *AllBlocks.BNN;
  //Algor_Block BNNN = *AllBlocks.BNNN;

  //Check Next 1
  if(B->blocked && (((BN.switches || BNN.switches) && BN.blocks > 0) || (BN.blocks == 0 || BNN.blocks == 0))){
    Block * tmp;
    for(int i = 0; i <= BN.blocks; i++){
      if(i == 0){
        tmp = B;
      }
      else{
        tmp = BN.B[i - 1];
      }

      // loggerf(DEBUG, "checking block next link %i:%i", tmp->module, tmp->id);
      if (tmp->type == SPECIAL) {
        continue;
      }
      if (tmp->blocked){
        continue;
      }

      struct rail_link * link = Next_link(tmp, NEXT);
      loggerf(INFO, "Switch_Checker scan block (%i,%i)", tmp->module, tmp->id);
      if (link->type != RAIL_LINK_R && link->type != RAIL_LINK_E) {
        if (!Next_check_Switch_Path(tmp, *link, NEXT | SWITCH_CARE)) {
          loggerf(INFO, "Switch next path!! %02i:%02i", tmp->module, tmp->id);

          if (set_switch_path(tmp, *link, NEXT | SWITCH_CARE)) {
            B->changed |= IO_Changed; // Recalculate
            return;
          }
        }
      }
      // else{
      //   loggerf(DEBUG, "Link is of type %x", link->type);
      // }

      if(link->p && (((link->type == RAIL_LINK_S || link->type == RAIL_LINK_s) && (((Switch *)link->p)->Detection->reserved)) || 
         ((link->type == RAIL_LINK_M || link->type == RAIL_LINK_m) && (((MSSwitch *)link->p)->Detection->reserved)))){
        reserve_switch_path(tmp, *link, NEXT | SWITCH_CARE);
      }
    }
  }
}

void Algor_special_search_Blocks(struct algor_blocks * Blocks, int flags){
  loggerf(INFO, "Algor_special_search_Blocks %i:%i", Blocks->B->module, Blocks->B->id);
  struct next_prev_Block {
    Block * prev;
    uint8_t prev_l;
    Block * next;
    uint8_t next_l;
  };

  struct next_prev_Block pairs[10];
  memset(pairs, 0, 10*sizeof(struct next_prev_Block));
  uint8_t pair_counter = 0;

  for(int i = 0; i < Blocks->B->switch_len; i++){
    Block * BlA = Blocks->B;
    Block * BlB = Blocks->B;
    int a = Switch_to_rail(&BlA, Blocks->B->Sw[i], RAIL_LINK_S, 0);
    int b = Switch_to_rail(&BlB, Blocks->B->Sw[i], RAIL_LINK_s, 0);

    if(a == 0 || b == 0){
      continue;
    }

    // printf("%i:%i == %i:%i\n", BlA->module, BlA->id, BlB->module, BlB->id);

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
    if(Next(pairs[0].next, NEXT | SWITCH_CARE, pairs[0].next_l) == Blocks->B)
      a_dir |= PREV;
    else if(Next(pairs[0].next, PREV | SWITCH_CARE, pairs[0].next_l) == Blocks->B)
      a_dir |= NEXT;

    if(Next(pairs[0].prev, NEXT | SWITCH_CARE, pairs[0].prev_l) == Blocks->B)
      b_dir |= PREV;
    else if(Next(pairs[0].prev, PREV | SWITCH_CARE, pairs[0].prev_l) == Blocks->B)
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

    char debug_output[200];
    sprintf(debug_output, "A%i    ", a_dir);
    for(int ab = 0; ab < 15; ab++){
      if(Aside[ab])
        sprintf(debug_output, "%s%2i:%2i\t", debug_output, Aside[ab]->module, Aside[ab]->id);
    }
    loggerf(TRACE, "%s", debug_output);

    sprintf(debug_output, "B%i    ", a_dir);
    for(int ab = 0; ab < 15; ab++){
      if(Bside[ab])
        sprintf(debug_output, "%s%2i:%2i\t", debug_output, Bside[ab]->module, Bside[ab]->id);
    }
    loggerf(TRACE, "%s", debug_output);

    // Put all blocks into Algor blocks
    int8_t dir = -1;

    //Determine direction for A side and B side
    if(Aside[0] && Bside[0]){
      // If A and B have the same direction
      if((Aside[0]->dir == Bside[0]->dir || (Aside[0]->dir ^ 0b101) == Bside[0]->dir)){
        // As the block itself
        if(Aside[0]->dir == Blocks->B->dir || Bside[0]->dir == Blocks->B->dir){
          dir = (b_dir & 1) ^ 1;
        }
        // Or as reversed blocks
        else if((Aside[0]->dir ^ 0b1) == Blocks->B->dir || (Bside[0]->dir ^ 0b1) == Blocks->B->dir){
          dir = (b_dir & 1);
        }
      }

      // If A has the same direction as block
      // If A is reversed to block
      else if(Aside[0]->dir == Blocks->B->dir || (Aside[0]->dir ^ 0b101) == Blocks->B->dir){
        dir = (a_dir & 1);
      }

      // If B has the same direction as block
      // If B is reversed to block
      else if(Bside[0]->dir == Blocks->B->dir || (Bside[0]->dir ^ 0b101) == Blocks->B->dir){
        dir = (b_dir & 1) ^ 1;
      }
    }
    else if(Bside[0]){
      if(Bside[0]->dir == Blocks->B->dir || (Blocks->B->dir ^ 0b101) == Bside[0]->dir){
        dir = (b_dir & 1) ^ 1;
      }
    }
    else if(Aside[0]){
      if(Aside[0]->dir == Blocks->B->dir || (Blocks->B->dir ^ 0b101) == Aside[0]->dir){
        dir = (a_dir & 1);
      }
    }

    // Debug stuf
    if(dir == -1){
      loggerf(ERROR, "No direction found");
      return;
    }

    Algor_Block * Aside_P = 0;
    Algor_Block * Bside_P = 0;

    uint8_t a = 0, b = 0;

    for(int p = 0; p < 3; p++){
      if(dir == PREV){
        if(p == 0){
          Bside_P = Blocks->BN;
          Aside_P = Blocks->BP;
        }
        else if(p == 1){
          Bside_P = Blocks->BNN;
          Aside_P = Blocks->BPP;
        }
        else if(p == 2){
          Bside_P = Blocks->BNNN;
          Aside_P = Blocks->BPPP;
        }
      }
      else if(dir == NEXT){
        if(p == 0){
          Bside_P = Blocks->BP;
          Aside_P = Blocks->BN;
        }
        else if(p == 1){
          Bside_P = Blocks->BPP;
          Aside_P = Blocks->BNN;
        }
        else if(p == 2){
          Bside_P = Blocks->BPPP;
          Aside_P = Blocks->BNNN;
        }
      }

      Aside_P->blocks = 0;
      Bside_P->blocks = 0;

      do{
        if(!Bside[b] || !Bside_P)
          break;
        Bside_P->B[Bside_P->blocks] = Bside[b];
        Bside_P->length += Bside[b]->length;
        Bside_P->blocks++;
        b++;
      }
      while(Bside_P->length < Block_Minimum_Size && Bside_P->blocks < 5);
      
      do{
        if(!Aside[a] || !Aside_P)
          break;
        Aside_P->B[Aside_P->blocks] = Aside[a];
        Aside_P->length += Aside[a]->length;
        Aside_P->blocks++;
        a++;
      }
      while(Aside_P->length < Block_Minimum_Size && Aside_P->blocks < 5);
    }
  }
  else{
    loggerf(ERROR, "Zero or 1+ pairs");
  }
}

void Algor_search_Blocks(struct algor_blocks * AllBlocks, int debug){
  loggerf(TRACE, "Algor_search_Blocks - %02i:%02i", AllBlocks->B->module, AllBlocks->B->id);
  Block * next = 0;
  Block * prev = 0;
  Block * B = AllBlocks->B;

  Algor_clear_Blocks(AllBlocks);

  if(B->type == SPECIAL){
    Algor_special_search_Blocks(AllBlocks, debug);
    return;
  }

  loggerf(INFO, "Search blocks %02i:%02i", B->module, B->id);

  int next_level = 1;
  int prev_level = 1;

  next = Next(B, NEXT | SWITCH_CARE,1);
  prev = Next(B, PREV | SWITCH_CARE,1);

  //Select all surrounding blocks
  if(next){
    for(int i = 0; i < 3; i++){
      Algor_Block * block_p;
      if(i == 0)
        block_p = AllBlocks->BN;
      else if(i == 1)
        block_p = AllBlocks->BNN;
      else if(i == 2)
        block_p = AllBlocks->BNNN;

      block_p->blocks = 0;

      do{
        if(i == 0 && block_p->blocks == 0){
          block_p->B[block_p->blocks] = next;
          next_level++;
        }
        else{
          block_p->B[block_p->blocks] = Next(B, NEXT | SWITCH_CARE, next_level++);
        }

        if(!block_p->B[block_p->blocks]){
          i = 4;
          break;
        }

        if(block_p->B[block_p->blocks]->NextSignal || block_p->B[block_p->blocks]->PrevSignal)
          block_p->signal = 1;

        if(block_p->B[block_p->blocks]->switch_len)
          block_p->switches = 1;

        if(block_p->B[block_p->blocks]->blocked)
          block_p->blocked = 1;

        block_p->length += block_p->B[block_p->blocks]->length;

      block_p->blocks += 1;

      }
      while(block_p->length < Block_Minimum_Size && block_p->blocks < 5);
    }
  }
  if(prev){
    for(int i = 0; i < 3; i++){
      Algor_Block * block_p;
      if(i == 0)
        block_p = AllBlocks->BP;
      else if(i == 1)
        block_p = AllBlocks->BPP;
      else if(i == 2)
        block_p = AllBlocks->BPPP;
      block_p->blocks = 0;

    do{
      if(i == 0 && block_p->blocks == 0){
        block_p->B[block_p->blocks] = prev;
        prev_level++;
      }
      else{
        block_p->B[block_p->blocks] = Next(B, PREV | SWITCH_CARE, prev_level++);
      }

      if(!block_p->B[block_p->blocks]){
        i = 4;
        break;
      }

      if(block_p->B[block_p->blocks]->NextSignal || block_p->B[block_p->blocks]->PrevSignal)
        block_p->signal = 1;

      if(block_p->B[block_p->blocks]->switch_len)
        block_p->switches = 1;

      if(block_p->B[block_p->blocks]->blocked)
        block_p->blocked = 1;

      block_p->length += block_p->B[block_p->blocks]->length;

      block_p->blocks += 1;

      }
      while(block_p->length < Block_Minimum_Size && block_p->blocks < 5);
    }
  }
}

void Algor_train_following(struct algor_blocks AllBlocks, int debug){
  loggerf(TRACE, "Algor_train_following");
  //Unpack AllBlocks
  // Algor_Block BPPP = *AllBlocks.BPPP;
  // Algor_Block BPP  = *AllBlocks.BPP;
  Algor_Block BP   = *AllBlocks.BP;
  Block * B        =  AllBlocks.B;
  Algor_Block BN   = *AllBlocks.BN;
  Algor_Block BNN  = *AllBlocks.BNN;
  // Algor_Block BNNN = *AllBlocks.BNNN;

  if(!B->blocked && B->train != 0){
    //Reset
    B->train = 0;

    loggerf(DEBUG, "RESET Train block %i:%i", B->module, B->id);
    Units[B->module]->changed |= Unit_Blocks_changed;
  }
  else if(B->blocked && B->train == 0){
    Units[B->module]->changed |= Unit_Blocks_changed;
  }
  // else if(B->blocked && B->train != 0 && train_link[B->train] && !train_link[B->train]->Block){
  //   // Set block of train
  //   train_link[B->train]->Block = B;
  //   if(debug) printf("SET_BLOCK");
  // }

  // else if(B->blocked && BNN.blocks > 0 && !BN.blocked && !BNN.blocked){

  // }

  if(B->blocked && BN.blocks > 0){
    //If only current and next blocks are occupied
    // Reverse immediate block
    if(((BP.blocks > 0 && !BP.blocked) || BP.blocks == 0) && BN.blocked && BN.B[0]->train && !B->train){
      //REVERSED
      Block_Reverse(B);
      loggerf(WARNING, "REVERSE BLOCK %i:%i", B->module, B->id);
      B->changed |= IO_Changed;
      // Block_Reverse_To_Next_Switch(B);
      // return;
    }

    if(BN.blocks > 0 && !BN.blocked && !dircmp(B, BN.B[0])){
      Block_Reverse(BN.B[0]);
      putAlgorQueue(BN.B[0], 1);
    }

    if(BN.B[0]->type == SPECIAL){
      loggerf(ERROR, "Blocked and next is switch lane %02i:%02i", B->module, B->id);
      for(int i = 1; i <= BN.blocks; i++){
        if(BN.blocks > i){
          if(BN.B[i]->type != SPECIAL){
            if(BN.B[i]->reserved != 0)
              break;

            if(!dircmp(B, BN.B[i])){
              loggerf(WARNING, "REVERSE BLOCK %i:%i after switchlane", BN.B[i]->module, BN.B[i]->id);
              BN.B[i]->dir ^= 0b100;
              BN.B[i]->state = RESERVED;
              Block_reserve(BN.B[i]);
              BN.B[i]->changed |= IO_Changed | Block_Algor_Changed | State_Changed;
              process(BN.B[i], 3 | NO_LOCK);
              //void Block_Reverse(B);
              Block_Reverse_To_Next_Switch(BN.B[i]);
            }
            else{
              //reserve untill next switchlane
              Block_reserve(BN.B[i]);
              BN.B[i]->state = RESERVED;
              BN.B[i]->changed |= State_Changed;
              Reserve_To_Next_Switch(BN.B[i]);
            }
            break;
          }
        }
        else if(BN.blocks == i && BNN.blocks > 0 && BN.B[0]->type != SPECIAL){
          if(BN.B[i]->reserved != 0)
            break;

          if(!dircmp(B, BNN.B[0])){
            loggerf(WARNING, "REVERSE BLOCK %i:%i after switchlane", BNN.B[0]->module, BNN.B[0]->id);
            BNN.B[0]->dir ^= 0b100;
            BNN.B[0]->state = RESERVED;
            Block_reserve(BN.B[i]);
            BNN.B[0]->changed |= IO_Changed | Block_Algor_Changed | State_Changed;
            process(BNN.B[0], 3 | NO_LOCK);
            Block_Reverse_To_Next_Switch(BNN.B[0]);
          }
          else{
            //reserve untill next switchlane
            Block_reserve(BN.B[i]);
            BN.B[i]->state = RESERVED;
            BN.B[i]->changed |= State_Changed;
            Reserve_To_Next_Switch(BN.B[i]);
          }
        }
      }
    }
  }


  // Check for new or split train
  // New train: If no surrounding blocks are occupied
  if(BP.blocks > 0 && BN.blocks > 0 && B->blocked && B->train == 0 && BN.B[0]->train == 0 && BP.B[0]->train == 0){
    //NEW TRAIN
    // find a new follow id
    // loggerf(ERROR, "FOLLOW ID INCREMENT, bTrain");
    B->train = new_railTrain();


    if(B->reserved){
      Block_dereserve(B);
    }

    //Create a message for WebSocket
    WS_NewTrain(B->train, B->module, B->id);
    loggerf(INFO, "NEW_TRAIN at %i\t", B->train);
  }

  // Split train: If current block is unoccupied and surrounding are occupied and have the same train pointer
  else if(BN.blocks > 0 && BP.blocks > 0 && BN.blocked && BP.blocked && !B->blocked && BN.B[0]->train == BP.B[0]->train){
    //A train has split
    WS_TrainSplit(BN.B[0]->train, BP.B[0]->module,BP.B[0]->id,BN.B[0]->module,BN.B[0]->id);
    loggerf(INFO, "SPLIT_TRAIN");
  }

  // If only current and prev blocks are occupied
  // and if next block is reversed
  if(BP.blocks > 0 && BN.blocks > 0 && B->blocked && BP.blocked && !BN.B[0]->blocked && !dircmp(B, BN.B[0])){
    //Reversed ahead
    loggerf(INFO, "%i:%i Reversed ahead (%i, %i)", B->module, B->id, BN.B[0]->module, BN.B[0]->id);
    BN.B[0]->dir ^= 0b100;
    // Block_Reverse_To_Next_Switch(BN.B[0]);
  }

  if(BP.blocks > 0 && BP.blocked && B->blocked && B->train == 0 && BP.B[0]->train != 0){
    // Copy train id from previous block
    B->train = BP.B[0]->train;

    if(B->reserved){
      Block_dereserve(B);
    }
    // if(train_link[B->train])
    //   train_link[B->train]->Block = B;
    loggerf(INFO, "COPY_TRAIN from %i:%i to %i:%i", BP.B[0]->module, BP.B[0]->id, B->module, B->id);
  }
}

void Algor_GetBlocked_Blocks(struct algor_blocks AllBlocks){
  //Scroll through all the pointers of allblocks
  for(int i = 0; i < 7; i++){
    if(i == 3)
      continue;

    Algor_Block * ABl = ((void **)&AllBlocks)[i];
    ABl->blocked = 0;
    for(int j = 0; j < ABl->blocks; j++){
      if(!ABl->B[j]){
        loggerf(ERROR, "Empty allblocks entry");
        continue;
      }
      if(ABl->B[j]->blocked){
        ABl->blocked = 1;
      }
    }
  }
}

void Algor_rail_state(struct algor_blocks AllBlocks, int debug){
  loggerf(TRACE, "Algor_rail_state");
  //Unpack AllBlocks
  Algor_Block BPPP = *AllBlocks.BPPP;
  Algor_Block BPP  = *AllBlocks.BPP;
  Algor_Block BP   = *AllBlocks.BP;
  Block * B        =  AllBlocks.B;
  Algor_Block BN   = *AllBlocks.BN;
  Algor_Block BNN  = *AllBlocks.BNN;
  // Algor_Block BNNN = *AllBlocks.BNNN;

  if(!B->blocked){
    if(B->state != RESERVED || B->state != RESERVED_SWITCH){
      B->reverse_state = PROCEED;
    }
    else{
      B->reverse_state = DANGER;
    }
  }

  if(B->blocked && !BP.blocked && BP.blocks > 0){
    if(BP.blocks > 0 && dircmp(B, BP.B[0]))
      Algor_apply_rail_state(BP, DANGER);
    if(BPP.blocks > 0 && !BP.blocked && dircmp(B, BPP.B[0]))
      Algor_apply_rail_state(BPP, CAUTION);

    if(BPPP.blocks > 0 && !BP.blocked && !BPP.blocked && dircmp(B, BPPP.B[0]))
      Algor_apply_rail_state(BPPP, PROCEED);
  }
  else if(!B->blocked && BN.blocks == 0){
    if(B->type != SPECIAL){
      B->state = CAUTION;
      B->changed |= State_Changed;
      // printf(" End of track %i:%i ",B->module, B->id);
      if(!BP.blocked && BP.blocks > 0 && dircmp(B, BP.B[0]))
        Algor_apply_rail_state(BP, PROCEED);

      if(!BPP.blocked && BPP.blocks > 0 && dircmp(B, BPP.B[0]))
        Algor_apply_rail_state(BPP, PROCEED);
    }
    //B->type == SPECIAL
    else{
      loggerf(INFO, "DANGER switch block %i:%i", B->module, B->id);
      B->state = DANGER;
      B->changed |= State_Changed;

      for(int i = 0; i < BP.blocks; i++){
        if(BP.B[i]->type != SPECIAL){
          BP.B[i]->state = CAUTION;
          BP.B[i]->changed |= State_Changed;
          break;
        }

        BP.B[i]->state = DANGER;
        BP.B[i]->changed |= State_Changed;
      }
    }
  }
  else if(!B->blocked && !BN.blocked && BN.blocks > 0 && BN.B[0]->state == RESERVED && !dircmp(B, BN.B[0])){
    B->state = DANGER;
    B->changed |= State_Changed;

    if(BP.blocks > 0 && !BP.blocked){
      BP.B[0]->state = CAUTION;
      BP.B[0]->changed |= State_Changed;
    }
  }
  else if(!B->blocked && !BN.blocked && !BNN.blocked && !BP.blocked && !BPP.blocked && BN.blocks > 0 && BP.blocks > 0 && B->state != RESERVED && B->state != RESERVED_SWITCH && BN.B[0]->state == PROCEED){
    // Algor_print_block_debug(AllBlocks);
    B->state = PROCEED;
    B->changed |= State_Changed;
  }
}

void Algor_apply_rail_state(Algor_Block b, enum Rail_states state){
  loggerf(TRACE, "Algor_apply_rail_state");
  uint8_t dir = b.B[0]->dir;
  for(int i = 0; i < b.blocks; i++){
    if(dir == b.B[i]->dir || dir == (b.B[i]->dir ^ 0b101) || dir == (b.B[i]->dir ^ 0b10)){
      if(b.B[i]->blocked)
        return;
      else if(state != PROCEED)
        b.B[i]->state = state;
      else if(b.B[i]->reserved > 0){
        if(b.B[i]->type == SPECIAL)
          b.B[i]->state = RESERVED_SWITCH;
        else
          b.B[i]->state = RESERVED;
      }
      else
        b.B[i]->state = PROCEED;
      b.B[i]->changed |= State_Changed;
      Units[b.B[i]->module]->block_state_changed |= 1;
    }
    else{
      break;
    }
  }
}

void Algor_signal_state(struct algor_blocks AllBlocks, int debug){
  loggerf(TRACE, "Algor_signal_state");
  //Unpack AllBlocks
  // Algor_Block BPPP = *AllBlocks.BPPP;
  // Algor_Block BPP  = *AllBlocks.BPP;
  // Algor_Block BP   = *AllBlocks.BP;
  Block * B        =  AllBlocks.B;

  Algor_Block tmp;
  tmp.blocks = 1;
  tmp.B[0] = B;

  if(B->NextSignal || B->PrevSignal)
    tmp.signal = 1;

  // Algor_Block BN   = *AllBlocks.BN;
  // Algor_Block BNN  = *AllBlocks.BNN;
  // Algor_Block BNNN = *AllBlocks.BNNN;
  Algor_Block * ABlocks[7];
  ABlocks[0] = AllBlocks.BPPP;
  ABlocks[1] = AllBlocks.BPP;
  ABlocks[2] = AllBlocks.BP;
  ABlocks[3] = &tmp;
  ABlocks[4] = AllBlocks.BN;
  ABlocks[5] = AllBlocks.BNN;
  ABlocks[6] = AllBlocks.BNNN;

  for(int i = 0; i <= 6; i++){
    if(ABlocks[i]->signal){

      for(int j = 0; j < ABlocks[i]->blocks; j++){
        if(ABlocks[i]->B[j]->NextSignal)
          check_Signal(ABlocks[i]->B[j]->NextSignal);
        if(ABlocks[i]->B[j]->PrevSignal)
          check_Signal(ABlocks[i]->B[j]->PrevSignal);
      }

    }
  }

  // for(int i = 0; i < 6; i++){
  //   for(int j = 0; j < ABlocks[i]->blocks; j++){
  //     if(ABlocks[i]->B[j]->NextSignal && i < 5){
  //       if(dircmp(AllBlocks.B, ABlocks[i]->B[j])){
  //         if(B->dir == 0 || B->dir == 2 || B->dir == 3){
  //           if(j == ABlocks[i]->blocks - 1){
  //             loggerf(INFO, "Signal at %02i:%02i", ABlocks[i]->B[j]->module, ABlocks[i]->B[j]->id);
  //             Algor_print_block_debug(AllBlocks);
  //             set_signal(ABlocks[i]->B[j]->NextSignal, ABlocks[i+1]->B[0]->state);
  //           }
  //           else{
  //             loggerf(INFO, "Signal at %02i:%02i", ABlocks[i]->B[j]->module, ABlocks[i]->B[j]->id);
  //             Algor_print_block_debug(AllBlocks);
  //             set_signal(ABlocks[i]->B[j]->NextSignal, ABlocks[i]->B[j+1]->state);
  //           }
  //         }
  //         else{

  //         }
  //       }
  //     }

  //     if(ABlocks[i]->B[j]->PrevSignal && i > 0){
  //       if(dircmp(AllBlocks.B, ABlocks[i]->B[j])){
  //         if(j == 0){
  //           loggerf(INFO, "Signal at %02i:%02i", ABlocks[i]->B[j]->module, ABlocks[i]->B[j]->id);
  //           Algor_print_block_debug(AllBlocks);
  //           set_signal(ABlocks[i]->B[j]->PrevSignal, ABlocks[i-1]->B[ABlocks[i-1]->blocks - 1]->state);
  //         }
  //         else{
  //           loggerf(INFO, "Signal at %02i:%02i", ABlocks[i]->B[j]->module, ABlocks[i]->B[j]->id);
  //           Algor_print_block_debug(AllBlocks);
  //           set_signal(ABlocks[i]->B[j]->PrevSignal, ABlocks[i]->B[j-1]->state);
  //         }
  //       }
  //     }
  //   }
  // }
}

void Algor_train_control(struct algor_blocks AllBlocks, int debug){
  loggerf(TRACE, "Algor_train_control");
  //Unpack AllBlocks
  Block * B        =  AllBlocks.B;
  Algor_Block BN   = *AllBlocks.BN;
  Algor_Block BNN  = *AllBlocks.BNN;

  RailTrain * T = B->train;

  if(T->target_speed > BN.B[0]->max_speed || T->speed > BN.B[0]->max_speed){
    loggerf(WARNING, "Next block speed limit");
    train_change_speed(T, BN.B[0]->max_speed, GRADUAL_SLOW_SPEED);
  }
  else if(BN.blocked){
    loggerf(WARNING, "Train Next block Blocked");
    train_change_speed(T, 0, IMMEDIATE_SPEED);
  }
  else if(BN.B[0]->state == DANGER){
    loggerf(WARNING, "Train Next block Blocked");
    train_change_speed(T, 0, GRADUAL_FAST_SPEED);
  }
  else if(BN.B[0]->state == CAUTION){
    if(T->speed > CAUTION_SPEED){
      loggerf(WARNING, "Train Next block Caution");
      train_change_speed(T, CAUTION_SPEED, GRADUAL_FAST_SPEED);
    }
  }
  else if(BN.B[0]->max_speed > T->speed && 
          ((BN.blocks > 1 && BN.B[1]->max_speed >= BN.B[0]->max_speed) || 
           (BN.blocks == 1 &&  BNN.blocks > 0 && BNN.B[0]->max_speed >= BN.B[0]->max_speed))) {
    loggerf(WARNING, "Train Speed Up");
    train_change_speed(T, BN.B[0]->max_speed, GRADUAL_FAST_SPEED);
  }

}


void procces_accessoire(){
  for(int i = 0;i<unit_len;i++){
  //TODO FIX, Output_changed into Node_changed
  /*
  if(Units[i] && Units[i]->output_changed){
    printf("Output of module %i changed\n",i);
    for(int j = 0;j<Units[i]->IO_Nodes;j++){
    for(int k = 0;k < Units[i]->Node[j].ioports; k++){
      printf("Signal id: %i\n",Units[i]->Sig[j]->id);
    }
    }

    Units[i]->output_changed = FALSE;
  }
  */
  Units[i]->io_out_changed = FALSE;
  }
}

int init_connect_Algor(struct ConnectList * List){
  // printf("init_connect_Algor\n");
  int return_value = 0;
  for(int i = 0;i < unit_len;i++){
  if(!Units[i])
    continue;

  for(int j = 0;j < Units[i]->block_len; j++){
    if(!Units[i]->B[j])
    continue;

    if(Units[i]->B[j]->next.type == RAIL_LINK_C || Units[i]->B[j]->prev.type == RAIL_LINK_C){
    printf("found block %i:%i\n",i,j);
    if(List->list_index <= List->length + 1){
      struct rail_link ** temp = _calloc(List->list_index+8, struct rail_link *);
      for(int q = 0;q < List->list_index;q++){
      temp[q] = List->R_L[q];
      }
      _free(List->R_L);
      List->R_L = temp;
      List->list_index += 8;
    }
    // printf("write index: %i\n",List->length);
    List->R_L[List->length] = _calloc(1, struct rail_link);
    List->R_L[List->length]->type = 'R';
    List->R_L[List->length++]->p  = Units[i]->B[j];
    }
  }

  for(int j = 0;j < Units[i]->switch_len; j++){
    if(!Units[i]->Sw[j])
    continue;

    if(Units[i]->Sw[j]->app.type == RAIL_LINK_C || Units[i]->Sw[j]->div.type == RAIL_LINK_C || Units[i]->Sw[j]->str.type == RAIL_LINK_C){
    printf("module %i, switch %i\n",i,j);
    if(List->list_index <= List->length + 1){
      struct rail_link ** temp = _calloc(List->list_index+8, struct rail_link *);
      for(int q = 0;q < List->list_index;q++){
      temp[q] = List->R_L[q];
      }
      _free(List->R_L);
      List->R_L = temp;
      List->list_index += 8;
    }
    List->R_L[List->length] = _calloc(1, struct rail_link);
    List->R_L[List->length]->type = 'S';
    List->R_L[List->length++]->p  = Units[i]->Sw[j];
    }
  }

  return_value += Units[i]->connections_len;
  }
  return return_value;
}

_Bool find_and_connect(uint8_t ModuleA, char anchor_A, uint8_t ModuleB, char anchor_B){
  //Node shouldn't be connected to the same Module
  if(ModuleA == ModuleB){return FALSE;}

  char typeA = 0;
  char typeB = 0;

  _Bool connected = FALSE;

  // printf("find_and_connect: %i:%i\t\t%i:%i\n",ModuleA,anchor_A,ModuleB,anchor_B);

  for(int Rail = 1;Rail<3;Rail++){
    struct rail_link A;A.p = 0;
    struct rail_link B;B.p = 0;

    //Find Anchor A
    // - Find a Block
      for(int k = 0;k<Units[ModuleA]->block_len;k++){
      if(Units[ModuleA]->B[k]){
          if(Units[ModuleA]->B[k]->prev.type == RAIL_LINK_C){
          // printf(" - A block Prev %i:%i",ModuleA,k);
          if(Units[ModuleA]->B[k]->prev.module == anchor_A && Units[ModuleA]->B[k]->prev.id == Rail){
            // printf("++++++\n");
            A.type = RAIL_LINK_R;
            typeA  = 'P';
            A.p = Units[ModuleA]->B[k];
            break;
          }
          // printf("\n");
          }
          else if(Units[ModuleA]->B[k]->next.type == RAIL_LINK_C){
          // printf(" - A block Next %i:%i",ModuleA,k);
          if(Units[ModuleA]->B[k]->next.module == anchor_A && Units[ModuleA]->B[k]->next.id == Rail){
            // printf("++++++\n");
            A.type = RAIL_LINK_R;
            typeA  = 'N';
            A.p = Units[ModuleA]->B[k];
            break;
          }
          // printf("\n");
          }
        }
      }
    // - Find a switch
      if(!A.p){
        for(int k = 0;k<Units[ModuleA]->switch_len;k++){
          if(Units[ModuleA]->Sw[k]){
            if(Units[ModuleA]->Sw[k]->app.type == RAIL_LINK_C){
              // printf(" - A Switch App %i:%i",ModuleA,k);
              if(Units[ModuleA]->Sw[k]->app.module == anchor_A && Units[ModuleA]->Sw[k]->app.id == Rail){
                // printf("++++++\n");
                A.type = RAIL_LINK_S;
                typeA  = 'A';
                A.p = Units[ModuleA]->Sw[k];
                break;
              }
              // printf("\n");
            }
            else if(Units[ModuleA]->Sw[k]->str.type == RAIL_LINK_C){
              // printf(" - A Switch Str %i:%i",ModuleA,k);
              if(Units[ModuleA]->Sw[k]->str.module == anchor_A && Units[ModuleA]->Sw[k]->str.id == Rail){
                // printf("++++++\n");
                A.type = RAIL_LINK_S;
                typeA  = 'S';
                A.p = Units[ModuleA]->Sw[k];
                break;
              }
              // printf("\n");
            }
            else if(Units[ModuleA]->Sw[k]->div.type == RAIL_LINK_C){
              // printf(" - A Switch Div %i:%i",ModuleA,k);
              if(Units[ModuleA]->Sw[k]->div.module == anchor_A && Units[ModuleA]->Sw[k]->div.id == Rail){
                // printf("++++++\n");
                A.type = RAIL_LINK_S;
                typeA  = 'D';
                A.p = Units[ModuleA]->Sw[k];
                break;
              }
              // printf("\n");
            }
          }
        }
      }
    // - Find a msswitch
      if(!A.p){
        printf("Mayby a MSwitch, but not IMPLEMENTED!!\n");
        continue;
      }

    //Find Anchor B
    // - Find a block
      for(int k = 0;k<Units[ModuleB]->block_len;k++){
        if(Units[ModuleB]->B[k]){
          if(Units[ModuleB]->B[k]->next.type == RAIL_LINK_C){
          printf(" - B block Prev %i:%i",ModuleB,k);
          if(Units[ModuleB]->B[k]->next.module == anchor_B && Units[ModuleB]->B[k]->next.id == Rail){
            printf("++++++\n");
            B.type = RAIL_LINK_R;
            typeB  = 'N';
            B.p = Units[ModuleB]->B[k];
            break;
          }
          printf("\n");
          }
          else if(Units[ModuleB]->B[k]->prev.type == RAIL_LINK_C){
          printf(" - B block Prev %i:%i",ModuleB,k);
          if(Units[ModuleB]->B[k]->prev.module == anchor_B && Units[ModuleB]->B[k]->prev.id == Rail){
            printf("++++++\n");
            B.type = RAIL_LINK_R;
            typeB  = 'P';
            B.p = Units[ModuleB]->B[k];
            break;
          }
          printf("\n");
          }
        }
      }
    // - Find a Switch
      if(!B.p){
        for(int k = 0;k<Units[ModuleB]->switch_len;k++){
          if(Units[ModuleB]->Sw[k]){
            if(Units[ModuleB]->Sw[k]->app.type == RAIL_LINK_C){
              printf(" - B switch App %i:%i",ModuleB,k);
              if(Units[ModuleB]->Sw[k]->app.module == anchor_B && Units[ModuleB]->Sw[k]->app.id == Rail){
              printf("++++++\n");
              B.type = RAIL_LINK_S;
              typeB  = 'A';
              B.p = Units[ModuleB]->Sw[k];
              break;
              }
              printf("\n");
            }
            else if(Units[ModuleB]->Sw[k]->str.type == RAIL_LINK_C){
              printf(" - B switch Str %i:%i",ModuleB,k);
              if(Units[ModuleB]->Sw[k]->str.module == anchor_B && Units[ModuleB]->Sw[k]->str.id == Rail){
              printf("++++++\n");
              B.type = RAIL_LINK_S;
              typeB  = 'S';
              B.p = Units[ModuleB]->Sw[k];
              break;
              }
              printf("\n");
            }
            else if(Units[ModuleB]->Sw[k]->div.type == RAIL_LINK_C){
              printf(" - B switch Div %i:%i",ModuleB,k);
              if(Units[ModuleB]->Sw[k]->div.module == anchor_B && Units[ModuleB]->Sw[k]->div.id == Rail){
              printf("++++++\n");
              B.type = RAIL_LINK_S;
              typeB  = 'D';
              B.p = Units[ModuleB]->Sw[k];
              break;
              }
              printf("\n");
            }
          }
        }
      }
    // - Find a MSwitch
      if(!B.p){
        printf("Mayby a MSwitch, but not IMPLEMENTED!!\n");
        continue;
      }

    if(A.type == RAIL_LINK_R && B.type == RAIL_LINK_R){
      printf("Connecting R %i:%i <==> %i:%i R\n",((Block *)A.p)->module,((Block *)A.p)->id,((Block *)B.p)->module,((Block *)B.p)->id);
    }
    else if(A.type == RAIL_LINK_S && B.type == RAIL_LINK_R){
      printf("Connecting S %i:%i <==> %i:%i R\n",((Switch *)A.p)->module,((Switch *)A.p)->id,((Block *)B.p)->module,((Block *)B.p)->id);
    }
    else if(A.type == RAIL_LINK_R && B.type == RAIL_LINK_S){
      printf("Connecting R %i:%i <==> %i:%i S\n",((Block *)A.p)->module,((Block *)A.p)->id,((Switch *)B.p)->module,((Switch *)B.p)->id);
    }

    connected = TRUE;

    //Connect
    if(A.type == RAIL_LINK_R && B.type == RAIL_LINK_R){
      if(typeA == 'P'){
        ((Block *)A.p)->prev.module = ((Block *)B.p)->module;
        ((Block *)A.p)->prev.id    = ((Block *)B.p)->id;
        ((Block *)A.p)->prev.type   = RAIL_LINK_R;
        ((Block *)B.p)->next.module = ((Block *)A.p)->module;
        ((Block *)B.p)->next.id    = ((Block *)A.p)->id;
        ((Block *)B.p)->next.type   = RAIL_LINK_R;
      }
      else{
        ((Block *)A.p)->next.module = ((Block *)B.p)->module;
        ((Block *)A.p)->next.id    = ((Block *)B.p)->id;
        ((Block *)A.p)->next.type   = RAIL_LINK_R;
        ((Block *)B.p)->prev.module = ((Block *)A.p)->module;
        ((Block *)B.p)->prev.id    = ((Block *)A.p)->id;
        ((Block *)B.p)->prev.type   = RAIL_LINK_R;
      }
    }
    else if(A.type == RAIL_LINK_S && B.type == RAIL_LINK_R){
      if(typeB == 'N'){
        ((Block *)B.p)->next.module = ((Switch *)A.p)->module;
        ((Block *)B.p)->next.id    = ((Switch *)A.p)->id;
        ((Block *)B.p)->next.type   = (typeA == 'A') ? RAIL_LINK_S : RAIL_LINK_s;
      }
      else{
        ((Block *)B.p)->prev.module = ((Switch *)A.p)->module;
        ((Block *)B.p)->prev.id    = ((Switch *)A.p)->id;
        ((Block *)B.p)->prev.type   = (typeA == 'A') ? RAIL_LINK_S : RAIL_LINK_s;
      }

      if(typeA == 'A'){
        ((Switch *)A.p)->app.module = ((Block *)B.p)->module;
        ((Switch *)A.p)->app.id    = ((Block *)B.p)->id;
        ((Switch *)A.p)->app.type   = RAIL_LINK_R;
      }
      else if(typeA == 'S'){
        ((Switch *)A.p)->str.module = ((Block *)B.p)->module;
        ((Switch *)A.p)->str.id    = ((Block *)B.p)->id;
        ((Switch *)A.p)->str.type   = RAIL_LINK_R;
      }
      else if(typeA == 'D'){
        ((Switch *)A.p)->div.module = ((Block *)B.p)->module;
        ((Switch *)A.p)->div.id    = ((Block *)B.p)->id;
        ((Switch *)A.p)->div.type   = RAIL_LINK_R;
      }
    }
    else if(A.type == RAIL_LINK_R && B.type == RAIL_LINK_S){
      if(typeA == 'N'){
        ((Block *)A.p)->next.module = ((Switch *)B.p)->module;
        ((Block *)A.p)->next.id    = ((Switch *)B.p)->id;
        ((Block *)A.p)->next.type   = (typeB == 'A') ? RAIL_LINK_S : RAIL_LINK_s;
      }
      else{
        ((Block *)A.p)->prev.module = ((Switch *)B.p)->module;
        ((Block *)A.p)->prev.id    = ((Switch *)B.p)->id;
        ((Block *)A.p)->prev.type   = (typeB == 'A') ? RAIL_LINK_S : RAIL_LINK_s;
      }

      if(typeB == 'A'){
        ((Switch *)B.p)->app.module = ((Block *)A.p)->module;
        ((Switch *)B.p)->app.id    = ((Block *)A.p)->id;
        ((Switch *)B.p)->app.type   = RAIL_LINK_R;
      }
      else if(typeB == 'S'){
        ((Switch *)B.p)->str.module = ((Block *)A.p)->module;
        ((Switch *)B.p)->str.id    = ((Block *)A.p)->id;
        ((Switch *)B.p)->str.type   = RAIL_LINK_R;
      }
      else if(typeB == 'D'){
        ((Switch *)B.p)->div.module = ((Block *)A.p)->module;
        ((Switch *)B.p)->div.id    = ((Block *)A.p)->id;
        ((Switch *)B.p)->div.type   = RAIL_LINK_R;
      }
    }
  }

  if(connected && ModuleA && anchor_A && ModuleB && anchor_B){
    Units[ModuleA]->connection[anchor_A-1] = Units[ModuleB];
    Units[ModuleB]->connection[anchor_B-1] = Units[ModuleA];
  }

  return connected;
}

int connect_Algor(struct ConnectList * List){
  struct rail_link * R = 0;

  int value = 0;

  for(int i = 0;i<List->length;i++){
    if(!List->R_L[i]->p)
      continue;

    if(List->R_L[i]->type == 'R'){
      if(((Block *)List->R_L[i]->p)->next.type != RAIL_LINK_C && ((Block *)List->R_L[i]->p)->prev.type != RAIL_LINK_C) {
        value++;
        continue;
      }
      if(((Block *)List->R_L[i]->p)->blocked){
        printf("Found block %i:%i %i\t",((Block*)List->R_L[i]->p)->module,((Block*)List->R_L[i]->p)->id,((Block*)List->R_L[i]->p)->blocked);
        //Blocked block
        if(!R)
          R = List->R_L[i];
        else
        {
          _Bool connected = FALSE;
          char anchor_A = 0;
          char anchor_B = 0;

          int ModuleA = 0;
          int ModuleB = 0;
          if(R->type == 'R'){
            ModuleA = ((Block *)R->p)->module;
            ModuleB = ((Block *)List->R_L[i]->p)->module;

            if(((Block *)R->p)->next.type == RAIL_LINK_C){
              anchor_A = ((Block *)R->p)->next.module;
              anchor_B = ((Block *)List->R_L[i]->p)->prev.module;
            }
            else if(((Block *)R->p)->prev.type == RAIL_LINK_C){
              anchor_A = ((Block *)R->p)->prev.module;
              anchor_B = ((Block *)List->R_L[i]->p)->next.module;
            }

            connected = find_and_connect(ModuleA,anchor_A,ModuleB,anchor_B);
          }
          else if(R->type == 'S'){
            ModuleA = ((Switch *)R->p)->module;
            ModuleB = ((Block *)List->R_L[i]->p)->module;
            if(((Block *)List->R_L[i]->p)->next.type == RAIL_LINK_C){
              anchor_B = ((Block *)List->R_L[i]->p)->next.module;
            }
            else{
              anchor_B = ((Block *)List->R_L[i]->p)->prev.module;
            }

            if(((Switch *)R->p)->app.type == RAIL_LINK_C){
              anchor_A = ((Switch *)R->p)->app.module;
            }
            else if(((Switch *)R->p)->str.type == RAIL_LINK_C){
              anchor_A = ((Switch *)R->p)->str.module;
            }
            else if(((Switch *)R->p)->div.type == RAIL_LINK_C){
              anchor_A = ((Switch *)R->p)->div.module;
            } //End Switch approach type

            connected = find_and_connect(ModuleA,anchor_A,ModuleB,anchor_B);
          } // End Switch type

          if(connected){
            WS_Partial_Layout(ModuleA,ModuleB);
            connected = FALSE;
          }
        }
      }
    }
    else if(List->R_L[i]->type == 'S' && ((Switch *)List->R_L[i]->p)->Detection->blocked){
      //Blocked switch
      if(!R){
        R = List->R_L[i];
      }
    }
  }

  value = 0;
  int total = 0;

  for(int i = 0;i < unit_len;i++){
    if(!Units[i])
      continue;

    for(int j = 0;j<Units[i]->connections_len;j++){
      total++;
      if(Units[i]->connection[j]){
      value++;
      }
    }
  }

  if(value == total){
    _SYS_change(STATE_Modules_Coupled, 1);
  }

  return value;
}
