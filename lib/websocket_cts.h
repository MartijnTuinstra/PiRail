#ifndef _INCLUDE_WEBSOCKET_CTS_H
#define _INCLUDE_WEBSOCKET_CTS_H
#define ACTIVATE 0
#define RELEASE  1

#include <stdint.h>
#include "websocket.h"
#include "websocket_control.h"
#include "train.h"

struct s_opc_SetSwitch;
struct s_opc_SetMultiSwitch;

void WS_cts_SetSwitch(struct s_opc_SetSwitch * data, struct web_client_t * client);
void WS_cts_SetMultiSwitch(struct s_opc_SetMultiSwitch * data, struct web_client_t * client);


void WS_cts_SetEmergencyStop(void * data, struct web_client_t * client);
void WS_cts_ClearEmergency(void * data, struct web_client_t * client);

struct s_opc_ChangeBroadcast;
void WS_cts_ChangeBroadcast(struct s_opc_ChangeBroadcast * data, struct web_client_t * client);

void WS_cts_SubmoduleState(void * d, struct web_client_t * client);

//System Messages
struct s_opc_enabledisableSubmoduleState;
void WS_cts_Enable_SubmoduleState(struct s_opc_enabledisableSubmoduleState * state, struct web_client_t * client);
void WS_cts_Disable_SubmoduleState(struct s_opc_enabledisableSubmoduleState * state, struct web_client_t * client);

//Admin Messages

//Train Messages
struct s_opc_LinkTrain;
struct s_opc_SetTrainSpeed;
struct s_opc_TrainControl;
struct s_opc_SubscribeTrain;
struct s_opc_TrainRoute;
struct s_opc_AddNewCartolib;
struct s_opc_EditCarlib;
struct s_opc_AddNewEnginetolib;
struct s_opc_EditEnginelib;
struct s_opc_AddNewTraintolib;
struct s_opc_EditTrainlib;


void WS_cts_LinkTrain(struct s_opc_LinkTrain * msg, struct web_client_t * client);
void WS_cts_SetTrainSpeed(struct s_opc_SetTrainSpeed * m, struct web_client_t * client);

// void WS_TrainData(char data[14]);

void WS_cts_TrainControl(struct s_opc_TrainControl * m, struct web_client_t * client);
void WS_cts_TrainSubscribe(struct s_opc_SubscribeTrain * m, struct web_client_t * client);
void WS_cts_TrainRoute(struct s_opc_TrainRoute * data, struct web_client_t * client);

void WS_cts_AddCartoLib(struct s_opc_AddNewCartolib * data, struct web_client_t * client);
void WS_cts_Edit_Car(struct s_opc_EditCarlib * data, struct web_client_t * client);

void WS_cts_AddEnginetoLib(struct s_opc_AddNewEnginetolib * data, struct web_client_t * client);
void WS_cts_Edit_Engine(struct s_opc_EditEnginelib * msg, struct web_client_t * client);

void WS_cts_AddTraintoLib(struct s_opc_AddNewTraintolib * data, struct web_client_t * client);
void WS_cts_Edit_Train(struct s_opc_EditTrainlib * data, struct web_client_t * client);

struct s_opc_TrackLayoutRawData;
struct s_opc_TrackLayoutUpdate;

void WS_cts_TrackLayoutRawData(struct s_opc_TrackLayoutRawData * data, struct web_client_t * client);
void WS_cts_TrackLayoutUpdate(struct s_opc_TrackLayoutUpdate * data, struct web_client_t * client);

struct s_opc_AdminLogin;

void WS_cts_Admin_Login(struct s_opc_AdminLogin * data, struct web_client_t * client);
void WS_cts_Admin_Logout(void * data, struct web_client_t * client);
#endif
