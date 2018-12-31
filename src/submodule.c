#define _BSD_SOURCE 

#include <unistd.h>

#include "logger.h"

#include "submodule.h"

#include "algorithm.h"
#include "com.h"
#include "train_sim.h"
#include "Z21.h"


pthread_t Algor_thread;

void Algor_start(){
  pthread_join(Algor_thread, NULL);

  if(_SYS->UART_State == _SYS_Module_Stop)
    UART_start();

  _SYS->LC_State = _SYS_Module_Init;
  pthread_create(&Algor_thread, NULL, Algor_Run, NULL);
}

void Algor_stop(){
  _SYS->TC_State = _SYS_Module_Stop;
  _SYS->LC_State = _SYS_Module_Stop;
  sem_post(&AlgorQueueNoEmpty);
}

pthread_t UART_thread;

void UART_start(){
  pthread_join(UART_thread, NULL);
  pthread_create(&UART_thread, NULL, UART, NULL);

  //Wait until UART is out of Stop state
  while(_SYS->UART_State == _SYS_Module_Stop){
    usleep(1000);
  }
  loggerf(DEBUG, "Done starting UART");
}

void UART_stop(){
  Algor_stop();
  _SYS->UART_State = _SYS_Module_Stop;
}

pthread_t pt_train_simA;

void SimA_start(){
  pthread_join(pt_train_simA, NULL);

  if(_SYS->LC_State == _SYS_Module_Stop){
    Algor_start();
  }

  while(_SYS->LC_State != _SYS_Module_Run){
    usleep(10000);
  }

  _SYS->SimA_State = _SYS_Module_Init;
  pthread_create(&pt_train_simA, NULL, TRAIN_SIMA, NULL);
}

void SimA_stop(){
  _SYS->SimA_State = _SYS_Module_Stop;
}

pthread_t z21_thread;
pthread_t z21_start_thread;

void Z21_start(){
  if(_SYS->Z21_State != _SYS_Module_Stop){
    loggerf(ERROR, "Z21 Allready running");
    return;
  }
  pthread_create(&z21_start_thread, NULL, Z21, NULL);
}

void Z21_stop(){
  _SYS->Z21_State = _SYS_Module_Stop;
  _SYS->TC_State = _SYS_Module_Stop;
}