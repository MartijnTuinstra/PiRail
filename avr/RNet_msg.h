
#define RNET_CHECKSUM_SEED 0b10101010

// General
#define RNet_OPC_DEV_ID        0x00
#define RNet_OPC_SetEmergency  0x01
#define RNet_OPC_RelEmergency  0x02
#define RNet_OPC_PowerOFF      0x03
#define RNet_OPC_PowerON       0x04
#define RNet_OPC_ResetALL      0x05
#define RNet_OPC_ACK           0x7F

//IO
#define RNet_OPC_SetOutput     0x10
#define RNet_OPC_ReadInput     0x11
#define RNet_OPC_ReadAll       0x12

//Settings
#define RNet_OPC_ChangeID      0x50
#define RNet_OPC_ChangeNode    0x51
#define RNet_OPC_SetBlink      0x52
#define RNet_OPC_SetPulse      0x53
#define RNet_OPC_SetCheck      0x54
