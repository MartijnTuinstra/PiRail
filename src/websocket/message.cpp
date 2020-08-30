#include "websocket/message.h"
#include "websocket/message_structure.h"
#include "websocket/stc.h"

#include "logger.h"
#include "mem.h"
#include "system.h"

#include "submodule.h"
#include "Z21.h"
#include "algorithm.h"
#include "config.h"

const char websocket_magic_string[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

namespace Websocket {

int Parse(uint8_t data[1024], Client * client){
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
      memcpy(Z21->IP, &data[1], 4);

      FILE * fp = fopen("configs/Z21.bin", "wb");
      if (!fp){
        loggerf(ERROR, "Failed to create new Z21 config file.");
        return 0;
      }
      fwrite(Z21->IP, 4, 1, fp);
      fclose(fp);
      WS_stc_Z21_IP(0);
    }
    else if(data[0] == WSopc_SubModuleState){
      loggerf(TRACE, "WSopc_SubModuleState");
    }
    else if(data[0] == WSopc_RestartApplication){
      loggerf(TRACE, "WSopc_RestartApplication");
      SYS_set_state(&SYS->WebSocket.state, Module_STOP);
      Z21_stop();
      Algor_stop();
      SYS_set_state(&SYS->SimA.state, Module_STOP);
      SYS_set_state(&SYS->SimB.state, Module_STOP);
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

int MessageGet(int fd, char outbuf[], int * length_out){
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

void MessageCreate(char * input, int length_in, char * output, int * length_out){
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

} // Namespace