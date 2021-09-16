#ifndef _INCLUDE_WEBSOCKET_STC_H
#define _INCLUDE_WEBSOCKET_STC_H
#define ACTIVATE 0
#define RELEASE  1

#include <stdint.h>
#include "websocket/server.h"
#include "websocket/client.h"

class Train;
class Car;
class Engine;

struct WS_Message {
  uint16_t type;
  char data[16];
  char data_length;
};

//System Messages
uint16_t WS_stc_ScanStatus(uint16_t msgID, uint16_t x, uint16_t y);
void WS_stc_Partial_Layout(uint8_t M_A);
void WS_stc_Track_Layout(Websocket::Client * client);

void WS_stc_Z21_info(Websocket::Client * client);
void WS_stc_Z21_IP(Websocket::Client * client);

void WS_stc_SubmoduleState(Websocket::Client * client);

//Admin Messages

//Train Messages
struct s_opc_LinkTrain;

void WS_stc_LinkTrain(struct s_opc_LinkTrain * msg);
// void WS_TrainData(char data[14]);
void WS_stc_UpdateTrain(Train * T);
void WS_stc_UpdateTrain(Train * T, Websocket::Client * client); // send to all except client
void WS_stc_TrainRouteUpdate(Train * T);
void WS_stc_DCCEngineUpdate(Engine * E);
void WS_stc_EnginesLib(Websocket::Client * client);
void WS_stc_CarsLib(Websocket::Client * client);
void WS_stc_TrainSetsLib(Websocket::Client * client);
void WS_stc_TrainCategories(Websocket::Client * client);

void WS_stc_NewTrain(Train * T,char M,char B);
void WS_stc_TrainSplit(Train * T, char M1,char B1,char M2,char B2);

//Track Messages
void WS_stc_trackUpdate(Websocket::Client * client);
void WS_stc_SwitchesUpdate(Websocket::Client * client);
void WS_stc_NewClient_track_Switch_Update(Websocket::Client * client);

void WS_stc_Track_Layout_Load(Websocket::Client * client);

void WS_stc_Track_LayoutDataOnly(int unit, Websocket::Client * client);
void WS_stc_TrackLayoutRawData(int unit, Websocket::Client * client);

void WS_stc_StationLib(Websocket::Client * client);

void WS_stc_reset_switches(Websocket::Client * client);

//General Messages
void WS_stc_EmergencyStop();
void WS_stc_ShortCircuit();
void WS_stc_ClearEmergency();

void WS_init_Message_List();
uint16_t WS_init_Message(char type);
void WS_add_Message(uint16_t ID, char length,char data[16]);
void WS_send_open_Messages(Websocket::Client * client);
void WS_clear_message(uint16_t ID, char ret_code);
#endif
