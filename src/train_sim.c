#include "system.h"
#include "train_sim.h"

#include "rail.h"
#include "train.h"

#include "module.h"

pthread_mutex_t mutex_lockA;

#define delayA 5000000
#define delayB 5000000


void *TRAIN_SIMA(){
  Block *B = Units[20]->B[8];
  Block *N = Units[20]->B[8];
  Block *A = 0;
  int i = 0;

  B->state = BLOCKED;
  B->blocked = 1;
  B->changed  = 1;

  while(1){
    if(B->train != 0 && train_link[B->train] != 0 || B->state == RESTRICTED){
      break;
    }
    else if(_SYS->_STATE & STATE_RUN == 0){
      break;
    }
    usleep(100);
  }


  while(_SYS->_STATE & STATE_RUN){
    printf("Train Sim Step (id:%i)\n",pthread_self());

    pthread_mutex_lock(&mutex_lockA);

    N = Next(B, NEXT,1+i);
    if(i > 0){
      A = Next(B, NEXT,i);
    }
    if(!N){
      printf("No N at %i:%i\n",B->module, B->id);
      while(1){
        usleep(100000);
      }
    }
    printf(" %i:%i\n",N->module,N->id);
    N->changed = 1;
    N->state = BLOCKED;
    N->blocked = 1;
    pthread_mutex_unlock(&mutex_lockA);
    usleep(delayA/2);
    pthread_mutex_lock(&mutex_lockA);
    if(i>0){
      A->changed = 1;
      A->blocked = 0;
      A->state = PROCEED;
    }else{
      B->changed = 1;
      B->blocked = 0;
      B->state = PROCEED;
    }
    pthread_mutex_unlock(&mutex_lockA);
    usleep(delayA/2);
    pthread_mutex_lock(&mutex_lockA);
    if(N->type == SPECIAL){
      i++;
    }else{
      B = N;
      i = 0;
    }
    pthread_mutex_unlock(&mutex_lockA);
  }
}

void *TRAIN_SIMB(){
  Block *B = Units[4]->B[23];
  Block *N = Units[4]->B[23];
  Block *A = 0;
  int i = 0;

  B->state = BLOCKED;
  B->changed  = 1;

  while(B->train == 0){}

  while(!train_link[B->train]){}

  train_link[B->train]->halt = 1;

  while(train_link[B->train]->halt == 1){}

  while(_SYS->_STATE & STATE_RUN){
    //printf("Train Sim Step (id:%i)\n",pthread_self());
    while(1){
      if(train_link[2] && train_link[2]->halt == 0){
        break;
      }
      usleep(1000);
    }

    pthread_mutex_lock(&mutex_lockA);

    N = Next(B,0,1+i);
    if(i > 0){
      A = Next(B,0,i);
    }
    if(!N){
      while(1){
        usleep(100000);
      }
    }
    //printf(" %i:%i:%i\n",N->Adr.M,N->Adr.B,N->Adr.S);
    N->changed = 1;
    N->state = BLOCKED;
    pthread_mutex_unlock(&mutex_lockA);
    usleep(delayA/2);
    pthread_mutex_lock(&mutex_lockA);
    if(i>0){
      A->changed = 1;
      A->state = DANGER;
    }else{
      B->changed = 1;
      B->state = DANGER;
    }
    pthread_mutex_unlock(&mutex_lockA);
    usleep(delayA/2);
    pthread_mutex_lock(&mutex_lockA);
    if(N->type == 'T'){
      i++;
    }else{
      B = N;
      i = 0;
    }
    pthread_mutex_unlock(&mutex_lockA);
  }
}
/*
void *TRAIN_SIMC(){
  struct Seg *B2 = blocks2[7][5];
  struct Seg *N2 = blocks2[7][5];
  struct Seg *A2[3] = {blocks2[0][0]};
  int i2 = 0;
  pthread_mutex_lock(&mutex_lockA);
  while(_SYS->_STATE & STATE_RUN){
    //printf("Train Sim Step (id:%i)\t",pthread_self());
    N2 = Next2(B2,1+i2);
    if(i2 > 0){
      A2[i2] = Next2(B2,i2);
    }
    if(!N2){
      while(1){
        usleep(100000);
      }
    }
    //printf(" %i:%i:%i\n",N2->Adr.M,N2->Adr.B,N2->Adr.S);
    N2->change = 1;
    N2->blocked = 1;
    pthread_mutex_unlock(&mutex_lockA);
    usleep(delayB/2);
    pthread_mutex_lock(&mutex_lockA);
    if(i2>0){
      A2[i2]->change = 1;
      A2[i2]->blocked = 0;
    }else{
      B2->change = 1;
      B2->blocked = 0;
    }
    pthread_mutex_unlock(&mutex_lockA);
    usleep(delayB/2);
    pthread_mutex_lock(&mutex_lockA);
    if(N2->Adr.S == 0){
      i2++;
    }else{
      B2 = N2;
      i2 = 0;
    }
  }
}
void *TRAIN_SIMD(){
  struct Seg *B2 = blocks2[7][2];
  struct Seg *N2 = blocks2[7][2];
  struct Seg *A2[3] = {blocks2[0][0]};
  int i2 = 0;
  pthread_mutex_lock(&mutex_lockA);
  while(_SYS->_STATE & STATE_RUN){
    //printf("Train Sim Step (id:%i)\t",pthread_self());
    N2 = Next2(B2,1+i2);
    if(i2 > 0){
      A2[i2] = Next2(B2,i2);
    }
    if(!N2){
      while(1){
        usleep(100000);
      }
    }
    //printf(" %i:%i:%i\n",N2->Adr.M,N2->Adr.B,N2->Adr.S);
    N2->change = 1;
    N2->blocked = 1;
    pthread_mutex_unlock(&mutex_lockA);
    usleep(delayB/2);
    pthread_mutex_lock(&mutex_lockA);
    if(i2>0){
      A2[i2]->change = 1;
      A2[i2]->blocked = 0;
    }else{
      B2->change = 1;
      B2->blocked = 0;
    }
    pthread_mutex_unlock(&mutex_lockA);
    usleep(delayB/2);
    pthread_mutex_lock(&mutex_lockA);
    if(N2->Adr.S == 0){
      i2++;
    }else{
      B2 = N2;
      i2 = 0;
    }
  }
}
*/