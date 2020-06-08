#ifndef _INCLUDE_WEBSOCKET_STC_H
#define _INCLUDE_WEBSOCKET_STC_H
#define ACTIVATE 0
#define RELEASE  1

#include <stdint.h>
#include "websocket.h"
#include "websocket_control.h"
#include "train.h"

struct WS_Message {
  uint16_t type;
  char data[16];
  char data_length;
};

//System Messages
void WS_stc_Partial_Layout(uint8_t M_A);
void WS_stc_Track_Layout(struct web_client_t * client);

void WS_stc_Z21_info(struct web_client_t * client);
void WS_stc_Z21_IP(struct web_client_t * client);

void WS_stc_SubmoduleState(struct web_client_t * client);

//Admin Messages

//Train Messages
struct s_opc_LinkTrain;

void WS_stc_LinkTrain(struct s_opc_LinkTrain * msg);
// void WS_TrainData(char data[14]);
void WS_stc_UpdateTrain(RailTrain * T);
void WS_stc_DCCEngineUpdate(Engines * E);
void WS_stc_EnginesLib(struct web_client_t * client);
void WS_stc_CarsLib(struct web_client_t * client);
void WS_stc_TrainsLib(struct web_client_t * client);
void WS_stc_TrainCategories(struct web_client_t * client);

void WS_stc_NewTrain(RailTrain * T,char M,char B);
void WS_stc_TrainSplit(RailTrain * T, char M1,char B1,char M2,char B2);

//Track Messages
void WS_stc_trackUpdate(struct web_client_t * client);
void WS_stc_SwitchesUpdate(struct web_client_t * client);
void WS_stc_NewClient_track_Switch_Update(struct web_client_t * client);

void WS_stc_Track_LayoutDataOnly(int unit, struct web_client_t * client);
void WS_stc_TrackLayoutRawData(int unit, struct web_client_t * client);

void WS_stc_StationLib(struct web_client_t * client);

void WS_stc_reset_switches(struct web_client_t * client);

//General Messages
void WS_stc_EmergencyStop();
void WS_stc_ShortCircuit();
void WS_stc_ClearEmergency();

void WS_init_Message_List();
char WS_init_Message(char type);
void WS_add_Message(uint16_t ID, char length,char data[16]);
void WS_send_open_Messages(struct web_client_t * client);
void WS_clear_message(uint16_t ID, char ret_code);
#endif
