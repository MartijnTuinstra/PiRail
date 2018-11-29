#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <errno.h>

#include "system.h"
#include "websocket_control.h"
#include "logger.h"
#include "algorithm.h"

#include "mem.h"

void _SYS_change(int STATE,char send){
  printf("_SYS_change %x\n",_SYS->_STATE);
  //printf("%x & %x = %i",_SYS->_STATE &);
  if(_SYS->_STATE & STATE && send & 0x02){
    _SYS->_STATE &= 0xFFFF ^ STATE;
  }else if(!(_SYS->_STATE & STATE)){
    _SYS->_STATE |= STATE;
  }
  loggerf(INFO, "_SYS_change %x", _SYS->_STATE);

  if(send & 0x01){
    char data[5];
    data[0] = WSopc_Service_State;
    data[1] = _SYS->_STATE >> 8;
    data[2] = _SYS->_STATE & 0xFF;
    ws_send_all(data,3,WS_Flag_Admin || WS_Flag_Track);
  }
}

void sigint_func(int sig){
  if(sig == SIGINT){
    printf("-- SIGINT -- STOPPING");
    logger("-- SIGINT -- STOPPING",INFO);
    _SYS_change(STATE_RUN | STATE_Client_Accept, 3);

    sem_post(&AlgorQueueNoEmpty);
  }
}

int _find_free_index(void *** list, int * length){
  if(!(*list)){
    logger("LIST DOESNT EXIST",CRITICAL);
    return -1;
  }
  for(int i = 0;i<(*length);i++){
    if(!(*list)[i]){
      return i;
    }
  }

  *list = _realloc(*list, (*length)+1, void *);
  for(int i = *length; i < (*length)+1; i++){
    (*list)[i] = 0;
  }
  *length += 1;
  return _find_free_index(list, length);
}

void move_file(char * src, char * dest){
  int ch;
  FILE *source, *target;

  source = fopen(src, "r");

  if (source == NULL){
    loggerf(ERROR, "Could not open source file (%s)", src);
    exit(EXIT_FAILURE);
  }

  target = fopen(dest, "w");

  if (target == NULL){
    fclose(source);
    loggerf(ERROR, "Could not create destination file (%s)", dest);
    exit(EXIT_FAILURE);
  }

  printf("Start Copying\n");

  while ((ch = fgetc(source)) != EOF)
    fputc((char)ch, target);

  printf("Stop Copying\n");

  fclose(source);
  fclose(target);
}

void mutex_lock(pthread_mutex_t * m, char * mutex_name){
  loggerf(TRACE, "Lock mutex %s", mutex_name);
  pthread_mutex_lock(m);
}

void mutex_unlock(pthread_mutex_t * m, char * mutex_name){
  loggerf(TRACE, "UNLock mutex %s", mutex_name);
  pthread_mutex_unlock(m);
}
