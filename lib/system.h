
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

  #define _calloc(elements, type) my_calloc(elements, sizeof(type), __FILE__, __LINE__)
  #define _realloc(p, elements, type) my_realloc(p, sizeof(type) * elements, __FILE__, __LINE__)
  #define _free(p) my_free(p, __FILE__, __LINE__)

  void * my_calloc(int elements, int size, char * file, int line);
  void * my_realloc(void * p, int size, char * file, int line);
  void * my_free(void * p, char * file, int line);

  struct systemState{
    uint16_t _STATE;
    uint16_t _Clients;
    int _COM_fd;
  };

  struct adr{
    int M;    // Module
    int B;    // Block
    int S;    // Section
    int type; // Type
  };

  void _SYS_change(int STATE, char send);

  void sigint_func(int sig);

  int find_free_index(void ** list, int * length);

  extern struct systemState * _SYS; 

  #ifndef TRUE
    #define FALSE 0
    #define TRUE  1
  #endif

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