#include "avr/io.h"
#include "avr/fuse.h"
#include "avr/interrupt.h"
#include "avr/eeprom.h"

//Set fuses for each target AVR
#if defined(__AVR_ATmega328__)
FUSES = {0xE2, 0xD9, 0xFF};
#define F_CPU 8000000U
#elif defined(__AVR_ATmega2560__)
FUSES = {0xC2, 0x99, 0xFF};
#define F_CPU 16000000U
#else
#error "Device not supported"
#endif



#include "util/delay.h"

uint8_t EEMEM TestEEPROM;

int main(){
	DDRB = 0x08;
	while (1){
		PORTB = 0x00;
		_delay_ms(500);
		PORTB = 0x08;
		_delay_ms(500);
	}
	return 0;
}
