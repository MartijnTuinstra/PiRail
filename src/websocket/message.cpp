#include "websocket/message.h"
#include "websocket/message_structure.h"
#include "websocket/stc.h"

#include "utils/logger.h"
#include "utils/mem.h"
#include "system.h"

#include "submodule.h"
#include "Z21.h"

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

int MessageGet(int fd, uint8_t ** outbuf, uint8_t ** packet, int bufferSize, int * length_out){
  // char * buf = (char *)_calloc(1024, char);
  // usleep(10000);

  int32_t recvlength = recv(fd, *outbuf, WEBSOCKET_HEADER_SIZE, 0);

  if(recvlength <= 0){
    return WEBSOCKET_NO_MESSAGE;
  }

  log_hex("WS Recv Message", *outbuf, recvlength);

  uint16_t byte = 2; // start of message
  uint8_t * buf = &(*outbuf)[0];

  //Websocket opcode
  int opcode = buf[0] & WEBSOCKET_OPCODE_MASK;

  uint64_t mes_length = buf[1] & WEBSOCKET_PAYLOAD_MASK;
  // Check for more length bytes
  if(mes_length == 126){
    mes_length  = (buf[2] << 8) + buf[3];
    byte += 2;
  }
  else if(mes_length == 127){
    mes_length  = (buf[2] << 24) + (buf[3] << 16) + (buf[4] << 8) + buf[5];
    mes_length <<= 32;
    mes_length  = (buf[6] << 24) + (buf[7] << 16) + (buf[8] << 8) + buf[9];
    loggerf(WARNING, "JUMBO packet");
    byte += 8;
    // return WEBSOCKET_FAILED_CLOSE;
  }

  buf = &(*outbuf)[byte]; // Set at start of mask key
  uint32_t masking_key = 0;

  if(buf[1] & WEBSOCKET_MASK_BIT){
    masking_key = ((uint32_t *)buf)[0];
    byte += 4;
  }

  buf = &(*outbuf)[byte];

  // allocate space for packet
  if(mes_length + WEBSOCKET_HEADER_SIZE > bufferSize){
    *outbuf = (uint8_t *)_realloc(*outbuf, mes_length + WEBSOCKET_HEADER_SIZE, uint8_t);
  }

  *packet = &(*outbuf)[byte];
  uint8_t timeout_counter = 0;

  loggerf(WARNING, "Packet starts at 0x%16x\n Header was %i bytes, allready read %i bytes\n need to read %i more", (unsigned long)(*packet), byte, recvlength, (byte+mes_length)-recvlength);
  // read whole packet
  while(recvlength < (byte + mes_length)){
    loggerf(WARNING, "Receiving %3i / %3i bytes,  0x%16x", (byte + mes_length) - recvlength, mes_length + byte, (unsigned long)&((*outbuf)[recvlength]));
    int32_t size = recv(fd, &((*outbuf)[recvlength]), (byte + mes_length) - recvlength, 0);
    loggerf(WARNING, "Got %i", size);

    if(size <= 0){
      timeout_counter++;

      if(timeout_counter > 5){
        loggerf(ERROR, "Failed to receive message");
        return WEBSOCKET_FAILED_CLOSE;
      }
    }
    else{
      timeout_counter = 0;
      recvlength += size;
    }
  }

  uint32_t itterations = (mes_length + 3) >> 2; // round up divide 4
  uint32_t * data = (uint32_t *)*packet;

  for(uint64_t i = 0; i < itterations; i++){
    data[i] ^= masking_key;
  }
  *length_out = mes_length;

  log_hex("WS Recv", *outbuf, mes_length + byte);

  (*packet)[mes_length] = 0;

  switch(opcode){
    case WEBSOCKET_CLOSE:
    case WEBSOCKET_PING:
    case WEBSOCKET_PONG:
      return WEBSOCKET_SUCCESS_CONTROL_FRAME;
  }

  return WEBSOCKET_SUCCESS;
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