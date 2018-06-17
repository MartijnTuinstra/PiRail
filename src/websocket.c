#include "system.h"

#include "websocket.h"
#include "websocket_control.h"

pthread_mutex_t m_websocket_send;

int recv_packet_procces(char data[1024], struct web_client_t * client){
  loggerf(ERROR, "FIX recv_packet_procces");  
}

int websocket_get_msg(int fd, char outbuf[], int * length_out){
  char * buf = _calloc(1024, char);
  usleep(10000);
  recv(fd,buf,1024,0);

  int byte = 0;
  
  //Websocket opcode
  int opcode = buf[byte++] & 0b00001111;
  
  unsigned int mes_length = buf[byte++] & 0b01111111;
  if(mes_length == 126){
    mes_length = (buf[byte++] << 8) + buf[byte++];
  }else if(mes_length == 127){
    loggerf(ERROR, "To large message");
    return -1;
  }

  int masking_index = byte;
  unsigned int masking_key = (buf[byte++] << 24) + (buf[byte++] << 16) + (buf[byte++] << 8) + (buf[byte++]);

  char output[mes_length+2];
  memset(output,0,mes_length+2);

  for(int i = 0;i<mes_length;0){
    unsigned int test;
    unsigned int text;
    test = (buf[byte++] << 24) + (buf[byte++] << 16) + (buf[byte++] << 8) + (buf[byte++]);
    text = test ^ masking_key;
      output[i++] = (text & 0xFF000000) >> 24;
      if(i<mes_length){
        output[i++] = (text & 0xFF0000) >> 16;
      }
      if(i<mes_length){
        output[i++] = (text & 0xFF00) >> 8;
      }
      if(i<mes_length){
        output[i++] = text & 0xFF;
      }
  }
  *length_out = mes_length;

  memcpy(outbuf, output, mes_length);
  printf("WS recieved data: ");
  for(int q = 0;q<mes_length;q++){
    printf("%02x ",output[q]);
  }
  printf("\n",output);

  if(opcode == 8){
    // Connection closed by client
    return -8;
  }
  return 1;
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

  *length_out = header_len + length_in;
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