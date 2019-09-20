#include <unistd.h>

#include "logger.h"

#include "submodule.h"

#include "algorithm.h"
#include "com.h"
// #include "train_sim.h"
#include "Z21.h"
// #include "pathfinding.h"


void Algor_start(){
  pthread_join(SYS->LC.th, NULL);

  if(SYS->UART.state == Module_STOP)
    UART_start();

  SYS->LC.state = Module_Init;
  pthread_create(&SYS->LC.th, NULL, Algor_Run, NULL);
}

void Algor_stop(){
  SYS->TC.state = Module_STOP;
  SYS->LC.state = Module_STOP;
  sem_post(&AlgorQueueNoEmpty);
}

void UART_start(){
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
  SYS->UART.state = Module_STOP;
}

pthread_t pt_train_simA;

void SimA_start(){
  // pthread_join(SYS->SimA.th, NULL);

  // if(SYS->LC.state == Module_STOP){
  //   Algor_start();
  // }

  // while(SYS->LC.state != Module_Run){
  //   usleep(10000);
  // }

  // SYS->SimA.state = Module_Init;
  // pthread_create(&SYS->SimA.th, NULL, TRAIN_SIMA, NULL);
}

void SimB_start(){
  // pthread_join(SYS->SimB.th, NULL);

  // if(SYS->LC.state == Module_STOP){
  //   Algor_start();
  // }

  // while(SYS->LC.state != Module_Run){
  //   usleep(10000);
  // }

  // usleep(100000);

  // Block * start = Units[22]->B[1];
  // Block * end   = Units[26]->B[5];

  // struct pathfindingstep path = pathfinding(start, end);
  // if(path.found){
  //   printf("f");
  // }

  // // SYS->SimB.state = Module_Init;
  // // pthread_create(&SYS->SimB.th, NULL, TRAIN_SIMB, NULL);
}

void Z21_start(){
  if(SYS->Z21.state != Module_STOP){
    loggerf(ERROR, "Z21 Allready running");
    return;
  }
  pthread_create(&SYS->Z21.start_th, NULL, Z21, NULL);
}

void Z21_stop(){
  SYS->Z21.state = Module_STOP;
  SYS->TC.state = Module_STOP;
}