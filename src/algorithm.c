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
  mutex_lock(&mutex_lockA, "Lock Mutex A");
  #ifdef En_UART
  for(int i = 0;i<strlen(List_of_Modules);i++){
    printf("R%i ",List_of_Modules[i]);
    struct COM_t C;
    memset(C.Data,0,32);
    C.Adr = List_of_Modules[i];
    C.Opcode = 6;
    C.Length = 0;

    pthread_mutex_lock(&mutex_UART);
    COM_Send(C);
    char COM_data[20];
    memset(COM_data,0,20);
    COM_Recv(COM_data);
    COM_Parse(COM_data);
    pthread_mutex_unlock(&mutex_UART);
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
  mutex_unlock(&mutex_lockA, "UNLock Mutex A");
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
  mutex_lock(&mutex_lockA, "Lock Mutex A");
  for(int i = 0;i<unit_len;i++){
  if(Units[i]){
    for(int j = 0;j<=Units[i]->block_len;j++){
    if(Units[i]->B[j]){
      //printf("%i:%i\n",i,j);
      process(Units[i]->B[j], 2);
    }
    }
  }
  }

  WS_trackUpdate(0);
  mutex_unlock(&mutex_lockA, "UNLock Mutex A");
}

sem_t AlgorQueueNoEmpty;
pthread_mutex_t AlgorQueueMutex;
struct s_AlgorQueue AlgorQueue;

void putAlgorQueue(Block * B, int enableQueue){
  mutex_lock(&AlgorQueueMutex, "Lock AlgorQueueMutex");
  AlgorQueue.B[AlgorQueue.writeIndex++] = B;

  int val;
  sem_getvalue(&AlgorQueueNoEmpty, &val);
  if(val == 0 && enableQueue){
    loggerf(INFO, "Sem_Post");
    sem_post(&AlgorQueueNoEmpty);
  }

  if(AlgorQueue.writeIndex == AlgorQueueLength)
    AlgorQueue.writeIndex = 0;
  mutex_unlock(&AlgorQueueMutex, "UnLock AlgorQueueMutex");
}

void putList_AlgorQueue(struct algor_blocks AllBlocks, int enable){
  putAlgorQueue(AllBlocks.B, 1);

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
  mutex_lock(&AlgorQueueMutex, "Lock AlgorQueueMutex");
  if(AlgorQueue.writeIndex == AlgorQueue.readIndex)
    result = 0;
  else
    result = AlgorQueue.B[AlgorQueue.readIndex++];

  if(AlgorQueue.readIndex == AlgorQueueLength)
    AlgorQueue.readIndex = 0;

  mutex_unlock(&AlgorQueueMutex, "UnLock AlgorQueueMutex");

  return result;
}

void processAlgorQueue(){
  Block * B = getAlgorQueue();
  while(B != 0){
    loggerf(TRACE, "Process %i:%i, %x, %x", B->module, B->id, B->changed, B->state);
    process(B, 2);
     if(B->changed & IO_Changed){
      loggerf(TRACE, "ReProcess");
      process(B, 1);
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
  
  usleep(2000000);
  _SYS->LC_State = _SYS_LC_Searching;
  WS_stc_SubmoduleState();
  JoinModules();
  usleep(10000000);
  _SYS->LC_State = _SYS_LC_Connecting;
  WS_stc_SubmoduleState();
  Connect_Rail_links();
  usleep(8000000);
  _SYS->LC_State = _SYS_Module_Run;
  WS_stc_SubmoduleState();
  scan_All();
  usleep(100000);

  while(_SYS->LC_State == _SYS_Module_Run){
    sem_wait(&AlgorQueueNoEmpty);
    processAlgorQueue();

    mutex_lock(&algor_mutex, "Algor Mutex");
    //Notify clients
    WS_trackUpdate(0);
    WS_SwitchesUpdate(0);

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
      blocks->B->changed |= Block_Algor_Changed;
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

  int debug = (flags & 1);
  int force = (flags & 2);

  if((B->changed & (IO_Changed | Block_Algor_Changed)) == 0 && !force){
    return;
  }

  mutex_lock(&algor_mutex, "Lock Algor_mutex");

  B->changed &= ~(IO_Changed);
  B->changed |= State_Changed;

  if(B->train && !B->blocked){
    if(B->Alg.BN->blocks > 0 && B->Alg.BN->B[0]->blocked){
      putAlgorQueue(B->Alg.BN->B[0], 1);
    }
    else if(B->Alg.BP->blocks > 0 && B->Alg.BP->B[0]->blocked){
      putAlgorQueue(B->Alg.BP->B[0], 1);
    }
  }

  // Algor_init_Blocks(&AllBlocks, B);

  //Find all surrounding blocks. Can be speeded up by storing this into the block. Update only if a (MS)switch changes or the direciton changes
  if(B->changed & Block_Algor_Changed){
    Algor_search_Blocks(&B->Alg, debug);
    B->changed &= ~Block_Algor_Changed;
  }

  Algor_GetBlocked_Blocks(B->Alg);

  Algor_print_block_debug(B->Alg);


  //Follow the train arround the layout
  Algor_train_following(B->Alg, debug);
  if (B->changed & IO_Changed){
    printf("Block Train ReProcess\n");
    Algor_clear_Blocks(&B->Alg);
    mutex_unlock(&algor_mutex, "UnLock AlgorMutex");
    return;
  }

  //Set oncomming switch to correct state
  Algor_Switch_Checker(B->Alg, debug);
  if (B->changed & IO_Changed){
    printf("Block Switch ReProcess\n");
    Algor_clear_Blocks(&B->Alg);
    mutex_unlock(&algor_mutex, "UnLock AlgorMutex");
    return;
  }

  //Apply block stating
  Algor_rail_state(B->Alg, debug);

  //Check Switch

  // Print all found blocks to stdout
  // Algor_print_block_debug(B->Alg);

  Algor_signal_state(B->Alg, debug);

  //Train Control

  mutex_unlock(&algor_mutex, "UnLock AlgorMutex");
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
    }else{
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
    for(int i = 1;i>=0;i--){
    if(BP.B[i]){
      sprintf(output, "%s%02i:%02i", output,BP.B[i]->module,BP.B[i]->id);
      if(BP.B[i]->blocked){
      sprintf(output, "%sB  ", output);
      }else{
      sprintf(output, "%s   ", output);
      }
    }else{
      sprintf(output, "%s        ", output);
    }
    }
  }else{
    sprintf(output, "%s                    ", output);
  }
  sprintf(output, "%sA%3i %2x%02i:%02i;T%-2iD%-2iS%-2i", output,B->length,B->type,B->module,B->id,B->train,B->dir,B->state);
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
    for(int i = 0;i<2;i++){
    if(BN.B[i]){
      sprintf(output, "%s%02i:%02i", output,BN.B[i]->module,BN.B[i]->id);
      if(BN.B[i]->blocked){
      sprintf(output, "%sB  ", output);
      }else{
      sprintf(output, "%s   ", output);
      }
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
    }else{
      sprintf(output, "%s        ", output);
    }
    }
  }

  loggerf(INFO, "%s", output);
}

void Algor_Switch_Checker(struct algor_blocks AllBlocks, int debug){
  //Unpack AllBlocks
  //Algor_Block BPPP = *AllBlocks.BPPP;
  //Algor_Block BPP  = *AllBlocks.BPP;
  //Algor_Block BP   = *AllBlocks.BP;
  Block * B        =  AllBlocks.B;
  Algor_Block BN   = *AllBlocks.BN;
  //Algor_Block BNN  = *AllBlocks.BNN;
  //Algor_Block BNNN = *AllBlocks.BNNN;

  //Check Next 1
  if(B->blocked && BN.blocks > 0){
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

      // loggerf(DEBUG, "Switch_Checker scan block (%i,%i)", tmp->module, tmp->id);
      struct rail_link link = Next_link(tmp, NEXT);
      if (link.type != RAIL_LINK_R && link.type != RAIL_LINK_E) {
        if (!Next_check_Switch_Path(tmp, link, NEXT | SWITCH_CARE)) {
          loggerf(INFO, "Switch next path!!");

          if (set_switch_path(tmp, link, NEXT | SWITCH_CARE)) {
            B->changed |= IO_Changed; // Recalculate
            return;
          }
        }
      }
      // else{
      //   loggerf(DEBUG, "Link is of type %x", link.type);
      // }

      if((link.type == RAIL_LINK_S || link.type == RAIL_LINK_s) && link.p){
        if(((Switch *)link.p)->Detection->state != BLOCKED &&
           ((Switch *)link.p)->Detection->state != RESERVED &&
           ((Switch *)link.p)->Detection->state != RESERVED_SWITCH){

          ((Switch *)link.p)->Detection->state = RESERVED_SWITCH;
          ((Switch *)link.p)->Detection->changed = State_Changed;
        }
      }
      else if((link.type == RAIL_LINK_M || link.type == RAIL_LINK_m) && link.p){
        if(((MSSwitch *)link.p)->Detection->state != BLOCKED &&
           ((MSSwitch *)link.p)->Detection->state != RESERVED &&
           ((MSSwitch *)link.p)->Detection->state != RESERVED_SWITCH){

          ((MSSwitch *)link.p)->Detection->state = RESERVED_SWITCH;
          ((MSSwitch *)link.p)->Detection->changed = State_Changed;
        }
      }
    }
  }
}

void Algor_special_search_Blocks(struct algor_blocks * Blocks, int flags){
  loggerf(TRACE, "Algor_special_search_Blocks");
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

    for(int ab = 0; ab < 15; ab++){
      if(Aside[ab])
        printf("%i:%i\t", Aside[ab]->module, Aside[ab]->id);
    }
    printf("\n");
    for(int ab = 0; ab < 15; ab++){
      if(Bside[ab])
        printf("%i:%i\t", Bside[ab]->module, Bside[ab]->id);
    }
    printf("\n");

    // Putt all blocks into Algor blocks
    int i = 0;
    for(int p = 0; p < 3; p++){
      if(!Bside[0])
        break;
      Algor_Block * C_Blocks_P = 0;
      // TODO: adds support for edge cases: 2 == 0, 5 == 0 ....
      if(Bside[0]->dir == Blocks->B->dir && (b_dir & 1) == NEXT){
        if(p == 0)
          C_Blocks_P = Blocks->BN;
        else if(p == 1)
          C_Blocks_P = Blocks->BNN;
        else if(p == 2)
          C_Blocks_P = Blocks->BNNN;
      }
      else if(Bside[0]->dir == Blocks->B->dir && (b_dir & 1) == PREV){
        if(p == 0)
          C_Blocks_P = Blocks->BP;
        else if(p == 1)
          C_Blocks_P = Blocks->BPP;
        else if(p == 2)
          C_Blocks_P = Blocks->BPPP;
      }
      else{
        loggerf(ERROR, "Unkown Direction B %x  %x", Bside[0]->dir, Blocks->B->dir);
      }
      do{
        if(!Bside[i] || !C_Blocks_P)
          break;
        C_Blocks_P->B[C_Blocks_P->blocks] = Bside[i++];
        C_Blocks_P->length += C_Blocks_P->B[C_Blocks_P->blocks]->length;
        C_Blocks_P->blocks++;
        printf("length: %i\n", C_Blocks_P->B[C_Blocks_P->blocks - 1]->length);
      }
      while(C_Blocks_P->length < Block_Minimum_Size && C_Blocks_P->blocks < 5);
    }
    i = 0;
    for(int n = 0; n < 3; n++){
      if(!Aside[0])
        break;
      Algor_Block * C_Blocks_P = 0;
      // TODO: adds support for edge cases: 2 == 0, 5 == 0 ....
      if(Aside[0]->dir == Blocks->B->dir && (a_dir & 1) == PREV){
        if(n == 0)
          C_Blocks_P = Blocks->BP;
        else if(n == 1)
          C_Blocks_P = Blocks->BPP;
        else if(n == 2)
          C_Blocks_P = Blocks->BPPP;
      }
      else if((a_dir & 1) == NEXT && (Aside[0]->dir == Blocks->B->dir)){
        if(n == 0)
          C_Blocks_P = Blocks->BN;
        else if(n == 1)
          C_Blocks_P = Blocks->BNN;
        else if(n == 2)
          C_Blocks_P = Blocks->BNNN;
      }
      else{
        loggerf(ERROR, "Unkown Direction A%x  %x", Aside[0]->dir, Blocks->B->dir);
      }
      do{
        if(!Aside[i] || !C_Blocks_P)
          break;
        C_Blocks_P->B[C_Blocks_P->blocks] = Aside[i++];
        C_Blocks_P->length += C_Blocks_P->B[C_Blocks_P->blocks]->length;
        C_Blocks_P->blocks++;
      }
      while(C_Blocks_P->length < Block_Minimum_Size && C_Blocks_P->blocks < 5);
    }
  }
  else{
    loggerf(ERROR, "Zero or 1+ pairs");
  }
  printf("Done\n");
}

void Algor_search_Blocks(struct algor_blocks * AllBlocks, int debug){
  loggerf(TRACE, "Algor_search_Blocks");
  Block * next = 0;
  Block * prev = 0;
  Block * B = AllBlocks->B;

  Algor_clear_Blocks(AllBlocks);

  if(B->type == SPECIAL){
    printf("Algor Special search blocks\n");
    Algor_special_search_Blocks(AllBlocks, debug);
    return;
  }

  int next_level = 1;
  int prev_level = 1;

  next = Next(B, NEXT | SWITCH_CARE,1);
  prev = Next(B, PREV | SWITCH_CARE,1);

  //Select all surrounding blocks
  if(next){
  for(int i = 0; i < 3; i++){
    Algor_Block * block_p;
    if(i == 0){
    block_p = AllBlocks->BN;
    }
    else if(i == 1){
    block_p = AllBlocks->BNN;
    }
    else if(i == 2){
    block_p = AllBlocks->BNNN;
    }

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
    if(i == 0){
    block_p = AllBlocks->BP;
    }
    else if(i == 1){
    block_p = AllBlocks->BPP;
    }
    else if(i == 2){
    block_p = AllBlocks->BPPP;
    }

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
  // Algor_Block BNN  = *AllBlocks.BNN;
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

  //If only current and next blocks are occupied
  if(BP.blocks > 0 && BN.blocks > 0 && B->blocked && !BP.blocked && BN.blocked && BN.B[0]->train && !B->train){
    //REVERSED
    B->dir ^= 0b100;
    loggerf(DEBUG, "REVERSE BLOCK %i:%i", B->module, B->id);
    B->changed |= IO_Changed;
    Block_Reverse_To_Next_Switch(B);
    return;
  }

  //If no surrounding blocks are occupied
  else if(BP.blocks > 0 && BN.blocks > 0 && B->blocked && B->train == 0 && BN.B[0]->train == 0 && BP.B[0]->train == 0){
    //NEW TRAIN
    // find a new follow id
    // loggerf(ERROR, "FOLLOW ID INCREMENT, bTrain");
    B->train = find_free_index(train_link, train_link_len);

    //Create a message for WebSocket
    WS_NewTrain(B->train, B->module, B->id);
    loggerf(DEBUG, "NEW_TRAIN at %i\t", B->train);
  }

  //If current block is unoccupied and surrounding are occupied and have the same train pointer
  else if(BN.blocks > 0 && BP.blocks > 0 && BN.blocked && BP.blocked && !B->blocked && BN.B[0]->train == BP.B[0]->train){
    //A train has split
    WS_TrainSplit(BN.B[0]->train, BP.B[0]->module,BP.B[0]->id,BN.B[0]->module,BN.B[0]->id);
    loggerf(DEBUG, "SPLIT_TRAIN");
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
    // if(train_link[B->train])
    //   train_link[B->train]->Block = B;
    loggerf(INFO, "COPY_TRAIN from %i:%i to %i:%i", BP.B[0]->module, BP.B[0]->id, B->module, B->id);
  }

  if(BN.blocks > 0 && BN.B[0]->type == 'T' && BN.blocked){
  if(BN.B[0]->train == 0 && BN.B[0]->blocked && B->blocked && B->train != 0){
    BN.B[0]->train = B->train;
    if(train_link[BN.B[0]->train])
    train_link[BN.B[0]->train]->Block = BN.B[0];
  }else if(BN.blocks > 1){
    for(int a = 1;a<BN.blocks;a++){
    if(BN.B[a-1]->blocked && BN.B[a]->blocked && BN.B[a]->train == 0 && BN.B[a-1]->train != 0){
      BN.B[a]->train = BN.B[a-1]->train;
      if(train_link[BN.B[a]->train])
      train_link[BN.B[a]->train]->Block = BN.B[a];
      break;
    }
    }
  }
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

  if(B->blocked && !BP.blocked && BP.blocks > 0){
    Algor_apply_rail_state(BP, DANGER);
    Algor_apply_rail_state(BPP, CAUTION);
    Algor_apply_rail_state(BPPP, PROCEED);
  }
  else if(!B->blocked && BN.blocks == 0){
    if(B->type != SPECIAL){
      B->state = CAUTION;
      B->changed |= State_Changed;
      // printf(" End of track %i:%i ",B->module, B->id);
      if(!BP.blocked && BP.blocks > 0){
        Algor_apply_rail_state(BP, PROCEED);

        if(!BPP.blocked && BPP.blocks > 0){
          Algor_apply_rail_state(BPP, PROCEED);
        }
      }
    }
    //B->type == SPECIAL
    else{
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
  else if(!B->blocked && !BN.blocked && !BNN.blocked && !BP.blocked && !BPP.blocked){
    B->state = PROCEED;
    B->changed |= State_Changed;
  }
}

void Algor_apply_rail_state(Algor_Block b, enum Rail_states state){
  loggerf(TRACE, "Algor_apply_rail_state");
  for(int i = 0; i < b.blocks; i++){
  b.B[i]->state = state;
  b.B[i]->changed |= State_Changed;
  Units[b.B[i]->module]->block_state_changed |= 1;
  }
}

void Algor_signal_state(struct algor_blocks AllBlocks, int debug){
  loggerf(TRACE, "Algor_signal_state");
  //Unpack AllBlocks
  // Algor_Block BPPP = *AllBlocks.BPPP;
  // Algor_Block BPP  = *AllBlocks.BPP;
  // Algor_Block BP   = *AllBlocks.BP;
  // Block * B        =  AllBlocks.B;
  // Algor_Block BN   = *AllBlocks.BN;
  // Algor_Block BNN  = *AllBlocks.BNN;
  // Algor_Block BNNN = *AllBlocks.BNNN;

  //TODO write Signal state

  // if(BN.blocks > 0){
  //   for(int i = 0; i < BN.blocks; i++){
  //     CB = BN.B[i]
  //     if(BN.B[i]->NextSignal){
  //       if(i == BN.blocks - 1 && BNN.blocks > 0 && (BN.B[i]->state != BNN.B[0]->state || BNN.B[0]->state != BN.B[i]->NextSignal->state)){
  //         set_signal(BN.B[i]->NextSignal, BNN.B[0]->state);
  //       }
  //       else(i < BN.blocks - 1 && (BN.B[i]->state != BN.B[i+1]->state || BN.B[i+1]->state != BN.B[i]->NextSignal->state)){
  //         set_signal(BN.B[i]->NextSignal, BN.B[i+1]->state);
  //       }
  //     }
  //   }
  //   if(BN.B[0]->NextSignal){
  //     if(BNN.blocks > 0){
  //       if(BNN.B[0]->state == DANGER && BN.B[0]->state != DANGER)
  //         printf("Signal Next red %i:%i ",BN.B[0]->module, BN.B[0]->id);
  //     }
  //     printf("Next block Next signal ");
  //   }
  //   else if(BN.B[0]->PrevSignal){
  //     if(B->state == DANGER && BN.B[0]->state != DANGER){
  //       printf("Signal Prev red %i:%i ",BN.B[0]->module, BN.B[0]->id);
  //     }
  //     printf("Next block Prev signal ");
  //   }
  // }

  // if(BP.blocks > 0){
  //   for(int i = 0; i < BP.blocks; i++){
  //     if(BP.B[i]->NextSignal){
  //       if(i == 0 && (BP.B[i]->state != B->state || B->state != BP.B[i]->NextSignal->state))
  //         set_signal(BP.B[i]->NextSignal, B->state);
  //       else if(i > 0 && (BP.B[i]->state != BP.B[i-1]->state || BP.B[i-1]->state != BP.B[i]->NextSignal->state))
  //         set_signal(BP.B[i]->NextSignal, BP.B[i-1]->state);
  //     }
  //     else if(BP.B[i]->PrevSignal){
  //       if(i == BP.blocks - 1 && BPP.blocks > 0 && (BPP.B[0]->state != BP.B[i]->state || BPP.B[i]->state != BP.B[i]->PrevSignal->state))
  //         set_signal(BP.B[i]->PrevSignal, BPP.B[0]->state);
  //       else if(i < BP.blocks - 1 && (BP.B[i]->state != BP.B[i-1]->state || BP.B[i-1]->state != BP.B[i]->PrevSignal->state))
  //         set_signal(BP.B[i]->PrevSignal, BP.B[i-1]->state);
  //     }
  //   }
  // }


}


void procces_accessoire(){
  for(int i = 0;i<unit_len;i++){
  //TODO FIX, Output_changed into Node_changed
  /*
  if(Units[i] && Units[i]->output_changed){
    printf("Signals of module %i changed\n",i);
    for(int j = 0;j<Units[i]->signal_len;j++){
    if(Units[i]->Sig[j]){
      printf("Signal id: %i\n",Units[i]->Sig[j]->id);
    }
    }

    Units[i]->output_changed = FALSE;
  }
  */
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
  if(!List->R_L[i]->p){
    continue;
  }
  if(List->R_L[i]->type == 'R'){
    if(((Block *)List->R_L[i]->p)->next.type != RAIL_LINK_C && ((Block *)List->R_L[i]->p)->prev.type != RAIL_LINK_C) {
    value++;
    continue;
    }
    // printf("Found block %i:%i %i\t",((Block*)List->R_L[i]->p)->module,((Block*)List->R_L[i]->p)->id,((Block*)List->R_L[i]->p)->blocked);
    if(((Block *)List->R_L[i]->p)->blocked){
    //Blocked block
    if(!R){
      R = List->R_L[i];
    }
    else{
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
  if(!Units[i]){
    continue;
  }

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
