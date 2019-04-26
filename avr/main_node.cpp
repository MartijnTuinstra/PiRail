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
#include "RNet.h"

uint8_t test = 4;

#include "uart.h"


void flash_number(uint8_t number){
  for(int i = 0; i < number; i++){
    #ifdef _BUFFER

    #else
    io.blink1();
    _delay_ms(150);
    io.blink1();
    _delay_ms(150);
    #endif
  }
  return;
}

RNet net;

int main(){

#if defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)
#define LED 13
#elif defined(__AVR_ATmega64A__)
#define LED 0
#elif defined(__AVR_ATmega2560__)
#define LED 23
#endif
  // eeprom_update_byte(&EE_Mem.ModuleID, 9);
  // eeprom_update_byte(&EE_Mem.NodeID, 2);

  // _delay_ms(1000);
  uart.init();
  #ifndef _BUFFER  
  io.init();


  io.out(LED);
  io.low(LED);
  io.set_blink1(LED);
  io.set_blink1(3);
  io.set_blink1(1);
  io.set_blink2(0);
  io.set_blink2(2);
  #endif

  UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
  UCSR0B = _BV(TXEN0);   /* Enable TX */

  eeprom_write_byte(&EE_Mem.ModuleID, 3);
  eeprom_write_byte(&EE_Mem.NodeID, 2);
  
  eeprom_write_byte(&EE_Mem.NodeID, 2);

  _delay_ms(1000);

  //Scope for ID
  {
    //Blink master ID
    uint8_t ID = eeprom_read_byte(&EE_Mem.ModuleID);
    flash_number(ID);

    _delay_ms(800);

    //Blink slave ID
    ID = eeprom_read_byte(&EE_Mem.NodeID);
    flash_number(ID);
  }

  _delay_ms(800);

  #if defined(IO_SPI)

  #else
  net.init(eeprom_read_byte(&EE_Mem.ModuleID), eeprom_read_byte(&EE_Mem.NodeID));
  #endif

  uart.transmit("RX\n", 3);

  // _delay_ms(5000);

  while (1){
    // if(net.state == IDLE && RNet_rx_buffer.read_index != RNet_rx_buffer.write_index){
    //  readRXBuf();
    // }
    // if(net.state == IDLE || net.state == HOLDOFF){
    //   net.transmit(5);
    // }
    #ifdef _BUFFER
    // Receive from UART
    if(uart.available()){
      //receive byte
      uint8_t c = uart.receive();
      net.add_to_tx_buf(c);
      if(net.checkTxReady()){
        net.transmit(20);
      }
    }
    #endif

    //Receive from Net
    if(net.checkReceived()){
      #ifndef _BUFFER
      // Message is copied to temp buffer
      // ready to be executed
      net.executeMessage();
      #else
      // Message is pass through to UART
      uint8_t size = net.getMsgSize(tmp_rx_msg);
      uart.transmit(tmp_rx_msg, size);
      #endif
    }
    #ifdef IO_SPI
    // uart_putchar('.');
    // io.blink1();
    // io.blink2();
    // io.writeOutput();
    // _delay_ms(1000);
    // io.blink1();
    // io.writeOutput();
    // _delay_ms(1000);
    // io.blink1();
    // io.blink2();
    // io.writeOutput();
    // _delay_ms(1000);
    // io.blink1();
    // io.writeOutput();
    // _delay_ms(1000);
    for(int i = 0; i < MAX_PORTS*8; i++){
      io.toggle(i);
      io.writeOutput();
      _delay_ms(100);
      io.toggle(i);
      io.writeOutput();
      _delay_ms(200);
    }
    // io.readInput();
    _delay_ms(5000);
    #endif
    // _delay_ms(5000);
    // _delay_ms(5000);
  }
  return 0;
}