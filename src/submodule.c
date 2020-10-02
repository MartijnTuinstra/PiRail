#include <unistd.h>

#include "utils/logger.h"

#include "submodule.h"

#include "algorithm/component.h"
#include "com.h"
#include "sim.h"
#include "Z21.h"
// #include "pathfinding.h"


void Algor_start(){
  if(SYS->LC.state != Module_Fail && SYS->LC.state != Module_STOP){
    loggerf(WARNING, "Algor allraedy running/started");
    return;
  }

  pthread_join(SYS->LC.th, NULL);

  if(SYS->UART.state == Module_STOP)
    UART_start();
  else if(SYS->UART.state == Module_Fail){
    SYS->LC.state = Module_Fail;
    return;
  }

  SYS_set_state(&SYS->LC.state, Module_Init);
  pthread_create(&SYS->LC.th, NULL, Algorithm::Run, NULL);
}

void Algor_stop(){
  SYS->TC.state = Module_STOP;
  SYS->LC.state = Module_STOP;
  // sem_post(&AlgorQueueNoEmpty);
}

void UART_start(){
  if(SYS->UART.state != Module_Fail && SYS->UART.state != Module_STOP){
    loggerf(WARNING, "UART allraedy running/started");
    return;
  }

  pthread_join(SYS->UART.th, NULL);
  pthread_create(&SYS->UART.th, NULL, UART, NULL);

  // Wait until UART is out of STOP state
  while(SYS->UART.state == Module_STOP){
    usleep(1000);
  }

  loggerf(DEBUG, "Done starting UART");
}

void UART_stop(){
  Algor_stop();
  SYS_set_state(&SYS->UART.state, Module_STOP);
}

pthread_t pt_train_simA;

void SimA_start(){
  if(SYS->SimA.state != Module_Fail && SYS->SimA.state != Module_STOP){
    loggerf(WARNING, "SimA allraedy running/started");
    return;
  }

  pthread_join(SYS->SimA.th, NULL);

  SYS_set_state(&SYS->SimA.state, Module_Init_Parent);

  if(SYS->LC.state == Module_STOP){
    Algor_start();
  }
  else if(SYS->LC.state == Module_Fail){
    SYS->SimA.state = Module_Fail;
    return;
  }

  SYS_set_state(&SYS->SimA.state, Module_Init);
  pthread_create(&SYS->SimA.th, NULL, TRAIN_SIMA, NULL);
}

void SimB_start(){
  if(SYS->SimB.state != Module_Fail && SYS->SimB.state != Module_STOP){
    loggerf(WARNING, "SimB allraedy running/started");
    return;
  }

  pthread_join(SYS->SimB.th, NULL);

  SYS_set_state(&SYS->SimB.state, Module_Init_Parent);

  if(SYS->LC.state == Module_STOP){
    Algor_start();
  }
  else if(SYS->LC.state == Module_Fail){
    SYS->SimB.state = Module_Fail;
    return;
  }

  usleep(100000);

  // Block * start = Units[22]->B[1];
  // Block * end   = Units[26]->B[5];

  // struct pathfindingstep path = pathfinding(start, end);
  // if(path.found){
  //   printf("f");
  // }

  SYS_set_state(&SYS->SimB.state, Module_Init);
  pthread_create(&SYS->SimB.th, NULL, TRAIN_SIMB, NULL);
}

void Z21_start(){
  if(SYS->Z21.state != Module_STOP){
    loggerf(ERROR, "Z21 Allready running");
    return;
  }
  Z21->start();
}

void Z21_stop(){
  SYS->Z21.state = Module_STOP;
  SYS->TC.state = Module_STOP;
}