#ifndef _INCLUDE_WEBSOCKET_CTS_H
#define _INCLUDE_WEBSOCKET_CTS_H
#define ACTIVATE 0
#define RELEASE  1

#include <stdint.h>
#include "websocket.h"
#include "websocket_control.h"
#include "train.h"


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
struct s_opc_AddNewEnginetolib;
struct s_opc_EditEnginelib;
struct s_opc_AddNewTraintolib;


void WS_cts_LinkTrain(struct s_opc_LinkTrain * msg, struct web_client_t * client);
void WS_cts_SetTrainSpeed(struct s_opc_SetTrainSpeed * m, struct web_client_t * client);

// void WS_TrainData(char data[14]);

void WS_cts_TrainControl(struct s_opc_TrainControl * m, struct web_client_t * client);
void WS_cts_TrainSubscribe(struct s_opc_SubscribeTrain * m, struct web_client_t * client);
void WS_cts_TrainRoute(struct s_opc_TrainRoute * data, struct web_client_t * client);

void WS_cts_AddCartoLib(struct s_opc_AddNewCartolib * data, struct web_client_t * client);
void WS_cts_Edit_Car(Cars * C, struct s_opc_AddNewCartolib * data, struct web_client_t * client);

void WS_cts_AddEnginetoLib(struct s_opc_AddNewEnginetolib * data, struct web_client_t * client);
void WS_cts_Edit_Engine(struct s_opc_EditEnginelib * msg, struct web_client_t * client);

void WS_cts_AddTraintoLib(struct s_opc_AddNewTraintolib * data, struct web_client_t * client);
void WS_cts_Edit_Train(Trains * T, struct s_opc_AddNewTraintolib * data, struct web_client_t * client);

#endif
