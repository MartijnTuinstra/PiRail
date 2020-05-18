
#define RNET_CHECKSUM_SEED 0b10101010

// General
#define RNet_OPC_DEV_ID        0x01
#define RNet_OPC_SetEmergency  0x02
#define RNet_OPC_RelEmergency  0x03
#define RNet_OPC_PowerOFF      0x04
#define RNet_OPC_PowerON       0x05
#define RNet_OPC_ResetALL      0x06
#define RNet_OPC_NACK          0x7E
#define RNet_OPC_ACK           0x7F

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

#define RNet_OPC_ReadEEPROM    0x60

#define RNet_msg_len_NotWhole  0xFF