#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <errno.h>
#include <semaphore.h>
#include <execinfo.h>

#include "system.h"
#include "utils/logger.h"
#include "utils/mem.h"

#include "websocket/stc.h"

void sigint_func(int sig){
  if(sig == SIGINT){
    loggerf(WARNING, "-- SIGINT -- STOPPING");
    SYS->WebSocket.accept_clients = 0;

    // Request Stop
    SYS->stop = 1;
  }
}

void sigint_crash_func(int sig){
  printf("\n\n---- SIGSEGV ----\n\n");
  void *array[20];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 20);

  // print out all the frames to stderr
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

int _find_free_index(void *** list, int * length){
  if(!(*list)){
    log("root", CRITICAL, "LIST DOESNT EXIST");
    return -1;
  }
  for(int i = 0;i<(*length);i++){
    if(!(*list)[i]){
      return i;
    }
  }

  *list = (void **)_realloc(*list, (*length)+1, void *);
  for(int i = *length; i < (*length)+1; i++){
    (*list)[i] = 0;
  }
  *length += 1;
  return _find_free_index(list, length);
}

void system_init(struct s_systemState * S){
  S->stop = 0;
  S->Clients = 0;

  S->Z21.state  = Module_STOP;
  S->UART.state = Module_STOP;
  S->TC.state   = Module_STOP;
  S->LC.state   = Module_STOP;
  S->WebSocket.state = Module_STOP;

  S->SimA.state = Module_STOP;
  S->SimB.state = Module_STOP;
}

void init_main(){
  SYS = (struct s_systemState *)_calloc(1, struct s_systemState);
  system_init(SYS);

  init_allocs();
}

void destroy_main(){
  _free(SYS);
  destroy_allocs();
}

const char * e_SYS_Module_State_string[8] = {
  "Module_STOP",
  "Module_Init_Parent",
  "Module_Init",
  "Module_Run",
  "Module_Fail",
  "Module_LC_Searching",
  "Module_LC_Connecting",
  "Module_SIM_State"
};

const char * e_SYS_Module_Names[10] = {
  "",
  "WebsocketServer",
  "Z21",
  "UART",
  "LayoutControl",
  "TrainControl",
  "SimA",
  "SimB"
};

void SYS_set_state(volatile enum e_SYS_Module_State * system, enum e_SYS_Module_State state){
  *system = state;
  uint8_t name = 0;
  if (system == &SYS->WebSocket.state)
      name = 1;
  else if(system == &SYS->Z21.state)
      name = 2;
  else if(system == &SYS->UART.state)
      name = 3;
  else if(system == &SYS->LC.state)
      name = 4;
  else if(system == &SYS->TC.state)
      name = 5;
  else if(system == &SYS->SimA.state)
      name = 6;
  else if(system == &SYS->SimB.state)
      name = 7;

  loggerf(INFO, "Setting module %s to state %s", e_SYS_Module_Names[name], e_SYS_Module_State_string[state]);
  WS_stc_SubmoduleState(0);
}

bool SYS_wait_for_state(volatile enum e_SYS_Module_State * system, enum e_SYS_Module_State state){
  while(*system != state){
    usleep(1000);
    if(*system == Module_Fail || *system == Module_STOP){
      return false;
    }
  }
  return true;
}