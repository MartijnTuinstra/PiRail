#ifndef _INCLUDE_WEBSOCKET_CTS_H
#define _INCLUDE_WEBSOCKET_CTS_H
#define ACTIVATE 0
#define RELEASE  1

#include <stdint.h>
#include "websocket/server.h"
#include "websocket/client.h"
#include "websocket/message_structure.h"
#include "train.h"

void WS_cts_SetSwitch(struct s_opc_SetSwitch * data, Websocket::Client * client);
void WS_cts_SetMultiSwitch(struct s_opc_SetMultiSwitch * data, Websocket::Client * client);


void WS_cts_SetEmergencyStop(void * data, Websocket::Client * client);
void WS_cts_ClearEmergency(void * data, Websocket::Client * client);

void WS_cts_ChangeBroadcast(struct s_opc_ChangeBroadcast * data, Websocket::Client * client);

void WS_cts_SubmoduleState(void * d, Websocket::Client * client);

//System Messages
void WS_cts_Enable_SubmoduleState(struct s_opc_enabledisableSubmoduleState * state, Websocket::Client * client);
void WS_cts_Disable_SubmoduleState(struct s_opc_enabledisableSubmoduleState * state, Websocket::Client * client);

//Admin Messages

//Train Messages
void WS_cts_LinkTrain(struct s_opc_LinkTrain * msg, Websocket::Client * client);
void WS_cts_SetTrainSpeed(struct s_opc_SetTrainSpeed * m, Websocket::Client * client);
void WS_cts_SetTrainFunction(struct s_opc_SetTrainFunction * m, Websocket::Client * client);

// void WS_TrainData(char data[14]);

void WS_cts_TrainControl(struct s_opc_TrainControl * m, Websocket::Client * client);
void WS_cts_TrainSubscribe(struct s_opc_SubscribeTrain * m, Websocket::Client * client);
void WS_cts_TrainRoute(struct s_opc_TrainRoute * data, Websocket::Client * client);

void WS_cts_DCCEngineSpeed(struct s_opc_DCCEngineSpeed * data, Websocket::Client * client);
void WS_cts_DCCEngineFunction(struct s_opc_DCCEngineFunction * data, Websocket::Client * client);

void WS_cts_AddCartoLib(struct s_opc_AddNewCartolib * data, Websocket::Client * client);
void WS_cts_Edit_Car(struct s_opc_EditCarlib * data, Websocket::Client * client);

void WS_cts_AddEnginetoLib(struct s_opc_AddNewEnginetolib * data, Websocket::Client * client);
void WS_cts_Edit_Engine(struct s_opc_EditEnginelib * msg, Websocket::Client * client);

void WS_cts_AddTraintoLib(struct s_opc_AddNewTraintolib * data, Websocket::Client * client);
void WS_cts_Edit_Train(struct s_opc_EditTrainlib * data, Websocket::Client * client);

void WS_cts_Track_Layout_Load(struct s_opc_Track_Layout_Load * data, Websocket::Client * client);

void WS_cts_TrackLayoutRawData(struct s_opc_TrackLayoutRawData * data, Websocket::Client * client);
void WS_cts_TrackLayoutUpdate(struct s_opc_TrackLayoutUpdate * data, Websocket::Client * client);

void WS_cts_Admin_Login(struct s_opc_AdminLogin * data, Websocket::Client * client);
void WS_cts_Admin_Logout(void * data, Websocket::Client * client);
#endif
