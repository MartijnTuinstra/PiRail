#ifndef _INCLUDE_system_H
#define _INCLUDE_system_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <openssl/sha.h>

#include <signal.h>
#include <errno.h>

enum e_SYS_Module_State {
  Module_STOP,
  Module_Init_Parent,
  Module_Init,
  Module_Run,
  Module_Fail,
  Module_LC_Searching,
  Module_LC_Connecting,
  Module_SIM_State
};

struct s_SYS_requiredSystem {
  void (*func)();
  enum e_SYS_Module_State * state;
};

struct s_systemState{
  uint16_t Clients;

  uint16_t track_scale;

  uint8_t emergency;

  uint8_t modules_loaded:1;
  uint8_t modules_linked:1;
  uint8_t trains_loaded:1;
  uint8_t digital:1;
  uint8_t stop:1;

  struct {
    volatile enum e_SYS_Module_State state;
    pthread_t start_th;
    pthread_t th;
  } Z21;

  struct {
    volatile enum e_SYS_Module_State state;
    uint8_t modules_found:1;
    uint8_t modules_coupled:1;
    pthread_t th;
  } UART;

  struct {
    volatile enum e_SYS_Module_State state;
    pthread_t th;
  } TC;

  struct {
    volatile enum e_SYS_Module_State state;
    pthread_t th;
  } LC;

  struct {
    volatile enum e_SYS_Module_State state;
    uint8_t accept_clients:1;
  } WebSocket;

  struct {
    volatile enum e_SYS_Module_State state;
    pthread_t th;
  } SimA;

  struct {
    volatile enum e_SYS_Module_State state;
    pthread_t th;
  } SimB;
};

void sigint_func(int sig);

#define find_free_index(list, length) _find_free_index((void ***)&list, (int *)&length)

int _find_free_index(void *** list, int * length);

uint8_t move_file(char * src, char * dest);

void system_init(struct s_systemState * S);
void init_main();

#define mutex_lock(mutex, name) loggerf(TRACE, "  Lock mutex %s", name);\
                                pthread_mutex_lock(mutex);
#define mutex_unlock(mutex, name) loggerf(TRACE, "unLock mutex %s", name);\
                                  pthread_mutex_unlock(mutex);


extern struct s_systemState * SYS; 

void SYS_set_state(volatile enum e_SYS_Module_State * system, enum e_SYS_Module_State state);

#ifndef TRUE
#define FALSE 0
#define TRUE  1
#endif

#endif