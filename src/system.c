#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <errno.h>

#include "system.h"
#include "websocket_control.h"
#include "logger.h"

void * my_calloc(int elements, int size, char * file, int line){
  void * p = calloc(elements, size);
  floggerf(MEMORY, file, line, "calloc\tsize: %i\tpointer: %08x", elements * size, p);
  return p;
}

void * my_realloc(void * p, int size, char * file, int line){
  void * old_p = p;
  p = realloc(p, size);
  floggerf(MEMORY, file, line, "realloc\told_pointer: %08x\tnew_size: %i\tnew_pointer: %08x", old_p, size, p);
  return p;
}

void * my_free(void * p, char * file, int line){
  free(p);
  floggerf(MEMORY, file, line, "free\tpointer: %08x", p);
  return 0;
}

void _SYS_change(int STATE,char send){
  printf("_SYS_change %x\n",_SYS->_STATE);
  //printf("%x & %x = %i",_SYS->_STATE &);
  if(_SYS->_STATE & STATE && send & 0x02){
    _SYS->_STATE &= 0xFFFF ^ STATE;
  }else if(!(_SYS->_STATE & STATE)){
    _SYS->_STATE |= STATE;
  }
  loggerf(INFO, "_SYS_change %x\n", _SYS->_STATE);

  if(send & 0x01){
    char data[5];
    data[0] = WSopc_Service_State;
    data[1] = _SYS->_STATE >> 8;
    data[2] = _SYS->_STATE & 0xFF;
    send_all(data,3,WS_Flag_Admin || WS_Flag_Track);
  }
}

void sigint_func(int sig){
  if(sig == SIGINT){
    printf("-- SIGINT -- STOPPING");
    logger("-- SIGINT -- STOPPING",INFO);
    _SYS_change(STATE_RUN | STATE_Client_Accept, 3);
  }
}

int find_free_index(void ** list, int * length){
  if(!list){
    logger("LIST DOESNT EXIST",CRITICAL);
    return -1;
  }
  for(int i = 0;i<*length;i++){
    if(!list[i]){
      return i;
    }
  }

  list = _realloc(list,*length+2, void *);
  *length += 2;
  logger("EXPANDING LIST",INFO);
  return find_free_index(list, length);
}
