#include "avr/io.h"
#include "avr/fuse.h"
#include "avr/interrupt.h"

#include "main_node.h"

//Set fuses for each target AVR
#if defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)
FUSES = {0xE2, 0xD9, 0xFF};
#elif defined(__AVR_ATmega64A__)
FUSES = {0xFF, 0x99, 0xFF};
#elif defined(__AVR_ATmega2560__)
FUSES = {0xC2, 0x99, 0xFF};
#else
#error "Device not supported"
#endif

#include "util/delay.h"

#include "IO.h"

#include "eeprom_layout.h"

#ifdef RNET_CSMA
#include "RNet_csma.h"
#else
#include "RNet.h"
#endif

#include "RNet_msg.h"

uint8_t test = 4;

#include "uart.h"


void flash_number(uint8_t number){
  for(int i = 0; i < number; i++){
    #if defined(_BUFFER) || defined(IO_SPI)

    #else
    io.blink1();
    _delay_ms(100);
    io.blink1();
    _delay_ms(100);
    #endif
  }
  return;
}

RNet net;

int main(){

  // eeprom_update_byte(&EE_Mem.ModuleID, 9);
  // eeprom_update_byte(&EE_Mem.NodeID, 2);

  // _delay_ms(1000);
  uart.init();
  #ifndef RNET_MASTER
  // {
  //   for(uint8_t * i = 0; *i < 0xFF; i += 1){
  //     eeprom_write_byte(i, 0);
  //     uart.transmit(*i, HEX);
  //     uart.transmit('\n');
  //   }
  // }

  eeprom_write_byte(&EE_Mem.IO[0].type, IO_InputToggle);
  eeprom_write_byte(&EE_Mem.IO[0].def, IO_event_High);

  eeprom_write_byte(&EE_Mem.IO[1].type, IO_Output);
  eeprom_write_byte(&EE_Mem.IO[1].def, IO_event_High);

  eeprom_write_byte(&EE_Mem.IO[2].type, IO_InputToggle);
  eeprom_write_byte(&EE_Mem.IO[2].def, IO_event_High);

  for (uint8_t i = 3; i < 12; i++){
    eeprom_write_byte(&EE_Mem.IO[i].type, IO_InputToggle);
    eeprom_write_byte(&EE_Mem.IO[i].def, IO_event_High);
  }
  eeprom_write_byte(&EE_Mem.IO[12].type, IO_Output);
  eeprom_write_byte(&EE_Mem.IO[12].def, IO_event_High);


  // eeprom_write_byte(&EE_Mem.ModuleID, 5);
  // eeprom_write_byte(&EE_Mem.NodeID, 0);
  
  eeprom_write_word(&EE_Mem.settings.blink1, 1000);
  eeprom_write_word(&EE_Mem.settings.blink2, 3200);

  io.init();
  // DDRB |= 0b00100000; // Set LED as output 
  // io.out(0);
  // io.out(1);
  // io.in(2);
  // io.in(3);
  // io.set_mask(LED, IO_event_Blink1);
  io.readInput();
  io.copyInput();

  for(uint8_t i = 0; i < MAX_PORTS; i++)
    uart.transmit(io.readMask[i], HEX, 2);
  uart.transmit('\n');
  #endif

  // _delay_ms(1000);

  //Scope for ID
  // {
  //   //Blink master ID
  //   uint8_t ID = eeprom_read_byte(&EE_Mem.ModuleID);
  //   flash_number(ID);

  //   _delay_ms(400);

  //   //Blink slave ID
  //   ID = eeprom_read_byte(&EE_Mem.NodeID);
  //   flash_number(ID);
  // }

  _delay_ms(400);

  #if defined(IO_SPI)
  uart.transmit("IO_SPI\n",7);
  #endif

  #ifdef RNET_MASTER
  _delay_ms(500);
  net.init();
  #else
  net.init(eeprom_read_byte(&EE_Mem.ModuleID), eeprom_read_byte(&EE_Mem.NodeID));
  #endif

  // #endif

  // _delay_ms(5000);

  #ifdef RNET_MASTER

  net.tx.buf[0] = 3;
  net.tx.buf[1] = RNet_OPC_SetAllOutput;
  net.tx.buf[2] = 0; // Node id
  net.tx.buf[3] = 0x02;
  net.tx.buf[4] = 0x22;
  net.tx.buf[5] = RNet_OPC_SetAllOutput^0x02^0x22^RNET_CHECKSUM_SEED;

  net.tx.write_index = 6;
  net.tx.read_index = 0;

  _delay_ms(1500);

  net.request_all();

  _delay_ms(1000);

  net.request_registered();

  // Loop
  while(1){
    net.try_transmit();

    net.request_registered();

    if(net.rx.read_index != net.rx.write_index){
      uart.start_tx();
    }

    _delay_ms(10);
  }

  #else // Slave

  net.tx.buf[0] = RNet_OPC_DEV_ID;
  net.tx.write_index++;
  net.txdata = 1;

  uint8_t i = 0;
  uint16_t j = 0;
  while(1){
    while(net.state == IDLE && net.available()){
      net.read();
    }

    _delay_ms(10);
    j++;
    if(j == 1000){

      uart.transmit('?');

      while(net.state != IDLE){}

      net.tx.buf[net.tx.write_index] = RNet_OPC_ChangeNode;
      net.tx.buf[(net.tx.write_index+1)%RNET_MAX_BUFFER] = i++;
      net.tx.buf[(net.tx.write_index+2)%RNET_MAX_BUFFER] = 0xED;
      net.tx.buf[(net.tx.write_index+3)%RNET_MAX_BUFFER] = 0x34;
      net.tx.buf[(net.tx.write_index+4)%RNET_MAX_BUFFER] = 0xBC;
      net.tx.write_index = (net.tx.write_index+5)%RNET_MAX_BUFFER;
      net.txdata = 1;
      j = 0;
    }



    io.copyInput();
    io.readInput();

    uint8_t diff = 0;

    for(int i = 0; i < MAX_PORTS; i++){
      if(io.oldreadData[i] ^ io.readData[i]){
        io.oldreadData[i] = io.readData[i];
        diff = 1;
      }
    }

    if(diff){
      uart.transmit("IO change: ", 11);
      net.tx.buf[net.tx.write_index] = RNet_OPC_ReadInput;
      net.tx.buf[(net.tx.write_index+1)%RNET_MAX_BUFFER] = net.node_id;
      net.tx.buf[(net.tx.write_index+2)%RNET_MAX_BUFFER] = MAX_PORTS;

      uint8_t checksum = RNET_CHECKSUM_SEED ^ RNet_OPC_ReadInput ^ MAX_PORTS ^ net.node_id;

      uint8_t x = 3;

      for(uint8_t i = 0; i < MAX_PORTS; i++){
        uart.transmit(io.readData[i], HEX, 2);
        net.tx.buf[(net.tx.write_index+(x++))%RNET_MAX_BUFFER] = io.readData[i];
        checksum ^= io.readData[i];
      }

      net.tx.buf[(net.tx.write_index+(x++))%RNET_MAX_BUFFER] = checksum;
      net.tx.write_index = (net.tx.write_index+x)%RNET_MAX_BUFFER;
      net.txdata = 1;
      uart.transmit('\n');
    }

  }

  #ifndef RNET_CSMA

  while (1){}

  #else
  while (1){

    // #ifndef IO_SPI
    // //Receive from Net
    // if(net.checkReceived()){
    //   #ifndef _BUFFER
    //   // Message is copied to temp buffer
    //   // ready to be executed
    //   net.executeMessage();
    //   #else
    //   // Message is pass through to UART
    //   uint8_t size = net.getMsgSize(tmp_rx_msg);
    //   uart.transmit(tmp_rx_msg, size);
    //   #endif
    // }
    // #endif

    #ifdef IO_SPI
    io.copyInput();
    io.readInput();

    uint8_t diff = 0;

    for(int i = 0; i < MAX_PORTS; i++){
      if((io.oldreadData[i] ^ io.readData[i])){
        diff = 1;
        //Determine which bits are set
        // if(diff & 1)  {addr[c_addr++] = i*8+0;}
        // if(diff & 2)  {addr[c_addr++] = i*8+1;}
        // if(diff & 4)  {addr[c_addr++] = i*8+2;}
        // if(diff & 8)  {addr[c_addr++] = i*8+3;}
        // if(diff & 16) {addr[c_addr++] = i*8+4;}
        // if(diff & 32) {addr[c_addr++] = i*8+5;}
        // if(diff & 64) {addr[c_addr++] = i*8+6;}
        // if(diff & 128){addr[c_addr++] = i*8+7;}
      }
    }

    if(diff){
      uart.transmit("IO change: ", 11);
      net.tx.buf[net.tx.write_index] = RNet_OPC_ReadInput;
      net.tx.buf[(net.tx.write_index+1)%RNET_MAX_BUFFER] = net.node_id;
      net.tx.buf[(net.tx.write_index+2)%RNET_MAX_BUFFER] = MAX_PORTS;

      uint8_t checksum = RNET_CHECKSUM_SEED ^ RNet_OPC_ReadInput ^ MAX_PORTS ^ net.node_id;

      uint8_t j = 3;

      for(uint8_t i = 0; i < MAX_PORTS; i++){
        uart.transmit(io.readData[i]);
        net.tx.buf[(net.tx.write_index+(j++))%RNET_MAX_BUFFER] = io.readData[i];
        checksum ^= io.readData[i];
      }

      net.tx.buf[(net.tx.write_index+(j++))%RNET_MAX_BUFFER] = checksum;
      net.tx.write_index = (net.tx.write_index+j)%RNET_MAX_BUFFER;
      net.txdata = 1;
      uart.transmit('\n');

      // Transmit to RNet
      // net.add_to_tx_buf(RNet_OPC_ReadInput);
      // net.add_to_tx_buf((c_addr/2)*3+3);

      // for(uint8_t i = 0; i < c_addr;){
      //   net.add_to_tx_buf(addr[i]);
      //   uint8_t state = (io.readData[addr[i]/8] & addr[i]%8)?0x10:0x00;
      //   i++;

      //   if(i < c_addr){
      //     state |= (io.readData[addr[i]/8] & addr[i]%8)?0x01:0x00;
      //     net.add_to_tx_buf(state);
      //     net.add_to_tx_buf(addr[i]);
      //   }
      //   else{
      //     net.add_to_tx_buf(state);
      //     net.add_to_tx_buf(0);
      //   }

      //   i++;
      // }

      // uart.transmit("RNetTX: ",8);
      // net.calculateTxChecksum();
      // uart.transmit('\n');
    }

    _delay_ms(10);
    #endif
    // _delay_ms(5000);
    // _delay_ms(5000);
  }

  #endif
  #endif
  return 0;
}
