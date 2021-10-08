#include "RNet.h"
#include "IO.h"
#include "RNet_msg.h"

#include "uart.h"

void NotifyInputChange(uint8_t * data){
  uart.transmit("IO change: ", 11);
  uart.transmit(RNet_OPC_ReadInput, HEX,2);
  uart.transmit(net.node_id, HEX, 2);
  uart.transmit(MAX_PORTS, HEX, 2);
  net.tx.write(RNet_OPC_ReadInput);
  net.tx.write(net.node_id);

  uint8_t checksum = RNET_CHECKSUM_SEED ^ RNet_OPC_ReadInput ^ net.node_id;

  #ifdef IO_SPI
  net.tx.write(MAX_PORTS);
  checksum ^= MAX_PORTS;
  #else
  net.tx.write((MAX_PINS + 7)/8);
  checksum ^= (MAX_PINS + 7)/8;
  #endif

  #ifdef IO_SPI

  for(uint8_t i = 0; i < MAX_PORTS; i++){
    uart.transmit(data[i], HEX, 2);
    net.tx.write(data[i]);
    checksum ^= data[i];
  }

  #else  // not IO_SPI

  for(uint8_t i = 0; i < ((MAX_PINS + 7) / 8); i++){
    uart.transmit(data[i], HEX, 2);
    net.tx.write(data[i]);
    checksum ^= data[i];
  }

  #endif // not IO_SPI

  uart.transmit(checksum, HEX, 2);
  net.tx.write(checksum);
  net.txdata++;
  uart.transmit('\r');
  uart.transmit('\n');
}
void NotifyInputChange(){
  NotifyInputChange(io.DebouncedData);
}

