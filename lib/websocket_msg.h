#ifndef _INCLUDE_WEBSOCKET_MSG_H
#define _INCLUDE_WEBSOCKET_MSG_H
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

// struct WS_Message MessageList[0x1FFF];
// char MessageCounter = 0;


//System Messages
void WS_Partial_Layout(uint8_t M_A,uint8_t M_B);
void WS_Track_Layout(struct web_client_t * client);

void WS_stc_Z21_info(struct web_client_t * client);
void WS_stc_Z21_IP(struct web_client_t * client);

void WS_cts_Enable_Disable_SubmoduleState(uint8_t opcode, uint8_t flags);
void WS_stc_SubmoduleState();

//Admin Messages

//Train Messages
struct s_opc_LinkTrain;
struct s_opc_SetTrainSpeed;
struct s_opc_TrainControl;
struct s_opc_SubscribeTrain;
struct s_opc_TrainRoute;
struct s_opc_AddNewCartolib;
struct s_opc_AddNewEnginetolib;
struct s_opc_EditEnginelib;
struct s_opc_AddNewTraintolib;


void WS_cts_LinkTrain(struct s_opc_LinkTrain * msg, struct web_client_t * client);
void WS_stc_LinkTrain(struct s_opc_LinkTrain * msg);

void WS_cts_SetTrainSpeed(struct s_opc_SetTrainSpeed * m, struct web_client_t * client);

// void WS_TrainData(char data[14]);

void WS_cts_TrainControl(struct s_opc_TrainControl * m, struct web_client_t * client);

void WS_stc_UpdateTrain(RailTrain * T);
void WS_cts_TrainSubscribe(struct s_opc_SubscribeTrain * m, struct web_client_t * client);

void WS_cts_TrainRoute(struct s_opc_TrainRoute * data, struct web_client_t * client);

void WS_EnginesLib(struct web_client_t * client);
void WS_CarsLib(struct web_client_t * client);
void WS_TrainsLib(struct web_client_t * client);
void WS_stc_TrainCategories(struct web_client_t * client);

void WS_NewTrain(RailTrain * T,char M,char B);
void WS_TrainSplit(RailTrain * T, char M1,char B1,char M2,char B2);

void WS_cts_AddCartoLib(struct s_opc_AddNewCartolib * data, struct web_client_t * client);
void WS_cts_Edit_Car(Cars * C, struct s_opc_AddNewCartolib * data, struct web_client_t * client);

void WS_cts_AddEnginetoLib(struct s_opc_AddNewEnginetolib * data, struct web_client_t * client);
void WS_cts_Edit_Engine(struct s_opc_EditEnginelib * msg, struct web_client_t * client);

void WS_cts_AddTraintoLib(struct s_opc_AddNewTraintolib * data, struct web_client_t * client);
void WS_cts_Edit_Train(Trains * T, struct s_opc_AddNewTraintolib * data, struct web_client_t * client);

//Track Messages
void WS_trackUpdate(struct web_client_t * client);
void WS_SwitchesUpdate(struct web_client_t * client);
void WS_NewClient_track_Switch_Update(struct web_client_t * client);

void WS_Track_LayoutDataOnly(int unit, struct web_client_t * client);
void WS_stc_TrackLayoutRawData(int unit, struct web_client_t * client);

void WS_stc_StationLib(struct web_client_t * client);

void WS_reset_switches(struct web_client_t * client);

//General Messages
void WS_EmergencyStop();
void WS_ShortCircuit();
void WS_ClearEmergency();

void WS_init_Message_List();
char WS_init_Message(char type);
void WS_add_Message(uint16_t ID, char length,char data[16]);
void WS_send_open_Messages(struct web_client_t * client);
void WS_clear_message(uint16_t ID, char ret_code);
#endif
