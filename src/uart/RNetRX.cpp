#include "uart/RNetRX.h"
#include "RNet_msg.h"
#include "uart/uart.h"
#include "system.h"
#include "utils/logger.h"
#include "switchboard/manager.h"
#include "switchboard/unit.h"
#include "websocket/stc.h"

using namespace switchboard;

//TODO Add these functions
  // else if(data[1] == RNet_OPC_SetEmergency){ //Set Emergency STOP
  //   WS_stc_EmergencyStop();
  //   Z21_TRACKPOWER_OFF;
  // }
  // else if(data[1] == RNet_OPC_RelEmergency){ //Release Emergency STOP
  //   WS_stc_ClearEmergency();
  //   Z21_TRACKPOWER_ON;
  // }

void UART_ACK(uint8_t device){
  loggerf(INFO, "ACK");
  uart.ACK = true;
}
void UART_NACK(uint8_t device){
  loggerf(WARNING, "NACK");
  uart.NACK = true;
}
void UART_DEV_ID(uint8_t device, uint8_t * data){
  //Add device to device list
  if (device != 0xFF)
    loggerf(ERROR, "UART DEVID from a node, not from master");

  for(uint16_t i = 0; i < 255; i++){
    Unit * U = Units(i);
    if(!U)
      continue;
    if(i >= SwManager->Units.size)
      break;

    U->on_layout = 0;

    if(data[i/8] & (1 << (i%8))){
      loggerf(INFO, "UART Found Module %d", i);
      U->on_layout = 1;
    }
  }

  // Update all clients with the new set of modules
  WS_stc_Track_Layout(0);

  SYS->UART.modules_found = 1;
}

void UART_ReadInput(uint8_t device, uint8_t * data){
  Unit * U = Units(device);

  uint8_t node = data[0];

  if(!U)
    return;

  IO_Node * Node = U->Node[node];

  if(!Node)
    return;

  //uint8_t node = data[3];
  uint8_t l = data[1];
  for(uint8_t i = 0; i < l*8; i++){
    if(i >= Node->io_ports)
      break;

    if(data[i/8+2] & (1 << (i%8))){
      Node->io[i]->setInput(IO_event_High);
    }
    else{
      Node->io[i]->setInput(IO_event_Low);
    }
  }
}

void (*UART_RecvCb[256])(uint8_t, uint8_t *) = {
  // 0x00
  0,
  // 0x01
  UART_DEV_ID,
  // 0x02 - 0x0F
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,

  // 0x10 - 0x12
  0,0,0,
  // 0x13
  UART_ReadInput,
  // 0x14 - 0x1F
  0,0,0,0,0,0,0,0,0,0,0,0,

  // 0x20 - 0x2F
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0x30 - 0x3F
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0x40 - 0x4F
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0x50 - 0x5F
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0x60 - 0x6F
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0x70 - 0x7D
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0x7E
  (void (*)(uint8_t, uint8_t *))&UART_NACK,
  // 0x7F
  (void (*)(uint8_t, uint8_t *))&UART_ACK,

  // 0x80 - 0x8F
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0x90 - 0x9F
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0xA0 - 0xAF
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0xB0 - 0xBF
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0xC0 - 0xCF
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0xD0 - 0xDF
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0xE0 - 0xEF
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0xF0 - 0xFF
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
