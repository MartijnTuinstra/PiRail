#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <errno.h>
#include <semaphore.h>

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

int _find_free_index(void *** list, int * length){
  if(!(*list)){
    log("LIST DOESNT EXIST",CRITICAL);
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

void SYS_set_state(volatile enum e_SYS_Module_State * system, enum e_SYS_Module_State state){
  *system = state;
  loggerf(INFO, "Setting module to state %s", e_SYS_Module_State_string[state]);
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