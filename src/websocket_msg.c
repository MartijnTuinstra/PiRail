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

void WS_Track_Layout(int Client_fd){

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
    if(Client_fd){
      ws_send(Client_fd, data,q,WS_Flag_Track);
    }
    else{
      ws_send_all(data,q,WS_Flag_Track);
    }
  }
}

void WS_stc_Z21_info(int client_fd){
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

  if(client_fd <= 0){
    ws_send_all((char *)data, 15, 0xff);
  }
  else{
    ws_send(client_fd, (char *)data, 15, 0xff);
  }
}

void WS_stc_Z21_IP(int client_fd){
  uint8_t data[10];
  data[0] = WSopc_Z21_Settings;
  memcpy(&data[1], &Z21_info.IP[0], 4);
  memcpy(&data[5], &Z21_info.Firmware[0], 2);

  if(client_fd <= 0){
    ws_send_all((char *)data, 7, 0xff);
  }
  else{
    ws_send(client_fd, (char *)data, 7, 0xff);
  }
}

void WS_cts_Enable_Disable_SubmoduleState(uint8_t opcode, uint8_t flags){
  if(opcode == WSopc_EnableSubModule){
    loggerf(TRACE, "WSopc_EnableSubModule");
    if(flags & 0x80){ //Websocket
      _SYS->Websocket_State = _SYS_Module_Run;
    }
    else if(flags & 0x40){ //Z21
      Z21_start();
    }
    else if(flags & 0x20){ //LayoutControl
      UART_start();
    }
    else if(flags & 0x10){ //LayoutControl
      Algor_start();
    }
    else if(flags & 0x04){ //SimA
      SimA_start();
    // }
    // else if(flags & 0x02){ //SimB
      SimB_start();
    }
    WS_stc_SubmoduleState();
  }
  else if(opcode == WSopc_DisableSubModule){
    loggerf(TRACE, "WSopc_DisableSubModule");
    if(flags & 0x80){  //Websocket
      _SYS->Websocket_State = _SYS_Module_Init;
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
      _SYS->SimA_State = _SYS_Module_Stop;
    }
    else if(flags & 0x02){
      _SYS->SimB_State = _SYS_Module_Stop;
    }
    WS_stc_SubmoduleState();
  }
}

void WS_stc_SubmoduleState(){
  char data[4];
  data[0] = WSopc_SubModuleState;

  loggerf(DEBUG, "WS_stc_SubmoduleState %x %x %x %x %x", _SYS->Websocket_State, _SYS->Z21_State, _SYS->UART_State, _SYS->LC_State, _SYS->TC_State);

  data[1] = (_SYS->Websocket_State << 6) | (_SYS->Z21_State << 4) | (_SYS->UART_State << 2) | _SYS->TC_State;
  data[2] = (_SYS->LC_State << 5) | (_SYS->SimA_State << 3) | (_SYS->SimB_State << 1);

  ws_send_all(data, 3, 0xFF);
}

//Admin Messages

//Train Messages
void WS_EnginesLib(int client_fd){
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
      printf("engine data: %4x %2x -> %2x %2x %2x\n", engines[i]->steps[j].speed, engines[i]->steps[j].step, data[len-3], data[len-2], data[len-1]);
    }
  }
  if(client_fd <= 0){
    ws_send_all(data, len, WS_Flag_Trains);
  }
  else{
    ws_send(client_fd, data, len, WS_Flag_Trains);
  }

  _free(data);
}

void WS_CarsLib(int client_fd){
  loggerf(INFO, "CarsLib for client %i", client_fd);
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

  _free(data);
}

void WS_TrainsLib(int client_fd){
  loggerf(INFO, "TrainsLib for client %i", client_fd);
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
    int ilen = len;
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

    for(;ilen < len; ilen++){
      printf("%02X ", data[ilen]);
    }
    printf("\n");
  }
  if(client_fd <= 0){
    ws_send_all(data, len, WS_Flag_Trains);
  }
  else{
    ws_send(client_fd, data, len, WS_Flag_Trains);
  }

  _free(data);
}

void WS_stc_TrainCategories(int client_fd){
  loggerf(INFO, "TrainCategories for client %i", client_fd);
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

  if(client_fd <= 0){
    ws_send_all(data, len, WS_Flag_Trains);
  }
  else{
    ws_send(client_fd, data, len, WS_Flag_Trains);
  }

  _free(data);
}

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
void WS_LinkTrain(uint8_t fID, uint8_t tID){
  ws_send_all((char []){WSopc_LinkTrain,fID,tID},3,0xFF);
}

void WS_TrainData(char data[14]){
  loggerf(TRACE,"WS_TrainData");
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

void WS_cts_AddCartoLib(struct s_opc_AddNewCartolib * data, struct web_client_t * client){
  loggerf(DEBUG, "WS_cts_AddCartoLib");
  struct s_WS_Data * rdata = _calloc(1, sizeof(struct s_WS_Data));

  rdata->opcode = WSopc_AddNewCartolib;
  rdata->data.opc_AddNewCartolib_res.nr = data->nr;

  char * name = _calloc(data->name_len + 1, 1);
  char * icon = _calloc(data->name_len + 8 + 3 + 20, 1); //Destination file
  char * sicon = _calloc(40, 1); //Source file

  memcpy(name, &data->strings, data->name_len);

  uint16_t icon_time = (data->timing / 60) * 100 + (data->timing % 60);

  loggerf(ERROR, "%04i", icon_time);

  if((data->filetype & 0b1) == 0){
    sprintf(icon, "%i_%s_ic.%s", data->nr, name, "png");
    sprintf(sicon, "%s.%04i.%s", "web/tmp_icon", icon_time, "png");
  }
  else{
    sprintf(icon, "%i_%s_ic.%s", data->nr, name, "jpg");
    sprintf(sicon, "%s.%04i.%s", "web/tmp_icon", icon_time, "jpg");
  }


  create_car(name, data->nr, icon, icon, data->type, data->length, data->max_speed);

  char * dicon = _calloc(strlen(icon)+10, 1);

  sprintf(dicon, "%s%s", "web/trains_img/", icon);

  move_file(sicon, dicon);

  train_write_confs();

  rdata->data.opc_AddNewCartolib_res.response = 1;
  ws_send(client->fd, (char *)rdata, WSopc_AddNewCartolib_res_len, 0xff);

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

  C->length = data->length;
  C->type = data->type;

  char * sicon = _calloc(40, 1); //Source file

  uint16_t icon_time = (data->timing / 60) * 100 + (data->timing % 60);

  loggerf(ERROR, "%04i", icon_time);

  char * dicon = 0;

  if(icon_time < 3000){
    C->icon_path = _realloc(C->icon_path, data->name_len + 8 + 3 + 20, 1); //Destination file
    if((data->filetype & 0b1) == 0){
      sprintf(C->icon_path, "%i_%s_ic.%s", data->nr, C->name, "png");
      sprintf(sicon, "%s.%04i.%s", "web/tmp_icon", icon_time, "png");
    }
    else{
      sprintf(C->icon_path, "%i_%s_ic.%s", data->nr, C->name, "jpg");
      sprintf(sicon, "%s.%04i.%s", "web/tmp_icon", icon_time, "jpg");
    }
    dicon = _calloc(strlen(C->icon_path)+10, 1);
    sprintf(dicon, "%s%s", "web/trains_img/", C->icon_path);
    move_file(sicon, dicon);
  }

  rdata->data.opc_AddNewCartolib_res.response = 1;
  ws_send(client->fd, (char *)rdata, WSopc_AddNewCartolib_res_len, 0xff);

  _free(dicon);
  _free(sicon);
}

void WS_cts_AddEnginetoLib(struct s_opc_AddNewEnginetolib * data, struct web_client_t * client){
  loggerf(DEBUG, "WS_cts_AddEnginetoLib");
  struct s_WS_Data * rdata = _calloc(1, sizeof(struct s_WS_Data));

  rdata->opcode = WSopc_AddNewEnginetolib;
  rdata->data.opc_AddNewEnginetolib_res.DCC_ID = data->DCC_ID;

  if (DCC_train[data->DCC_ID]){
    loggerf(ERROR, "DCC %i allready in use", data->DCC_ID);
    rdata->data.opc_AddNewEnginetolib_res.response = 255;
    ws_send(client->fd, (char *)rdata, WSopc_AddNewEnginetolib_res_len, 0xff);
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

  uint16_t image_time = data->timing[0] + ((data->timing[1] & 0xf0) << 4);
  image_time = (image_time / 60) * 100 + (image_time % 60);
  uint16_t icon_time = (data->timing[1] & 0x0f) + (data->timing[2] << 4);
  icon_time = (icon_time / 60) * 100 + (icon_time % 60);

  loggerf(ERROR, "%04i - %04i", image_time, icon_time);

  if((data->flags & 0b10) == 0){
    sprintf(img, "%i_%s_im.%s", data->DCC_ID, name, "png");
    sprintf(simg, "%s.%04i.%s", "web/tmp_img", image_time, "png");
  }
  else{
    sprintf(img, "%i_%s_im.%s", data->DCC_ID, name, "jpg");
    sprintf(simg, "%s.%04i.%s", "web/tmp_img", image_time, "jpg");
  }

  if((data->flags & 0b1) == 0){
    sprintf(icon, "%i_%s_ic.%s", data->DCC_ID, name, "png");
    sprintf(sicon, "%s.%04i.%s", "web/tmp_icon", icon_time, "png");
  }
  else{
    sprintf(icon, "%i_%s_ic.%s", data->DCC_ID, name, "jpg");
    sprintf(sicon, "%s.%04i.%s", "web/tmp_icon", icon_time, "jpg");
  }

  create_engine(name, data->DCC_ID, img, icon, data->type, data->length, data->steps, (struct engine_speed_steps *)steps);

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

  //Update clients Train Library
  WS_EnginesLib(0);
}

void WS_cts_Edit_Engine(Engines * E, struct s_opc_AddNewEnginetolib * data, struct web_client_t * client){
  loggerf(DEBUG, "WS_cts_Edit_Engine");
  struct s_WS_Data * rdata = _calloc(1, sizeof(struct s_WS_Data));

  rdata->opcode = WSopc_AddNewEnginetolib;
  rdata->data.opc_AddNewEnginetolib_res.DCC_ID = data->DCC_ID;

  loggerf(WARNING, "Editing %s", E->name);

  if (DCC_train[data->DCC_ID] && data->DCC_ID != E->DCC_ID){
    loggerf(ERROR, "DCC %i (%i) allready in use", data->DCC_ID, E->DCC_ID);
    rdata->data.opc_AddNewEnginetolib_res.response = 255;
    ws_send(client->fd, (char *)rdata, WSopc_AddNewEnginetolib_res_len, 0xff);
    return;
  }

  E->name = _realloc(E->name, data->name_len + 1, 1);
  memcpy(E->name, &data->strings, data->name_len);
  E->name[data->name_len] = 0;

  loggerf(INFO, "New name: %s", E->name);

  E->steps_len = data->steps;
  E->steps = _realloc(E->steps, data->steps, 3);
  memcpy(E->steps, &data->strings + data->name_len, data->steps * 3);

  for(int i = 0; i < E->steps_len; i++){
    printf("  %d, %d", E->steps[i].step, E->steps[i].speed);
  }

  E->length = data->length;
  E->type = data->type;

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
    E->img_path = _realloc(E->img_path, data->name_len + 8 + 3 + 20, 1);  //Destination file
    if((data->flags & 0b10) == 0){
      sprintf(E->img_path, "%i_%s_im.%s", data->DCC_ID, E->name, "png");
      sprintf(simg, "%s.%04i.%s", "web/tmp_img", image_time, "png");
    }
    else{
      sprintf(E->img_path, "%i_%s_im.%s", data->DCC_ID, E->name, "jpg");
      sprintf(simg, "%s.%04i.%s", "web/tmp_img", image_time, "jpg");
    }

    dimg = _calloc(strlen(E->img_path)+10, 1);
    sprintf(dimg, "%s%s", "web/trains_img/", E->img_path);
    move_file(simg,  dimg);
  }

  if(icon_time < 3000){
    E->icon_path = _realloc(E->icon_path, data->name_len + 8 + 3 + 20, 1); //Destination file
    if((data->flags & 0b1) == 0){
      sprintf(E->icon_path, "%i_%s_ic.%s", data->DCC_ID, E->name, "png");
      sprintf(sicon, "%s.%04i.%s", "web/tmp_icon", icon_time, "png");
    }
    else{
      sprintf(E->icon_path, "%i_%s_ic.%s", data->DCC_ID, E->name, "jpg");
      sprintf(sicon, "%s.%04i.%s", "web/tmp_icon", icon_time, "jpg");
    }
    dicon = _calloc(strlen(E->icon_path)+10, 1);
    sprintf(dicon, "%s%s", "web/trains_img/", E->icon_path);
    move_file(sicon, dicon);
  }

  // create_engine(name, data->DCC_ID, img, icon, data->type, data->length, data->steps, (struct engine_speed_steps *)steps);

  rdata->data.opc_AddNewEnginetolib_res.response = 1;
  ws_send(client->fd, (char *)rdata, WSopc_AddNewEnginetolib_res_len, 0xff);

  _free(dimg);
  _free(dicon);
  _free(simg);
  _free(sicon);
  _free(filetype);
}

void WS_cts_AddTraintoLib(struct s_opc_AddNewTraintolib * data, struct web_client_t * client){
  loggerf(DEBUG, "WS_cts_AddTraintoLib");
  struct s_WS_Data * rdata = _calloc(1, sizeof(struct s_WS_Data));

  rdata->opcode = WSopc_AddNewTraintolib;

  char * name = _calloc(data->name_len + 1, 1);
  char * comps = _calloc(data->nr_stock, 3);

  loggerf(DEBUG, "Name:%i\tComp: %i", data->name_len, data->nr_stock);
  print_hex(&data->strings + data->name_len, data->nr_stock*3);

  memcpy(name, &data->strings, data->name_len);
  memcpy(comps, &data->strings + data->name_len, data->nr_stock*3);

  create_train(name, data->nr_stock, (struct train_comp_ws *)comps, data->catagory, data->save);

  train_write_confs();

  rdata->data.opc_AddNewTraintolib_res.response = 1;
  ws_send(client->fd, (char *)rdata, WSopc_AddNewTraintolib_res_len, 0xff);

  //Update clients Train Library
  WS_TrainsLib(0);
}

void WS_cts_Edit_Train(Trains * T, struct s_opc_AddNewTraintolib * data, struct web_client_t * client){
  loggerf(DEBUG, "WS_cts_Edit_Train ");
  struct s_WS_Data * rdata = _calloc(1, sizeof(struct s_WS_Data));

  rdata->opcode = WSopc_AddNewTraintolib;

  T->name = _realloc(T->name, data->name_len + 1, 1);
  T->composition = _realloc(T->composition, data->nr_stock, 3);

  loggerf(DEBUG, "Name:%i\tComp: %i", data->name_len, data->nr_stock);
  print_hex(&data->strings + data->name_len, data->nr_stock*3);

  memcpy(T->name, &data->strings, data->name_len);
  T->name[data->name_len] = 0;
  memcpy(T->composition, &data->strings + data->name_len, data->nr_stock*3);

  rdata->data.opc_AddNewTraintolib_res.response = 1;
  ws_send(client->fd, (char *)rdata, WSopc_AddNewTraintolib_res_len, 0xff);

  //Update clients Train Library
  WS_TrainsLib(0);
}


//Track Messages
void WS_trackUpdate(int Client_fd){
  loggerf(TRACE, "WS_trackUpdate");
  mutex_lock(&mutex_lockB, "Lock Mutex B");
  char data[4096];

  data[0] = WSopc_BroadTrack; //Opcode

  int data_len;
  _Bool content = 0;

  int q = 1;

  loggerf(TRACE, "WS_trackUpdate");

  for(int i = 0;i<unit_len;i++){
    if(!Units[i])// || (Units[i]->changed & Unit_Blocks_changed) == 0)
      continue;

    Units[i]->changed &= ~Unit_Blocks_changed;

    for(int j = 0;j<Units[i]->block_len;j++){
      Block * B = Units[i]->B[j];
      if(B && (B->changed & State_Changed)){
        content = 1;

        data[(q-1)*4+1] = B->module;
        data[(q-1)*4+2] = B->id;
        data[(q-1)*4+3] = (B->dir << 7) + B->state;
        data[(q-1)*4+4] = B->train;
        q++;

        B->changed &= ~(State_Changed);
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
  mutex_unlock(&mutex_lockB, "UnLock Mutex B");
}

void WS_SwitchesUpdate(int Client_fd){
  loggerf(TRACE, "WS_SwitchesUpdate (%i)", Client_fd);
  mutex_lock(&mutex_lockB, "Lock Mutex B");
  char buf[4096];
  memset(buf, 0, 4096);
  /*Switches*/

    buf[0] = WSopc_BroadSwitch;
    int buf_l  = 0;
    _Bool content   = 0;

    int q = 1;
    //printf("\n\n3");

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
      printf("ws_send client");
      ws_send(Client_fd, buf, buf_l, WS_Flag_Switches);
    }else{
      printf("ws_send all");
      ws_send_all(buf, buf_l, WS_Flag_Switches);
    }
  }
  else
    loggerf(DEBUG, "WS Switches no content");

  mutex_unlock(&mutex_lockB, "UnLock Mutex B");
}

void WS_NewClient_track_Switch_Update(int Client_fd){
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
          buf[(q-1)*4+4] = B->train;
          q++;

          B->changed &= ~(State_Changed);
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

  mutex_unlock(&mutex_lockB, "UnLock Mutex B");
}

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

void WS_Track_LayoutDataOnly(int unit, int Client_fd){
  loggerf(DEBUG, "WS_Track_LayoutDataOnly");

  char * data = _calloc(Units[unit]->Layout_length + 20, 1);

  data[0] = WSopc_TrackLayoutOnlyRawData;
  data[1] = unit;
  memcpy(&data[2], Units[unit]->Layout, Units[unit]->Layout_length);

  print_hex(data, Units[unit]->Layout_length + 20);


  if(Client_fd){
    ws_send(Client_fd,data, Units[unit]->Layout_length+2, WS_Flag_Track);
  }else{
    ws_send_all(data, Units[unit]->Layout_length+2, WS_Flag_Track);
  }
}

void WS_stc_TrackLayoutRawData(int unit, int Client_fd){
  Unit * U = Units[unit];
  char * data = _calloc(U->raw_length+2, 1);
  data[0] = WSopc_TrackLayoutRawData;
  data[1] = unit;

  memcpy(&data[2], U->raw, U->raw_length);

  if(Client_fd){
    ws_send(Client_fd, data, U->raw_length+2, WS_Flag_Track);
  }
  else{
    ws_send_all(data, U->raw_length+2, WS_Flag_Track);
  }
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
