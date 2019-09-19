#include "system.h"
#include "train_sim.h"
#include "algorithm.h"
#include "websocket_msg.h"
#include "rail.h"
#include "train.h"
#include "logger.h"
#include "module.h"

#include "submodule.h"

pthread_mutex_t mutex_lockA;

#define delayA 5000000
#define delayB 5000000
#define OneSec 1000000

#define TRAIN_A_LEN   5 //cm
#define TRAIN_A_SPEED 5 //cm/s

#define TRAIN_B_LEN   5 //cm
#define TRAIN_B_SPEED 5 //cm/s


void change_Block(Block * B, enum Rail_states state){
  B->changed |= IO_Changed;
  B->state = state;
  if (state == BLOCKED)
    B->blocked = 1;
  else
    B->blocked = 0;

  putAlgorQueue(B, 1);
  // process(B, 3);
}

void *TRAIN_SIMA(){
  while(_SYS->LC_State != _SYS_Module_Run){
    usleep(10000);
  }
  Block *B = Units[21]->B[1];
  Block *N = Units[21]->B[1];
  Block *N2 = 0;

  B->state = BLOCKED;
  B->blocked = 1;
  B->changed |= IO_Changed;

  putAlgorQueue(B, 1);

  usleep(100000);

  if(B->train){
    while(!B->train->p){
      usleep(1000);
    }
  }

  _SYS->SimA_State = _SYS_Module_Run;
  WS_stc_SubmoduleState();

  while(_SYS->SimA_State & _SYS_Module_Run){

    N = B->Alg.BN->B[0];
    if(!N){
      loggerf(WARNING, "Sim A reached end of the line");
      return 0;
    }
    loggerf(INFO, "Sim A step %i:%i", N->module, N->id);
    change_Block(N, BLOCKED);

    // IF len(N) < len(TRAIN)
    if(N->length < TRAIN_A_LEN){
      usleep((N->length/TRAIN_A_SPEED) * OneSec);
      N2 = N->Alg.BN->B[0];
      if(!N2){
        loggerf(WARNING, "Sim A reached end of the line");
        return 0;
      }
      loggerf(DEBUG, "Sim A substep %i:%i", N2->module, N2->id);

      change_Block(N2, BLOCKED);
      usleep(((TRAIN_A_LEN - N->length)/TRAIN_A_SPEED) * OneSec);
      change_Block(B, PROCEED);
      if(N2 && N2->length > TRAIN_A_LEN){
        usleep(((N2->length - (TRAIN_A_LEN - N->length))/TRAIN_A_SPEED) * OneSec);
        change_Block(N, PROCEED);
        usleep(((N2->length - TRAIN_A_LEN)/TRAIN_A_SPEED) * OneSec);

        B = N2;
      }
      else{
        loggerf(WARNING, "Two short blocks smaller than train A");
        change_Block(N, PROCEED);
        usleep(OneSec);
        B = N2;
      }
    }
    else{
      usleep((TRAIN_A_LEN/TRAIN_A_SPEED) * OneSec);
      change_Block(B, PROCEED);
      usleep(((N->length - TRAIN_A_LEN)/TRAIN_A_SPEED) * OneSec);

      B = N;
    }
  }

  return 0;
}

void *TRAIN_SIMB(){
  while(_SYS->LC_State != _SYS_Module_Run){
    usleep(10000);
  }
  Block *B = Units[26]->B[2];
  Block *N = Units[26]->B[2];
  Block *N2 = 0;

  Reserve_To_Next_Switch(B);

  B->state = BLOCKED;
  B->blocked = 1;
  B->changed |= IO_Changed;

  putAlgorQueue(B, 1);

  usleep(100000);

  _SYS->SimB_State = _SYS_Module_Run;
  WS_stc_SubmoduleState();

  while(_SYS->SimB_State & _SYS_Module_Run){

    N = B->Alg.BN->B[0];
    if(!N){
      loggerf(WARNING, "Sim B reached end of the line");
      return 0;
    }
    loggerf(INFO, "Sim B step %i:%i", N->module, N->id);
    change_Block(N, BLOCKED);

    // IF len(N) < len(TRAIN)
    if(N->length < TRAIN_B_LEN){
      usleep((N->length/TRAIN_B_SPEED) * OneSec);
      N2 = N->Alg.BN->B[0];
      if(!N2){
        loggerf(WARNING, "Sim B reached end of the line");
        return 0;
      }
      loggerf(DEBUG, "Sim B substep %i:%i", N2->module, N2->id);

      change_Block(N2, BLOCKED);
      usleep(((TRAIN_B_LEN - N->length)/TRAIN_B_SPEED) * OneSec);
      change_Block(B, PROCEED);
      if(N2 && N2->length > TRAIN_B_LEN){
        usleep(((N2->length - (TRAIN_B_LEN - N->length))/TRAIN_B_SPEED) * OneSec);
        change_Block(N, PROCEED);
        usleep(((N2->length - TRAIN_B_LEN)/TRAIN_B_SPEED) * OneSec);

        B = N2;
      }
      else{
        loggerf(WARNING, "Two short blocks smaller than train B");
        change_Block(N, PROCEED);
        usleep(OneSec);
        B = N2;
      }
    }
    else{
      usleep((TRAIN_B_LEN/TRAIN_B_SPEED) * OneSec);
      change_Block(B, PROCEED);
      usleep(((N->length - TRAIN_B_LEN)/TRAIN_B_SPEED) * OneSec);

      B = N;
    }
  }

  return 0;
}
