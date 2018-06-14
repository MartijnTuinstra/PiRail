#ifndef INCLUDE_WEBSOCKET_CONTROL_H 
  #define INCLUDE_WEBSOCKET_CONTROL_H

  #include "websocket.h"

  #define WEBSOCKET_PORT 9000

  struct web_client_t{
    int fd_client;
    int client_type; /*Flags for client type
                      255 = All messages
                      1  = Trains
                      2  = Track
                      4  = Switches
                      8  = Messages
                      16 = Admin
                      32 = 
                      64 =
                      128=*/
    int state;        /* If this client is active
                      0 = free
                      1 = in use
                      2 = stopping / ready to join thread*/
  };

  struct websocket_client_thread_args{
    int fd_client;
    int thread_id;
  };


  int websocket_connect(struct web_client_t * C);

  int recv_packet(int fd_client, char outbuf[], int * L);

  int send_packet(int fd_client, char data[],int length,int flag);

  void send_all(char data[],int length,int flag);
  
  void * websocket_client(void * thread_data);

  void *clear_clients();

  void * web_server();

  #include "websocket_msg.h"

#endif