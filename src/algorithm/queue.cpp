#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include "algorithm/queue.h"

#include "utils/logger.h"

AlgorQueue AlQueue = AlgorQueue();

AlgorQueue::AlgorQueue(){
  queue = new Queue<Block *>(500);
  tempQueue = new Queue<Block *>(25);
}

AlgorQueue::~AlgorQueue(){
  delete queue;
  delete tempQueue;
}

void AlgorQueue::put(Algor_Blocks * ABs){
  put(ABs->B);

  for(int i = 0; i < ABs->prev; i++){
    put(ABs->P[i]);
  }
  for(int i = 0; i < ABs->next; i++){
    put(ABs->N[i]);
  }
}
void AlgorQueue::puttemp(Algor_Blocks * ABs){
  put(ABs->B);

  for(int i = 0; i < ABs->prev; i++){
    put(ABs->P[i]);
  }
  for(int i = 0; i < ABs->next; i++){
    put(ABs->N[i]);
  }
}

void AlgorQueue::cpytmp(){
  while(tempQueue->getItems() > 0){
    queue->AddOnce(tempQueue->Get());
  }
}


// void putAlgorQueue(Block * B, int enableQueue){
//   loggerf(TRACE, "putAlgorQueue %x, %i", (unsigned int)B, enableQueue);
//   mutex_lock(&AlgorQueueMutex, "AlgorQueueMutex");
//   AlgorQueue.B[AlgorQueue.writeIndex++] = B;

//   algor_queue_enable(enableQueue);

//   if(AlgorQueue.writeIndex == AlgorQueueLength)
//     AlgorQueue.writeIndex = 0;
//   mutex_unlock(&AlgorQueueMutex, "AlgorQueueMutex");
// }

// void putList_AlgorQueue(Algor_Blocks ABs, int enable){
//   putAlgorQueue(ABs.B, enable);

//   for(int i = 0; i < ABs.prev; i++){
//     putAlgorQueue(ABs.P[i], enable);
//   }
//   for(int i = 0; i < ABs.next; i++){
//     putAlgorQueue(ABs.N[i], enable);
//   }
// }