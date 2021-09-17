#include "uart/RNetTX.h"
#include "RNet_msg.h"
#include "uart/uart.h"
// #include "system.h"
#include "utils/logger.h"
#include "switchboard/unit.h"
#include "switchboard/manager.h"
// #include "websocket/stc.h"

void COM_change_A_signal(int M){
  COM_change_Output(M);
}

void COM_DevReset(){
  struct COM_t Tx;
  Tx.data[0] = 0xFF;  //Broadcast
  Tx.data[1] = RNet_OPC_DEV_ID;
  Tx.length  = 2;
  uart.send(&Tx);
}

void COM_set_single_Output_output(int M, int io, enum e_IO_output_event event){
  union u_IO_event type;
  type.output = event;
  COM_set_single_Output(M, io, type);
}

void COM_set_single_Output(int M, int io, union u_IO_event type){
  struct COM_t TX;
  TX.data[0] = M;
  TX.data[1] = RNet_OPC_SetOutput;
  TX.data[2] = io;
  TX.data[3] = type.value;
  TX.data[4] = UART_CHECKSUM_SEED ^ RNet_OPC_SetOutput ^ io ^ type.value;
  TX.length = 5;
  uart.send(&TX);
}

void COM_change_Output(IO_Node * N){
  struct COM_t TX;
  TX.data[0] = N->U->module;
  TX.data[1] = RNet_OPC_SetAllOutput;

  TX.data[2] = (N->io_ports + 3) / 4; // round up

  uint8_t CheckSum = UART_CHECKSUM_SEED ^ RNet_OPC_SetAllOutput ^ TX.data[2];
  uint8_t j = 3;
  for(uint8_t i = 0; i < N->io_ports; i++){
    if(i%4 == 0)
      TX.data[j] = 0;

    TX.data[j] |= (N->io[i]->w_state.value & 0b11) << ((i % 4) * 2);

    N->io[i]->r_state.value = N->io[i]->w_state.value;

    if(i%4 == 3){
      CheckSum ^= TX.data[j];
      j++;
    }
  }

  TX.length = 4 + TX.data[2];
  TX.data[TX.length-1] =  CheckSum;
  uart.send(&TX);
}
void COM_change_Output(int M){
  Unit * U = switchboard::Units(M);
  IO_Node * N = U ? U->Node[0] : 0;

  if(!N)
    return;

  COM_change_Output(N);
}

void COM_request_Inputs(uint8_t M){
  struct COM_t data;
  data.data[0] = M;
  data.data[1] = RNet_OPC_ReqReadInput;

  data.length = 2;

  uart.send(&data);
}

void COM_Configure_IO(uint8_t M, uint8_t ioPort, uint16_t config){
  struct COM_t TX;
  TX.data[0] = M;
  TX.data[1] = RNet_OPC_SetIO;
  TX.data[2] = ioPort;
  TX.data[3] = (config & 0xFF);
  TX.data[4] = (config >> 8);
  TX.data[5] = UART_CHECKSUM_SEED ^ RNet_OPC_SetIO ^ ioPort ^ (config & 0xFF) ^ (config >> 8);
  TX.length = 6;
  uart.send(&TX);
}

void COM_DisconnectNotify(){
  // auto p = UART::Packet({0, 0}); // Broadcast
  // p.setOpcode(RNet_OPC_DisconnectNotify);

  // uart.send(&p);

  struct COM_t Tx;
  Tx.data[0] = 0xFF; // Broadcast
  Tx.data[1] = RNet_OPC_DisconnectNotify;
  Tx.length  = 2;
  uart.send(&Tx);
}