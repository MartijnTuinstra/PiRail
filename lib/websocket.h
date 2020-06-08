#ifndef _INCLUDE_WEBSOCKET_H
#define _INCLUDE_WEBSOCKET_H

struct web_client_t;

#include "websocket_control.h"
#include "websocket_msg_structure.h"

extern pthread_mutex_t m_websocket_send;

struct websocket_client_thread_args;

int websocket_get_msg(int fd_client, char outbuf[], int * L);

void websocket_create_msg(char * input, int length_in, char * output, int * length_out);

void ws_send(struct web_client_t * client, char * data, int length, int flag);

void ws_send_all(char * data,int length,int flag);

int websocket_parse(uint8_t data[1024], struct web_client_t * client);

typedef void (*websocket_cts_func)(void *, struct web_client_t *);
extern websocket_cts_func websocket_cts[256];

#define WEBSOCKET_CLIENT_TIMEOUT 5

//Broadcast Flag
#define WS_Flag_Trains   0x01
#define WS_Flag_Track    0x02
#define WS_Flag_Switches 0x04
#define WS_Flag_Messages 0x08
#define WS_Flag_Admin    0x10
#define WS_Flag_20       0x20
#define WS_Flag_40       0x40
#define WS_Flag_80       0x80


//Opcodes
//System
#define WSopc_ClearTrack          0x80
#define WSopc_ReloadTrack         0x81
#define WSopc_Track_Scan_Progress 0x82
#define WSopc_Track_Layout_Update 0x83
#define WSopc_Track_Layout_Config 0x84
#define WSopc_Track_Info          0x86
#define WSopc_Z21_Settings        0x87
#define WSopc_Reset_Switches      0x8C
#define WSopc_TrainsToDepot       0x8F

#define WSopc_EnableSubModule     0x90
#define WSopc_DisableSubModule    0x91
#define WSopc_SubModuleState      0x92
#define WSopc_RestartApplication  0x9F
//Admin
#define WSopc_EmergencyStopAdmin  0xC0
#define WSopc_EmergencyStopAdminR 0xC1
#define WSopc_Admin_Logout        0xCE
#define WSopc_Admin_Login         0xCF

//Trains
#define WSopc_LinkTrain          0x41
#define WSopc_TrainSpeed         0x42
#define WSopc_TrainFunction      0x43
#define WSopc_TrainControl       0x44
#define WSopc_UpdateTrain        0x45
#define WSopc_TrainAddRoute      0x46
#define WSopc_DCCEngineUpdate    0x4A
#define WSopc_DCCEngineSpeed     0x4B
#define WSopc_DCCEngineFunction  0x4C
#define WSopc_TrainSubscribe     0x4F

#define WSopc_AddNewEnginetolib  0x50
#define WSopc_EditEnginelib      0x51
#define WSopc_EnginesLibrary     0x52

#define WSopc_AddNewCartolib     0x53
#define WSopc_EditCarlib         0x54
#define WSopc_CarsLibrary        0x55

#define WSopc_AddNewTraintolib   0x56
#define WSopc_EditTrainlib       0x57
#define WSopc_TrainsLibrary      0x58

#define WSopc_TrainCategories    0x5A

//Track and switches
#define WSopc_SetSwitch             0x20
#define WSopc_SetMultiSwitch        0x21
#define WSopc_SetSwitchReserved     0x22
#define WSopc_ChangeSwitchReserved  0x23
#define WSopc_SetSwitchRoute        0x25
#define WSopc_BroadTrack            0x26
#define WSopc_BroadSwitch           0x27

#define WSopc_TrackLayoutOnlyRawData 0x30
#define WSopc_TrackLayoutRawData     0x31
#define WSopc_TrackLayoutUpdateRaw   0x33
#define WSopc_StationLibrary         0x36

//Client / General
#define WSopc_EmergencyStop      0x10
#define WSopc_ShortCircuitStop   0x11
#define WSopc_ClearEmergency     0x12
#define WSopc_NewMessage         0x13
#define WSopc_UpdateMessage      0x14
#define WSopc_ClearMessage       0x15
#define WSopc_ChangeBroadcast    0x16
#define WSopc_Service_State      0x17
#define WSopc_Canvas_Data        0x18
#endif