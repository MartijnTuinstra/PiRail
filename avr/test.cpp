#include "avr/io.h"
#include "avr/fuse.h"
#include "avr/eeprom.h"

#define F_CPU 16000000U
#include "util/delay.h"

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

int EEMEM Mem = 10;


int main(){
  DDRB |= (1 << 7);
  PORTB &= ~(1 << 7);

  while(1){
    PORTB |= (1 << 7);
    _delay_ms(500);
    PORTB &= ~(1 << 7);
    _delay_ms(500);
  }
}
