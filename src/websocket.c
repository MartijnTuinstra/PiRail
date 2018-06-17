#include "system.h"

#include "websocket.h"
#include "websocket_control.h"

int recv_packet_procces(char data[1024], struct web_client_t * client){
  loggerf(ERROR, "FIX recv_packet_procces");
}

int websocket_get_msg(int fd_client, char outbuf[], int * L){
  loggerf(ERROR, "FIX recv_packet");
}

int send_packet(int fd_client, char data[],int length,int flag){
  loggerf(ERROR, "FIX send_packet");
}

void send_all(char data[],int length,int flag){
  loggerf(ERROR, "FIX send_all");
}