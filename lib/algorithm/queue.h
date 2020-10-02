#ifndef _INCLUDE_ALGORITHM_QUEUE_H
#define _INCLUDE_ALGORITHM_QUEUE_H

#include "utils/queue.h"
#include "switchboard/rail.h"

class AlgorQueue {
  public:
    Queue<Block *> * queue;
    Queue<Block *> * tempQueue;

    AlgorQueue();
    ~AlgorQueue();

    inline void put(Block * B){
      queue->AddOnce(B);
    }
    void put(Algor_Blocks * ABs);

    inline void puttemp(Block * B){
      tempQueue->AddOnce(B);
    }
    void puttemp(Algor_Blocks * ABs);

    void cpytmp(); // Copies temp to queue

    inline Block * get(){
      return queue->Get();
    }
    inline Block * getWait(){
      return queue->waitGet();
    }

    inline void clear(){
      queue->clear();
    }
    inline void cleartmp(){
      tempQueue->clear();
    }
};

extern class AlgorQueue AlQueue;

#endif