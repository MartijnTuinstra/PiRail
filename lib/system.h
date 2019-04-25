
#ifndef _INCLUDE_system_H
  #define _INCLUDE_system_H

  
  #define _BSD_SOURCE
  #define _GNU_SOURCE

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

  #include <wiringPi.h>
  #include <signal.h>

  #include <errno.h>

  struct systemState{
    uint16_t _STATE;
    uint16_t _Clients;

    uint8_t emergency;

    volatile uint8_t Z21_State:2;       // Z21  State
    volatile uint8_t UART_State:2;      // UART State
    volatile uint8_t TC_State:2;        // Train Control State
    volatile uint8_t LC_State:3;        // Layout Control State
    volatile uint8_t Websocket_State:2; // Websocket State
    volatile uint8_t SimA_State:2; // SimA State
    volatile uint8_t SimB_State:2; // SimB State
  };

  #define _SYS_Module_Stop 0
  #define _SYS_Module_Init 1
  #define _SYS_Module_Run  2
  #define _SYS_Module_Fail 3
  #define _SYS_LC_Searching 4
  #define _SYS_LC_Connecting 5

  struct adr{
    int M;    // Module
    int B;    // Block
    int S;    // Section
    int type; // Type
  };

  void _SYS_change(int STATE, char send);

  void sigint_func(int sig);

  #define find_free_index(list, length) _find_free_index((void ***)&list, (int *)&length)

  int _find_free_index(void *** list, int * length);

  void move_file(char * src, char * dest);

  #define mutex_lock(mutex, name) loggerf(TRACE, "  Lock mutex %s", name);\
                                  pthread_mutex_lock(mutex);
  #define mutex_unlock(mutex, name) loggerf(TRACE, "unLock mutex %s", name);\
                                    pthread_mutex_unlock(mutex);


  extern struct systemState * _SYS; 

  #ifndef TRUE
    #define FALSE 0
    #define TRUE  1
  #endif

  #define NO_LOCK 0x80

  #define TRACK_SCALE 160

  #define STATE_Z21_FLAG        0x8000
  #define STATE_WebSocket_FLAG  0x4000
  #define STATE_COM_FLAG        0x2000
  #define STATE_Client_Accept   0x1000

  #define STATE_TRACK_DIGITAL   0x0200
  #define STATE_RUN             0x0100

  #define STATE_Modules_Coupled 0x0008
  #define STATE_Modules_Loaded  0x0004

  #define STATE_TRAIN_LOADED    0x0001
#endif