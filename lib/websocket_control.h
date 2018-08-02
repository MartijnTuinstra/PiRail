#ifndef INCLUDE_WEBSOCKET_CONTROL_H 
  #define INCLUDE_WEBSOCKET_CONTROL_H

  #include <pthread.h>

  #define WEBSOCKET_PORT 9000
  #define MAX_WEB_CLIENTS 10

  struct web_client_t{
    int fd;
    int type; /*Flags for client type
                      255 = All messages
                      1  = Trains
                      2  = Track
                      4  = Switches
                      8  = Messages
                      16 = Admin
                      32 = 
                      64 =
                      128=*/
    int id;
    pthread_t thread;
    int state;        /* If this client is active
                      0 = free
                      1 = in use
                      2 = stopping / ready to join thread*/
  };

  #include "websocket.h"

  extern char * WS_password;

  #include "websocket_control.h"
  #include "system.h"
  #include "logger.h"

  extern struct web_client_t * websocket_clients;
  extern char * WS_password;

  int websocket_client_check(struct web_client_t * C);

  void * websocket_client_connect(void * p);

  void *clear_clients();

  void read_password();

  void new_websocket_client(int fd);

  void * websocket_server();

  #include "websocket_msg.h"

#endif