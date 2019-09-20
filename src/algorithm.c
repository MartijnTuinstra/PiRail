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
// #include "train.h"
#include "switch.h"
#include "signals.h"

#include "modules.h"
// #include "com.h"
// #include "websocket_msg.h"

// #include "submodule.h"
// #include "pathfinding.h"

pthread_mutex_t mutex_lockA;
pthread_mutex_t algor_mutex;

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

  for(int i = 0; i < AllBlocks.prev; i++){
    putAlgorQueue(AllBlocks.P[i], enable);
  }
  for(int i = 0; i < AllBlocks.next; i++){
    putAlgorQueue(AllBlocks.N[i], enable);
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
    // process(B, 2);
     if(B->IOchanged){
      loggerf(TRACE, "ReProcess");
      // process(B, 0);
    }
    B = getAlgorQueue();
  }
}

void * Algor_Run(){
  loggerf(INFO, "Algor_run started");

  while(SYS->UART.state != Module_Run){
    if(SYS->UART.state == Module_Fail || SYS->UART.state == Module_STOP){
      loggerf(ERROR, "Cannot run Algor when UART FAIL or STOP %x", SYS->UART.state);
      return 0;
    }
  }

  //UART_Send_Search();
  
  usleep(200000);
  SYS->LC.state = Module_LC_Searching;
  WS_stc_SubmoduleState();
  SIM_JoinModules();
  usleep(100000);
  SYS->LC.state = Module_LC_Connecting;
  WS_stc_SubmoduleState();
  SIM_Connect_Rail_links();
  WS_Track_Layout(0);
  usleep(800000);
  SYS->LC.state = Module_Run;
  WS_stc_SubmoduleState();
  // scan_All();
  // throw_switch(U_Sw(20, 5), 1, 0);
  // throw_switch(U_Sw(20, 6), 1, 1);
  // throw_switch(U_Sw(20, 2), 1);
  usleep(10000);

  while(SYS->LC.state == Module_Run && SYS->stop == 0){
    sem_wait(&AlgorQueueNoEmpty);
    processAlgorQueue();

    mutex_lock(&algor_mutex, "Algor Mutex");
    //Notify clients
    // WS_trackUpdate(0);
    // WS_SwitchesUpdate(0);

    update_IO();

    mutex_unlock(&algor_mutex, "Algor Mutex");

    usleep(1000);
  }

  loggerf(INFO, "Algor_run done");
  return 0;
}

void Algor_init_Blocks(struct algor_blocks * AllBlocks, Block * B){
  //init_Algor_Blocks and clear
  Block ** P   = _calloc(10, void *);
  Block ** N   = _calloc(10, void *);

  // Link algor_blocks
  AllBlocks->P = P;
  AllBlocks->B = B;
  AllBlocks->N = N;

  Algor_clear_Blocks(AllBlocks);
}

void Algor_clear_Blocks(struct algor_blocks * AllBlocks){
  memset(AllBlocks->P, 0, 10*sizeof(void *));
  memset(AllBlocks->N, 0, 10*sizeof(void *));
  // AllBlocks->BPPP->blocks = 0;
  // AllBlocks->BPP->blocks = 0;
  // AllBlocks->BP->blocks = 0;
  // AllBlocks->BN->blocks = 0;
  // AllBlocks->BNN->blocks = 0;
  // AllBlocks->BNNN->blocks = 0;

  // AllBlocks->BPPP->signal = 0;
  // AllBlocks->BPP->signal = 0;
  // AllBlocks->BP->signal = 0;
  // AllBlocks->BN->signal = 0;
  // AllBlocks->BNN->signal = 0;
  // AllBlocks->BNNN->signal = 0;

  // AllBlocks->BPPP->switches = 0;
  // AllBlocks->BPP->switches = 0;
  // AllBlocks->BP->switches = 0;
  // AllBlocks->BN->switches = 0;
  // AllBlocks->BNN->switches = 0;
  // AllBlocks->BNNN->switches = 0;

  // AllBlocks->BPPP->length = 0;
  // AllBlocks->BPP->length = 0;
  // AllBlocks->BP->length = 0;
  // AllBlocks->BN->length = 0;
  // AllBlocks->BNN->length = 0;
  // AllBlocks->BNNN->length = 0;
  AllBlocks->prev = 0;
  AllBlocks->next = 0;
}

void Algor_free_Blocks(struct algor_blocks * AllBlocks){
  _free(AllBlocks->P);
  _free(AllBlocks->N);
}

void Algor_Set_Changed(struct algor_blocks * blocks){
  loggerf(TRACE, "Algor_Set_Changed");
  //Scroll through all the pointers of allblocks
  for(int i = 0; i < blocks->prev; i++){
    blocks->P[i]->algorchanged = 1;
  }
  for(int i = 0; i < blocks->next; i++){
    blocks->N[i]->algorchanged = 1;
  }
  blocks->B->algorchanged = 1;
}


void Algor_search_Blocks(struct algor_blocks * AllBlocks, int debug){
  loggerf(TRACE, "Algor_search_Blocks - %02i:%02i", AllBlocks->B->module, AllBlocks->B->id);
  // Block * next = 0;
  // Block * prev = 0;
  // Block * B = AllBlocks->B;

  // Algor_clear_Blocks(AllBlocks);

  // if(B->type == SPECIAL){
  //   Algor_special_search_Blocks(AllBlocks, debug);
  //   return;
  // }

  // loggerf(DEBUG, "Search blocks %02i:%02i", B->module, B->id);

  // int next_level = 1;
  // int prev_level = 1;

  // next = Next(B, NEXT | SWITCH_CARE,1);
  // prev = Next(B, PREV | SWITCH_CARE,1);

  // //Select all surrounding blocks
  // if(next){
  //   for(int i = 0; i < 3; i++){
  //     Algor_Block * block_p;
  //     if(i == 0)
  //       block_p = AllBlocks->BN;
  //     else if(i == 1)
  //       block_p = AllBlocks->BNN;
  //     else if(i == 2)
  //       block_p = AllBlocks->BNNN;

  //     block_p->blocks = 0;

  //     do{
  //       if(i == 0 && block_p->blocks == 0){
  //         block_p->B[block_p->blocks] = next;
  //         next_level++;
  //       }
  //       else{
  //         block_p->B[block_p->blocks] = Next(B, NEXT | SWITCH_CARE, next_level++);
  //       }

  //       if(!block_p->B[block_p->blocks]){
  //         i = 4;
  //         break;
  //       }

  //       if(block_p->B[block_p->blocks]->NextSignal || block_p->B[block_p->blocks]->PrevSignal)
  //         block_p->signal = 1;

  //       if(block_p->B[block_p->blocks]->switch_len)
  //         block_p->switches = 1;

  //       if(block_p->B[block_p->blocks]->blocked)
  //         block_p->blocked = 1;

  //       block_p->length += block_p->B[block_p->blocks]->length;

  //     block_p->blocks += 1;

  //     }
  //     while(block_p->length < Block_Minimum_Size && block_p->blocks < 5);
  //   }
  // }
  // if(prev){
  //   for(int i = 0; i < 3; i++){
  //     Algor_Block * block_p;
  //     if(i == 0)
  //       block_p = AllBlocks->BP;
  //     else if(i == 1)
  //       block_p = AllBlocks->BPP;
  //     else if(i == 2)
  //       block_p = AllBlocks->BPPP;
  //     block_p->blocks = 0;

  //   do{
  //     if(i == 0 && block_p->blocks == 0){
  //       block_p->B[block_p->blocks] = prev;
  //       prev_level++;
  //     }
  //     else{
  //       block_p->B[block_p->blocks] = Next(B, PREV | SWITCH_CARE, prev_level++);
  //     }

  //     if(!block_p->B[block_p->blocks]){
  //       i = 4;
  //       break;
  //     }

  //     if(block_p->B[block_p->blocks]->NextSignal || block_p->B[block_p->blocks]->PrevSignal)
  //       block_p->signal = 1;

  //     if(block_p->B[block_p->blocks]->switch_len)
  //       block_p->switches = 1;

  //     if(block_p->B[block_p->blocks]->blocked)
  //       block_p->blocked = 1;

  //     block_p->length += block_p->B[block_p->blocks]->length;

  //     block_p->blocks += 1;

  //     }
  //     while(block_p->length < Block_Minimum_Size && block_p->blocks < 5);
  //   }
  // }
}
