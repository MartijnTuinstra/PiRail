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
#include "RNet_msg.h"

uint8_t test = 4;

#include "uart.h"


void flash_number(uint8_t number){
  for(int i = 0; i < number; i++){
    #if defined(_BUFFER) || defined(IO_SPI)

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
  io.high(40);
  io.set_mask(38, IO_event_Blink1);
  io.set_mask(43, IO_event_Blink1);
  io.set_mask(46, IO_event_Blink1);
  #endif

  UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
  UCSR0B = _BV(TXEN0);   /* Enable TX */

  eeprom_write_byte(&EE_Mem.ModuleID, 3);
  eeprom_write_byte(&EE_Mem.NodeID, 2);
  
  eeprom_write_word(&EE_Mem.settings.blink1, 1000);
  eeprom_write_word(&EE_Mem.settings.blink2, 3200);

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
  uart.transmit("IO_SPI\n",7);
  #else
  uart.transmit("RNet Init\n",10);
  net.init(eeprom_read_byte(&EE_Mem.ModuleID), eeprom_read_byte(&EE_Mem.NodeID));
  #endif

  // _delay_ms(5000);

  uart.transmit("Loop\n", 5);

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

    #ifndef IO_SPI
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
    #endif

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
    // for(int j = 0; j < 10; j++){
    //   for(int i = 37; i < 40; i++){
    //     io.toggle(i);
    //     io.writeOutput();
    //     _delay_ms(50);
    //     io.toggle(i);
    //     io.writeOutput();
    //     _delay_ms(100);
    //   }
    // }
    // _delay_ms(5000);
    // io.readInput();
    // io.set_mask(25, IO_event_Pulse);
    // uart.transmit("Pulse\n",6);
    // io.pulse_high();
    // _delay_ms(5000);
    // io.readInput();
    // io.set_mask(24, IO_event_Pulse);
    // io.pulse_high();
    // uart.transmit("Pulse\n",6);

    io.copyInput();
    io.readInput();

    uint8_t diff = 0;
    uint8_t addr[8*MAX_PORTS];
    uint8_t c_addr = 0;

    for(int i = 0; i < MAX_PORTS; i++){
      if((diff = io.oldreadData[i] ^ io.readData[i])){

        //Determine which bits are set
        if(diff & 1)  {addr[c_addr++] = i*8+0;}
        if(diff & 2)  {addr[c_addr++] = i*8+1;}
        if(diff & 4)  {addr[c_addr++] = i*8+2;}
        if(diff & 8)  {addr[c_addr++] = i*8+3;}
        if(diff & 16) {addr[c_addr++] = i*8+4;}
        if(diff & 32) {addr[c_addr++] = i*8+5;}
        if(diff & 64) {addr[c_addr++] = i*8+6;}
        if(diff & 128){addr[c_addr++] = i*8+7;}
      }
    }

    if(c_addr){
      uart.transmit("IO change: ", 11);
      for(uint8_t i = 0; i < c_addr; i++){
        printHex(addr[i]);
        uart.transmit(' ');
      }
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
  return 0;
}