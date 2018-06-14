
#ifndef _INCLUDE_WEBSOCKET_H
  #define _INCLUDE_WEBSOCKET_H

  #define WEBSOCKET_PORT 9000

  struct web_client_t{
    int fd_client;
    int client_type; /*Flags for client type
                      255 = All messages

                      1  = Trains
                      2  = Track
                      4  = Switches
                      8  = Messages
                      16 = Admin
                      32 = 
                      64 =
                      128=*/
    int state;        /* If this client is active
                      0 = free
                      1 = in use
                      2 = stopping / ready to join thread*/
  };

  struct client_thread_args{
    int fd_client;
    int thread_id;
  };

  int websocket_connect(struct web_client_t * C);

  int recv_packet(int fd_client, char outbuf[], int * L);

  int send_packet(int fd_client, char data[],int length,int flag);

  void send_all(char data[],int length,int flag);

  int recv_packet_procces(char data[1024],struct client_thread_args * client_data);

  void * websocket_client(void * thread_data);

  void *clear_clients();

  void * web_server();

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
  #define WSopc_Track_Scan         0x82
  #define WSopc_Track_PUp_Layout   0x83
  #define WSopc_Track_Info         0x84

  #define WSopc_LinkTrain          0x41
  #define WSopc_TrainSpeed         0x42
  #define WSopc_TrainFunction      0x43
  #define WSopc_TrainOperation     0x44
  #define WSopc_Z21TrainData       0x45
  #define WSopc_TrainAddRoute      0x46

  #define WSopc_AddNewCartolib     0x50
  #define WSopc_CarsLibrary         0x51
  #define WSopc_AddNewEnginetolib  0x52
  #define WSopc_EnginesLibrary      0x53
  #define WSopc_AddNewTraintolib   0x54
  #define WSopc_TrainsLibrary       0x55

  #define WSopc_SetSwitch          0x20
  #define WSopc_SetMultiSwitch     0x21
  #define WSopc_SetSwitchReserved  0x22
  #define WSopc_ChangeSwitchReserved  0x23
  #define WSopc_SetSwitchRoute     0x25
  #define WSopc_BroadTrack         0x26
  #define WSopc_BroadSwitch        0x27
  #define WSopc_Track_Layout       0x30
  #define WSopc_StationLibrary     0x31

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