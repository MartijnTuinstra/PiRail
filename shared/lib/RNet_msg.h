#ifndef RNET_MSG_HEADER
#define RNET_MSG_HEADER


#define RNET_CHECKSUM_SEED 0b10101010
#define RNET_BROADCAST_ID 0xFF

// General
#define RNet_OPC_DEV_ID           0x01
#define RNet_OPC_SetEmergency     0x02
#define RNet_OPC_RelEmergency     0x03
#define RNet_OPC_PowerOFF         0x04
#define RNet_OPC_PowerON          0x05
#define RNet_OPC_ResetALL         0x06

#define RNet_OPC_DisconnectNotify 0xF0
#define RNet_OPC_NACK             0x7E
#define RNet_OPC_ACK              0x7F

//IO
#define RNet_OPC_SetOutput     0x10
#define RNet_OPC_SetAllOutput  0x11
#define RNet_OPC_ReadInput     0x13
#define RNet_OPC_ReqReadInput  0x14
#define RNet_OPC_ReadAll       0x15

//Settings
#define RNet_OPC_ChangeID      0x50
#define RNet_OPC_ChangeNode    0x51
#define RNet_OPC_SetBlink      0x52
#define RNet_OPC_SetPulse      0x53
#define RNet_OPC_SetCheck      0x54
#define RNet_OPC_SetIO         0x55

#define RNet_OPC_DisconnectNotify 0xEE

#define RNet_OPC_ReadEEPROM    0x60

#define RNet_msg_len_NotWhole  0xFF

#if !defined(packedstruct)
#define packedstruct struct __attribute__((__packed__))
#endif

packedstruct RNet_TrackPower {
  uint8_t onoff:1;
  uint8_t emergency:1;
  uint8_t opcode:6;
};

packedstruct RNet_DeviceDiscovery {
  uint8_t opcode;
};

packedstruct RNet_DeviceControlStates {
  uint8_t opcode;
  uint16_t states;
};

packedstruct RNet_ProgramField {
  uint8_t opcode;
  uint16_t field;
  uint8_t data[10];
};

packedstruct RNet_GetProgramField {
  uint8_t opcode;
  uint16_t field;
};

packedstruct RNet_GetProgramFieldResponse {
  uint8_t opcode;
  uint16_t field;
  uint8_t data[10];
};

packedstruct RNet_Acknowledge {
  uint8_t acknowledged:1;
  uint8_t opcode:7;
};

packedstruct RNet_SetIOPorts {
  uint8_t opcode;
  uint8_t ports[10];
};

packedstruct RNet_SetIOPort {
  uint8_t opcode;
  uint8_t port;
  uint8_t state;
};

packedstruct RNet_GetIO {
  uint8_t opcode;
};

packedstruct RNet_GetIOResponse {
  uint8_t opcode;
  uint8_t ports[10];
};

packedstruct RNet_GetTrainInBlock {
  uint8_t opcode;
  uint8_t module;
  uint8_t block;
};

packedstruct RNet_GetTrainInBlockResponse {
  uint8_t opcode;
  uint8_t trainID;
};

packedstruct RNet_SetTrainSpeed {
  uint8_t opcode;
  uint8_t train;
  uint8_t speed;
};

packedstruct RNet_SetAnalogController {
  uint8_t opcode;
  uint8_t channel;
  uint8_t speed;
};

packedstruct RNet_Header {
  uint8_t destination_device;
  uint8_t destination_node:4;
  uint8_t source_node:4;
  uint8_t source_device;
  uint8_t size;
  uint8_t checksum;
};

packedstruct RNet_Message {
  struct RNet_Header header;
  union {
    uint8_t raw[100];


    struct RNet_TrackPower                TrackPower;
    struct RNet_DeviceDiscovery           DeviceDiscovery;
    struct RNet_DeviceControlStates       DeviceControlStates;
    struct RNet_ProgramField              ProgramField;
    struct RNet_GetProgramField           GetProgramField;
    struct RNet_GetProgramFieldResponse   GetProgramFieldResponse;
    struct RNet_Acknowledge               Acknowledge;
    struct RNet_SetIOPorts                SetIOPorts;
    struct RNet_SetIOPort                 SetIOPort;
    struct RNet_GetIO                     GetIO;
    struct RNet_GetIOResponse             GetIOResponse;
    struct RNet_GetTrainInBlock           GetTrainInBlock;
    struct RNet_GetTrainInBlockResponse   GetTrainInBlockResponse;
    struct RNet_SetTrainSpeed             SetTrainSpeed;
    struct RNet_SetAnalogController       SetAnalogController;
  } payload;
};

#endif
