#include "websocket_control.h"
#include "logger.h"

int websocket_connect(struct web_client_t * C){
  loggerf(ERROR, "FIX websocket_connect");
}

int recv_packet(int fd_client, char outbuf[], int * L){
  loggerf(ERROR, "FIX recv_packet");
}

int send_packet(int fd_client, char data[],int length,int flag){
  loggerf(ERROR, "FIX send_packet");
}

void send_all(char data[],int length,int flag){
  loggerf(ERROR, "FIX send_all");
}

void * websocket_client(void * thread_data){
  loggerf(ERROR, "FIX websocket_client");
}

void *clear_clients(){
  loggerf(ERROR, "FIX clear_clients");
}

void * web_server(){
  loggerf(ERROR, "FIX web_server");
}