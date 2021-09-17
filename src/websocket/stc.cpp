#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include <pthread.h>

//Websocket opcodes
#include "websocket/server.h"
#include "websocket/client.h"
#include "websocket/message.h"
#include "websocket/message_structure.h"
#include "websocket/stc.h"

#include "system.h"
#include "utils/mem.h"
#include "utils/logger.h"

#include "switchboard/manager.h"
#include "switchboard/unit.h"
#include "switchboard/rail.h"
#include "switchboard/station.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "train.h"
#include "pathfinding.h"

#include "Z21.h"

#include "submodule.h"


#define ACTIVATE 0
#define RELEASE  1

using namespace switchboard;

pthread_mutex_t mutex_lockB;

struct WS_Message MessageList[0x1FFF];
uint16_t MessageCounter = 0;


//System Messages
uint16_t WS_stc_ScanStatus(uint16_t msgID, uint16_t x, uint16_t y){ // x connected out of y (x/y)
  if(msgID == (uint16_t)-1)
    msgID = WS_init_Message(WS_MESSAGE_SCANSTATUS);

  if(x == y){
    WS_clear_message(msgID, 1);
    return -1;
  }


  char data[6];
  data[0] = WSopc_NewMessage;
  data[1] = ((msgID >> 8) & 0x1F) + (WS_MESSAGE_SCANSTATUS << 5); //type = 0xE0 / 0x7
  data[2] = (msgID & 0xFF);
  data[3] = x;
  data[4] = y;

  loggerf(INFO, "WS_stc_ScanStatus %02x%02x", data[1], data[2]);
  WSServer->send_all(data, 5, WS_Flag_Messages);
  WS_add_Message(msgID, 5, data);

  return msgID;
}

void WS_stc_Partial_Layout(uint8_t M){
  loggerf(DEBUG, "WS_stc_Partial_Layout %i", M);
  char data[20];
  int q = 1;
  memset(data,0,20);
  data[0] = WSopc_Track_Layout_Update;

  data[q++] = M;

  Unit * U = Units(M);
  for(int i = 0;i< U->connections_len;i++){
    data[q++] = U->connection[i].unit;
  }

  WSServer->send_all(data,q,WS_Flag_Admin);
}

void WS_stc_Track_Layout(Websocket::Client * client){

  char data[100];
  int q = 1;
  memset(data,0,100);
  data[0] = WSopc_Track_Layout_Config;

  loggerf(DEBUG, "WS_stc_Track_Layout");

  for(int i = 0;i< SwManager->Units.size;i++){
    Unit * U = Units(i);
    if(!U || !U->on_layout)
      continue;

    data[q++] = i;

    for(int j = 0;j< U->connections_len;j++){
      data[q++] = U->connection[j].unit;
    }
  }

  if(q > 1){
    if(client){
      client->send(data,q,WS_Flag_Track);
    }
    else{
      WSServer->send_all(data,q,WS_Flag_Track);
    }
  }
}

void WS_stc_Z21_info(Websocket::Client * client){
  uint8_t data[20];
  data[0] = WSopc_Track_Info;
  data[1] = Z21->sensors.MainCurrent & 0xFF;
  data[2] = Z21->sensors.MainCurrent >> 8;
  data[3] = Z21->sensors.FilteredMainCurrent & 0xFF;
  data[4] = Z21->sensors.FilteredMainCurrent >> 8;
  data[5] = Z21->sensors.ProgCurrent & 0xFF;
  data[6] = Z21->sensors.ProgCurrent >> 8;
  data[7] = Z21->sensors.VCCVoltage & 0xFF;
  data[8] = Z21->sensors.VCCVoltage >> 8;
  data[9] = Z21->sensors.SupplyVoltage & 0xFF;
  data[10] = Z21->sensors.SupplyVoltage >> 8;
  data[11] = Z21->sensors.Temperature & 0xFF;
  data[12] = Z21->sensors.Temperature >> 8;
  data[13] = Z21->sensors.CentralState;
  data[14] = Z21->sensors.CentralStateEx;

  if(client)
    client->send((char *)data, 15, 0xff);
  else
    WSServer->send_all((char *)data, 15, 0xff);
}

void WS_stc_Z21_IP(Websocket::Client * client){
  uint8_t data[10];
  data[0] = WSopc_Z21_Settings;
  memcpy(&data[1], &Z21->IP[0], 4);
  memcpy(&data[5], &Z21->Firmware[0], 2);

  if(client)
    client->send((char *)data, 7, 0xff);
  else
    WSServer->send_all((char *)data, 7, 0xff);
}

void WS_stc_SubmoduleState(Websocket::Client * client){
  char data[5];
  data[0] = WSopc_SubModuleState;

  // loggerf(INFO, "WS_stc_SubmoduleState %x %x %x %x %x", SYS->WebSocket.state, SYS->Z21.state, SYS->UART.state, SYS->LC.state, SYS->TC.state);

  data[1] = ((SYS->WebSocket.state & 0xF) << 4) | (SYS->Z21.state & 0xF);
  data[2] = ((SYS->UART.state & 0xF) << 4) | (SYS->LC.state & 0xF);
  data[3] = (SYS->TC.state & 0xF) << 4;
  data[4] = ((SYS->SimA.state & 0xF) << 4) | (SYS->SimB.state & 0xF);

  if(client)
    client->send((char *)data, 5, 0xff);
  else
    WSServer->send_all((char *)data, 5, 0xff);
}

//Admin Messages

//Train Messages
void WS_stc_LinkTrain(struct s_opc_LinkTrain * msg){
  struct s_WS_Data return_msg;
  return_msg.opcode = WSopc_LinkTrain;
  memcpy(&return_msg.data, msg, sizeof(struct s_opc_LinkTrain));
  return_msg.data.opc_LinkTrain.message_id_H = 0;
  return_msg.data.opc_LinkTrain.message_id_L = 0;
  
  WSServer->send_all((char *)&return_msg, 5, 0xFF);
}

void WS_stc_UpdateTrain(Train * T){
  WS_stc_UpdateTrain(T, 0);
}
void WS_stc_UpdateTrain(Train * T, Websocket::Client * client){
  // send to all except client

  if(!T)
    return;

  struct s_WS_Data return_msg;
  return_msg.opcode = WSopc_UpdateTrain;
  struct s_opc_UpdateTrain * msg = &return_msg.data.opc_UpdateTrain;

  msg->follow_id = T->id;

  msg->dir = T->dir;
  msg->control = (((uint8_t)T->fullAuto) << 1) || (uint8_t)(T->manual ^ 1);
  msg->speed_high = (T->speed & 0x0F00) >> 8;
  msg->speed_low  = (T->speed & 0xFF);

  // T->exportFunctions(msg->functions);

  if(T->type == TRAIN_ENGINE_TYPE){
    for(uint8_t i = 0; i < 29; i++){
      if(T->p.E->function[i].state){
        msg->functions[i / 8] |= (0x80 >> (i % 8));
      }
    }
  }
  else{
    // Train type
    msg->functions[0] = 0;
    msg->functions[1] = 0;
    msg->functions[2] = 0;
    msg->functions[3] = 0;
  }

  loggerf(TRACE, "train Update_Train id: %i, d: %i, c: %i, sp: %x%02x", msg->follow_id, msg->dir, msg->control, msg->speed_high, msg->speed_low);

  if(!WSServer)
    return;
  for(uint8_t i = 0; i < WSServer->clients.size(); i++){
    auto c = WSServer->clients[i];
    if(c == client)
      continue;

    if((c->subscribedTrains[0] == T->id) || (c->subscribedTrains[1] == T->id)){
      c->send((char *)&return_msg, sizeof(struct s_opc_UpdateTrain) + 1, WS_Flag_Trains);
    }
  }
}

void WS_stc_TrainRouteUpdate(Train * T){
  if(!T)
    return;

  struct s_WS_Data return_msg;
  return_msg.opcode = WSopc_TrainAddRoute;
  struct s_opc_TrainRoute * msg = &return_msg.data.opc_TrainRoute;

  msg->train_id = T->id;

  if(T->route){
    msg->RouteEnabled = 1;
    msg->RouteHigh = T->route->destination >> 8;
    msg->RouteLow  = T->route->destination & 0xFF;
    msg->RouteType = T->route->routeType;
  }
  else{
    msg->RouteEnabled = 0;
    msg->RouteHigh = 0;
    msg->RouteLow  = 0;
    msg->RouteType = 0;
  }

  if(!WSServer)
    return;
  for(uint8_t i = 0; i < WSServer->clients.size(); i++){
    if((WSServer->clients[i]->subscribedTrains[0] == T->id) || 
       (WSServer->clients[i]->subscribedTrains[1] == T->id)){
      WSServer->clients[i]->send((char *)&return_msg, sizeof(struct s_opc_TrainRoute) + 1, WS_Flag_Trains);
    }
  }
}

void WS_stc_DCCEngineUpdate(Engine * E){
  loggerf(TRACE, "WS_cts_DCCEngineUpdate");

  if(!E)
    return;

  struct s_WS_Data return_msg;
  return_msg.opcode = WSopc_DCCEngineUpdate;
  struct s_opc_DCCEngineUpdate * msg = &return_msg.data.opc_DCCEngineUpdate;

  msg->follow_id = E->id;

  msg->dir = E->dir;
  msg->control = 0;
  msg->speed_high = (E->cur_speed & 0x0F00) >> 8;
  msg->speed_low  = (E->cur_speed & 0xFF);

  memset(msg->functions, 0, 4);

  for(uint8_t i = 0; i < 29; i++){
    if(E->function[i].state){
      msg->functions[i / 8] |= (0x80 >> (i % 8));
    }
  }

  loggerf(DEBUG, "train Update_Train id: %i, d: %i, c: %i, sp: %x%02x", msg->follow_id, msg->dir, msg->control, msg->speed_high, msg->speed_low);

  WSServer->send_all((char *)&return_msg, sizeof(struct s_opc_DCCEngineUpdate) + 1, WS_Flag_Trains);
}

void WS_stc_EnginesLib(Websocket::Client * client){
  int buffer_size = 1024;

  char * data = (char *)_calloc(buffer_size, char);

  int len = 0;

  data[len++] = WSopc_EnginesLibrary;

  for(int i = 0; i < RSManager->Engines.items;i++){
    if(buffer_size < (len + 100)){
      buffer_size += 1024;
      data = (char *)_realloc(data, buffer_size, char);
    }
    Engine * E = RSManager->getEngine(i);
    if(!E){
      continue;
    }

    data[len++] = E->DCC_ID & 0xFF;
    data[len++] = E->DCC_ID >> 8;
    data[len++] = E->max_speed & 0xFF;
    data[len++] = E->max_speed >> 8;
    data[len++] = E->length & 0xFF;
    data[len++] = E->length >> 8;
    data[len++] = E->type;

    data[len++] = E->speed_step_type & 0x3;

    data[len++] = E->configSteps_len;
    data[len++] = strlen(E->name);
    data[len++] = strlen(E->img_path);
    data[len++] = strlen(E->icon_path);

    for(uint8_t j = 0; j < 29; j++){
      data[len++] = (E->function[j].button << 6) | E->function[j].type;
    }

    int l = strlen(E->name);
    memcpy(&data[len], E->name, l);
    len += l;

    l = strlen(E->img_path);
    memcpy(&data[len], E->img_path, l);
    len += l;

    l = strlen(E->icon_path);
    memcpy(&data[len], E->icon_path, l);
    len += l;

    for(int j = 0; j < E->configSteps_len; j++){
      data[len++] = E->configSteps[j].speed & 0xFF;
      data[len++] = E->configSteps[j].speed >> 8;
      data[len++] = E->configSteps[j].step;
    }
  }
  if(client)
    client->send(data, len, WS_Flag_Trains);
  else
    WSServer->send_all(data, len, WS_Flag_Trains);
  
  _free(data);
}

void WS_stc_CarsLib(Websocket::Client * client){
  loggerf(INFO, "CarsLib for client %i", client);
  int buffer_size = 1024;

  char * data = (char *)_calloc(buffer_size, char);

  int len = 0;

  data[len++] = WSopc_CarsLibrary;

  for(int i = 0; i < RSManager->Cars.items; i++){
    if(buffer_size < (len + 100)){
      buffer_size += 1024;
      data = (char *)_realloc(data, buffer_size, char);
    }
    Car * C = RSManager->getCar(i);
    if(!C){
      continue;
    }
    data[len++] = C->nr & 0xFF;
    data[len++] = C->nr >> 8;
    data[len++] = C->max_speed & 0xFF;
    data[len++] = C->max_speed >> 8;
    data[len++] = C->length & 0xFF;
    data[len++] = C->length >> 8;
    data[len++] = C->type;
    data[len++] = strlen(C->name);
    data[len++] = strlen(C->icon_path);

    int l = strlen(C->name);
    memcpy(&data[len], C->name, l);
    len += l;

    l = strlen(C->icon_path);
    memcpy(&data[len], C->icon_path, l);
    len += l;
  }
  if(client)
    client->send(data, len, WS_Flag_Trains);
  else
    WSServer->send_all(data, len, WS_Flag_Trains);
  
  _free(data);
}

void WS_stc_TrainSetsLib(Websocket::Client * client){
  loggerf(INFO, "TrainSetsLib for client %i", client);
  int buffer_size = 1024;

  char * data = (char *)_calloc(buffer_size, char);

  int len = 0;

  data[len++] = WSopc_TrainsLibrary;

  for(int i = 0; i < RSManager->TrainSets.items; i++){
    if(buffer_size < (len + 100)){
      buffer_size += 1024;
      data = (char *)_realloc(data, buffer_size, char);
    }
    TrainSet * T = RSManager->getTrainSet(i);
    if(!T)
      continue;
    
    data[len++] = T->max_speed & 0xFF;
    data[len++] = T->max_speed >> 8;

    data[len++] = T->length & 0xFF;
    data[len++] = T->length >> 8;

    data[len] = T->type << 1;
    data[len++] |= T->in_use;

    data[len++] = strlen(T->name);

    data[len++] = T->nr_stock;

    int l = strlen(T->name);
    memcpy(&data[len], T->name, l);
    len += l;

    for(int c = 0; c<T->nr_stock; c++){
      data[len++] = T->composition[c].type;
      data[len++] = T->composition[c].id & 0xFF;
      data[len++] = T->composition[c].id >> 8;
    }
  }
  if(client)
    client->send(data, len, WS_Flag_Trains);
  else
    WSServer->send_all(data, len, WS_Flag_Trains);
  
  _free(data);
}

void WS_stc_TrainCategories(Websocket::Client * client){
  loggerf(INFO, "TrainCategories for client %i", client);
  int buffer_size = 1024;

  char * data = (char *)_calloc(buffer_size, char);

  int len = 0;

  data[len++] = WSopc_TrainCategories;

  for(int i = 0;i<train_P_cat_len;i++){
    if(buffer_size < (len + 100)){
      buffer_size += 1024;
      data = (char *)_realloc(data, buffer_size, char);
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
      data = (char *)_realloc(data, buffer_size, char);
    }
    data[len++] = i | 0x80;

    data[len++] = strlen(train_C_cat[i].name);

    int l = strlen(train_C_cat[i].name);
    memcpy(&data[len], train_C_cat[i].name, l);
    len += l;
  }

  if(client)
    client->send(data, len, WS_Flag_Trains);
  else
    WSServer->send_all(data, len, WS_Flag_Trains);
  
  _free(data);
}

void WS_stc_NewTrain(Train * T,char M,char B){
  //Nr:   follow id of train
  //M,B:  module nr and block nr
  uint16_t msg_ID = WS_init_Message(0);

  loggerf(TRACE, "WS_stc_NewTrain %i", T->id);

  char data[6];
  data[0] = WSopc_NewMessage;
  data[1] = ((msg_ID >> 8) & 0x1F) + 0; //type = 0
  data[2] = (msg_ID & 0xFF);
  data[3] = T->id;
  data[4] = M;
  data[5] = B;
  WSServer->send_all(data,6,WS_Flag_Messages);
  WS_add_Message(msg_ID,6,data);
}

void WS_stc_TrainSplit(Train * T, char M1,char B1,char M2,char B2){
  //Nr:   follow id of train
  //M,B:  module nr and block nr
  uint16_t msg_ID = WS_init_Message(1);

  loggerf(INFO, "WS_TrainSplit");

  char data[8];
  data[0] = WSopc_NewMessage;
  data[1] = ((msg_ID >> 8) & 0x1F) + 0x20; //type = 1
  data[2] = (msg_ID & 0xFF);
  data[3] = T->id;
  data[4] = M1;
  data[5] = B1;
  data[6] = M2;
  data[7] = B2;
  WSServer->send_all(data,8,WS_Flag_Messages);
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
    WSServer->send_all(data,5,1);
  }else if(i == RELEASE){
    data[1] = 6;
    data[2] = tID;
    WSServer->send_all(data,3,1);
  }else{
    return;
  }
}
*/

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

//   WSServer->send_all(s_data,9,WS_Flag_Trains);
// }

//Track Messages
void WS_stc_trackUpdate(Websocket::Client * client){
  // 0x26 - Broadcast track occupation
  //         and block status

  loggerf(TRACE, "WS_trackUpdate");
  mutex_lock(&mutex_lockB, "Lock Mutex B");
  char data[4096];

  data[0] = WSopc_BroadTrack; //Opcode

  int data_len;
  bool content = 0;

  int q = 1;

  for(int i = 0; i < SwManager->Units.size; i++){
    Unit * U = Units(i);
    if(!U || !U->on_layout || !U->block_state_changed)
      continue;

    U->block_state_changed = 0;

    for(int j = 0;j<U->block_len;j++){
      Block * B = U->B[j];
      if(B && (B->statechanged || B->IOchanged)){
        content = 1;

        data[(q-1)*4+1] = B->module;
        data[(q-1)*4+2] = B->id;
        data[(q-1)*4+3] = (B->dir << 7) | (B->polarity_status << 6) | (B->state & 0x3F);
        data[(q-1)*4+4] = 0; //B->train;
        q++;

        B->statechanged = 0;
      }
    }
  }

  data_len = (q-1)*4+1;

  if(content == 1){
    if(client){
      client->send(data, data_len, WS_Flag_Track);
    }else{
      WSServer->send_all(data, data_len, WS_Flag_Track);
    }
  }
  mutex_unlock(&mutex_lockB, "UnLock Mutex B");
}

void WS_stc_SwitchesUpdate(Websocket::Client * client){
  loggerf(TRACE, "WS_SwitchesUpdate (%x)", (unsigned int)client);
  mutex_lock(&mutex_lockB, "Lock Mutex B");
  char buf[4096];
  memset(buf, 0, 4096);
  /*Switches*/

    buf[0] = WSopc_BroadSwitch;
    int buf_l  = 0;
    bool content   = 0;

    int q = 1;

    for(int i = 0; i < SwManager->Units.size; i++){
      Unit * U = Units(i);
      if(!U || !U->on_layout || !U->switch_state_changed)
        continue;

      U->switch_state_changed = 0;

      for(int j = 0;j<U->switch_len;j++){
        Switch * S = U->Sw[j];
        if(!S)
          continue;

        if(!S->updatedState){
          loggerf(TRACE, "%i:%i no new state", S->module, S->id);
          continue;
        }

        content = 1;

        buf[(q-1)*3+1] = S->module;
        buf[(q-1)*3+2] = S->id & 0x7F;
        buf[(q-1)*3+3] = S->state;

        S->updatedState = false;

        loggerf(DEBUG, "%i,%i,%i", S->module, S->id, S->state);
        q++;
      }
    }

    buf_l = (q-1)*3;

  // MSSwitches
  //buf[0] = 5;
  // buf_l = 0;
  q = 1;
  for(int i = 0; i < SwManager->Units.size; i++){
    Unit * U = Units(i);
    if(!U || !U->msswitch_state_changed)
      continue;
    content = 1;
    for(int j = 0;j<=U->msswitch_len;j++){
      MSSwitch * Sw = U->MSSw[j];
      if(!Sw)
        continue;

      if(!Sw->updatedState){
        loggerf(TRACE, "%i:%i no new state", Sw->module, Sw->id);
        continue;
      }

      buf[(q-1)*4+1+buf_l] = Sw->module;
      buf[(q-1)*4+2+buf_l] = (Sw->id & 0x7F) + 0x80;
      buf[(q-1)*4+3+buf_l] = Sw->state;
      buf[(q-1)*4+4+buf_l] = Sw->state_len;

      Sw->updatedState = false;

      q++;
    }
  }
  
  buf_l += (q-1)*4+1;
  if(content == 1){
    if(client){
      client->send(buf, buf_l, WS_Flag_Switches);
    }else{
      WSServer->send_all(buf, buf_l, WS_Flag_Switches);
    }
  }
  // else
  //   loggerf(DEBUG, "WS Switches no content");

  mutex_unlock(&mutex_lockB, "UnLock Mutex B");
}

void WS_stc_NewClient_track_Switch_Update(Websocket::Client * client){
  mutex_lock(&mutex_lockB, "Lock Mutex B");

  //Track
  char buf[4096];

  buf[0] = WSopc_BroadTrack; //Opcode

  int data_len;
  bool content = 0;

  int q = 1;

  for(int i = 0; i < SwManager->Units.size; i++){
    Unit * U = Units(i);
    if(!U || !U->on_layout)
      continue;

    for(int j = 0; j < U->block_len; j++){
      Block * B = U->B[j];
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

  data_len = (q-1)*4+1;

  if(content == 1){
    if(client){
      client->send(buf, data_len, WS_Flag_Track);
    }else{
      WSServer->send_all(buf, data_len, WS_Flag_Track);
    }
  }


  /*Switches*/

  memset(buf,0,4096);
  /*Switches*/

    buf[0] = WSopc_BroadSwitch;
    int buf_len = 0;
    content   = 0;

    q = 1; // Counter for switches

    for(int i = 0;i< SwManager->Units.size;i++){
      Unit * U = Units(i);
      if(!U || !U->on_layout)
        continue;
      for(int j = 0; j < U->switch_len;j++){
        Switch * S = U->Sw[j];
        if(S){
          content = 1;
          buf[(q-1)*3+1] = S->module;
          buf[(q-1)*3+2] = S->id & 0x7F;
          buf[(q-1)*3+3] = S->state;
          // printf(",%i,%i,%i",S->Module,S->id,S->state);
          q++;
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
      client->send(buf, buf_len, WS_Flag_Switches);
    }else{
      WSServer->send_all(buf, buf_len, WS_Flag_Switches);
    }
  }

  memset(buf,0,4096);
  /*Stations*/

  buf[0] = 6;
  buf_len = 1;
    bool data = 0;

  if(SwManager->uniqueStation.items > 0){
    data = 1;
  }
  for(int i = 0; i < SwManager->uniqueStation.size; i++){
    Station * St = SwManager->uniqueStation[i];
    if(!St)
      continue;

    buf[buf_len]   = St->module;
    buf[buf_len+1] = St->id;
    buf[buf_len+2] = strlen(St->name);
    strcpy(&buf[buf_len+3],St->name);

    buf_len+=3+strlen(St->name);
  }

  if(data == 1){
    if(client){
      client->send(buf, buf_len, WS_Flag_Switches);
    }else{
      WSServer->send_all(buf, buf_len, WS_Flag_Switches);
    }
  }

  memset(buf,0,4096);

  loggerf(INFO, "Z21_GET_LOCO_INFO check");
  for(int i = 1; i < RSManager->Trains.size;i++){
    Train * T = RSManager->getTrain(i);
    if(T){
      //TODO fix for Train
      //for(int j = 0; j < T->nr_engines; j++){
      //  Z21_get_engine(T->engines[j]->DCC_ID);
      //}
    }
  }

  mutex_unlock(&mutex_lockB, "UnLock Mutex B");
}

void WS_stc_reset_switches(Websocket::Client * client){
  //Check if client has admin rights
  char admin = 1;
  if(admin){
    //Go through all switches
    for(int i = 0; i < SwManager->Units.size; i++){
      Unit * U = Units(i);
      if(!U)
        continue;

      for(int j = 0; j < U->switch_len; j++){
        if(U->Sw[j]){
          U->Sw[j]->state = U->Sw[j]->default_state + 0x80;
        }
      }
    }

    //Send all switch updates
    WS_stc_SwitchesUpdate(client);
  }
}

void WS_stc_Track_Layout_Load(Websocket::Client * client){
  loggerf(INFO, "WS_stc_Track_Layout_Load");
  if(!client)
    return;

  char data[2048] = "";

  data[0] = WSopc_Track_Layout_Load;
  data[1] = switchboard::SwManager->setups.items;

  uint16_t j = 2;
  for(uint8_t i = 0; i < data[1]; i++){
    const char * s = switchboard::SwManager->setups[i]->string;

    loggerf(INFO, "  %2d, %3d: %s", j, strlen(s), s);

    data[j] = strlen(s);
    strcpy(&data[j+1], s);
    j += 1 + data[j];
  }

  client->send(data, j  , WS_Flag_Track);
}

void WS_stc_Track_LayoutDataOnly(int unit, Websocket::Client * client){
  loggerf(DEBUG, "WS_Track_LayoutDataOnly %i", unit);

  Unit * U = Units(unit);

  if(!U)
    return;

  char * data = (char *)_calloc(U->Layout_length + 20, char);

  data[0] = WSopc_TrackLayoutOnlyRawData;
  data[1] = unit;
  memcpy(&data[2], U->Layout, U->Layout_length);

  if(client){
    client->send(data, U->Layout_length+2, WS_Flag_Track);
  }else{
    WSServer->send_all(data, U->Layout_length+2, WS_Flag_Track);
  }

  _free(data);
}

void WS_stc_TrackLayoutRawData(int unit, Websocket::Client * client){
  Unit * U = Units(unit);
  char * data = (char *)_calloc(U->raw_length+2, char);
  data[0] = WSopc_TrackLayoutRawData;
  data[1] = unit;

  memcpy(&data[2], U->raw, U->raw_length);

  if(client){
    client->send(data, U->raw_length+2, WS_Flag_Track);
  }
  else{
    WSServer->send_all(data, U->raw_length+2, WS_Flag_Track);
  }

  _free(data);
}

void WS_stc_StationLib(Websocket::Client * client){
  char * data = (char *)_calloc(SwManager->uniqueStation.size, Station);
  data[0] = WSopc_StationLibrary;
  char * length = &data[1];
  char * d = &data[2];

  for(uint8_t i = 0; i < SwManager->uniqueStation.size; i++){
    Station * St = SwManager->uniqueStation[i];
    if(!St)
      continue;

    (*length)++;
    d[0] = St->module;
    d[1] = St->id;
    d[2] = St->uid;

    if(St->parent){
      d[3] = St->parent->uid >> 8;
      d[4] = St->parent->uid & 0xFF;
    }
    else{
      d[3] = 0xFF;
      d[4] = 0xFF;
    }

    d[5] = St->type;
    d[6] = strlen(St->name);
    memcpy(&d[7], St->name, d[6]);

    d += d[6] + 7;
  }

  if(client){
    client->send(data, d - data, WS_Flag_Track);
  }
  else{
    WSServer->send_all(data, d - data, WS_Flag_Track);
  }

  _free(data);
}

//General Messages
void WS_stc_EmergencyStop(){
  loggerf(WARNING, "EMERGENCY STOP");
  char msg[1] = {WSopc_EmergencyStop};
  WSServer->send_all(msg, 1, 0xFF); //Everyone
}

void WS_stc_ShortCircuit(){
  loggerf(WARNING, "SHORT CIRCUIT");
  char msg[1] = {WSopc_ShortCircuitStop};
  WSServer->send_all(msg, 1, 0xFF); //Everyone
}

void WS_stc_ClearEmergency(){
  loggerf(INFO, "EMERGENCY Released");
  char msg[1] = {WSopc_ClearEmergency};
  WSServer->send_all(msg, 1, 0xFF); //Everyone
}


void WS_init_Message_List(){
  memset(MessageList, 0, 0x1FFF * sizeof(WS_Message));
  MessageCounter = 0;
}

uint16_t WS_init_Message(char type){
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

void WS_add_Message(uint16_t ID, char length, char * data){
  memcpy(MessageList[ID].data, data, length);
  MessageList[ID].data_length = length;

  loggerf(DEBUG, "create_message %x", ID);
}

void WS_send_open_Messages(Websocket::Client * client){
  for(int i = 0; i <= 0x1FFF; i++){
    if(MessageList[i].type & 0x8000){
      client->send(MessageList[i].data, MessageList[i].data_length, 0xFF);
    }
  }
}

void WS_clear_message(uint16_t ID, char ret_code){
  if(ret_code == 1)
    MessageList[ID].type = 0;

  loggerf(INFO, "clear_message %x", ID);

  char msg[3] = {};
  msg[0] = WSopc_ClearMessage;
  msg[1] = (char)( ((ID >> 8) & 0x1F) + (ret_code << 5) );
  msg[2] = (char)(ID & 0xFF);
  WSServer->send_all(msg, 3, 0xFF);
}
