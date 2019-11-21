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
#include "config.h"

#include "modules.h"
#include "Z21.h"

#include "submodule.h"


#define ACTIVATE 0
#define RELEASE  1

pthread_mutex_t mutex_lockB;

struct WS_Message MessageList[0x1FFF];
uint16_t MessageCounter = 0;


//System Messages
void WS_Partial_Layout(uint8_t M_A,uint8_t M_B){

  char data[20];
  int q = 1;
  int x = 1;
  memset(data,0,20);
  data[0] = WSopc_Track_Layout_Update;

  loggerf(INFO, "WS_Partial_Layout\n");
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

  x = q;

  printf("Checking Module B, %i\n",M_B);

  data[q++] = M_B;
  for(int i = 0;i<Units[M_B]->connections_len;i++){
    if(Units[M_B]->connection[i]){
      printf(" - Connect found, module %i\n",Units[M_B]->connection[i]->module);
      data[q++] = Units[M_B]->connection[i]->module;
    }
    else{
      printf("Reset\n");
      q = x;
      break;
    }
  }

  if(q > 1){
    printf("Send %i\n",q);
    ws_send_all(data,q,WS_Flag_Admin);
  }
}

void WS_Track_Layout(struct web_client_t * client){

  char data[100];
  int q = 1;
  memset(data,0,100);
  data[0] = WSopc_Track_Layout_Config;

  printf("WS_Track_Layout\n");

  for(int i = 0;i<unit_len;i++){
    if(!Units[i])
      continue;

    data[q++] = i;
    printf("Module %i\n", i);

    for(int j = 0;j<Units[i]->connections_len;j++){
      if(Units[i]->connection[j]){
        printf(" - Connect found, module %i\n",Units[i]->connection[j]->module);
        data[q++] = Units[i]->connection[j]->module;
      }
      else{
        printf(" - No Connect found\n");
        data[q++] = 0;
      }
    }
  }
  
  printf("Send %i\n",q);

  if(q > 1){
    if(client){
      ws_send(client, data,q,WS_Flag_Track);
    }
    else{
      ws_send_all(data,q,WS_Flag_Track);
    }
  }
}

void WS_stc_Z21_info(struct web_client_t * client){
  uint8_t data[20];
  data[0] = WSopc_Track_Info;
  data[1] = Z21_info.MainCurrent & 0xFF;
  data[2] = Z21_info.MainCurrent >> 8;
  data[3] = Z21_info.FilteredMainCurrent & 0xFF;
  data[4] = Z21_info.FilteredMainCurrent >> 8;
  data[5] = Z21_info.ProgCurrent & 0xFF;
  data[6] = Z21_info.ProgCurrent >> 8;
  data[7] = Z21_info.VCCVoltage & 0xFF;
  data[8] = Z21_info.VCCVoltage >> 8;
  data[9] = Z21_info.SupplyVoltage & 0xFF;
  data[10] = Z21_info.SupplyVoltage >> 8;
  data[11] = Z21_info.Temperature & 0xFF;
  data[12] = Z21_info.Temperature >> 8;
  data[13] = Z21_info.CentralState;
  data[14] = Z21_info.CentralStateEx;

  if(client)
    ws_send(client, (char *)data, 15, 0xff);
  else
    ws_send_all((char *)data, 15, 0xff);
}

void WS_stc_Z21_IP(struct web_client_t * client){
  uint8_t data[10];
  data[0] = WSopc_Z21_Settings;
  memcpy(&data[1], &Z21_info.IP[0], 4);
  memcpy(&data[5], &Z21_info.Firmware[0], 2);

  if(client)
    ws_send(client, (char *)data, 7, 0xff);
  else
    ws_send_all((char *)data, 7, 0xff);
}

void WS_cts_Enable_Disable_SubmoduleState(uint8_t opcode, uint8_t flags){
  if(opcode == WSopc_EnableSubModule){
    loggerf(INFO, "WSopc_EnableSubModule");
    if(flags & 0x80){ //Websocket
      SYS->WebSocket.state = Module_Run;
    }
    else if(flags & 0x40){ //Z21
      Z21_start();
    }
    else if(flags & 0x20){ //UART
      UART_start();
    }
    else if(flags & 0x10){ //LayoutControl
      Algor_start();
    }
    else if(flags & 0x04){ //SimA
      SimA_start();
    }
    else if(flags & 0x02){ //SimB
      SimB_start();
    }
    WS_stc_SubmoduleState();
  }
  else if(opcode == WSopc_DisableSubModule){
    loggerf(INFO, "WSopc_DisableSubModule");
    if(flags & 0x80){  //Websocket
      SYS->WebSocket.state = Module_Init;
    }
    else if(flags & 0x40){ // Z21
      Z21_stop();
    }
    else if(flags & 0x20){ //UART
      UART_stop();
    }
    else if(flags & 0x10){
      Algor_stop();
    }
    else if(flags & 0x04){
      SYS->SimA.state = Module_STOP;
    }
    else if(flags & 0x02){
      SYS->SimB.state = Module_STOP;
    }
    WS_stc_SubmoduleState();
  }
}

void WS_stc_SubmoduleState(){
  char data[5];
  data[0] = WSopc_SubModuleState;

  loggerf(INFO, "WS_stc_SubmoduleState %x %x %x %x %x", SYS->WebSocket.state, SYS->Z21.state, SYS->UART.state, SYS->LC.state, SYS->TC.state);

  data[1] = ((SYS->WebSocket.state & 0xF) << 4) | (SYS->Z21.state & 0xF);
  data[2] = ((SYS->UART.state & 0xF) << 4) | (SYS->LC.state & 0xF);
  data[3] = (SYS->TC.state & 0xF) << 4;
  data[4] = ((SYS->SimA.state & 0xF) << 4) | (SYS->SimB.state & 0xF);

  ws_send_all(data, 5, 0xFF);
}

//Admin Messages

//Train Messages
void WS_cts_LinkTrain(struct s_opc_LinkTrain * msg, struct web_client_t * client){
  // uint8_t * data = (uint8_t *)msg;
  // uint8_t fID = data[0]; //follow ID
  // uint8_t tID = data[1]; //TrainID
  // uint16_t mID = ((data[2] & 0x1F) << 8)+data[3];
  char return_value;
  if(msg->type == 0)
    loggerf(INFO, "Linking train %i with T-%s\n",msg->follow_id, trains[msg->real_id]->name);
  else
    loggerf(INFO, "Linking train %i with E-%s\n",msg->follow_id, engines[msg->real_id]->name);

  if((return_value = link_train(msg->follow_id, msg->real_id, msg->type)) == 1){
    WS_stc_LinkTrain(msg);

    WS_clear_message((msg->message_id_H << 8) + msg->message_id_L, 1);

    Z21_get_train(trains[msg->real_id]);
  }
  else{
    loggerf(WARNING, "Failed link_train()\n");
    WS_clear_message((msg->message_id_H << 8) + msg->message_id_L, 0); //Failed
  }
}

void WS_stc_LinkTrain(struct s_opc_LinkTrain * msg){
  struct s_WS_Data return_msg;
  return_msg.opcode = WSopc_LinkTrain;
  memcpy(&return_msg.data, msg, sizeof(struct s_opc_LinkTrain));
  return_msg.data.opc_LinkTrain.message_id_H = 0;
  return_msg.data.opc_LinkTrain.message_id_L = 0;
  
  ws_send_all((char *)&return_msg, 5, 0xFF);
}

void WS_cts_TrainControl(struct s_opc_TrainControl * m, struct web_client_t * client){

  if(!train_link[m->follow_id]){
    loggerf(WARNING, "Trying to set speed of undefined RailTrain");
    return;
  }

  loggerf(INFO, "WS_cts_TrainControl %i -> %i", m->follow_id, m->control);

  RailTrain * T = train_link[m->follow_id];

  T->control = m->control;
}

void WS_cts_SetTrainSpeed(struct s_opc_SetTrainSpeed * m, struct web_client_t * client){
  // uint16_t id = m.follow_id;
  // uint16_t speed = m.speed;

  if(!train_link[m->follow_id]){
    loggerf(WARNING, "Trying to set speed of undefined RailTrain");
    return;
  }

  uint16_t speed = (m->speed_high << 8) + m->speed_low;

  loggerf(INFO, "WS_cts_SetTrainSpeed %i -> %i", m->follow_id, speed);

  RailTrain * T = train_link[m->follow_id];

  T->speed = speed;
  T->target_speed = speed;
  T->dir   = m->dir;

  if(T->type == TRAIN_ENGINE_TYPE){
    ((Engines *)T->p)->dir = m->dir;
    engine_set_speed(T->p, speed);
    // Z21_Set_Loco_Drive_Train(T->p);
  }
  else{
    ((Trains *)T->p)->cur_speed = speed;
    ((Trains *)T->p)->dir = m->dir;
    train_calc_speed(T->p);
    // Z21_Set_Loco_Drive_Engine(T->p);
  }

  loggerf(INFO, "IMPLEMENT Z21");
  // if(data[2] & 0x20 && id < trains_len){ //Train
  //   trains[id]->cur_speed = speed;
  //   trains[id]->dir = (data[2] & 0x10) >> 4;

  //   train_calc_speed(trains[id]);

  // }
  // else if(id < engines_len){ //Engine
  //   engines[id]->dir = (data[2] & 0x10) >> 4;

  //   engine_set_speed(engines[id], speed);

  // }
}

void WS_stc_UpdateTrain(RailTrain * T){
  if(!T)
    return;

  struct s_WS_Data return_msg;
  return_msg.opcode = WSopc_UpdateTrain;
  struct s_opc_UpdateTrain * msg = &return_msg.data.opc_UpdateTrain;

  msg->follow_id = T->link_id;

  msg->dir = T->dir;
  msg->control = T->control & 0x03;
  msg->speed_high = (T->speed & 0x0F00) >> 8;
  msg->speed_low  = (T->speed & 0xFF);

  loggerf(DEBUG, "train Update_Train id: %i, d: %i, c: %i, sp: %x%02x", msg->follow_id, msg->dir, msg->control, msg->speed_high, msg->speed_low);

  for(int i = 0; i<MAX_WEB_CLIENTS; i++){
    if((websocket_clients[i].trains[0] == T->link_id) || 
       (websocket_clients[i].trains[1] == T->link_id)){
      ws_send(&websocket_clients[i], (void *)&return_msg, sizeof(struct s_opc_UpdateTrain) + 1, WS_Flag_Trains);
    }
  }
}

void WS_cts_TrainSubscribe(struct s_opc_SubscribeTrain * m, struct web_client_t * client){
  client->trains[0] = m->followA;
  client->trains[1] = m->followB;

  if(client->trains[0] < 0xFF && train_link[client->trains[0]]){
    WS_stc_UpdateTrain(train_link[client->trains[0]]);
  }
  if(client->trains[1] < 0xFF && train_link[client->trains[1]]){
    WS_stc_UpdateTrain(train_link[client->trains[1]]);
  }

  loggerf(INFO, "WS_cts_TrainSubscribe client %i = %i, %i", client->id, client->trains[0], client->trains[1]);
}

void WS_EnginesLib(struct web_client_t * client){
  int buffer_size = 1024;

  char * data = _calloc(buffer_size, 1);

  int len = 0;

  data[len++] = WSopc_EnginesLibrary;

  for(int i = 0;i<engines_len;i++){
    if(buffer_size < (len + 100)){
      buffer_size += 1024;
      data = _realloc(data, buffer_size, 1);
    }
    if(!engines[i]){
      continue;
    }
    loggerf(WARNING, "Exporting engine %i, %s", i, engines[i]->name);

    data[len++] = engines[i]->DCC_ID & 0xFF;
    data[len++] = engines[i]->DCC_ID >> 8;
    data[len++] = engines[i]->max_speed & 0xFF;
    data[len++] = engines[i]->max_speed >> 8;
    data[len++] = engines[i]->length & 0xFF;
    data[len++] = engines[i]->length >> 8;
    data[len++] = engines[i]->type;

    data[len++] = engines[i]->speed_step_type & 0x3;

    data[len++] = engines[i]->steps_len;
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

    for(int j = 0; j < engines[i]->steps_len; j++){
      data[len++] = engines[i]->steps[j].speed & 0xFF;
      data[len++] = engines[i]->steps[j].speed >> 8;
      data[len++] = engines[i]->steps[j].step;
    }
  }
  if(client)
    ws_send(client, data, len, WS_Flag_Trains);
  else
    ws_send_all(data, len, WS_Flag_Trains);
  
  _free(data);
}

void WS_CarsLib(struct web_client_t * client){
  loggerf(INFO, "CarsLib for client %i", client);
  int buffer_size = 1024;

  char * data = _calloc(buffer_size, 1);

  int len = 0;

  data[len++] = WSopc_CarsLibrary;

  for(int i = 0;i<cars_len;i++){
    if(buffer_size < (len + 100)){
      buffer_size += 1024;
      data = _realloc(data, buffer_size, 1);
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
      data[len++] = strlen(cars[i]->icon_path);

      int l = strlen(cars[i]->name);
      memcpy(&data[len], cars[i]->name, l);
      len += l;

      l = strlen(cars[i]->icon_path);
      memcpy(&data[len], cars[i]->icon_path, l);
      len += l;
    }
  }
  if(client)
    ws_send(client, data, len, WS_Flag_Trains);
  else
    ws_send_all(data, len, WS_Flag_Trains);
  
  _free(data);
}

void WS_TrainsLib(struct web_client_t * client){
  loggerf(INFO, "TrainsLib for client %i", client);
  int buffer_size = 1024;

  char * data = _calloc(buffer_size, 1);

  int len = 0;

  data[len++] = WSopc_TrainsLibrary;

  for(int i = 0;i<trains_len;i++){
    if(!trains[i]){
      continue;
    }
    if(buffer_size < (len + 100)){
      buffer_size += 1024;
      data = _realloc(data, buffer_size, 1);
    }
    
    data[len++] = trains[i]->max_speed & 0xFF;
    data[len++] = trains[i]->max_speed >> 8;

    data[len++] = trains[i]->length & 0xFF;
    data[len++] = trains[i]->length >> 8;

    data[len] = trains[i]->type << 1;
    data[len++] |= trains[i]->in_use;

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
  if(client)
    ws_send(client, data, len, WS_Flag_Trains);
  else
    ws_send_all(data, len, WS_Flag_Trains);
  
  _free(data);
}

void WS_stc_TrainCategories(struct web_client_t * client){
  loggerf(INFO, "TrainCategories for client %i", client);
  int buffer_size = 1024;

  char * data = _calloc(buffer_size, 1);

  int len = 0;

  data[len++] = WSopc_TrainCategories;

  for(int i = 0;i<train_P_cat_len;i++){
    if(buffer_size < (len + 100)){
      buffer_size += 1024;
      data = _realloc(data, buffer_size, 1);
    }
    data[len++] = i;

    data[len++] = strlen(train_P_cat[i].name);

    int l = strlen(train_P_cat[i].name);
    memcpy(&data[len], train_P_cat[i].name, l);
    len += l;
  }

  for(int i = 0;i<train_C_cat_len;i++){
    if(buffer_size < (len + 100)){
      buffer_size += 1024;
      data = _realloc(data, buffer_size, 1);
    }
    data[len++] = i | 0x80;

    data[len++] = strlen(train_C_cat[i].name);

    int l = strlen(train_C_cat[i].name);
    memcpy(&data[len], train_C_cat[i].name, l);
    len += l;
  }

  if(client)
    ws_send(client, data, len, WS_Flag_Trains);
  else
    ws_send_all(data, len, WS_Flag_Trains);
  
  _free(data);
}

void WS_NewTrain(RailTrain * T,char M,char B){
  //Nr:   follow id of train
  //M,B:  module nr and block nr
  uint16_t msg_ID = WS_init_Message(0);

  loggerf(INFO, "WS_NewTrain");

  char data[6];
  data[0] = WSopc_NewMessage;
  data[1] = ((msg_ID >> 8) & 0x1F) + 0; //type = 0
  data[2] = (msg_ID & 0xFF);
  data[3] = T->link_id;
  data[4] = M;
  data[5] = B;
  ws_send_all(data,6,WS_Flag_Messages);
  WS_add_Message(msg_ID,6,data);
}

void WS_TrainSplit(RailTrain * T, char M1,char B1,char M2,char B2){
  //Nr:   follow id of train
  //M,B:  module nr and block nr
  uint16_t msg_ID = WS_init_Message(1);

  loggerf(INFO, "WS_TrainSplit");

  char data[8];
  data[0] = WSopc_NewMessage;
  data[1] = ((msg_ID >> 8) & 0x1F) + 0x20; //type = 1
  data[2] = (msg_ID & 0xFF);
  data[3] = T->link_id;
  data[4] = M1;
  data[5] = B1;
  data[6] = M2;
  data[7] = B2;
  ws_send_all(data,8,WS_Flag_Messages);
  WS_add_Message(msg_ID,8,data);
}

/*
void Web_Train_Split(int i,char tID,char B[]){
  loggerf(DEBUG, "Web_Train_Split(%i,%i,{%i,%i});",i,tID,B[0],B[1]);
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

void WS_cts_TrainRoute(struct s_opc_TrainRoute * data, struct web_client_t * client){
  RailTrain * T = train_link[data->train_id];

  Station * St = Units[data->module_id]->St[data->station_id];

  train_set_route(T, St->blocks[0]);
}

void WS_stc_TrainRoute(){}

// void WS_TrainData(char data[14]){
//   loggerf(TRACE,"WS_TrainData");
//   char s_data[20];
//   s_data[0] = WSopc_Z21TrainData;

//   for(int i = 0;i<7;i++){
//     s_data[i+2] = data[i];
//   }

//   loggerf(ERROR, "FIX ID");
//   return;
//   // s_data[1] = DCC_train[((s_data[2] << 8) + s_data[3])]->ID;

//   ws_send_all(s_data,9,WS_Flag_Trains);
// }

void WS_cts_AddCartoLib(struct s_opc_AddNewCartolib * data, struct web_client_t * client){
  loggerf(DEBUG, "WS_cts_AddCartoLib");
  struct s_WS_Data * rdata = _calloc(1, sizeof(struct s_WS_Data));

  rdata->opcode = WSopc_AddNewCartolib;
  rdata->data.opc_AddNewCartolib_res.nr = data->nr;

  char * name = _calloc(data->name_len + 1, 1);
  char * filename = _calloc(data->name_len + 1, 1);
  char * icon = _calloc(data->name_len + 8 + 3 + 20, 1); //Destination file
  char * sicon = _calloc(40, 1); //Source file

  memcpy(name, &data->strings, data->name_len);
  memcpy(filename, &data->strings, data->name_len);
  for (char* current_pos = NULL; (current_pos = strchr(filename, ' ')) != NULL; *current_pos = '_');
  for (char* current_pos = NULL; (current_pos = strchr(filename, '.')) != NULL; *current_pos = '-');
    loggerf(WARNING, "Filename: %s", filename);

  uint16_t icon_time = (data->timing / 60) * 100 + (data->timing % 60);

  loggerf(ERROR, "%04i", icon_time);

  if((data->filetype & 0b1) == 0){
    sprintf(icon, "%i_%s.%s", data->nr, filename, "png");
    sprintf(sicon, "%s.%04i.%s", "web/tmp_icon", icon_time, "png");
  }
  else{
    sprintf(icon, "%i_%s.%s", data->nr, filename, "jpg");
    sprintf(sicon, "%s.%04i.%s", "web/tmp_icon", icon_time, "jpg");
  }


  Create_Car(name, data->nr, icon, data->type, data->length, data->max_speed);

  char * dicon = _calloc(strlen(icon)+10, 1);

  sprintf(dicon, "%s%s", "web/trains_img/", icon);

  move_file(sicon, dicon);

  write_rolling_Configs();

  // Delete temp file
  remove(sicon);

  rdata->data.opc_AddNewCartolib_res.response = 1;
  ws_send(client, (char *)rdata, WSopc_AddNewCartolib_res_len, 0xff);

  _free(dicon);
  _free(sicon);
}

void WS_cts_Edit_Car(Cars * C, struct s_opc_AddNewCartolib * data, struct web_client_t * client){
  loggerf(DEBUG, "WS_cts_Edit_Car");
  struct s_WS_Data * rdata = _calloc(1, sizeof(struct s_WS_Data));

  rdata->opcode = WSopc_AddNewCartolib;
  rdata->data.opc_AddNewCartolib_res.nr = data->nr;

  C->name = _realloc(C->name, data->name_len + 1, 1);
  memcpy(C->name, &data->strings, data->name_len);
  C->name[data->name_len] = 0;

  char * filename = _calloc(data->name_len +1,1);
  memcpy(filename, C->name, data->name_len);
  for (char* current_pos = NULL; (current_pos = strchr(filename, ' ')) != NULL; *current_pos = '_');
  for (char* current_pos = NULL; (current_pos = strchr(filename, '.')) != NULL; *current_pos = '-');
    loggerf(WARNING, "Filename: %s", filename);

  C->length = data->length;
  C->type = data->type;

  char * sicon = _calloc(40, 1); //Source file

  uint16_t icon_time = (data->timing / 60) * 100 + (data->timing % 60);

  loggerf(ERROR, "%04i", icon_time);

  char * dicon = 0;

  if(icon_time < 3000){
    C->icon_path = _realloc(C->icon_path, data->name_len + 8 + 3 + 20, 1); //Destination file
    if((data->filetype & 0b1) == 0){
      sprintf(C->icon_path, "%iC_%s.%s", data->nr, filename, "png");
      sprintf(sicon, "%s.%04i.%s", "web/tmp_icon", icon_time, "png");
    }
    else{
      sprintf(C->icon_path, "%iC_%s.%s", data->nr, filename, "jpg");
      sprintf(sicon, "%s.%04i.%s", "web/tmp_icon", icon_time, "jpg");
    }
    dicon = _calloc(strlen(C->icon_path)+20, 1);
    sprintf(dicon, "%s%s", "web/trains_img/", C->icon_path);
    move_file(sicon, dicon);

    // Delete temp file
    remove(sicon);
  }
  else{
    sprintf(sicon, "web/trains_img/%s", C->icon_path);

    char filetype[5];
    sprintf(filetype, "%s", &C->icon_path[strlen(C->icon_path)-3]);

    sprintf(C->icon_path, "%iC_%s.%s", data->nr, filename, filetype);
    dicon = _calloc(strlen(C->icon_path)+20, 1);
    sprintf(dicon, "%s%s", "web/trains_img/", C->icon_path);

    move_file(sicon, dicon);

    if(strcmp(sicon, dicon) != 0)
      remove(sicon);
  }

  rdata->data.opc_AddNewCartolib_res.response = 1;
  ws_send(client, (char *)rdata, WSopc_AddNewCartolib_res_len, 0xff);

  _free(dicon);
  _free(sicon);
}

void WS_cts_AddEnginetoLib(struct s_opc_AddNewEnginetolib * data, struct web_client_t * client){
  loggerf(DEBUG, "WS_cts_AddEnginetoLib");
  struct s_WS_Data * rdata = _calloc(1, sizeof(struct s_WS_Data));

  rdata->opcode = WSopc_AddNewEnginetolib;

  if (DCC_train[data->DCC_ID]){
    loggerf(ERROR, "DCC %i allready in use", data->DCC_ID);
    rdata->data.opc_AddNewEnginetolib_res.response = 255;
    ws_send(client, (char *)rdata, WSopc_AddNewEnginetolib_res_len, 0xff);
    return;
  }

  char * name = _calloc(data->name_len + 1, 1);
  char * steps = _calloc(data->steps, 3);
  char * img = _calloc(data->name_len + 8 + 3 + 20, 1);  //Destination file
  char * icon = _calloc(data->name_len + 8 + 3 + 20, 1); //Destination file
  char * simg = _calloc(40, 1); //Source file
  char * sicon = _calloc(40, 1); //Source file
  char * filetype = _calloc(4, 1);

  memcpy(name, &data->strings, data->name_len);
  memcpy(steps, &data->strings + data->name_len, data->steps * 3);

  char * filename = _calloc(data->name_len +1,1);
  memcpy(filename, &data->strings, data->name_len);
  for (char* current_pos = NULL; (current_pos = strchr(filename, ' ')) != NULL; *current_pos = '_');
  for (char* current_pos = NULL; (current_pos = strchr(filename, '.')) != NULL; *current_pos = '-');
    loggerf(WARNING, "Filename: %s", filename);

  uint16_t image_time = data->timing[0] + ((data->timing[1] & 0xf0) << 4);
  image_time = (image_time / 60) * 100 + (image_time % 60);
  uint16_t icon_time = (data->timing[1] & 0x0f) + (data->timing[2] << 4);
  icon_time = (icon_time / 60) * 100 + (icon_time % 60);

  loggerf(ERROR, "%04i - %04i", image_time, icon_time);

  if((data->flags & 0b10) == 0){
    sprintf(img, "%i_%s_im.%s", data->DCC_ID, filename, "png");
    sprintf(simg, "%s.%04i.%s", "web/tmp_img", image_time, "png");
  }
  else{
    sprintf(img, "%i_%s_im.%s", data->DCC_ID, filename, "jpg");
    sprintf(simg, "%s.%04i.%s", "web/tmp_img", image_time, "jpg");
  }

  if((data->flags & 0b1) == 0){
    sprintf(icon, "%i_%s_ic.%s", data->DCC_ID, filename, "png");
    sprintf(sicon, "%s.%04i.%s", "web/tmp_icon", icon_time, "png");
  }
  else{
    sprintf(icon, "%i_%s_ic.%s", data->DCC_ID, filename, "jpg");
    sprintf(sicon, "%s.%04i.%s", "web/tmp_icon", icon_time, "jpg");
  }

  Create_Engine(name, data->DCC_ID, img, icon, data->type, data->length, data->steps, (struct engine_speed_steps *)steps);

  char * dimg = _calloc(strlen(img)+20, 1);
  char * dicon = _calloc(strlen(icon)+20, 1);
  sprintf(dimg, "%s%s", "web/trains_img/", img);
  sprintf(dicon, "%s%s", "web/trains_img/", icon);

  move_file(simg,  dimg);
  move_file(sicon, dicon);

  write_rolling_Configs();

  // Delete temp file
  remove(sicon);
  remove(simg);

  rdata->data.opc_AddNewEnginetolib_res.response = 1;
  ws_send(client, (char *)rdata, WSopc_AddNewEnginetolib_res_len, 0xff);

  _free(dimg);
  _free(dicon);
  _free(simg);
  _free(sicon);
  _free(filetype);

  //Update clients Train Library
  WS_EnginesLib(0);
}

void WS_cts_Edit_Engine(struct s_opc_EditEnginelib * msg, struct web_client_t * client){
  loggerf(DEBUG, "WS_cts_Edit_Engine");

  struct s_WS_Data * rdata = _calloc(1, sizeof(struct s_WS_Data));

  uint16_t id = msg->id_l + (msg->id_h << 8);
  Engines * E = engines[id];

  if(msg->remove){
    // Remove Engine
    // Remove images
    remove(E->img_path);
    remove(E->icon_path);
    DCC_train[E->DCC_ID] = NULL;

    Clear_Engine(&E);

    for(int i = id; i < (engines_len - 1); i++){
      engines[i] = engines[i+1];
    }

    engines_len--;
  }
  else{
    // Edit engine
    struct s_opc_AddNewEnginetolib * data = &msg->data;

    rdata->opcode = WSopc_AddNewEnginetolib;

    if (DCC_train[data->DCC_ID] && data->DCC_ID != E->DCC_ID){
      loggerf(ERROR, "DCC %i (%i) allready in use", data->DCC_ID, E->DCC_ID);
      rdata->data.opc_AddNewEnginetolib_res.response = 255;
      ws_send(client, (char *)rdata, WSopc_AddNewEnginetolib_res_len, 0xff);
      return;
    }

    DCC_train[E->DCC_ID] = NULL;
    E->DCC_ID = data->DCC_ID;
    DCC_train[E->DCC_ID] = E;

    // Copy name
    E->name = _realloc(E->name, data->name_len + 1, 1);
    memcpy(E->name, &data->strings, data->name_len);
    E->name[data->name_len] = 0;

    char * filename = _calloc(data->name_len +1,1);
    memcpy(filename, E->name, data->name_len);
    for (char* current_pos = NULL; (current_pos = strchr(filename, ' ')) != NULL; *current_pos = '_');
    for (char* current_pos = NULL; (current_pos = strchr(filename, '.')) != NULL; *current_pos = '-');
      loggerf(WARNING, "Filename: %s", filename);

    // Copy speedsteps
    E->steps_len = data->steps;
    E->steps = _realloc(E->steps, data->steps, 3);
    memcpy(E->steps, &data->strings + data->name_len, data->steps * 3);

    E->length = data->length;
    E->type = data->type;


    // Copy image/icon
    char * simg = _calloc(40, 1); //Source file
    char * sicon = _calloc(40, 1); //Source file
    char * filetype = _calloc(4, 1);

    uint16_t image_time = data->timing[0] + ((data->timing[1] & 0xf0) << 4);
    image_time = (image_time / 60) * 100 + (image_time % 60);
    uint16_t icon_time = (data->timing[1] & 0x0f) + (data->timing[2] << 4);
    icon_time = (icon_time / 60) * 100 + (icon_time % 60);

    loggerf(ERROR, "%04i - %04i", image_time, icon_time);

    char * dimg = 0;
    char * dicon = 0;

    if(image_time < 3000){
      remove(E->img_path); //Remove original

      E->img_path = _realloc(E->img_path, data->name_len + 8 + 3 + 20, 1);  //Destination file
      if((data->flags & 0b10) == 0){
        sprintf(E->img_path, "%i_%s_im.%s", data->DCC_ID, filename, "png");
        sprintf(simg, "%s.%04i.%s", "web/tmp_img", image_time, "png");
      }
      else{
        sprintf(E->img_path, "%i_%s_im.%s", data->DCC_ID, filename, "jpg");
        sprintf(simg, "%s.%04i.%s", "web/tmp_img", image_time, "jpg");
      }

      dimg = _calloc(strlen(E->img_path)+20, 1);
      sprintf(dimg, "%s%s", "web/trains_img/", E->img_path);
      move_file(simg,  dimg);

      // Delete temp file
      remove(simg);
    }
    else{
      sprintf(simg, "web/trains_img/%s", E->img_path);

      char filetype[5];
      sprintf(filetype, "%s", &E->img_path[strlen(E->img_path)-3]);

      sprintf(E->img_path, "%i_%s_im.%s", data->DCC_ID, filename, filetype);
      dimg = _calloc(strlen(E->img_path)+20, 1);
      sprintf(dimg, "%s%s", "web/trains_img/", E->img_path);

      move_file(simg, dimg);

      if(strcmp(simg, dimg) != 0)
        remove(simg);
    }

    if(icon_time < 3000){
      remove(E->icon_path); //Remove original

      E->icon_path = _realloc(E->icon_path, data->name_len + 8 + 3 + 20, 1); //Destination file
      if((data->flags & 0b1) == 0){
        sprintf(E->icon_path, "%i_%s_ic.%s", data->DCC_ID, filename, "png");
        sprintf(sicon, "%s.%04i.%s", "web/tmp_icon", icon_time, "png");
      }
      else{
        sprintf(E->icon_path, "%i_%s_ic.%s", data->DCC_ID, filename, "jpg");
        sprintf(sicon, "%s.%04i.%s", "web/tmp_icon", icon_time, "jpg");
      }
      dicon = _calloc(strlen(E->icon_path)+20, 1);
      sprintf(dicon, "%s%s", "web/trains_img/", E->icon_path);
      move_file(sicon, dicon);

      // Delete temp file
      remove(sicon);
    }
    else{
      sprintf(sicon, "web/trains_img/%s", E->icon_path);

      char filetype[5];
      sprintf(filetype, "%s", &E->icon_path[strlen(E->icon_path)-3]);

      sprintf(E->icon_path, "%i_%s_ic.%s", data->DCC_ID, filename, filetype);
      dicon = _calloc(strlen(E->icon_path)+20, 1);
      sprintf(dicon, "%s%s", "web/trains_img/", E->icon_path);

      move_file(sicon, dicon);

      if(strcmp(sicon, dicon) != 0)
        remove(sicon);
    }

    // Send succes response
    rdata->data.opc_AddNewEnginetolib_res.response = 1;
    ws_send(client, (char *)rdata, WSopc_AddNewEnginetolib_res_len, 0xff);

    _free(dimg);
    _free(dicon);
    _free(simg);
    _free(sicon);
    _free(filetype);
  }

  write_rolling_Configs();
  WS_EnginesLib(0);
}

void WS_cts_AddTraintoLib(struct s_opc_AddNewTraintolib * data, struct web_client_t * client){
  loggerf(DEBUG, "WS_cts_AddTraintoLib");
  struct s_WS_Data * rdata = _calloc(1, sizeof(struct s_WS_Data));

  rdata->opcode = WSopc_AddNewTraintolib;

  char * name = _calloc(data->name_len + 1, 1);
  char * comps = _calloc(data->nr_stock, 3);

  //Copy name
  memcpy(name, &data->strings, data->name_len);

  // Copy configuration
  memcpy(comps, &data->strings + data->name_len, data->nr_stock*3);

  Create_Train(name, data->nr_stock, (struct train_comp_ws *)comps, data->catagory, data->save);

  write_rolling_Configs();

  // Send succes response
  rdata->data.opc_AddNewTraintolib_res.response = 1;
  ws_send(client, (char *)rdata, WSopc_AddNewTraintolib_res_len, 0xff);

  //Update clients Train Library
  WS_TrainsLib(0);
}

void WS_cts_Edit_Train(Trains * T, struct s_opc_AddNewTraintolib * data, struct web_client_t * client){
  loggerf(ERROR, "WS_cts_Edit_Train ");
  struct s_WS_Data * rdata = _calloc(1, sizeof(struct s_WS_Data));

  rdata->opcode = WSopc_AddNewTraintolib;

  //Copy name
  T->name = _realloc(T->name, data->name_len + 1, 1);
  memcpy(T->name, &data->strings, data->name_len);
  T->name[data->name_len] = 0;

  // Copy traincomp
  T->composition = _realloc(T->composition, data->nr_stock, sizeof(struct train_comp));
  T->nr_stock = data->nr_stock;

  struct train_comp_ws * cdata = (void *)&data->strings + data->name_len;
  for(int c = 0; c<T->nr_stock; c++){
    T->composition[c].type = cdata[c].type;
    T->composition[c].id = cdata[c].id;
    if(cdata[c].type == 0){
      T->composition[c].p = (void *)engines[T->composition[c].id];
    }
    else{
      T->composition[c].p = (void *)cars[T->composition[c].id];
    }
  }

  // Send success response
  rdata->data.opc_AddNewTraintolib_res.response = 1;
  ws_send(client, (char *)rdata, WSopc_AddNewTraintolib_res_len, 0xff);
}


//Track Messages
void WS_trackUpdate(struct web_client_t * client){
  loggerf(TRACE, "WS_trackUpdate");
  mutex_lock(&mutex_lockB, "Lock Mutex B");
  char data[4096];

  data[0] = WSopc_BroadTrack; //Opcode

  int data_len;
  _Bool content = 0;

  int q = 1;

  for(int i = 0;i<unit_len;i++){
    if(!Units[i] || Units[i]->block_state_changed == 0)
      continue;

    Units[i]->block_state_changed = 0;

    for(int j = 0;j<Units[i]->block_len;j++){
      Block * B = Units[i]->B[j];
      if(B && (B->statechanged || B->IOchanged)){
        content = 1;

        data[(q-1)*4+1] = B->module;
        data[(q-1)*4+2] = B->id;
        data[(q-1)*4+3] = (B->dir << 7) + B->state;
        data[(q-1)*4+4] = 0;//B->train;
        q++;

        B->statechanged = 0;
      }
    }
  }

  data_len = (q-1)*4+1;

  if(content == 1){
    if(client){
      ws_send(client, data, data_len, WS_Flag_Track);
    }else{
      ws_send_all(data, data_len, WS_Flag_Track);
    }
  }
  mutex_unlock(&mutex_lockB, "UnLock Mutex B");
}

void WS_SwitchesUpdate(struct web_client_t * client){
  loggerf(TRACE, "WS_SwitchesUpdate (%x)", (unsigned int)client);
  mutex_lock(&mutex_lockB, "Lock Mutex B");
  char buf[4096];
  memset(buf, 0, 4096);
  /*Switches*/

    buf[0] = WSopc_BroadSwitch;
    int buf_l  = 0;
    _Bool content   = 0;

    int q = 1;

    for(int i = 0;i<unit_len;i++){
      if(!Units[i])// || !Units[i]->switch_state_changed)
        continue;
      // Units[i]->switch_state_changed = 0;

      for(int j = 0;j<Units[i]->switch_len;j++){
        Switch * S = Units[i]->Sw[j];
        if(!S)
          continue;

        if((S->state & 0x80) != 0x80){
          loggerf(TRACE, "%i:%i no new state", S->module, S->id);
          continue;
        }

        content = 1;

        buf[(q-1)*3+1] = S->module;
        buf[(q-1)*3+2] = S->id & 0x7F;
        buf[(q-1)*3+3] = S->state & 0x7F;

        S->state &= ~0x80;

        loggerf(DEBUG, "%i,%i,%i", S->module, S->id, S->state);
        q++;
      }
    }

    buf_l = (q-1)*3;

  // MSSwitches
  //buf[0] = 5;
  // buf_l = 0;
  q = 1;
  for(int i = 0;i<unit_len;i++){
    if(!Units[i] || !Units[i]->msswitch_state_changed)
      continue;
    content = 1;
    for(int j = 0;j<=Units[i]->msswitch_len;j++){
      MSSwitch * Sw = Units[i]->MSSw[j];
      if(Sw){
        buf[(q-1)*4+1+buf_l] = Sw->module;
        buf[(q-1)*4+2+buf_l] = (Sw->id & 0x7F) + 0x80;
        buf[(q-1)*4+3+buf_l] = Sw->state & 0x7F;
        buf[(q-1)*4+4+buf_l] = Sw->state_len;

        Sw->state &= ~0x80;

        q++;
      }
    }
  }
  
  buf_l += (q-1)*4+1;
  if(content == 1){
    if(client){
      ws_send(client, buf, buf_l, WS_Flag_Switches);
    }else{
      ws_send_all(buf, buf_l, WS_Flag_Switches);
    }
  }
  else
    loggerf(DEBUG, "WS Switches no content");

  mutex_unlock(&mutex_lockB, "UnLock Mutex B");
}

void WS_NewClient_track_Switch_Update(struct web_client_t * client){
  mutex_lock(&mutex_lockB, "Lock Mutex B");

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
          buf[(q-1)*4+4] = 0;//B->train;
          q++;

          B->statechanged = 0;
        }
      }
    }
  }

  data_len = (q-1)*4+1;

  if(content == 1){
    if(client){
      ws_send(client, buf, data_len, WS_Flag_Track);
    }else{
      ws_send_all(buf, data_len, WS_Flag_Track);
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
    if(client){
      ws_send(client,buf,buf_len,WS_Flag_Switches);
    }else{
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

    buf[buf_len]   = stations[i]->module;
    buf[buf_len+1] = stations[i]->id;
    buf[buf_len+2] = strlen(stations[i]->name);
    strcpy(&buf[buf_len+3],stations[i]->name);

    buf_len+=3+strlen(stations[i]->name);
  }

  if(data == 1){
    if(client){
      ws_send(client, buf, buf_len, WS_Flag_Switches);
    }else{
      ws_send_all(buf, buf_len, WS_Flag_Switches);
    }
  }

  memset(buf,0,4096);

  loggerf(INFO, "Z21_GET_LOCO_INFO check");
  for(int i = 1;i<trains_len;i++){
    if(train_link[i]){
      //TODO fix for RailTrain
      //for(int j = 0; j < train_link[i]->nr_engines; j++){
      //  Z21_get_engine(train_link[i]->engines[j]->DCC_ID);
      //}
    }
  }

  mutex_unlock(&mutex_lockB, "UnLock Mutex B");
}

void WS_reset_switches(struct web_client_t * client){
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
    WS_SwitchesUpdate(client);
  }
}

void WS_Track_LayoutDataOnly(int unit, struct web_client_t * client){
  loggerf(DEBUG, "WS_Track_LayoutDataOnly");

  char * data = _calloc(Units[unit]->Layout_length + 20, 1);

  data[0] = WSopc_TrackLayoutOnlyRawData;
  data[1] = unit;
  memcpy(&data[2], Units[unit]->Layout, Units[unit]->Layout_length);


  if(client){
    ws_send(client, data, Units[unit]->Layout_length+2, WS_Flag_Track);
  }else{
    ws_send_all(data, Units[unit]->Layout_length+2, WS_Flag_Track);
  }

  _free(data);
}

void WS_stc_TrackLayoutRawData(int unit, struct web_client_t * client){
  Unit * U = Units[unit];
  char * data = _calloc(U->raw_length+2, 1);
  data[0] = WSopc_TrackLayoutRawData;
  data[1] = unit;

  memcpy(&data[2], U->raw, U->raw_length);

  if(client){
    ws_send(client, data, U->raw_length+2, WS_Flag_Track);
  }
  else{
    ws_send_all(data, U->raw_length+2, WS_Flag_Track);
  }

  _free(data);
}

void WS_stc_StationLib(struct web_client_t * client){
  uint8_t * data = _calloc(stations_len, Station);
  data[0] = WSopc_StationLibrary;
  uint8_t * length = &data[1];
  uint8_t * d = &data[2];

  for(uint8_t i = 0; i < stations_len; i++){
    if(!stations[i])
      continue;

    (*length)++;
    d[0] = stations[i]->module;
    d[1] = stations[i]->id;
    d[2] = stations[i]->type;
    d[3] = strlen(stations[i]->name);
    memcpy(&d[4], stations[i]->name, d[3]);

    d += d[3] + 4;
  }

  if(client){
    ws_send(client, data, d - data, WS_Flag_Track);
  }
  else{
    ws_send_all(data, d - data, WS_Flag_Track);
  }

  _free(data);
}

//General Messages
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

  loggerf(INFO, "create_message %x", ID);
}

void WS_send_open_Messages(struct web_client_t * client){
  for(int i = 0;i<=0x1FFF;i++){
    if(MessageList[i].type & 0x8000){
      ws_send(client, MessageList[i].data, MessageList[i].data_length, 0xFF);
    }
  }
}

void WS_clear_message(uint16_t ID, char ret_code){
  if(ret_code == 1)
    MessageList[ID].type = 0;

  loggerf(INFO, "clear_message %x", ID);

  ws_send_all((char [3]){WSopc_ClearMessage,((ID >> 8) & 0x1F) + (ret_code << 5),ID&0xFF},3,0xFF);
}
