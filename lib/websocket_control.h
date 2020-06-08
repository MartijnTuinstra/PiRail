#ifndef INCLUDE_WEBSOCKET_CONTROL_H 
  #define INCLUDE_WEBSOCKET_CONTROL_H

  #include <pthread.h>

  #define WEBSOCKET_PORT 9000
  #define MAX_WEB_CLIENTS 20
  #define WS_BUF_SIZE 1024

  #include "websocket.h"

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

    uint8_t trains[2];
  };

  extern char * WS_password;

  #include "system.h"
  #include "logger.h"

  extern struct web_client_t * websocket_clients;
  extern char * WS_password;

  #define WEBSOCKET_FIN 0x80
  #define WEBSOCKET_CONT_FRAME 0x00
  #define WEBSOCKET_TEXT_FRAME 0x01
  #define WEBSOCKET_BIN_FRAME 0x02

  #define WEBSOCKET_CLOSE 0x08
  #define WEBSOCKET_PING 0x09
  #define WEBSOCKET_PONG 0x0A

  int websocket_ping(int fd);

  int websocket_client_check(struct web_client_t * C);

  uint8_t websocket_client_first_connect(struct web_client_t * client, char * buf, int * length);

  void * websocket_client(void * p);

  void Websocket_ClearUnusedSockets(void * args);

  void read_password();

  void new_websocket_client(int fd);

  void * websocket_server();

  #include "websocket_stc.h"
  #include "websocket_cts.h"

#endif