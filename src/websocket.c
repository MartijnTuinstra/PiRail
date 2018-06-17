#include "system.h"

#include "websocket.h"
#include "websocket_control.h"

pthread_mutex_t m_websocket_send;

int recv_packet_procces(char data[1024], struct web_client_t * client){
  loggerf(ERROR, "FIX recv_packet_procces");  
}

int websocket_get_msg(int fd_client, char outbuf[], int * L){
  loggerf(ERROR, "FIX recv_packet");
}

void websocket_create_msg(char * input, int length_in, char * output, int * length_out){
  char buf[10];
  int header_len = 0;
  buf[0] = 0b10000000 + 0b00000010;
  if(length_in < 126){
    buf[1] = length_in;
    header_len = 2;
  }else if(length_in < 65535){
    buf[1] = 126;
    buf[2] = 0xFF;
    buf[3] = 0xFF;
    //printf("data length: %i\n",strlen(data));
    buf[2] = (length_in & 0xFF00) >> 8;
    buf[3] = length_in & 0xFF;
    header_len = 4;
  }else{
    loggerf(ERROR, "Too large message, OPCODE: %02X", input[0]);
  }

  memcpy(output, buf, header_len);
  memcpy(&output[header_len], input, length_in);
}

int ws_send(int fd, char data[], int length, int flag){
  char outbuf[4096];

  websocket_create_msg(data, length, outbuf, &length);

  pthread_mutex_lock(&m_websocket_send);

  printf("WS send (%i)\t",fd);
  for(int zi = 0;zi<(length);zi++){printf("%02X ",data[zi]);};
  printf("\n");

  write(fd, outbuf, length);

  pthread_mutex_unlock(&m_websocket_send);
}

void ws_send_all(char data[],int length,int flag){
  char outbuf[4096];

  websocket_create_msg(data, length, outbuf, &length);

  pthread_mutex_lock(&m_websocket_send);
  
  for(int i = 0; i<MAX_WEB_CLIENTS; i++){
    if(websocket_clients[i].state == 1 && (websocket_clients[i].type & flag) != 0){
      printf("WS send (%i)\t",i);
      for(int zi = 0; zi<(length); zi++){ printf("%02X ",data[zi]); };
      printf("\n");

      if(write(websocket_clients[i].fd, outbuf, length) == -1){
        if(errno == EPIPE){
          printf("Broken Pipe!!!!!\n\n");
          close(websocket_clients[i].fd);
          _SYS->_Clients--;
          websocket_clients[i].state = 2;
        }
      }
    }
  }
  pthread_mutex_unlock(&m_websocket_send);
}