#include "avr/io.h"
#include "avr/fuse.h"
#include "avr/interrupt.h"

#include "main_node.h"

//Set fuses for each target AVR
#if defined(__AVR_ATmega328__)
FUSES = {0xE2, 0xD9, 0xFF};
#elif defined(__AVR_ATmega328P__)
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

#define USE_2X 0

static void
uart_38400(void)
{
#define BAUD 38400
#include <util/setbaud.h>
UBRR0H = UBRRH_VALUE;
UBRR0L = UBRRL_VALUE;
#if USE_2X
UCSR0A |= (1 << U2X0);
#else
UCSR0A &= ~(1 << U2X0);
#endif
}

void flash_number(uint8_t number, uint8_t pin){
  for(int i = 0; i < number; i++){
    set_high(pin);
    _delay_ms(150);
    set_low(pin);
    _delay_ms(150);
  }
  return;
}

int main(){

#if defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)
#define LED 12
#elif defined(__AVR_ATmega64A__)
#define LED 0
#elif defined(__AVR_ATmega2560__)
#define LED 0x17
#endif
  // eeprom_update_byte(&EE_Mem.ModuleID, 9);
  // eeprom_update_byte(&EE_Mem.NodeID, 2);

  //Disable SPI
  SPCR &= ~SPE;

  set_out(LED); // Set pin to output
  set_low(LED);

  _delay_ms(1000);

  uart_38400();

  UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
  UCSR0B = _BV(TXEN0);   /* Enable TX */

  eeprom_write_byte(&EE_Mem.ModuleID, 1);
  eeprom_write_byte(&EE_Mem.NodeID, 1);

  //Blink master ID
  uint8_t ID = eeprom_read_byte(&EE_Mem.ModuleID);
  flash_number(ID, LED);

  _delay_ms(800);

  //Blink slave ID
  ID = eeprom_read_byte(&EE_Mem.NodeID);
  flash_number(ID, LED);

  _delay_ms(800);

  RNet_init();

  while (1){
    if(BusSt != RX && BusSt != TX){
      readRXBuf();
    }
    //TX_try(25);
  }
  return 0;
}

void uart_putchar(char c) {
    loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
    UDR0 = c;
}
