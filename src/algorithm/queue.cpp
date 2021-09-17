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
  TrainQueue = new Queue<Train *>(50);
}

AlgorQueue::~AlgorQueue(){
  delete queue;
  delete tempQueue;
  delete TrainQueue;
}

void AlgorQueue::put(struct blockAlgorithm * ABs){
  put(ABs->B);

  for(int i = 0; i < ABs->P->group[3]; i++){
    put(ABs->P->B[i]);
  }
  for(int i = 0; i < ABs->N->group[3]; i++){
    put(ABs->N->B[i]);
  }
}

void AlgorQueue::puttemp(struct blockAlgorithm * ABs){
  puttemp(ABs->B);

  for(int i = 0; i < ABs->P->group[3]; i++){
    puttemp(ABs->P->B[i]);
  }
  for(int i = 0; i < ABs->N->group[3]; i++){
    puttemp(ABs->N->B[i]);
  }
}

void AlgorQueue::cpytmp(){
  while(tempQueue->getItems() > 0){
    queue->AddOnce(tempQueue->Get());
  }
}


// void putAlgorQueue(Block * B, int enableQueue){
//   loggerf(TRACE, "putAlgorQueue %x, %i", (unsigned long)B, enableQueue);
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