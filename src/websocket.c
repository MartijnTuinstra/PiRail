#include "system.h"
#include "config.h"
#include "mem.h"
#include "train.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "modules.h"
#include "Z21.h"
#include "Z21_msg.h"
#include "websocket.h"
#include "websocket_control.h"
#include "algorithm.h"

#include "websocket_cts.h"
#include "websocket_stc.h"

pthread_mutex_t m_websocket_send;


// websocket_cts[WSopc_LinkTrain] = WS_cts_LinkTrain;

int websocket_parse(uint8_t data[1024], struct web_client_t * client){
  // Flag Admin Settings    0x80
  // Train stuff flag       0x40
  // Rail stuff flag        0x20
  // General Operation flag 0x10

  struct s_WS_Data * d = (struct s_WS_Data *)data;

  if(websocket_cts[d->opcode]){
    websocket_cts[d->opcode]((void *)&d->data, client);
    return 0;
  }

  loggerf(WARNING, "Not Using function array");
  if((data[0] & 0xC0) == 0x80){ // System settings
    loggerf(TRACE, "System Settings");
    if(data[0] == WSopc_ClearTrack){

    }
    else if(data[0] == WSopc_ReloadTrack){
      loggerf(TRACE, "WSopc_ReloadTrack");

    }
    else if(data[0] == WSopc_Track_Scan_Progress){
      loggerf(TRACE, "WSopc_Track_Scan_Progress");

    }
    else if(data[0] == WSopc_Track_Info){
      loggerf(TRACE, "WSopc_Track_Info");

    }
    else if(data[0] == WSopc_Reset_Switches){
      loggerf(TRACE, "WSopc_Reset_Switches");

    }
    else if(data[0] == WSopc_TrainsToDepot){
      loggerf(TRACE, "WSopc_TrainsToDepot");

    }
    else if(data[0] == WSopc_Z21_Settings){
      Z21_info.IP[0] = data[1];
      Z21_info.IP[1] = data[2];
      Z21_info.IP[2] = data[3];
      Z21_info.IP[3] = data[4];
      WS_stc_Z21_IP(0);
    }
    else if(data[0] == WSopc_SubModuleState){
      loggerf(TRACE, "WSopc_SubModuleState");
    }
    else if(data[0] == WSopc_RestartApplication){
      loggerf(TRACE, "WSopc_RestartApplication");

    }

  }else if((data[0] & 0xC0) == 0xC0){ //Admin settings
    loggerf(TRACE, "Admin Settings  %02X", data[0]);
    if((client->type & 0x10) == 0){
      //Client is not an admin
      loggerf(INFO, "Not an Admin client");
      return 0;
    }


    if(data[0] == WSopc_Track_Scan_Progress){
      if(data[1] == 1){
        //Stop connecting
        SYS->modules_linked = 1;
      }else if(data[1] == 2){
        //reload setup
        loggerf(ERROR, "Reload setup not implemented");
      }
    }
    else if(data[0] == WSopc_EmergencyStopAdmin){

    }
    else if(data[0] == WSopc_EmergencyStopAdminR){

    }
  }
  else if(data[0] & 0x40){ //Train stuff
    loggerf(TRACE, "Train Settings");
    if(data[0] == WSopc_TrainFunction){ //Train function control

    }
    else{
      loggerf(WARNING, "Opcode %x not found", data[0]);
    }
  }
  else if(data[0] & 0x20){ //Track stuff
    loggerf(INFO, "Track Settings");
    if(data[0] == WSopc_SetSwitchReserved){ //Set switch reserved

    }
    else if(data[0] == WSopc_SetSwitchRoute){ //Set a route for switches

    }
  }


  else if(data[0] & 0x10){ // General Operation
    loggerf(TRACE, "General Settings");
   if(data[0] == WSopc_ClearMessage){

    }
  }
  return 0;
}

int websocket_get_msg(int fd, char outbuf[], int * length_out){
  char * buf = (char *)_calloc(1024, char);
  usleep(10000);
  int32_t recvlength = recv(fd,buf,1024,0);

  if(recvlength <= 0){
    _free(buf);
    return -7;
  }

  print_hex(buf, recvlength);

  uint16_t byte = 0;

  //Websocket opcode
  int opcode = buf[byte++] & 0b00001111;

  uint16_t mes_length = buf[byte++] & 0b01111111;
  if(mes_length == 126){
    mes_length  = (buf[byte++] << 8);
    mes_length += buf[byte++];
  }else if(mes_length == 127){
    loggerf(ERROR, "To large message");
    _free(buf);
    return -1;
  }

  unsigned int masking_key = (buf[byte++] << 24);
  masking_key += (buf[byte++] << 16);
  masking_key += (buf[byte++] << 8);
  masking_key += (buf[byte++]);

  char output[mes_length+2];
  memset(output,0,mes_length+2);

  for(uint16_t i = 0;i<mes_length;){
    uint32_t data, masked;
    masked =  (buf[byte++] << 24);
    masked += (buf[byte++] << 16);
    masked += (buf[byte++] << 8);
    masked += (buf[byte++]);
    data = masked ^ masking_key;
      output[i++] = (data & 0xFF000000) >> 24;
      if(i<mes_length){
        output[i++] = (data & 0xFF0000) >> 16;
      }
      if(i<mes_length){
        output[i++] = (data & 0xFF00) >> 8;
      }
      if(i<mes_length){
        output[i++] = data & 0xFF;
      }
  }
  *length_out = mes_length;

  memcpy(outbuf, output, mes_length);

  printf("websocket recv ");
  print_hex(output, mes_length);

  _free(buf);

  if(opcode == 8){
    // Connection closed by client
    return -8;
  }
  return 1;
}

void websocket_create_msg(char * input, int length_in, char * output, int * length_out){
  char buf[10];
  int header_len = 0;
  buf[0] = WEBSOCKET_FIN | WEBSOCKET_BIN_FRAME;
  if(length_in < 126){
    buf[1] = length_in;
    header_len = 2;
  }else if(length_in < 65535){
    buf[1] = 126;
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

void ws_send(struct web_client_t * client, char * data, int length, int flag){
  char * outbuf = (char *)_calloc(length + 100, char);
  int outlength = 0;

  if(!(SYS->WebSocket.state == Module_Run || SYS->WebSocket.state == Module_Init)){
    _free(outbuf);
    return;
  }

  websocket_create_msg(data, length, outbuf, &outlength);

  pthread_mutex_lock(&m_websocket_send);

  if(write(client->fd, outbuf, outlength) == -1){
    if(errno == EPIPE){
      printf("Broken Pipe!!!!!\n\n");
    }
    else if(errno == EFAULT){
      loggerf(ERROR, "EFAULT ERROR");
    }
    else{
      loggerf(ERROR, "Unknown write error, closing connection");
    }
    close(client->fd);
    SYS->Clients--;
    client->state = 2;
  };

  pthread_mutex_unlock(&m_websocket_send);

  _free(outbuf);
}

void ws_send_all(char * data,int length,int flag){
  char * outbuf = (char *)_calloc(length + 100, char);
  int outlength = 0;

  if(!(SYS->WebSocket.state == Module_Run || SYS->WebSocket.state == Module_Init)){
    _free(outbuf);
    return;
  }

  websocket_create_msg(data, length, outbuf, &outlength);

  // printf("WS send (all)\t");
  // print_hex(data, length);

  pthread_mutex_lock(&m_websocket_send);

  for(int i = 0; i<MAX_WEB_CLIENTS; i++){
    if(websocket_clients[i].state == 1 && (websocket_clients[i].type & flag) != 0){
      if(write(websocket_clients[i].fd, outbuf, outlength) == -1){
        loggerf(WARNING, "socket write error %x", errno); 
        if(errno == EPIPE){
          printf("Broken Pipe!!!!!\n\n");
        }
        else if(errno == EFAULT){
          loggerf(ERROR, "EFAULT ERROR");
        }
        close(websocket_clients[i].fd);
        SYS->Clients--;
        websocket_clients[i].state = 2;
      }
    }
  }
  pthread_mutex_unlock(&m_websocket_send);

  _free(outbuf);
}
