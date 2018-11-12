#include "system.h"
#include "config.h"
#include "mem.h"
#include "train.h"
#include "module.h"
#include "Z21.h"
#include "websocket.h"
#include "websocket_control.h"

pthread_mutex_t m_websocket_send;

int websocket_decode(uint8_t data[1024], struct web_client_t * client){
  // Flag Admin Settings    0x80
  // Train stuff flag       0x40
  // Rail stuff flag        0x20
  // General Operation flag 0x10

  struct s_WS_Data * d = (struct s_WS_Data *)data;

  if((data[0] & 0xB0) == 0x80){ // System settings
    if(data[0] == WSopc_ClearTrack){

    }
    else if(data[0] == WSopc_ReloadTrack){

    }
    else if(data[0] == WSopc_Track_Scan_Progress){

    }
    else if(data[0] == WSopc_Track_Info){

    }
    else if(data[0] == WSopc_Reset_Switches){

    }
    else if(data[0] == WSopc_TrainsToDepot){

    }
    else if(data[0] == WSopc_EnableSubModule){

    }
    else if(data[0] == WSopc_DisableSubModule){

    }
    else if(data[0] == WSopc_RestartApplication){

    }

  }else if(data[0] & 0xB0){ //Admin settings
    if(data[0] == WSopc_Admin_Login){ //Admin login
      if(strcmp((char *)&data[1],WS_password) == 1){
        loggerf(INFO, "SUCCESSFULL LOGIN");
        //0xc3,0xbf,0x35,0x66,0x34,0x64,0x63,0x63,0x33,0x62,0x35,0x61,0x61,0x37,0x36,0x35,0x64,0x36,0x31,0x64,0x38,0x33,0x32,0x37,0x64,0x65,0x62,0x38,0x38,0x32,0x63,0x66,0x39,0x39
        client->type |= 0x10;

        ws_send(client->fd,(char [2]){WSopc_ChangeBroadcast,client->type},2,255);
      }else{
        loggerf(INFO, "FAILED LOGIN!!");
      }
    }
    if((client->type & 0x10) == 0){
      //Client is not an admin
      loggerf(INFO, "Not an Admin client");
      return 0;
    }


    if(data[0] == WSopc_Track_Scan_Progress){
      if(data[1] == 1){
        //Stop connecting
        _SYS_change(STATE_Modules_Coupled,1);
      }else if(data[1] == 2){
        //reload setup
        loggerf(ERROR, "Reload setup not implemented");
      }
    }
    else if(data[0] == WSopc_Admin_Logout){
      client->type &= ~0x10;

      ws_send(client->fd,(char [2]){WSopc_ChangeBroadcast,client->type},2,255);
    }
    else if(data[0] == WSopc_EmergencyStopAdmin){

    }
    else if(data[0] == WSopc_EmergencyStopAdminR){

    }
  }
  else if(data[0] & 0x40){ //Train stuff
    if(data[0] == WSopc_LinkTrain){ //Link train
      uint8_t fID = data[1]; //follow ID
      uint8_t tID = data[2]; //TrainID
      uint16_t mID = ((data[3] & 0x1F) << 8)+data[4];
      char return_value;
      loggerf(INFO, "Linking train %i with %s\n",fID,trains[tID]->name);
      #warning FIX
      if((return_value = link_train(fID,tID,data[3] & 0x80)) == 1){
        WS_clear_message(mID, 1);

        Z21_get_train(trains[tID]);
      }
      else{
        loggerf(WARNING, "Failed link_train()\n");
        WS_clear_message(mID, return_value); //Failed
      }
    }
    else if(data[0] == WSopc_TrainSpeed){ //Train speed control
      loggerf(INFO, "Train speed control\n");
      loggerf(ERROR,"RE-IMPLEMENT WSopc_TrainSpeed");
      uint8_t tID = data[1];
      uint8_t speed = data[2];
      trains[tID]->cur_speed = speed & 0x7F;
      trains[tID]->dir       = speed >> 7;

      Z21_get_train(trains[tID]);
    }
    else if(data[0] == WSopc_TrainFunction){ //Train function control

    }
    else if(data[0] == WSopc_TrainOperation){ //Train operation change

    }
    else if(data[0] == WSopc_TrainAddRoute){ //Add route to train

    }

    else if(data[0] == WSopc_AddNewCartolib){
      WS_cts_AddCartoLib((void *)&d->data, client);
    }
    else if(d->opcode == WSopc_EditCarlib){
      uint16_t id = d->data.opc_EditCarlib.id_l + (d->data.opc_EditCarlib.id_h << 8);
      if(d->data.opc_EditCarlib.remove){
        clear_car(&cars[id]);
      }
      else{
        loggerf(ERROR, "Implement Car Edit id:%i", id);
      }
      WS_CarsLib(0);
    }

    else if(data[0] == WSopc_AddNewEnginetolib){
      WS_cts_AddEnginetoLib((void *)&d->data, client);
    }
    else if(data[0] == WSopc_EditEnginelib){ //Edit / Remove Engine
      uint16_t id = d->data.opc_EditEnginelib.id_l + (d->data.opc_EditEnginelib.id_h << 8);
      if(d->data.opc_EditEnginelib.remove){
        clear_engine(&engines[id]);
      }
      else{
        loggerf(ERROR, "Implement Engine Edit id:%i", id);
      }
      WS_EnginesLib(0);
    }

    else if(data[0] == WSopc_AddNewTraintolib){
      loggerf(WARNING, "Opcode %x found", data[0]);
      WS_cts_AddTraintoLib((void *)&d->data, client);
    }
    else if(d->opcode == WSopc_EditTrainlib){
      uint16_t id = d->data.opc_EditTrainlib.id_l + (d->data.opc_EditTrainlib.id_h << 8);
      if(d->data.opc_EditTrainlib.remove){
        clear_train(&trains[id]);
      }
      else{
        loggerf(ERROR, "Implement Train Edit id:%i", id);
      }
      WS_TrainsLib(0);
    }

    else{
      loggerf(WARNING, "Opcode %x not found", data[0]);
    }
  }
  else if(data[0] & 0x20){ //Track stuff
    if(data[0] == WSopc_SetSwitch){ //Toggle switch
      if(Units[data[1]] && U_Sw(data[1], data[2])){ //Check if switch exists
        loggerf(INFO, "throw switch %i:%i to state: \t%i->%i",
                data[1], data[2], U_Sw(data[1], data[2])->state, !U_Sw(data[1], data[2])->state);
        throw_switch(U_Sw(data[1], data[2]), data[3]);
      }
    }
    else if(data[0] == WSopc_SetMultiSwitch){ // Set mulitple switches at once
      loggerf(INFO, "Throw multiple switches\n");
      throw_multiple_switches(data[1], (char *)&data[2]);
    }
    else if(data[0] == WSopc_SetSwitchReserved){ //Set switch reserved

    }
    else if(data[0] == WSopc_SetSwitchRoute){ //Set a route for switches

    }
  }
  else if(data[0] & 0x10){ // General Operation
    if(data[0] == WSopc_EmergencyStop){ //Enable Emergency Stop!!
      WS_EmergencyStop();
    }
    else if(data[0] == WSopc_ClearEmergency){ //Disable Emergency Stop!!
      WS_ClearEmergency();
    }
    else if(data[0] == WSopc_ClearMessage){

    }
    else if(data[0] == WSopc_ChangeBroadcast){
      // clients[client_data->thread_id]->client_type; //current flags
      // data[1]; //new flags
      if(data[1] & 0x10){ //Admin flag
        return 0; //Not allowed to set admin flag
        printf("Changing admin flag: NOT ALLOWED\n");
      }else if(data[1] != 0){
        client->type = data[1];
        printf("Changing flags\n");
      }
      loggerf(DEBUG,"Websocket:\t%02x - New flag for client %d\n",client->type, client->id);
      ws_send(client->fd,(char [2]){WSopc_ChangeBroadcast,client->type},2,255);
    }
  }
  return 0;
}

int websocket_get_msg(int fd, char outbuf[], int * length_out){
  char * buf = _calloc(1024, char);
  usleep(10000);
  recv(fd,buf,1024,0);

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

  for(uint16_t q = 0;q<mes_length;q++){
    printf("%02x ",output[q]);
  }
  printf(" %s\n",output);

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

void ws_send(int fd, char data[], int length, int flag){
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

  if(!(_SYS->_STATE & STATE_WebSocket_FLAG)){
    return;
  }

  websocket_create_msg(data, length, outbuf, &length);

  pthread_mutex_lock(&m_websocket_send);

  for(int i = 0; i<MAX_WEB_CLIENTS; i++){
    if(websocket_clients[i].state == 1 && (websocket_clients[i].type & flag) != 0){
      // printf("WS send (%i)\t",i);
      // for(int zi = 0; zi<(length); zi++){ printf("%02X ",data[zi]); };
      // printf("\n");

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
