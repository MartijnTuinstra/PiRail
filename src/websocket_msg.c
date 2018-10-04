#define _BSD_SOURCE
// #define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include <pthread.h>

//Websocket opcodes
#include "websocket_control.h"
#include "websocket_msg.h"
#include "websocket.h"

#include "system.h"
#include "mem.h"

#include "rail.h"
#include "switch.h"
#include "train.h"
#include "logger.h"

#include "module.h"
#include "Z21.h"


#define ACTIVATE 0
#define RELEASE  1

pthread_mutex_t mutex_lockB;

struct WS_Message MessageList[0x1FFF];
uint16_t MessageCounter = 0;

void WS_init_Message_List(){
  memset(MessageList,0,64);
  MessageCounter = 0;
}

char WS_init_Message(char type){
  if(MessageCounter >= 0x1FFF){
    MessageCounter = 0;
  }
  while((MessageList[MessageCounter].type & 0x8000) != 0){
    MessageCounter++;
    printf("Busy Message %i, Skip index %02X\n",MessageCounter,MessageList[MessageCounter].type);
    if(MessageCounter >= 0x1FFF){
      MessageCounter = 0;
    }
  }
  MessageList[MessageCounter].type = type + 0x8000;
  return MessageCounter++;
}

void WS_add_Message(uint16_t ID, char length,char data[16]){
  memcpy(MessageList[ID].data,data,length);
  MessageList[ID].data_length = length;
}

void WS_send_open_Messages(int Client_fd){
  for(int i = 0;i<=0x1FFF;i++){
    if(MessageList[i].type & 0x8000){
      ws_send(Client_fd,MessageList[i].data,MessageList[i].data_length,0xFF);
    }
  }
}

void WS_clear_message(uint16_t ID, char ret_code){
  if(ret_code == 1)
    MessageList[ID].type = 0;

  ws_send_all((char [3]){WSopc_ClearMessage,((ID >> 8) & 0x1F) + (ret_code << 5),ID&0xFF},3,0xFF);
}







void WS_EmergencyStop(){
  loggerf(WARNING, "EMERGENCY STOP");
  ws_send_all((char []){WSopc_EmergencyStop},1,0xFF); //Everyone
}

void WS_ShortCircuit(){
  loggerf(WARNING, "SHORT CIRCUIT");
  ws_send_all((char []){WSopc_ShortCircuitStop},1,0xFF); //Everyone
}

void WS_ClearEmergency(){
  loggerf(INFO, "EMERGENCY Released");
  ws_send_all((char []){WSopc_ClearEmergency},1,0xFF); //Everyone
}



void WS_EnginesLib(int client_fd){
  int buffer_size = 1024;

  char * data = (char *)calloc(buffer_size, 1);

  int len = 0;

  data[len++] = WSopc_EnginesLibrary;

  for(int i = 0;i<engines_len;i++){
    if(buffer_size < (len + 100)){
      buffer_size += 1024;
      data = realloc(data, buffer_size);
    }
    if(engines[i]){
      data[len++] = engines[i]->DCC_ID & 0xFF;
      data[len++] = engines[i]->DCC_ID >> 8;
      data[len++] = engines[i]->max_speed & 0xFF;
      data[len++] = engines[i]->max_speed >> 8;
      data[len++] = engines[i]->length & 0xFF;
      data[len++] = engines[i]->length >> 8;
      data[len++] = engines[i]->type;
      data[len++] = strlen(engines[i]->name);
      data[len++] = strlen(engines[i]->img_path);
      data[len++] = strlen(engines[i]->icon_path);

      int l = strlen(engines[i]->name);
      memcpy(&data[len], engines[i]->name, l);
      len += l;

      l = strlen(engines[i]->img_path);
      memcpy(&data[len], engines[i]->img_path, l);
      len += l;

      l = strlen(engines[i]->icon_path);
      memcpy(&data[len], engines[i]->icon_path, l);
      len += l;
    }
  }
  if(client_fd <= 0){
    ws_send_all(data, len, WS_Flag_Trains);
  }
  else{
    ws_send(client_fd, data, len, WS_Flag_Trains);
  }
}

void WS_CarsLib(int client_fd){
  loggerf(INFO, "CarsLib for client %i", client_fd);
  int buffer_size = 1024;

  char * data = (char *)calloc(buffer_size, 1);

  int len = 0;

  data[len++] = WSopc_CarsLibrary;

  for(int i = 0;i<cars_len;i++){
    if(buffer_size < (len + 100)){
      buffer_size += 1024;
      data = realloc(data, buffer_size);
    }
    if(cars[i]){
      data[len++] = cars[i]->nr & 0xFF;
      data[len++] = cars[i]->nr >> 8;
      data[len++] = cars[i]->max_speed & 0xFF;
      data[len++] = cars[i]->max_speed >> 8;
      data[len++] = cars[i]->length & 0xFF;
      data[len++] = cars[i]->length >> 8;
      data[len++] = cars[i]->type;
      data[len++] = strlen(cars[i]->name);
      data[len++] = strlen(cars[i]->img_path);
      data[len++] = strlen(cars[i]->icon_path);

      int l = strlen(cars[i]->name);
      memcpy(&data[len], cars[i]->name, l);
      len += l;

      l = strlen(cars[i]->img_path);
      memcpy(&data[len], cars[i]->img_path, l);
      len += l;

      l = strlen(cars[i]->icon_path);
      memcpy(&data[len], cars[i]->icon_path, l);
      len += l;
    }
  }
  if(client_fd <= 0){
    ws_send_all(data, len, WS_Flag_Trains);
  }
  else{
    ws_send(client_fd, data, len, WS_Flag_Trains);
  }
}

void WS_TrainsLib(int client_fd){
  loggerf(INFO, "TrainsLib for client %i", client_fd);
  int buffer_size = 1024;

  char * data = (char *)calloc(buffer_size, 1);

  int len = 0;

  data[len++] = WSopc_TrainsLibrary;

  for(int i = 0;i<trains_len;i++){
    if(buffer_size < (len + 100)){
      buffer_size += 1024;
      data = realloc(data, buffer_size);
    }
    if(trains[i]){
      data[len++] = trains[i]->max_speed & 0xFF;
      data[len++] = trains[i]->max_speed >> 8;
      data[len++] = trains[i]->length & 0xFF;
      data[len++] = trains[i]->length >> 8;
      data[len++] = trains[i]->type;
      data[len++] = strlen(trains[i]->name);
      data[len++] = trains[i]->nr_stock;

      int l = strlen(trains[i]->name);
      memcpy(&data[len], trains[i]->name, l);
      len += l;

      for(int c = 0; c<trains[i]->nr_stock; c++){
        data[len++] = trains[i]->composition[c].type;
        data[len++] = trains[i]->composition[c].id & 0xFF;
        data[len++] = trains[i]->composition[c].id >> 8;
      }
    }
  }
  if(client_fd <= 0){
    ws_send_all(data, len, WS_Flag_Trains);
  }
  else{
    ws_send(client_fd, data, len, WS_Flag_Trains);
  }
}



void WS_Partial_Layout(uint8_t M_A,uint8_t M_B){

  char data[20];
  int q = 1;
  memset(data,0,20);
  data[0] = WSopc_Track_PUp_Layout;

  printf("WS_Partial_Layout\n");
  printf("Checking Module A, %i\n",M_A);
  data[q++] = M_A;
  for(int i = 0;i<Units[M_A]->connections_len;i++){
    if(Units[M_A]->connection[i]){
      printf(" - Connect found, module %i\n",Units[M_A]->connection[i]->module);
      data[q++] = Units[M_A]->connection[i]->module;
    }
    else{
      printf("Reset\n");
      q = 1;
      break;
    }
  }

  printf("Checking Module B, %i\n",M_B);

  data[q++] = M_B;
  for(int i = 0;i<Units[M_B]->connections_len;i++){
    if(Units[M_B]->connection[i]){
      printf(" - Connect found, module %i\n",Units[M_B]->connection[i]->module);
      data[q++] = Units[M_B]->connection[i]->module;
    }
    else{
      printf("Reset\n");
      q = 1;
      break;
    }
  }

  if(q > 1){
    printf("Send %i\n",q);
    ws_send_all(data,q,WS_Flag_Admin);
  }

}

void WS_Track_Layout(){

  char data[100];
  int q = 1;
  memset(data,0,100);
  data[0] = WSopc_Track_Layout;

  printf("WS_Track_Layout\n");

  for(int i = 0;i<unit_len;i++){
    if(Units[i]){
      data[q++] = i;
      // loggerf(ERROR, "Fix CONNECT_POINTS UNIT");
      // for(int j = 0;j<Units[i]->connect_points;j++){
      //   if(Units[i]->Connect[j]){
      //     printf(" - Connect found, module %i\n",Units[i]->Connect[j]->Module);
      //     data[q++] = Units[i]->Connect[j]->Module;
      //   }
      //   else{
      //     printf(" - No Connect found\n");
      //     data[q++] = 0;
      //   }
      // }
    }
  }

  if(q > 1){
    printf("Send %i\n",q);
    ws_send_all(data,q,WS_Flag_Track);
  }

}


void WS_trackUpdate(int Client_fd){
  pthread_mutex_lock(&mutex_lockB);
  char data[4096];

  data[0] = WSopc_BroadTrack; //Opcode

  int data_len;
  _Bool content = 0;

  int q = 1;

  loggerf(TRACE, "WS_trackUpdate");

  for(int i = 0;i<unit_len;i++){
    if(!Units[i] || !Units[i]->block_state_changed)
      continue;

    loggerf(INFO, "WS_Block Update module %i", i);
    Units[i]->block_state_changed = 0;

    for(int j = 0;j<Units[i]->block_len;j++){
      Block * B = Units[i]->B[j];
      if(B && (B->changed & State_Changed)){
        content = 1;

        data[(q-1)*4+1] = B->module;
        data[(q-1)*4+2] = B->id;
        data[(q-1)*4+3] = (B->dir << 7) + B->state;
        data[(q-1)*4+4] = B->train;
        q++;

        B->changed = 0;
      }
    }
  }

  data_len = (q-1)*4+1;

  if(content == 1){
    if(Client_fd){
      ws_send(Client_fd,data,data_len,WS_Flag_Track);
    }else{
      ws_send_all(data,data_len,WS_Flag_Track);
    }
  }
  pthread_mutex_unlock(&mutex_lockB);
}

void WS_SwitchesUpdate(int Client_fd){
  loggerf(TRACE, "WS_SwitchesUpdate (%i)", Client_fd);
  pthread_mutex_lock(&mutex_lockB);
  char buf[4096];
  memset(buf, 0, 4096);
  /*Switches*/

    buf[0] = WSopc_BroadSwitch;
    int buf_l  = 0;
    _Bool content   = 0;

    int q = 1;
    //printf("\n\n3");

    for(int i = 0;i<unit_len;i++){
      if(!Units[i] || !Units[i]->switch_state_changed)
        continue;
      Units[i]->switch_state_changed = 0;

      for(int j = 0;j<Units[i]->switch_len;j++){
        Switch * S = Units[i]->Sw[j];
        if(S){
          if((S->state & 0x80) != 0x80){
            continue;
          }
          content = 1;
          buf[(q-1)*3+1] = S->module;
          buf[(q-1)*3+2] = S->id & 0x7F;
          buf[(q-1)*3+3] = S->state & 0x7F;
          S->state ^= 0x80;
          loggerf(DEBUG, "%i,%i,%i",S->module,S->id,S->state);
          q++;
        }
      }
    }

    buf_l = (q-1)*3+1;

  /*MSSwitches
    //buf[0] = 5;
    //buf_l = 0;
    q = 1;
    for(int i = 0;i<MAX_Modules;i++){
      if(!Units[i] && !Units[i]->switch_state_changed)
        continue;
      content = 1;
      for(int j = 0;j<=Units[i]->Mod_nr;j++){
        struct Mod * M = Units[i]->M[j];
        if(M){
          buf[(q-1)*4+1+buf_l] = M->Module;
          buf[(q-1)*4+2+buf_l] = (M->id & 0x7F) + 0x80;
          buf[(q-1)*4+3+buf_l] = M->state;
          buf[(q-1)*4+4+buf_l] = M->length;
          q++;
        }
      }
    }
  */
  //buf_l += (q-1)*4+1;
  if(content == 1){
    if(Client_fd){
      ws_send(Client_fd,buf,buf_l,WS_Flag_Switches);
    }else{
      ws_send_all(buf,buf_l,WS_Flag_Switches);
    }
  }
  
  pthread_mutex_unlock(&mutex_lockB);
}

void WS_NewClient_track_Switch_Update(int Client_fd){
  pthread_mutex_lock(&mutex_lockB);

  //Track
  char buf[4096];

  buf[0] = WSopc_BroadTrack; //Opcode

  int data_len;
  _Bool content = 0;

  int q = 1;

  for(int i = 0;i<unit_len;i++){
    if(Units[i]){
      for(int j = 0;j<=Units[i]->block_len;j++){
        Block * B = Units[i]->B[j];
        if(B){
          content = 1;

          buf[(q-1)*4+1] = B->module;
          buf[(q-1)*4+2] = B->id;
          buf[(q-1)*4+3] = (B->dir << 7) + B->state;
          buf[(q-1)*4+4] = B->train;
          q++;

          B->changed = 0;
        }
      }
    }
  }

  data_len = (q-1)*4+1;

  if(content == 1){
    if(Client_fd){
      ws_send(Client_fd,buf,data_len,WS_Flag_Track);
    }else{
      ws_send_all(buf,data_len,WS_Flag_Track);
    }
  }
  

  /*Switches*/

  memset(buf,0,4096);
  /*Switches*/

    buf[0] = WSopc_BroadSwitch;
    int buf_len = 0;
    content   = 0;

    q = 1; // Counter for switches

    for(int i = 0;i<unit_len;i++){
      if(Units[i]){
        for(int j = 0;j<=Units[i]->switch_len;j++){
          Switch * S = Units[i]->Sw[j];
          if(S){
            content = 1;
            buf[(q-1)*3+1] = S->module;
            buf[(q-1)*3+2] = S->id & 0x7F;
            buf[(q-1)*3+3] = S->state & 0x7F;
           // printf(",%i,%i,%i",S->Module,S->id,S->state);
            q++;
          }
        }
      }
    }

    buf_len = (q-1)*3+1;

  /*MSSwitches
    q = 1; // reset counter for MSSwitchs
    for(int i = 0;i<MAX_Modules;i++){
      if(Units[i]){
        content = 1;
        for(int j = 0;j<=Units[i]->Mod_nr;j++){
          struct Mod * M = Units[i]->M[j];
          if(M){
            buf[(q-1)*4+1+buf_l] = M->Module;
            buf[(q-1)*4+2+buf_l] = (M->id & 0x7F) + 0x80;
            buf[(q-1)*4+3+buf_l] = M->state;
            buf[(q-1)*4+4+buf_l] = M->length;
            q++;
          }
        }
      }
    }*/
  //buf_len += (q-1)*4+1;

  if(content == 1){
    if(Client_fd){
      printf("WS_SwitchesUpdate Custom Client");
      ws_send(Client_fd,buf,buf_len,WS_Flag_Switches);
    }else{
      printf("WS_SwitchesUpdate ALL");
      ws_send_all(buf,buf_len,WS_Flag_Switches);
    }
  }
  
  memset(buf,0,4096);
  /*Stations*/

  buf[0] = 6;
  buf_len = 1;
    _Bool data = 0;

  if(stations_len>0){
    data = 1;
  }
  for(int i = 0; i<stations_len;i++){
    printf("entry %i\tStation %i:%i\t%s\tbuf_l: %i\n",i,stations[i]->module,stations[i]->id,stations[i]->name,buf_len);

    buf[buf_len]   = stations[i]->module;
    buf[buf_len+1] = stations[i]->id;
    buf[buf_len+2] = strlen(stations[i]->name);
    strcpy(&buf[buf_len+3],stations[i]->name);

    buf_len+=3+strlen(stations[i]->name);
  }

  if(data == 1){
    ws_send(Client_fd,buf,buf_len,8);
  }

  memset(buf,0,4096);

  loggerf(INFO, "Z21_GET_LOCO_INFO check");
  for(int i = 1;i<trains_len;i++){
    if(train_link[i]){
      for(int j = 0; j < train_link[i]->nr_engines; j++){
        printf("Recall #%i\n",train_link[i]->engines[j]->DCC_ID);
        Z21_get_engine(train_link[i]->engines[j]->DCC_ID);
      }
    }
  }

  pthread_mutex_unlock(&mutex_lockB);
}


/*
void Web_Emergency_Stop(int i){
  char data[5];
  data[0] = 1;
  if(i == ACTIVATE){
    data[1] = 1;
  }else if(i == RELEASE){
    data[1] = 2;
  }else{
    return;
  }
  printf("Emergency_Stop (%i):[%i][%i]",i,data[0],data[1]);
  ws_send_all(data,2,1);
}
void Web_Electrical_Stop(int i){
  char data[5];
  data[0] = 1;
  if(i == ACTIVATE){
    data[1] = 3;
  }else if(i == RELEASE){
    data[1] = 4;
  }else{
    return;
  }
  ws_send_all(data,2,1);
}
void Web_Train_Split(int i,char tID,char B[]){
  printf("\n\nWeb_Train_Split(%i,%i,{%i,%i});\n\n",i,tID,B[0],B[1]);
  char data[8];
  data[0] = 1;
  if(i == ACTIVATE){
    data[1] = 5;
    data[2] = tID;
    data[3] = B[0];
    data[4] = B[1];
    ws_send_all(data,5,1);
  }else if(i == RELEASE){
    data[1] = 6;
    data[2] = tID;
    ws_send_all(data,3,1);
  }else{
    return;
  }
}
*/
void WS_NewTrain(char nr,char M,char B){
  //Nr:   follow id of train
  //M,B:  module nr and block nr
  uint16_t msg_ID = WS_init_Message(0);

  loggerf(INFO, "WS_NewTrain");

  char data[6];
  data[0] = WSopc_NewMessage;
  data[1] = ((msg_ID >> 8) & 0x1F) + 0; //type = 0
  data[2] = (msg_ID & 0xFF);
  data[3] = nr;
  data[4] = M;
  data[5] = B;
  ws_send_all(data,6,WS_Flag_Messages);
  WS_add_Message(msg_ID,6,data);
}

void WS_TrainSplit(char nr,char M1,char B1,char M2,char B2){
  //Nr:   follow id of train
  //M,B:  module nr and block nr
  uint16_t msg_ID = WS_init_Message(1);

  loggerf(INFO, "WS_TrainSplit");

  char data[8];
  data[0] = WSopc_NewMessage;
  data[1] = ((msg_ID >> 8) & 0x1F) + 0x20; //type = 1
  data[2] = (msg_ID & 0xFF);
  data[3] = nr;
  data[4] = M1;
  data[5] = B1;
  data[6] = M2;
  data[7] = B2;
  ws_send_all(data,8,WS_Flag_Messages);
  WS_add_Message(msg_ID,8,data);
}
/*
void Web_Link_Train(int type,char nr,char B[]){
  //Type: type of message ACTIVATE or RELEASE
  //Nr:   follow id of train
  //B:    a two byte array containing the module nr and block nr
  char data[8];
  if(type == ACTIVATE){
    data[0] = WSopc_NewMessage;
    data[1] = (WS_add_Message(0) & 0x1F) + 0;
    data[2] = B[0];
    data[3] = B[1];
    data[4] = nr;
    ws_send_all(data,5,1);
  }else if(type == RELEASE){
    data[0] = WSopc_ClearMessage;
    data[1] = (WS_add_Message(0) & 0x1F) + 0;
    data[1] = 12;
    data[2] = nr;
    data[3] = B[0];
    data[4] = B[1];
    data[5] = B[2];
    ws_send_all(data,6,1);
  }else{
    return;
  }
}*/

void WS_reset_switches(int client_fd){
  //Check if client has admin rights
  char admin = 1;
  if(admin){
    //Go through all switches
    for(int i = 0;i<unit_len;i++){
      for(int j = 0;j<Units[i]->switch_len;j++){
        if(Units[i]->Sw[j]){
          Units[i]->Sw[j]->state = Units[i]->Sw[j]->default_state + 0x80;
        }
      }
    }

    //Send all switch updates
    WS_SwitchesUpdate(client_fd);
  }
}

void WS_LinkTrain(uint8_t fID, uint8_t tID){
  ws_send_all((char []){WSopc_LinkTrain,fID,tID},3,0xFF);
}

void WS_TrainData(char data[14]){
  printf("\n\nWeb_Train_Data\n\n");
  char s_data[20];
  s_data[0] = WSopc_Z21TrainData;

  for(int i = 0;i<7;i++){
    s_data[i+2] = data[i];
  }

  loggerf(ERROR, "FIX ID");
  return;
  // s_data[1] = DCC_train[((s_data[2] << 8) + s_data[3])]->ID;

  ws_send_all(s_data,9,WS_Flag_Trains);
}
/*
void Web_Train_Data(char data[14]){
  printf("\n\nWeb_Train_Data;\n\n");
  char s_data[20];
  s_data[0] = 7;
  for(int i = 0;i<7;i++){
    s_data[i+1] = data[i];
  }
  ws_send_all(s_data,8,1);
}
*/




// Client to server
void WS_cts_AddCartoLib(struct s_opc_AddNewCartolib * data, struct web_client_t * client){
  loggerf(DEBUG, "WS_cts_AddCartoLib");
  struct s_WS_Data * rdata = _calloc(1, sizeof(struct s_WS_Data));

  rdata->opcode = WSopc_AddNewCartolib;
  rdata->data.opc_AddNewCartolib_res.nr = data->nr;

  char * name = _calloc(data->name_len, 1);
  char * img = _calloc(data->name_len + 8 + 3 + 20, 1);
  char * icon = _calloc(data->name_len + 8 + 3 + 20, 1);
  char * simg = _calloc(20 + 3, 1);
  char * sicon = _calloc(20 + 3, 1);
  char * filetype = _calloc(4, 1);

  memcpy(name, &data->strings, data->name_len);

  if((data->filetype & 0xf0) == 0){
    sprintf(img, "%s_%i.%s", name, data->nr, "png");
    sprintf(simg, "%s.%s", "web/tmp_img", "png");
  }
  else{
    sprintf(img, "%s_%i.%s", name, data->nr, "jpg");
    sprintf(simg, "%s.%s", "web/tmp_img", "jpg");
  }

  if((data->filetype & 0x0f) == 0){
    sprintf(icon, "%s_%i.%s", name, data->nr, "png");
    sprintf(sicon, "%s.%s", "web/tmp_icon", "png");
  }
  else{
    sprintf(icon, "%s_%i.%s", name, data->nr, "jpg");
    sprintf(sicon, "%s.%s", "web/tmp_icon", "jpg");
  }

  create_car(name, data->nr, img, icon, data->type, data->length, data->max_speed);

  char * dimg = _calloc(strlen(img)+10, 1);
  char * dicon = _calloc(strlen(icon)+10, 1);

  sprintf(dimg, "%s%s", "web/trains_img/", img);
  sprintf(dicon, "%s%s", "web/trains_img/", icon);

  move_file(simg,  dimg);
  move_file(sicon, dicon);

  train_write_confs();

  rdata->data.opc_AddNewCartolib_res.response = 1;
  ws_send(client->fd, (char *)rdata, WSopc_AddNewCartolib_res_len, 0xff);

  _free(dimg);
  _free(dicon);
  _free(simg);
  _free(sicon);
  _free(filetype);
}

void WS_cts_AddEnginetoLib(struct s_opc_AddNewEnginetolib * data, struct web_client_t * client){
  loggerf(DEBUG, "WS_cts_AddEnginetoLib");
  struct s_WS_Data * rdata = _calloc(1, sizeof(struct s_WS_Data));

  rdata->opcode = WSopc_AddNewEnginetolib;
  rdata->data.opc_AddNewEnginetolib_res.DCC_ID = data->DCC_ID;

  if (DCC_train[data->DCC_ID]){
    loggerf(ERROR, "DCC allready in use");
    rdata->data.opc_AddNewEnginetolib_res.response = 255;
    ws_send(client->fd, (char *)rdata, WSopc_AddNewEnginetolib_res_len, 0xff);
    return;
  }

  char * name = _calloc(data->name_len, 1);
  char * steps = _calloc(data->steps, 3);
  char * img = _calloc(data->name_len + 8 + 3 + 20, 1);
  char * icon = _calloc(data->name_len + 8 + 3 + 20, 1);
  char * simg = _calloc(20 + 3, 1);
  char * sicon = _calloc(20 + 3, 1);
  char * filetype = _calloc(4, 1);

  memcpy(name, &data->strings, data->name_len);
  memcpy(steps, &data->strings + data->name_len, data->steps);

  if((data->filetype & 0xf0) == 0){
    sprintf(img, "%i_%s.%s", data->DCC_ID, name, "png");
    sprintf(simg, "%s.%s", "web/tmp_img", "png");
  }
  else{
    sprintf(img, "%i_%s.%s", data->DCC_ID, name, "jpg");
    sprintf(simg, "%s.%s", "web/tmp_img", "jpg");
  }

  if((data->filetype & 0x0f) == 0){
    sprintf(icon, "%i_%s.%s", data->DCC_ID, name, "png");
    sprintf(sicon, "%s.%s", "web/tmp_icon", "png");
  }
  else{
    sprintf(icon, "%i_%s.%s", data->DCC_ID, name, "jpg");
    sprintf(sicon, "%s.%s", "web/tmp_icon", "jpg");
  }

  create_engine(name, data->DCC_ID, img, icon, data->fl, data->length, data->steps, (struct engine_speed_steps *)steps);

  char * dimg = _calloc(strlen(img)+10, 1);
  char * dicon = _calloc(strlen(icon)+10, 1);

  sprintf(dimg, "%s%s", "web/trains_img/", img);
  sprintf(dicon, "%s%s", "web/trains_img/", icon);

  move_file(simg,  dimg);
  move_file(sicon, dicon);

  train_write_confs();

  rdata->data.opc_AddNewEnginetolib_res.response = 1;
  ws_send(client->fd, (char *)rdata, WSopc_AddNewEnginetolib_res_len, 0xff);

  _free(dimg);
  _free(dicon);
  _free(simg);
  _free(sicon);
  _free(filetype);
}