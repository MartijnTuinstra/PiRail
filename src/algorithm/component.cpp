#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include "algorithm/component.h"
#include "algorithm/core.h"
#include "algorithm/queue.h"

#include "switchboard/manager.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/unit.h"

#include "websocket/stc.h"

#include "logger.h"
#include "system.h"

#include "com.h"
#include "sim.h"

namespace Algorithm {
using namespace switchboard;

void * Run(void * args){
  loggerf(INFO, "Algor_run started");

  while(SYS->UART.state != Module_Run && SYS->UART.state != Module_SIM_State){
    if(SYS->UART.state == Module_Fail || SYS->UART.state == Module_STOP){
      loggerf(ERROR, "Cannot run Algor when UART FAIL or STOP %x", SYS->UART.state);
      return 0;
    }
  }

  //UART_Send_Search();
  
  usleep(1000000);
  SYS_set_state(&SYS->LC.state, Module_LC_Searching);
  if(SYS->UART.state == Module_SIM_State){
    SIM_JoinModules();
    usleep(1000000);
    SYS_set_state(&SYS->LC.state, Module_LC_Connecting);
    usleep(1000000);
  }
  else{
    InitFindModules();
    InitConnectModules();
  }
  SIM_Connect_Rail_links();
  WS_stc_Track_Layout(0);

  // Scan All Blocks
  InitProcess();
  
  //Notify clients
  WS_stc_trackUpdate(0);
  WS_stc_SwitchesUpdate(0);

  usleep(10000);

  loggerf(INFO, "Algor Ready");

  while(SYS->LC.state == Module_Run && SYS->stop == 0){
    tick();

    // mutex_lock(&algor_mutex, "Algor Mutex"); // FIXME
    //Notify clients
    WS_stc_trackUpdate(0);
    WS_stc_SwitchesUpdate(0);

    update_IO();

    // mutex_unlock(&algor_mutex, "Algor Mutex"); // FIXME

    usleep(1000);
  }

  SYS_set_state(&SYS->LC.state, Module_STOP);
  loggerf(INFO, "Algor_run done");
  return 0;
}

int InitFindModules(void){
  if(!SYS_wait_for_state(&SYS->UART.state, Module_Run)){
    SYS_set_state(&SYS->LC.state, Module_Fail);
    return 0;
  }
  COM_DevReset();
  while(!SYS->UART.modules_found){
    usleep(1000);
  }
  SYS_set_state(&SYS->LC.state, Module_LC_Connecting);

  return 1;
}
int InitConnectModules(void){
  Connect_Rails();
  return 1;
}
int InitProcess(void){
  for(int i = 0; i < SwManager->Units.size; i++){
    Unit * U = Units(i);
    if(!U)
      continue;

    for(int j = 0; j < U->block_len; j++){
      if(U->B[j]){
        process(U->B[j], _FORCE);
      }
    }
    for(int j = 0; j < U->switch_len; j++){
      if(U->Sw[j]){
        U->Sw[j]->updatedState = true;
      }
    }
    for(int j = 0; j < U->msswitch_len; j++){
      if(U->MSSw[j]){
        U->MSSw[j]->updatedState = true;
      }
    }
  }
  SYS_set_state(&SYS->LC.state, Module_Run);
  return 1;
}

void tick(void){
  Block * B = AlQueue.getWait();
  while(B != 0){
    loggerf(TRACE, "Process %i:%i, %x, %x", B->module, B->id, B->IOchanged + (B->statechanged << 1) + (B->algorchanged << 2), B->state);
    process(B, 0);
     while(B->recalculate){
      loggerf(INFO, "ReProcess");
      B->recalculate = 0;
      process(B, 0);
    }
    B = AlQueue.get();
  }
}

}; // namespace