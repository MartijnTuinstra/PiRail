#ifndef _INCLUDE_WEBSOCKET_H
  #define _INCLUDE_WEBSOCKET_H

  #include "websocket_control.h"


  struct __attribute__((__packed__)) s_opc_AddNewCartolib {
    uint16_t nr;
    uint16_t max_speed;
    uint16_t length;
    uint8_t type;
    uint8_t filetype;
    uint8_t name_len;
    char strings;
  };

  #define WSopc_AddNewCartolib_res_len 4
  struct __attribute__((__packed__)) s_opc_AddNewCartolib_res {
    uint16_t nr;
    uint8_t response;
  };

  struct __attribute__((__packed__)) s_opc_EditCarlib {
    uint8_t id_h:7;
    uint8_t remove:1;
    uint8_t id_l;
    struct s_opc_AddNewCartolib data;
  };



  struct __attribute__((__packed__)) s_opc_AddNewEnginetolib {
    uint16_t DCC_ID;
    uint16_t length;
    uint8_t fl;
    uint8_t name_len;
    uint8_t filetype;
    uint8_t steps;
    char strings;
  };

  #define WSopc_AddNewEnginetolib_res_len 4
  struct __attribute__((__packed__)) s_opc_AddNewEnginetolib_res {
    uint16_t DCC_ID;
    uint8_t response;
  };

  struct __attribute__((__packed__)) s_opc_EditEnginelib {
    uint8_t id_h:7;
    uint8_t remove:1;
    uint8_t id_l;
    struct s_opc_AddNewEnginetolib data;
  };



  struct __attribute__((__packed__)) s_opc_AddNewTraintolib {
    uint8_t name_len;
    uint8_t nr_stock:7;
    uint8_t save:1;
    uint8_t catagory;
    char strings;
  };

  #define WSopc_AddNewTraintolib_res_len 2
  struct __attribute__((__packed__)) s_opc_AddNewTraintolib_res {
    uint8_t response;
  };

  struct __attribute__((__packed__)) s_opc_EditTrainlib {
    uint8_t id_h:7;
    uint8_t remove:1;
    uint8_t id_l;
    struct s_opc_AddNewTraintolib data;
  };






  struct s_WS_Data {
    uint8_t opcode;
    union {
      struct s_opc_AddNewCartolib        opc_AddNewCartolib;
      struct s_opc_AddNewCartolib_res    opc_AddNewCartolib_res;
      struct s_opc_EditCarlib            opc_EditCarlib;

      struct s_opc_AddNewEnginetolib     opc_AddNewEnginetolib;
      struct s_opc_AddNewEnginetolib_res opc_AddNewEnginetolib_res;
      struct s_opc_EditEnginelib         opc_EditEnginelib;

      struct s_opc_AddNewTraintolib      opc_AddNewTraintolib;
      struct s_opc_AddNewTraintolib_res  opc_AddNewTraintolib_res;
      struct s_opc_EditTrainlib          opc_EditTrainlib;
    } data;
  };

  struct websocket_client_thread_args;

  int websocket_get_msg(int fd_client, char outbuf[], int * L);

  void ws_send(int fd_client, char data[],int length,int flag);

  void ws_send_all(char data[],int length,int flag);

  int websocket_decode(uint8_t data[1024], struct web_client_t * client);

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

  #define WSopc_AddNewEnginetolib  0x50
  #define WSopc_EditEnginelib      0x51
  #define WSopc_EnginesLibrary     0x52
  #define WSopc_AddNewCartolib     0x53
  #define WSopc_EditCarlib         0x54
  #define WSopc_CarsLibrary        0x55
  #define WSopc_AddNewTraintolib   0x56
  #define WSopc_EditTrainlib       0x57
  #define WSopc_TrainsLibrary      0x58

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