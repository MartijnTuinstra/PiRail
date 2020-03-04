#ifndef _INCLUDE_COM_H
  #define _INCLUDE_COM_H

  #define Serial_Port "/dev/ttyAMB0"
  #define Serial_Baud B115200

  #define UART_BUFFER_SIZE 200
  #define UART_Msg_NotComplete 0xFF
  #define UART_CHECKSUM_SEED 0b10101010

  #include "signals.h"

  struct train;

  struct signal;

  struct Seg;
  struct Swi;

  struct COM_t{
    char length;
    char data[32];
  };

  struct fifobuffer {
    char buffer[UART_BUFFER_SIZE];
    uint8_t read;
    uint8_t write;
  };

  void * UART();

  void COM_Send(struct COM_t DATA);

  int COM_Recv(struct fifobuffer * buf);

  void COM_Parse(struct fifobuffer * buf);

  void COM_change_Output(int M);

  void COM_change_A_signal(int M);

  void COM_change_signal(Signal * Si);

  void COM_change_switch(int M);
  void COM_update_switch(int M);

  void COM_DevReset();

#define COM_CHECKSUM_SEED 0b10101010

// General
#define COM_OPC_DEV_ID        0x01
#define COM_OPC_SetEmergency  0x02
#define COM_OPC_RelEmergency  0x03
#define COM_OPC_PowerOFF      0x04
#define COM_OPC_PowerON       0x05
#define COM_OPC_ResetALL      0x06
#define COM_OPC_ACK           0x7F

//IO
#define COM_OPC_SetOutput     0x10
#define COM_OPC_ReadInput     0x11
#define COM_OPC_ReadAll       0x12

//Settings
#define COM_OPC_ChangeID      0x50
#define COM_OPC_ChangeNode    0x51
#define COM_OPC_SetBlink      0x52
#define COM_OPC_SetPulse      0x53
#define COM_OPC_SetCheck      0x54

#endif