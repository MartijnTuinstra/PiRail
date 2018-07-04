#include "system.h"
#include "train.h"
#include "module.h"
#include "Z21.h"
#include "websocket.h"
#include "websocket_control.h"

pthread_mutex_t m_websocket_send;

int websocket_decode(char data[1024], struct web_client_t * client){
  // Flag Admin Settings    0x80
  // Train stuff flag       0x40
  // Rail stuff flag        0x20
  // General Operation flag 0x10
  printf("recv_packet_procces\n");

  if(data[0] & 0x80){ //Admin settings
    printf("Admin settings: %02X\n",data[0]);
    if(data[0] == 0xFF){ //Admin login
      printf("\nAdmin Login\n\n");
      if(strcmp(&data[1],WS_password) == 1){
        printf("\n\n\n\n\nSUCCESSFULL LOGIN\n\n");
        //0xc3,0xbf,0x35,0x66,0x34,0x64,0x63,0x63,0x33,0x62,0x35,0x61,0x61,0x37,0x36,0x35,0x64,0x36,0x31,0x64,0x38,0x33,0x32,0x37,0x64,0x65,0x62,0x38,0x38,0x32,0x63,0x66,0x39,0x39
        client->type |= 0x10;

        ws_send(client->fd,(char [2]){WSopc_ChangeBroadcast,client->type},2,255);
      }else{
        printf("\n\n\n\n\nFAILED LOGIN!!\n\n");
      }
    }
    if((client->type & 0x10) != 0x10){
      //Client is not an admin
      printf("Not an Admin client");
      return 0;
    }
    

    if(data[0] == WSopc_Track_Scan){
      if(data[1] == 1){
        //Stop connecting
        _SYS_change(STATE_Modules_Coupled,1);
      }else if(data[1] == 2){
        //reload setup
        printf("\n\nReload setup not implemented\n\n");
      }
    }


    if(data[0] == 0x83){ //Reset switches to default

    }
    else if(data[0] == 0x84){ //Toggle Light Output

    }
    else if(data[0] == 0x88){ //All trains back to depot

    }

    else if(data[0] == 0xA0){ //Force switch

    }

    else if(data[0] == 0x90){ //Emergency stop, Admin authority / Disable track voltage

    }
    else if(data[0] == 0x91){ //Emergency stop, Admin authority / Enable track voltage

    }
  }
  else if(data[0] & 0x40){ //Train stuff
    if(data[0] == WSopc_AddNewTraintolib){ //New train

    }
    else if(data[0] == WSopc_LinkTrain){ //Link train
      uint8_t fID = data[1]; //follow ID
      uint8_t tID = data[2]; //TrainID
      printf("Linking train %i with %s\n",fID,trains[tID]->name);
      #warning FIX
      if(link_train(fID,tID,data[3] & 0x80)){
        WS_clear_message(((data[3] & 0x1F) >> 8)+data[4]);
        
        Z21_get_train(trains[tID]);
      }
    }
    else if(data[0] == WSopc_TrainSpeed){ //Train speed control
      printf("Train speed control\n");
      loggerf(ERROR,"RE-IMPLEMENT WSopc_TrainSpeed");
      char tID = data[1];
      char speed = data[2];
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
      logger("WSopc_AddNewCartolib TODO",WARNING);
    }
    else if(data[0] == WSopc_AddNewEnginetolib){
      logger("WSopc_AddNewEnginetolib TODO",WARNING);
    }
    else if(data[0] == WSopc_AddNewTraintolib){
      logger("WSopc_AddNewTraintolib TODO",WARNING);
    }
  }
  else if(data[0] & 0x20){ //Track stuff
    if(data[0] == WSopc_SetSwitch){ //Toggle switch
      if(Units[data[1]] && Units[data[1]]->Sw[data[2]]){ //Check if switch exists
        printf("throw switch %i:%i to state: \t",data[1],data[2]);
        printf("%i->%i",Units[data[1]]->Sw[data[2]]->state, !Units[data[1]]->Sw[data[2]]->state);
        set_switch(Units[data[1]]->Sw[data[2]],data[3]);
      }
    }
    else if(data[0] == WSopc_SetMultiSwitch){ // Set mulitple switches at once
      printf("Throw multiple switches\n");
      set_multiple_switches(data[1],&data[2]);
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

  // printf("WS send (%i)\t",fd);
  // for(int zi = 0;zi<(length);zi++){printf("%02X ",data[zi]);};
  // printf("\n");

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