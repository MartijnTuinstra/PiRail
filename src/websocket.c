#include "system.h"
#include "config.h"
#include "mem.h"
#include "train.h"
#include "switch.h"
#include "modules.h"
#include "Z21.h"
#include "Z21_msg.h"
#include "websocket.h"
#include "websocket_control.h"
#include "algorithm.h"

pthread_mutex_t m_websocket_send;

int websocket_decode(uint8_t data[1024], struct web_client_t * client){
  // Flag Admin Settings    0x80
  // Train stuff flag       0x40
  // Rail stuff flag        0x20
  // General Operation flag 0x10

  struct s_WS_Data * d = (struct s_WS_Data *)data;

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
    else if(data[0] == WSopc_DisableSubModule ||
            data[0] == WSopc_EnableSubModule){
      WS_cts_Enable_Disable_SubmoduleState(data[0], data[1]);
      
    }
    else if(data[0] == WSopc_SubModuleState){
      loggerf(TRACE, "WSopc_SubModuleState");
      WS_stc_SubmoduleState();
    }
    else if(data[0] == WSopc_RestartApplication){
      loggerf(TRACE, "WSopc_RestartApplication");

    }

  }else if((data[0] & 0xC0) == 0xC0){ //Admin settings
    loggerf(TRACE, "Admin Settings  %02X", data[0]);
    if(d->opcode == WSopc_Admin_Login){ //Admin login
      if(strcmp((char *)&data[1],WS_password) == 0){
        loggerf(INFO, "SUCCESSFULL LOGIN");
        //0xc3,0xbf,0x35,0x66,0x34,0x64,0x63,0x63,0x33,0x62,0x35,0x61,0x61,0x37,0x36,0x35,0x64,0x36,0x31,0x64,0x38,0x33,0x32,0x37,0x64,0x65,0x62,0x38,0x38,0x32,0x63,0x66,0x39,0x39
        client->type |= 0x10;

	      loggerf(INFO, "Change client flags to %x", client->type);

        ws_send(client,(char [2]){WSopc_ChangeBroadcast,client->type},2,255);
      }else{
        loggerf(INFO, "FAILED LOGIN!! %d", strcmp((char *)&data[1],WS_password));
        loggerf(INFO, "%s", &data[1]);
        loggerf(INFO, "%s", WS_password);
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
        SYS->modules_linked = 1;
      }else if(data[1] == 2){
        //reload setup
        loggerf(ERROR, "Reload setup not implemented");
      }
    }
    else if(data[0] == WSopc_Admin_Logout){
      client->type &= ~0x10;

      ws_send(client,(char [2]){WSopc_ChangeBroadcast,client->type},2,255);
    }
    else if(data[0] == WSopc_EmergencyStopAdmin){

    }
    else if(data[0] == WSopc_EmergencyStopAdminR){

    }
  }
  else if(data[0] & 0x40){ //Train stuff
    loggerf(TRACE, "Train Settings");
    if(d->opcode == WSopc_LinkTrain){ //Link train
      WS_cts_LinkTrain((void *)&d->data, client);
    }
    else if(data[0] == WSopc_TrainSpeed){ //Train speed control
      WS_cts_SetTrainSpeed((void *)&d->data, client);
    }
    else if(data[0] == WSopc_TrainFunction){ //Train function control

    }
    else if(data[0] == WSopc_TrainControl){ //Train operation change
      WS_cts_TrainControl(&d->data.opc_TrainControl, client);
    }
    else if(data[0] == WSopc_TrainAddRoute){ //Add route to train
      WS_cts_TrainRoute(&d->data.opc_TrainRoute, client);
    }
    else if(data[0] == WSopc_TrainSubscribe){
      WS_cts_TrainSubscribe((void *)&d->data, client);
    }

    else if(data[0] == WSopc_AddNewCartolib){
      WS_cts_AddCartoLib((void *)&d->data, client);
    }
    else if(d->opcode == WSopc_EditCarlib){
      uint16_t id = d->data.opc_EditCarlib.id_l + (d->data.opc_EditCarlib.id_h << 8);
      if(d->data.opc_EditCarlib.remove && cars[id]){
        Clear_Car(&cars[id]);
      }
      else{
        WS_cts_Edit_Car(cars[id], &(d->data.opc_EditCarlib.data), client);
      }
      write_rolling_Configs();
      WS_CarsLib(0);
    }

    else if(data[0] == WSopc_AddNewEnginetolib){
      WS_cts_AddEnginetoLib((void *)&d->data, client);
    }
    else if(data[0] == WSopc_EditEnginelib){ //Edit / Remove Engine
      WS_cts_Edit_Engine((void *)&(d->data), client);
    }

    else if(data[0] == WSopc_AddNewTraintolib){
      WS_cts_AddTraintoLib((void *)&d->data, client);
    }
    else if(d->opcode == WSopc_EditTrainlib){
      uint16_t id = d->data.opc_EditTrainlib.id_l + (d->data.opc_EditTrainlib.id_h << 8);
      if(d->data.opc_EditTrainlib.remove){
        Clear_Train(&trains[id]);
      }
      else{
        WS_cts_Edit_Train(trains[id], &(d->data.opc_EditTrainlib.data), client);
      }
      write_rolling_Configs();
      WS_TrainsLib(0);
    }

    else{
      loggerf(WARNING, "Opcode %x not found", data[0]);
    }
  }
  else if(data[0] & 0x20){ //Track stuff
    loggerf(INFO, "Track Settings");
    if(d->opcode == WSopc_SetSwitch){ //Toggle switch
      if(d->data.opc_SetSwitch.mssw){
        loggerf(INFO, "SetMSSw %2x %2x", d->data.opc_SetSwitch.module, d->data.opc_SetSwitch.id);
        if(Units[d->data.opc_SetSwitch.module] && U_MSSw(d->data.opc_SetSwitch.module, d->data.opc_SetSwitch.id)){
          loggerf(INFO, "throw msswitch %i:%i to state: \t%i->%i",
                  d->data.opc_SetSwitch.module, d->data.opc_SetSwitch.id, U_MSSw(d->data.opc_SetSwitch.module, d->data.opc_SetSwitch.id)->state, d->data.opc_SetSwitch.state);
          lock_Algor_process();
          throw_msswitch(U_MSSw(d->data.opc_SetSwitch.module, d->data.opc_SetSwitch.id), d->data.opc_SetSwitch.state, 1);
          unlock_Algor_process();
        }
      }
      else{
        loggerf(INFO, "SetSw %2x %2x", d->data.opc_SetSwitch.module, d->data.opc_SetSwitch.id);
        if(Units[data[1]] && U_Sw(d->data.opc_SetSwitch.module, d->data.opc_SetSwitch.id)){ //Check if switch exists
          loggerf(INFO, "throw switch %i:%i to state: \t%i->%i",
                  d->data.opc_SetSwitch.module, d->data.opc_SetSwitch.id, U_Sw(d->data.opc_SetSwitch.module, d->data.opc_SetSwitch.id)->state, !U_Sw(d->data.opc_SetSwitch.module, d->data.opc_SetSwitch.id)->state);
          lock_Algor_process();
          throw_switch(U_Sw(d->data.opc_SetSwitch.module, d->data.opc_SetSwitch.id), d->data.opc_SetSwitch.state, 1);
          unlock_Algor_process();
        }
      }
    }
    else if(data[0] == WSopc_SetMultiSwitch){ // Set mulitple switches at once
      loggerf(INFO, "Throw multiple switches\n");
      lock_Algor_process();
      throw_multiple_switches(data[1], (char *)&data[2]);
      unlock_Algor_process();
    }
    else if(data[0] == WSopc_SetSwitchReserved){ //Set switch reserved

    }
    else if(data[0] == WSopc_SetSwitchRoute){ //Set a route for switches

    }

    else if(data[0] == WSopc_TrackLayoutRawData){
      WS_stc_TrackLayoutRawData(data[1], client);
    }
  }


  else if(data[0] & 0x10){ // General Operation
    loggerf(TRACE, "General Settings");
    if(data[0] == WSopc_EmergencyStop){ //Enable Emergency Stop!!
      WS_EmergencyStop();
      Z21_TRACKPOWER_OFF;
    }
    else if(data[0] == WSopc_ClearEmergency){ //Disable Emergency Stop!!
      WS_ClearEmergency();
      Z21_TRACKPOWER_ON;
    }
    else if(data[0] == WSopc_ClearMessage){

    }
    else if(data[0] == WSopc_ChangeBroadcast){
      // clients[client_data->thread_id]->client_type; //current flags
      // data[1]; //new flags
      if(data[1] & 0x10){ //Admin flag
        loggerf(WARNING, "Changing admin flag: NOT ALLOWED");
        return 0; //Not allowed to set admin flag
      }else if(data[1] != 0){
        client->type = data[1];
        loggerf(DEBUG, "Changing flags");
      }
      loggerf(DEBUG,"Websocket:\t%02x - New flag for client %d\n",client->type, client->id);
      ws_send(client,(char [2]){WSopc_ChangeBroadcast,client->type},2,255);
    }
  }
  return 0;
}

int websocket_get_msg(int fd, char outbuf[], int * length_out){
  char * buf = _calloc(1024, char);
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
  char * outbuf = _calloc(length + 100, 1);
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
  char * outbuf = _calloc(length + 100, 1);
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
