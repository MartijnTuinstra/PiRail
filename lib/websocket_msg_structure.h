#ifndef _INCLUDE_WEBSOCKET_MSG_STRUCTURE_H
#define _INCLUDE_WEBSOCKET_MSG_STRUCTURE_H

#define packedstruct struct __attribute__((__packed__))

// 0x16
packedstruct s_opc_ChangeBroadcast {
  uint8_t flags;
};

// 0x20
packedstruct s_opc_SetSwitch {
  uint8_t module;
  uint8_t mssw:1;
  uint8_t id:7;
  uint8_t state;
};

// 0x21
packedstruct s_opc_SetMultiSwitch {
  uint8_t nr;
  struct s_opc_SetSwitch switches[1];
};

// 0x31
packedstruct s_opc_TrackLayoutRawData {
  uint8_t module;
};

// 0x33
packedstruct s_opc_TrackLayoutUpdate {
  uint8_t module;
  uint16_t length;
  uint8_t data[1];
};

// 0x41
packedstruct s_opc_LinkTrain {
  uint8_t follow_id;
  uint8_t real_id;
  uint8_t message_id_H:7;
  uint8_t type:1;
  uint8_t message_id_L;
};

// 0x42
packedstruct s_opc_SetTrainSpeed {
  uint8_t follow_id;
  uint8_t empty1:3;
  uint8_t dir:1;
  uint8_t speed_high:4;
  uint8_t speed_low;
};

// 0x43
packedstruct s_opc_SetTrainFunction {
  uint8_t id;
  uint8_t type:2;
  uint8_t function:6;
};

// 0x44
packedstruct s_opc_TrainControl {
  uint8_t follow_id;
  uint8_t control;
};

// 0x45
packedstruct s_opc_UpdateTrain {
  uint8_t follow_id;
  uint8_t speed_high:4;
  uint8_t control:3;
  uint8_t dir:1;
  uint8_t speed_low;
  uint8_t routeStation;
  uint8_t routeModule;
  uint8_t functions[4];
};

// 0x46
packedstruct s_opc_TrainRoute {
  uint8_t train_id;
  uint8_t station_id;
  uint8_t module_id;
};

// 0x4A
packedstruct s_opc_DCCEngineUpdate {
  uint8_t follow_id;
  uint8_t speed_high:4;
  uint8_t control:3;
  uint8_t dir:1;
  uint8_t speed_low;
  uint8_t routeStation;
  uint8_t routeModule;
  uint8_t functions[4];
};

// 0x4B
packedstruct s_opc_DCCEngineSpeed {
  uint8_t id;
  uint8_t empty1:3;
  uint8_t dir:1;
  uint8_t speed_high:4;
  uint8_t speed_low;
};

// 0x4C
packedstruct s_opc_DCCEngineFunction {
  uint8_t id;
  uint8_t type:2;
  uint8_t function:6;
};

// 0x4F
packedstruct s_opc_SubscribeTrain {
  uint8_t followA;
  uint8_t followB;
};

// 0x50
packedstruct s_opc_AddNewCartolib {
  uint16_t nr;
  uint16_t max_speed;
  uint16_t length;
  uint8_t type;
  uint8_t filetype;
  uint8_t name_len;
  uint16_t timing;
  char strings;
};

#define WSopc_AddNewCartolib_res_len 4
packedstruct s_opc_AddNewCartolib_res {
  uint16_t nr;
  uint8_t response;
};

// 0x51
packedstruct s_opc_EditCarlib {
  uint8_t id_h:7;
  uint8_t remove:1;
  uint8_t id_l;
  struct s_opc_AddNewCartolib data;
};

// 0x53
packedstruct s_opc_AddNewEnginetolib {
  uint16_t DCC_ID;
  uint16_t length;
  uint8_t type;
  uint8_t flags;
  uint8_t name_len;
  uint8_t steps;
  uint8_t functions[29];
  uint8_t timing[3];
  char strings;
};

#define WSopc_AddNewEnginetolib_res_len 2
packedstruct s_opc_AddNewEnginetolib_res {
  uint8_t response;
};

// 0x54
packedstruct s_opc_EditEnginelib {
  uint8_t id_h:7;
  uint8_t remove:1;
  uint8_t id_l;
  struct s_opc_AddNewEnginetolib data;
};


// 0x56
packedstruct s_opc_AddNewTraintolib {
  uint8_t name_len;
  uint8_t nr_stock:7;
  uint8_t save:1;
  uint8_t catagory;
  char strings;
};

#define WSopc_AddNewTraintolib_res_len 2
packedstruct s_opc_AddNewTraintolib_res {
  uint8_t response;
};

// 0x57
packedstruct s_opc_EditTrainlib {
  uint8_t id_h:7;
  uint8_t remove:1;
  uint8_t id_l;
  struct s_opc_AddNewTraintolib data;
};


// 0x90 0x91
packedstruct s_opc_enabledisableSubmoduleState {
  uint8_t flags;
};

// 0xCF
packedstruct s_opc_AdminLogin {
  char password[1];
};




struct s_WS_Data {
  uint8_t opcode;
  union {
    struct s_opc_SetSwitch                   opc_SetSwitch;

    struct s_opc_TrackLayoutRawData          opc_TrackLayoutRawData;
    struct s_opc_TrackLayoutUpdate           opc_TrackLayoutUpdate;

    struct s_opc_LinkTrain                   opc_LinkTrain;
    struct s_opc_SetTrainSpeed               opc_SetTrainSpeed;
    struct s_opc_SetTrainFunction            opc_SetTrainFunction;

    struct s_opc_TrainControl                opc_TrainControl;

    struct s_opc_UpdateTrain                 opc_UpdateTrain;
    struct s_opc_TrainRoute                  opc_TrainRoute;

    struct s_opc_DCCEngineUpdate             opc_DCCEngineUpdate;
    struct s_opc_DCCEngineSpeed              opc_DCCEngineSpeed;
    struct s_opc_DCCEngineFunction           opc_DCCEngineFunction;

    struct s_opc_SubscribeTrain              opc_SubscribeTrain;

    struct s_opc_AddNewCartolib              opc_AddNewCartolib;
    struct s_opc_AddNewCartolib_res          opc_AddNewCartolib_res;
    struct s_opc_EditCarlib                  opc_EditCarlib;

    struct s_opc_AddNewEnginetolib           opc_AddNewEnginetolib;
    struct s_opc_AddNewEnginetolib_res       opc_AddNewEnginetolib_res;
    struct s_opc_EditEnginelib               opc_EditEnginelib;

    struct s_opc_AddNewTraintolib            opc_AddNewTraintolib;
    struct s_opc_AddNewTraintolib_res        opc_AddNewTraintolib_res;
    struct s_opc_EditTrainlib                opc_EditTrainlib;

    struct s_opc_enabledisableSubmoduleState opc_EnableSubModule;
    struct s_opc_enabledisableSubmoduleState opc_DisableSubModule;
  } data;
};

#endif