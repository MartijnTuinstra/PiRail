#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include <pthread.h>

//Websocket opcodes
#include "websocket_control.h"
#include "websocket_cts.h"
#include "websocket.h"

#include "system.h"
#include "mem.h"

#include "rail.h"
#include "switch.h"
#include "train.h"
#include "logger.h"
#include "config.h"
#include "algorithm.h"

#include "modules.h"
#include "Z21.h"
#include "Z21_msg.h"

#include "submodule.h"


#define ACTIVATE 0
#define RELEASE  1

extern pthread_mutex_t mutex_lockB;


websocket_cts_func websocket_cts[256] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

  // 0x10 WSopc_EmergencyStop
  (void (*)(void *, struct web_client_t *))&WS_cts_SetEmergencyStop,
  // 0x11 WSopc_ShortCircuitStop
  0,
  // 0x12 WSopc_ClearEmergency
  (void (*)(void *, struct web_client_t *))&WS_cts_ClearEmergency,
  // 0x13 WSopc_NewMessage
  0,
  // 0x14 WSopc_UpdateMessage
  0,
  // 0x15 WSopc_ClearMessage
  0,
  // 0x16 WSopc_ChangeBroadcast
  0,
  // 0x17 WSopc_Service_State
  0,
  // 0x18 WSopc_Canvas_Data
  0,
  // 0x19 - 0x1F Reserved
  0,0,0,0,0,0,0,

  // 0x20 WSopc_SetSwitch
  (void (*)(void *, struct web_client_t *))&WS_cts_SetSwitch,
  // 0x21 WSopc_SetMultiSwitch
  (void (*)(void *, struct web_client_t *))&WS_cts_SetMultiSwitch,
  // 0x22 WSopc_SetSwitchReserved
  0,
  // 0x23 WSopc_ChangeSwitchReserved
  0,
  // 0x24 Reserved
  0,
  // 0x25 WSopc_SetSwitchRoute
  0,
  // 0x26 WSopc_BroadTrack
  0,
  // 0x27 WSopc_BroadSwitch
  0,
  // 0x28 - 0x2F Reserved
  0,0,0,0,0,0,0,0,

  // 0x30 WSopc_TrackLayoutOnlyRawData
  0,
  // 0x31
  (void (*)(void *, struct web_client_t *))&WS_cts_TrackLayoutRawData,
  // 0x32 Reserved
  0,
  // 0x33
  (void (*)(void *, struct web_client_t *))&WS_cts_TrackLayoutUpdate,
  // 0x34 - 0x35 Reserved
  0,0,
  // 0x36 WSopc_StationLibrary
  0,
  // 0x37 - 0x3F Reserved
  0,0,0,0,0,0,0,0,0,

  // 0x40 Reserved
  0,
  // 0x41
  (void (*)(void *, struct web_client_t *))&WS_cts_LinkTrain,
  // 0x42
  (void (*)(void *, struct web_client_t *))&WS_cts_SetTrainSpeed,
  // 0x43
  (void (*)(void *, struct web_client_t *))&WS_cts_SetTrainFunction,
  // 0x44
  (void (*)(void *, struct web_client_t *))&WS_cts_TrainControl,
  // 0x45 WSopc_TrainUpdate
  0,
  // 0x46 SetTrainRoute
  (void (*)(void *, struct web_client_t *))&WS_cts_TrainRoute,
  // 0x47-0x49 Reserved
  0,0,0,
  // 0x4A IndependentEngineUpdate (stc only)
  0,
  // 0x4B IndependentEngineSpeed
  (void (*)(void *, struct web_client_t *))&WS_cts_DCCEngineSpeed,
  // 0x4C IndependentEngineFunction
  (void (*)(void *, struct web_client_t *))&WS_cts_DCCEngineFunction,
  // 0x4D-0x4E Reserved
  0,0,
  // 0x4F
  (void (*)(void *, struct web_client_t *))&WS_cts_TrainSubscribe,

  // 0x50
  (void (*)(void *, struct web_client_t *))&WS_cts_AddEnginetoLib,
  // 0x51
  (void (*)(void *, struct web_client_t *))&WS_cts_Edit_Engine,
  // 0x52 WSopc_EnginesLibrary
  0,
  // 0x53
  (void (*)(void *, struct web_client_t *))&WS_cts_AddCartoLib,
  // 0x54 WSopc_EditCarlib
  (void (*)(void *, struct web_client_t *))&WS_cts_Edit_Car,
  // 0x55 WSopc_CarsLibrary
  0,
  // 0x56 WSopc_AddNewTraintolib
  (void (*)(void *, struct web_client_t *))&WS_cts_AddTraintoLib,
  // 0x57 WSopc_EditTrainlib
  (void (*)(void *, struct web_client_t *))&WS_cts_Edit_Train,
  // 0x58 WSopc_TrainsLibrary
  0,
  // 0x59 Reserved
  0,
  // 0x5A WSopc_TrainCategories
  0,
  // 0x5B - 0x5F Reserved
  0,0,0,0,0,

  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 6x
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 7x
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 8x

  // 0x90 WSopc_EnableSubModule
  (void (*)(void *, struct web_client_t *))&WS_cts_Enable_SubmoduleState,
  // 0x91 WSopc_DisableSubModule
  (void (*)(void *, struct web_client_t *))&WS_cts_Disable_SubmoduleState,
  // 0x92 WSopc_SubModuleState
  (void (*)(void *, struct web_client_t *))&WS_cts_SubmoduleState,
  // 0x93 - 0x9E Reserved
  0,0,0,0,0,0,0,0,0,0,0,0,
  // 0x9F WSopc_RestartApplication
  0, // 9x

  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // Ax
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // Bx

  // 0xC0 WSopc_EmergencyStopAdmin
  0,
  // 0xC1 WSopc_EmergencyStopAdminR
  0,
  // 0xC2 - 0xCD Reserved
  0,0,0,0,0,0,0,0,0,0,0,0,
  // 0xCE WSopc_Admin_Logout
  (void (*)(void *, struct web_client_t *))&WS_cts_Admin_Logout,
  // 0xCF WSopc_Admin_Login
  (void (*)(void *, struct web_client_t *))&WS_cts_Admin_Login,

  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // Dx
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // Ex
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // Fx
};

void WS_cts_SetSwitch(struct s_opc_SetSwitch * data, struct web_client_t * client){
  if(data->mssw){
    loggerf(INFO, "SetMSSw %2x %2x", data->module, data->id);
    if(Units[data->module] && U_MSSw(data->module, data->id)){
      loggerf(INFO, "throw msswitch %i:%i to state: \t%i->%i",
              data->module, data->id, U_MSSw(data->module, data->id)->state, data->state);
      lock_Algor_process();
      throw_msswitch(U_MSSw(data->module, data->id), data->state, 1);
      unlock_Algor_process();
    }
  }
  else{
    loggerf(INFO, "SetSw %2x %2x", data->module, data->id);
    if(Units[data->module] && U_Sw(data->module, data->id)){ //Check if switch exists
      loggerf(INFO, "throw switch %i:%i to state: \t%i->%i",
              data->module, data->id, U_Sw(data->module, data->id)->state, !U_Sw(data->module, data->id)->state);
      lock_Algor_process();
      throw_switch(U_Sw(data->module, data->id), data->state, 1);
      unlock_Algor_process();
    }
  }
}

void WS_cts_SetMultiSwitch(struct s_opc_SetMultiSwitch * data, struct web_client_t * client){
  loggerf(INFO, "Throw multiple switches\n");
  lock_Algor_process();
  throw_multiple_switches(data->nr, (char *)data->switches);
  unlock_Algor_process();
}

void WS_cts_SetEmergencyStop(void * data, struct web_client_t * client){
  WS_stc_EmergencyStop();
  Z21_TRACKPOWER_OFF;
}

void WS_cts_ClearEmergency(void * data, struct web_client_t * client){
  WS_stc_ClearEmergency();
  Z21_TRACKPOWER_ON;
}

void WS_cts_ChangeBroadcast(struct s_opc_ChangeBroadcast * data, struct web_client_t * client){
  if(data->flags & 0x10){ //Admin flag
    loggerf(WARNING, "Changing admin flag: NOT ALLOWED");
    return; //Not allowed to set admin flag
  }else if(data->flags != 0){
    client->type = data->flags;
    loggerf(DEBUG, "Changing flags");
  }
  loggerf(DEBUG,"Websocket:\t%02x - New flag for client %d\n",client->type, client->id);
  char msg[2] = {WSopc_ChangeBroadcast,(char)client->type};
  ws_send(client, msg, 2, 255);
}

//System Messages
void WS_cts_Enable_SubmoduleState(struct s_opc_enabledisableSubmoduleState * state, struct web_client_t * client){
  loggerf(INFO, "WSopc_EnableSubModule");
  if(state->flags & 0x80){ //Websocket
    SYS_set_state(&SYS->WebSocket.state, Module_Run);
  }
  else if(state->flags & 0x40){ //Z21
    Z21_start();
  }
  else if(state->flags & 0x20){ //UART
    UART_start();
  }
  else if(state->flags & 0x10){ //LayoutControl
    Algor_start();
  }
  else if(state->flags & 0x04){ //SimA
    SimA_start();
  }
  else if(state->flags & 0x02){ //SimB
    SimB_start();
  }
}
void WS_cts_Disable_SubmoduleState(struct s_opc_enabledisableSubmoduleState * state, struct web_client_t * client){
  loggerf(INFO, "WSopc_DisableSubModule");
  if(state->flags & 0x80){  //Websocket
    SYS_set_state(&SYS->WebSocket.state, Module_Init);
  }
  else if(state->flags & 0x40){ // Z21
    Z21_stop();
  }
  else if(state->flags & 0x20){ //UART
    UART_stop();
  }
  else if(state->flags & 0x10){
    Algor_stop();
  }
  else if(state->flags & 0x04){
    SYS_set_state(&SYS->SimA.state, Module_STOP);
  }
  else if(state->flags & 0x02){
    SYS_set_state(&SYS->SimB.state, Module_STOP);
  }
}

void WS_cts_SubmoduleState(void * d, struct web_client_t * client){
  WS_stc_SubmoduleState(client);
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

    Z21_get_train(train_link[msg->follow_id]);
  }
  else{
    loggerf(WARNING, "Failed link_train()\n");
    WS_clear_message((msg->message_id_H << 8) + msg->message_id_L, 0); //Failed
  }
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

void WS_cts_SetTrainFunction(struct s_opc_SetTrainFunction * m, struct web_client_t * client){
  loggerf(INFO, "WS_cts_SetTrainFunction");

  RailTrain * T = train_link[m->id];

  if(!T){
    loggerf(WARNING, "No Railtrain %i", m->id);
    return;
  }

  if(T->type == TRAIN_ENGINE_TYPE){
    Engines * E = (Engines *)T->p;

    if(E->function[m->function].button == TRAIN_FUNCTION_TOGGLE){
      m->type = 2;
    }

    Z21_setLocoFunction(E, m->function, m->type);
  }
  else{
    // TRAIN_TRAIN_TYPE
    loggerf(INFO, "TODO: implement set function for train");
  }
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

  T->changing_speed = RAILTRAIN_SPEED_T_DONE;

  T->speed = speed;
  T->target_speed = speed;
  T->dir   = m->dir;

  if(T->type == TRAIN_ENGINE_TYPE){
    Engines * E = (Engines *)T->p;
    E->dir = m->dir;
    engine_set_speed(E, speed);
    Z21_Set_Loco_Drive_Engine(E);
  }
  else{
    Trains * tmpT = (Trains *)T->p;
    tmpT->cur_speed = speed;
    tmpT->dir = m->dir;
    train_calc_speed(tmpT);
    Z21_Set_Loco_Drive_Train(tmpT);
  }

  // loggerf(INFO, "IMPLEMENT Z21");
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

void WS_cts_DCCEngineSpeed(struct s_opc_DCCEngineSpeed * m, struct web_client_t * client){
  Engines * E = engines[m->id];

  if(!E)
    return;

  uint16_t speed = (m->speed_high << 8) + m->speed_low;

  E->dir = m->dir;
  engine_set_speed(E, speed);
  Z21_Set_Loco_Drive_Engine(E);
}

void WS_cts_DCCEngineFunction(struct s_opc_DCCEngineFunction * m, struct web_client_t * client){
  loggerf(INFO, "WS_cts_DCCEngineFunction");

  Engines * E = engines[m->id];

  if(!E){
    loggerf(WARNING, "No Engine %i", m->id);
    return;
  }

  if(E->function[m->function].button == TRAIN_FUNCTION_TOGGLE){
    m->type = 2;
  }

  Z21_setLocoFunction(E, m->function, m->type);
}

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
  struct s_WS_Data * rdata = (struct s_WS_Data *)_calloc(1, struct s_WS_Data);

  rdata->opcode = WSopc_AddNewCartolib;
  rdata->data.opc_AddNewCartolib_res.nr = data->nr;

  char * name = (char *)_calloc(data->name_len + 1, char);
  char * filename = (char *)_calloc(data->name_len + 1, char);
  char * icon = (char *)_calloc(data->name_len + 8 + 3 + 20, char); //Destination file
  char * sicon = (char *)_calloc(40, char); //Source file

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

  char * dicon = (char *)_calloc(strlen(icon)+10, char);

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

void WS_cts_Edit_Car(struct s_opc_EditCarlib * data, struct web_client_t * client){
  loggerf(DEBUG, "WS_cts_Edit_Car");
  struct s_WS_Data * rdata = (struct s_WS_Data *)_calloc(1, struct s_WS_Data);

  rdata->opcode = WSopc_AddNewCartolib;
  rdata->data.opc_AddNewCartolib_res.nr = data->data.nr;

  uint16_t id = data->id_l + (data->id_h << 8);

  if(data->remove){
    if(cars[id]){
      Clear_Car(&cars[id]);

      rdata->data.opc_AddNewCartolib_res.response = 1;
    }
    else
      rdata->data.opc_AddNewCartolib_res.response = 0;

    ws_send(client, (char *)rdata, WSopc_AddNewCartolib_res_len, 0xff);
    return;
  }

  // Check if car exists
  if(!cars[id]){
    rdata->data.opc_AddNewCartolib_res.response = 0;
    ws_send(client, (char *)rdata, WSopc_AddNewCartolib_res_len, 0xff);
    return;
  }

  Cars * C = cars[id];

  C->name = (char *)_realloc(C->name, data->data.name_len + 1, char);
  memcpy(C->name, &data->data.strings, data->data.name_len);
  C->name[data->data.name_len] = 0;

  char * filename = (char *)_calloc(data->data.name_len +1, char);
  memcpy(filename, C->name, data->data.name_len);
  for (char* current_pos = NULL; (current_pos = strchr(filename, ' ')) != NULL; *current_pos = '_');
  for (char* current_pos = NULL; (current_pos = strchr(filename, '.')) != NULL; *current_pos = '-');
    loggerf(WARNING, "Filename: %s", filename);

  C->length = data->data.length;
  C->type = data->data.type;

  char * sicon = (char *)_calloc(40, char); //Source file

  uint16_t icon_time = (data->data.timing / 60) * 100 + (data->data.timing % 60);

  loggerf(ERROR, "%04i", icon_time);

  char * dicon = 0;

  if(icon_time < 3000){
    C->icon_path = (char *)_realloc(C->icon_path, data->data.name_len + 8 + 3 + 20, char); //Destination file
    if((data->data.filetype & 0b1) == 0){
      sprintf(C->icon_path, "%iC_%s.%s", data->data.nr, filename, "png");
      sprintf(sicon, "%s.%04i.%s", "web/tmp_icon", icon_time, "png");
    }
    else{
      sprintf(C->icon_path, "%iC_%s.%s", data->data.nr, filename, "jpg");
      sprintf(sicon, "%s.%04i.%s", "web/tmp_icon", icon_time, "jpg");
    }
    dicon = (char *)_calloc(strlen(C->icon_path)+20, char);
    sprintf(dicon, "%s%s", "web/trains_img/", C->icon_path);
    move_file(sicon, dicon);

    // Delete temp file
    remove(sicon);
  }
  else{
    sprintf(sicon, "web/trains_img/%s", C->icon_path);

    char filetype[5];
    sprintf(filetype, "%s", &C->icon_path[strlen(C->icon_path)-3]);

    sprintf(C->icon_path, "%iC_%s.%s", data->data.nr, filename, filetype);
    dicon = (char *)_calloc(strlen(C->icon_path)+20, char);
    sprintf(dicon, "%s%s", "web/trains_img/", C->icon_path);

    move_file(sicon, dicon);

    if(strcmp(sicon, dicon) != 0)
      remove(sicon);
  }

  rdata->data.opc_AddNewCartolib_res.response = 1;
  ws_send(client, (char *)rdata, WSopc_AddNewCartolib_res_len, 0xff);

  _free(dicon);
  _free(sicon);

  write_rolling_Configs();
  WS_stc_CarsLib(0);
}

void WS_cts_AddEnginetoLib(struct s_opc_AddNewEnginetolib * data, struct web_client_t * client){
  loggerf(DEBUG, "WS_cts_AddEnginetoLib");
  struct s_WS_Data * rdata = (struct s_WS_Data *)_calloc(1, struct s_WS_Data);

  rdata->opcode = WSopc_AddNewEnginetolib;

  if (DCC_train[data->DCC_ID]){
    loggerf(ERROR, "DCC %i allready in use", data->DCC_ID);
    rdata->data.opc_AddNewEnginetolib_res.response = 255;
    ws_send(client, (char *)rdata, WSopc_AddNewEnginetolib_res_len, 0xff);
    return;
  }

  char * name = (char *)_calloc(data->name_len + 1, char);
  char * steps = (char *)_calloc(data->steps, char);
  char * img = (char *)_calloc(data->name_len + 8 + 3 + 20, char);  //Destination file
  char * icon = (char *)_calloc(data->name_len + 8 + 3 + 20, char); //Destination file
  char * simg = (char *)_calloc(40, char); //Source file
  char * sicon = (char *)_calloc(40, char); //Source file
  char * filetype = (char *)_calloc(4, char);

  memcpy(name, &data->strings, data->name_len);
  memcpy(steps, &data->strings + data->name_len, data->steps * 3);

  char * filename = (char *)_calloc(data->name_len +1, char);
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

  Create_Engine(name, data->DCC_ID, img, icon, data->type, data->length, data->steps, (struct engine_speed_steps *)steps, data->functions);

  char * dimg = (char *)_calloc(strlen(img)+20, char);
  char * dicon = (char *)_calloc(strlen(icon)+20, char);
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
  WS_stc_EnginesLib(0);
}

void WS_cts_Edit_Engine(struct s_opc_EditEnginelib * msg, struct web_client_t * client){
  loggerf(DEBUG, "WS_cts_Edit_Engine");

  struct s_WS_Data * rdata = (struct s_WS_Data *)_calloc(1, struct s_WS_Data);

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

    // Send succes response
    rdata->data.opc_AddNewEnginetolib_res.response = 1;
    ws_send(client, (char *)rdata, WSopc_AddNewEnginetolib_res_len, 0xff);
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

    // Copy functions
    memcpy(&E->function, data->functions, 29);

    // Copy name
    E->name = (char *)_realloc(E->name, data->name_len + 1, char);
    memcpy(E->name, &data->strings, data->name_len);
    E->name[data->name_len] = 0;

    char * filename = (char *)_calloc(data->name_len +1, char);
    memcpy(filename, E->name, data->name_len);
    for (char* current_pos = NULL; (current_pos = strchr(filename, ' ')) != NULL; *current_pos = '_');
    for (char* current_pos = NULL; (current_pos = strchr(filename, '.')) != NULL; *current_pos = '-');
      loggerf(WARNING, "Filename: %s", filename);

    // Copy speedsteps
    E->steps_len = data->steps;
    E->steps = (struct engine_speed_steps *)_realloc(E->steps, data->steps, struct engine_speed_steps);
    memcpy(E->steps, &data->strings + data->name_len, data->steps * 3);

    E->length = data->length;
    E->type = data->type;


    // Copy image/icon
    char * simg = (char *)_calloc(40, char); //Source file
    char * sicon = (char *)_calloc(40, char); //Source file
    char * filetype = (char *)_calloc(4, char);

    uint16_t image_time = data->timing[0] + ((data->timing[1] & 0xf0) << 4);
    image_time = (image_time / 60) * 100 + (image_time % 60);
    uint16_t icon_time = (data->timing[1] & 0x0f) + (data->timing[2] << 4);
    icon_time = (icon_time / 60) * 100 + (icon_time % 60);

    loggerf(ERROR, "%04i - %04i", image_time, icon_time);

    char * dimg = 0;
    char * dicon = 0;

    remove(E->img_path); //Remove original

    E->img_path = (char *)_realloc(E->img_path, data->name_len + 8 + 3 + 20, char);  //Destination file
    if((data->flags & 0b10) == 0){
      sprintf(E->img_path, "%i_%s_im.%s", data->DCC_ID, filename, "png");
      sprintf(simg, "%s.%04i.%s", "web/tmp_img", image_time, "png");
    }
    else{
      sprintf(E->img_path, "%i_%s_im.%s", data->DCC_ID, filename, "jpg");
      sprintf(simg, "%s.%04i.%s", "web/tmp_img", image_time, "jpg");
    }

    dimg = (char *)_calloc(strlen(E->img_path)+20, char);
    sprintf(dimg, "%s%s", "web/trains_img/", E->img_path);
    loggerf(INFO, "remove and move file %s -> %s", simg, dimg);
    move_file(simg, dimg);

    // Delete temp file
    remove(simg);

    remove(E->icon_path); //Remove original

    E->icon_path = (char *)_realloc(E->icon_path, data->name_len + 8 + 3 + 20, char); //Destination file
    if((data->flags & 0b1) == 0){
      sprintf(E->icon_path, "%i_%s_ic.%s", data->DCC_ID, filename, "png");
      sprintf(sicon, "%s.%04i.%s", "web/tmp_icon", icon_time, "png");
    }
    else{
      sprintf(E->icon_path, "%i_%s_ic.%s", data->DCC_ID, filename, "jpg");
      sprintf(sicon, "%s.%04i.%s", "web/tmp_icon", icon_time, "jpg");
    }
    dicon = (char *)_calloc(strlen(E->icon_path)+20, char);
    sprintf(dicon, "%s%s", "web/trains_img/", E->icon_path);
    loggerf(INFO, "remove and move file %s -> %s", simg, dimg);
    move_file(sicon, dicon);

    // Delete temp file
    remove(sicon);

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
  WS_stc_EnginesLib(0);
}

void WS_cts_AddTraintoLib(struct s_opc_AddNewTraintolib * data, struct web_client_t * client){
  loggerf(DEBUG, "WS_cts_AddTraintoLib");
  struct s_WS_Data * rdata = (struct s_WS_Data *)_calloc(1, struct s_WS_Data);

  rdata->opcode = WSopc_AddNewTraintolib;

  char * name = (char *)_calloc(data->name_len + 1, char);
  char * comps = (char *)_calloc(data->nr_stock * 3, char);

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
  WS_stc_TrainsLib(0);
}

void WS_cts_Edit_Train(struct s_opc_EditTrainlib * data, struct web_client_t * client){
  loggerf(ERROR, "WS_cts_Edit_Train ");
  struct s_WS_Data * rdata = (struct s_WS_Data *)_calloc(1, struct s_WS_Data);

  rdata->opcode = WSopc_AddNewTraintolib;

  uint16_t id = data->id_l + (data->id_h << 8);
  if(data->remove){
    if(trains[id]){
      Clear_Train(&trains[id]);
      rdata->data.opc_AddNewTraintolib_res.response = 1;
    }
    else
      rdata->data.opc_AddNewTraintolib_res.response = 0;
    ws_send(client, (char *)rdata, WSopc_AddNewTraintolib_res_len, 0xff);
    return;
  }

  Trains * T = trains[id];

  //Copy name
  T->name = (char *)_realloc(T->name, data->data.name_len + 1, char);
  memcpy(T->name, &data->data.strings, data->data.name_len);
  T->name[data->data.name_len] = 0;

  // Copy traincomp
  T->composition = (struct train_comp *)_realloc(T->composition, data->data.nr_stock, struct train_comp);
  T->nr_stock = data->data.nr_stock;

  struct train_comp_ws * cdata = (struct train_comp_ws *)((char *)&data->data.strings + data->data.name_len);
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

  write_rolling_Configs();
  WS_stc_TrainsLib(0);
}

void WS_cts_TrackLayoutRawData(struct s_opc_TrackLayoutRawData * data, struct web_client_t * client){
  WS_stc_TrackLayoutRawData(data->module, client);
}

void WS_cts_TrackLayoutUpdate(struct s_opc_TrackLayoutUpdate * data, struct web_client_t * client){
  loggerf(CRITICAL, "IMPLEMENT WS_cts_TrackLayoutUpdate");


  // Respond with failure
  struct s_WS_Data * rdata = (struct s_WS_Data *)_calloc(1, struct s_WS_Data);

  rdata->opcode = WSopc_TrackLayoutUpdateRaw;
  rdata->data.opc_AddNewTraintolib_res.response = 0;
  ws_send(client, (char *)rdata, WSopc_AddNewTraintolib_res_len, 0xff);
}

void WS_cts_Admin_Login(struct s_opc_AdminLogin * data, struct web_client_t * client){
  if(strcmp((char *)data->password,WS_password) == 0){
    loggerf(INFO, "SUCCESSFULL LOGIN");
    //0xc3,0xbf,0x35,0x66,0x34,0x64,0x63,0x63,0x33,0x62,0x35,0x61,0x61,0x37,0x36,0x35,0x64,0x36,0x31,0x64,0x38,0x33,0x32,0x37,0x64,0x65,0x62,0x38,0x38,0x32,0x63,0x66,0x39,0x39
    client->type |= 0x10;

    loggerf(INFO, "Change client flags to %x", client->type);

    char msg[2] = {WSopc_ChangeBroadcast, (char)client->type};
    ws_send(client, msg, 2, 255);
  }else{
    loggerf(INFO, "FAILED LOGIN!! %d", strcmp((char *)data->password,WS_password));
    loggerf(INFO, "%s", data->password);
    loggerf(INFO, "%s", WS_password);
  }
}

void WS_cts_Admin_Logout(void * data, struct web_client_t * client){
  client->type &= ~0x10;

  char msg[2] = {WSopc_ChangeBroadcast, (char)client->type};
  ws_send(client, msg, 2, 255);
}
