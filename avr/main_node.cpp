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

#ifdef RNET_CSMA

struct RNet_Message RNet_TX_Msg;

#endif

int main(){
  uart.init();
  #ifndef RNET_MASTER

  #ifndef IO_SPI
// eeprom_write_byte(&EE_Mem.IO[0].type, IO_Input_Block);
// eeprom_write_byte(&EE_Mem.IO[0]._default, IO_event_High);

// eeprom_write_byte(&EE_Mem.IO[1].type, IO_Output);
// eeprom_write_byte(&EE_Mem.IO[1]._default, IO_event_High);

// eeprom_write_byte(&EE_Mem.IO[2].type, IO_Input_Block);
// eeprom_write_byte(&EE_Mem.IO[2]._default, IO_event_High);

  // for (uint8_t i = 3; i < 12; i++){
  //   eeprom_write_byte(&EE_Mem.IO[i].type, IO_Input_Block);
  //   eeprom_write_byte(&EE_Mem.IO[i]._default, IO_event_High);
  // }
  // eeprom_write_byte(&EE_Mem.IO[12].type, IO_Output);
  // eeprom_write_byte(&EE_Mem.IO[12]._default, IO_event_High);
  #endif // IO_SPI

  eeprom_write_word(&EE_Mem.settings.blink1, 1000);
  eeprom_write_word(&EE_Mem.settings.blink2, 3200);

  io.init();

  io.readInput();
  io.copyInput();

  for(uint8_t i = 0; i < MAX_PORTS; i++)
    uart.transmit(io.readMask[i], HEX, 2);
  uart.transmit('\n');
  #endif // not RNET_MASTER

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

  #ifdef RNET_MASTER

  _delay_ms(1000);
  net.init();
  #else
  net.init(eeprom_read_byte(&EE_Mem.ModuleID), eeprom_read_byte(&EE_Mem.NodeID));
  #endif

  #ifdef RNET_MASTER

  // net.reset_bus();

  _delay_ms(1000);

  // net.request_registered();

  // Loop
  while(1){
    net.try_transmit();

    net.request_registered();

    if(net.rx.read_index != net.rx.write_index){
      uart.start_tx();
    }

    _delay_us(10);
  }

  #else // Slave

  //RNET_TX_SET_HIGH
  //RNET_TX_SET_LOW
  //RNET_DUPLEX_SET_RX
  //RNET_READ_RX

  net.reset_bus();

  uint8_t checkcounter = 0;

  while(1){
    while(net.state == IDLE && net.available()){
      net.read();
    }

    _delay_us(100);

    #ifdef IO_SPI
    io.readwrite();
    #endif

    checkcounter++;

    if(checkcounter > 10){
      checkcounter = 0;

    //continue;
      #ifndef IO_SPI
      io.copyInput();
      io.readInput();
      #endif

      uint8_t diff = 0;

      for(int i = 0; i < MAX_PORTS; i++){
        if(io.oldreadData[i] ^ io.readData[i]){
          diff = 1;
          break;
        }
      }

      if(diff){
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
          uart.transmit(io.readData[i], HEX, 2);
          net.tx.write(io.readData[i]);
          checksum ^= io.readData[i];
        }

        #else  // not IO_SPI

        for(uint8_t i = 0; i < ((MAX_PINS + 7) / 8); i++){
          uart.transmit(io.readData[i], HEX, 2);
          net.tx.write(io.readData[i]);
          checksum ^= io.readData[i];
        }

        #endif // not IO_SPI

        uart.transmit(checksum, HEX, 2);
        net.tx.write(checksum);
        net.txdata++;
        uart.transmit('\n');
        uart.transmit('\r');
      }


      #ifdef IO_SPI
      io.copyInput();
      #endif
    }
  }

  #endif
  return 0;
}
