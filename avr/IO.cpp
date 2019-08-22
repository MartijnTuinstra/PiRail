#include <string.h>

#include "avr/io.h"
#include "avr/interrupt.h"
#include "IO.h"

#include "RNet.h"
#include "main_node.h"
#include "uart.h"
#include "eeprom_layout.h"

#include "util/delay.h"

IO io;

// #if defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)

// const uint16_t PROGMEM _PORT[] = {
// 	0,
// 	(uint16_t) &PORTB,
// 	(uint16_t) &PORTC,
// 	(uint16_t) &PORTD
// };

// const uint16_t PROGMEM _DDR[] = {
// 	0,
// 	(uint16_t) &DDRB,
// 	(uint16_t) &DDRC,
// 	(uint16_t) &DDRD
// };

// const uint16_t PROGMEM _PIN[] = {
// 	0,
// 	(uint16_t) &PINB,
// 	(uint16_t) &PINC,
// 	(uint16_t) &PIND
// };

// uint8_t pin_to_Port[] = {
// 	// 0 - 7
// 	PD, PD, PD, PD, PD, PD, PC, PC,
// 	// 8 - 15
// 	PC, PC, PC, PC, PB
// };

// uint8_t pin_to_BIT[] = {
// 	// 0 - 7
// 	4, 3, 5, 2, 1, 0, 5, 4, 
// 	// 8 - 15
// 	3, 2, 1, 0, 5
// };

// #elif defined(__AVR_ATmega64A__)

// const uint16_t PROGMEM _PORT[] = {
// 	(uint16_t) &PORTA,
// 	(uint16_t) &PORTB,
// 	(uint16_t) &PORTC,
// 	(uint16_t) &PORTD,
// 	(uint16_t) &PORTE,
// 	(uint16_t) &PORTF,
// 	(uint16_t) &PORTG
// };

// const uint16_t PROGMEM _DDR[] = {
// 	(uint16_t) &DDRA,
// 	(uint16_t) &DDRB,
// 	(uint16_t) &DDRC,
// 	(uint16_t) &DDRD,
// 	(uint16_t) &DDRE,
// 	(uint16_t) &DDRF,
// 	(uint16_t) &DDRG
// };

// const uint16_t PROGMEM _PIN[] = {
// 	(uint16_t) &PINA,
// 	(uint16_t) &PINB,
// 	(uint16_t) &PINC,
// 	(uint16_t) &PIND,
// 	(uint16_t) &PINE,
// 	(uint16_t) &PINF,
// 	(uint16_t) &PING
// };

// uint8_t pin_to_Port[] = {
// 	// 0 - 7
// 	PG, PG, PB, PD, PB, PB, PB, PB,
// 	// 8 - 15
// 	PE, PE, PE, PE, PE, PE, PE, PE,
// 	// 16 - 23
// 	PF, PF, PF, PF, PF, PF, PF, PF,
// 	// 24 - 31
// 	PA, PA, PA, PA, PA, PA, PA, PA,
// 	// 32 - 39
// 	PC, PC, PC, PC, PC, PC, PC, PC
// };

// uint8_t pin_to_BIT[] = {
// 	// 0 - 7
// 	4, 3, 7, 0, 6, 5, 4, 0,
// 	// 8 - 15
// 	7, 6, 5, 4, 3, 2, 1, 0,
// 	// 16 - 23
// 	0, 1, 2, 3, 4, 5, 6, 7,
// 	// 24 - 31
// 	0, 1, 2, 3, 4, 5, 6, 7,
// 	// 32 - 39
// 	7, 6, 5, 4, 3, 2, 1, 0
// };

// #elif defined(__AVR_ATmega2560__)

// const uint16_t PROGMEM _PORT[] = {
// 	(uint16_t) &PORTA,
// 	(uint16_t) &PORTB,
// 	(uint16_t) &PORTC,
// 	(uint16_t) &PORTD,
// 	(uint16_t) &PORTE,
// 	(uint16_t) &PORTF,
// 	(uint16_t) &PORTG,
// 	(uint16_t) &PORTH,
// 	(uint16_t) &PORTJ,
// 	(uint16_t) &PORTK,
// 	(uint16_t) &PORTL
// };

// const uint16_t PROGMEM _DDR[] = {
// 	(uint16_t) &DDRA,
// 	(uint16_t) &DDRB,
// 	(uint16_t) &DDRC,
// 	(uint16_t) &DDRD,
// 	(uint16_t) &DDRE,
// 	(uint16_t) &DDRF,
// 	(uint16_t) &DDRG,
// 	(uint16_t) &DDRH,
// 	(uint16_t) &DDRJ,
// 	(uint16_t) &DDRK,
// 	(uint16_t) &DDRL
// };

// const uint16_t PROGMEM _PIN[] = {
// 	(uint16_t) &PINA,
// 	(uint16_t) &PINB,
// 	(uint16_t) &PINC,
// 	(uint16_t) &PIND,
// 	(uint16_t) &PINE,
// 	(uint16_t) &PINF,
// 	(uint16_t) &PING,
// 	(uint16_t) &PINH,
// 	(uint16_t) &PINJ,
// 	(uint16_t) &PINK,
// 	(uint16_t) &PINL
// };

// uint8_t pin_to_Port[] = {
// 	// 0 - 7
// 	PE, PE, PE, PE, PE, PE, PE, PE,
// 	// 8 - 15
// 	PH, PH, PH, PH, PH, PH, PH, PH,
// 	// 16 - 23
// 	PD, PD, PD, PB, PB, PB, PB, PB,
// 	// 24 - 31
// 	PL, PL, PL, PL, PL, PL, PL, PL,
// 	// 32 - 39
// 	PC, PC, PC, PC, PC, PC, PC, PC,
// 	// 40 - 47
// 	PJ, PJ, PJ, PJ, PJ, PJ, PJ, PJ,
// 	// 48 - 55
// 	PA, PA, PA, PA, PA, PA, PA, PA,
// 	// 56 - 63
// 	PK, PK, PK, PK, PK, PK, PK, PK,
// 	// 64 - 81
// 	PF, PF, PF, PF, PF, PF, PF, PF
// };

// uint8_t pin_to_BIT[] = {
// 	// 0 - 7
// 	7, 6, 5, 4, 3, 2, 1, 0,
// 	// 8 - 15
// 	7, 6, 5, 4, 3, 2, 1, 0,
// 	// 16 - 23
// 	7, 6, 5, 0, 4, 5, 6, 7,
// 	// 24 - 31
// 	7, 6, 5, 4, 3, 2, 1, 0,
// 	// 32 - 39
// 	7, 6, 5, 4, 3, 2, 1, 0,
// 	// 40 - 47
// 	7, 6, 5, 4, 3, 2, 1, 0,
// 	// 48 - 55
// 	0, 1, 2, 3, 4, 5, 6, 7,
// 	// 56 - 63
// 	0, 1, 2, 3, 4, 5, 6, 7,
// 	// 64 - 81
// 	0, 1, 2, 3, 4, 5, 6, 7
// };
// #endif

// void set_out(uint8_t pin){
// 	*portModeRegister(pin_to_Port[pin]) |= (1 << pin_to_BIT[pin]);
// 	return;
// }
// void set_in(uint8_t pin){
// 	*portModeRegister(pin_to_Port[pin]) &= ~(1 << pin_to_BIT[pin]);
// 	return;
// }

// void set_high(uint8_t pin){
// 	*portOutputRegister(pin_to_Port[pin]) |= (1 << pin_to_BIT[pin]);
// 	return;
// }
// void set_low(uint8_t pin){
// 	*portOutputRegister(pin_to_Port[pin]) &= ~(1 << pin_to_BIT[pin]);
// 	return;
// }

// void set_toggle(uint8_t pin){
// 	*portOutputRegister(pin_to_Port[pin]) ^= (1 << pin_to_BIT[pin]);
// 	return;
// }

// uint8_t read(uint8_t pin){
// 	if (portInputRegister(pin_to_Port[pin])) return 1;
// 	return 0;
// }


void IO::high(uint8_t pin){
	#ifdef IO_SPI
	writeData[pin/8] |= 1 << (pin % 8);
	#else
	*_portOutputRegister(list[pin]/8) |= 1 << (list[pin] % 8);
	#endif
}

void IO::low(uint8_t pin){
	#ifdef IO_SPI
	writeData[pin/8] &= ~(1 << (pin % 8));
	#else
	*_portOutputRegister(list[pin]/8) &= ~(1 << (list[pin] % 8));
	#endif
}

void IO::toggle(uint8_t pin){
	#ifdef IO_SPI
	writeData[pin/8] ^= 1 << (pin % 8);
	#else
	*_portOutputRegister(list[pin]/8) ^= 1 << (list[pin] % 8);
	#endif
}


void IO::out(uint8_t pin){
	#ifdef IO_SPI
	return (void)pin;
	#else
	*_portModeRegister(list[pin]/8) |= 1 << (list[pin] % 8);
	#endif
}

void IO::in(uint8_t pin){
	#ifdef IO_SPI
	return (void)pin;
	#else
	*_portModeRegister(list[pin]/8) &= ~(1 << (list[pin] % 8));
	#endif
}

uint8_t IO::read(uint8_t pin){
	#ifdef IO_SPI
	return readData[pin/8] & (1 << (pin % 8));
	#else
	return (*(volatile uint8_t *)pinlist[list[pin]/8] & (1 << (list[pin] % 8)));
	#endif
}

#ifdef IO_SPI

#define LATCH_OUT_PORT PORTD
#define LATCH_OUT_PIN  PD3
#define LATCH_IN_PORT PORTC
#define LATCH_IN_PIN  PC0
#define LOAD_IN_PORT PORTC
#define LOAD_IN_PIN  PC2

#define LATCH_OUT_ENABLE  LATCH_OUT_PORT &= ~(1 << LATCH_OUT_PIN) // Low
#define LATCH_OUT_DISABLE LATCH_OUT_PORT |=  (1 << LATCH_OUT_PIN) // High

#define LATCH_IN_ENABLE   LATCH_IN_PORT &= ~(1 << LATCH_IN_PIN)
#define LATCH_IN_DISABLE  LATCH_IN_PORT |=  (1 << LATCH_IN_PIN)
#define IN_LOAD           LOAD_IN_PORT &= ~(1 << LOAD_IN_PIN); \
                          _delay_ms(50); \
                          LOAD_IN_PORT |=  (1 << LOAD_IN_PIN)

#endif

void IO::init(){
	memset(blink1Mask, 0, MAX_PORTS);
	memset(blink2Mask, 0, MAX_PORTS);

	memset(servo1Mask, 0, MAX_PORTS);
	memset(servo2Mask, 0, MAX_PORTS);
	memset(servo3Mask, 0, MAX_PORTS);
	memset(servo4Mask, 0, MAX_PORTS);

	#ifndef IO_SPI
	memset(readMask, 0, MAX_PORTS);
	#endif
	memset(readData, 0xFF, MAX_PORTS);
	memset(oldreadData, 0xFF, MAX_PORTS);

	#ifdef IO_SPI

	memset(writeData, 0, MAX_PORTS);
	memset(readMask, 0xFF, MAX_PORTS);

	#if defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)
	// set mosi and sck output
	// DDRB = (1<<DDB2)|(1<<DDB3)|(1<<DDB5);
	_set_out(DDR(B), PB5); //SCK
	_set_in(DDR(B),  PB4); //MISO
	_set_out(DDR(B), PB3); //MOSI
	_set_out(DDR(B), PB2); //SS

	_set_out(DDR(D), LATCH_OUT_PIN);
	_set_out(DDR(C), LATCH_IN_PIN);
	_set_out(DDR(C), LOAD_IN_PIN);

	_set_high(PORT(B), PB2); //SS

	#endif

	// enable SPI at fsck/128
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<SPR1);

	LATCH_OUT_DISABLE;
	LATCH_IN_DISABLE;
	IN_LOAD;

	writeOutput();

	#endif

	//Init blink timer
	blink1_period = calculateTimer(eeprom_read_word(&EE_Mem.settings.blink1));
	blink1_counter = blink1_period;
	blink2_period = calculateTimer(eeprom_read_word(&EE_Mem.settings.blink2));
	blink2_counter = blink2_period;

	pulse_counter = 0xFFFF;
	pulse_length = calculateTimer((uint16_t)eeprom_read_byte(&EE_Mem.settings.pulse));

	servo1Pulse = eeprom_read_byte(&EE_Mem.settings.servo1);
	servo2Pulse = eeprom_read_byte(&EE_Mem.settings.servo2);

	compareInt = 0;

	cli();

	#if defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)
	TCCR0A = 0;
	#endif
	IO_TIMER = IO_TIMER_PRESCALER;
	IO_TIMER_INT = IO_TIMER_OVERFLOW_INT_REG;

	sei();
}

void IO::blink1(){
	for(int i = 0; i < MAX_PORTS; i++){
		#ifdef IO_SPI
		writeData[i] = writeData[i] ^ blink1Mask[i];
		#else
		*_portOutputRegister(i) ^= blink1Mask[i];
		#endif
	}
}

void IO::blink2(){
	for(int i = 0; i < MAX_PORTS; i++){
		#ifdef IO_SPI
		writeData[i] = writeData[i] ^ blink2Mask[i];
		#else
		*_portOutputRegister(i) ^= blink2Mask[i];
		#endif
	}
}


void IO::pulse_high(){
	for(int i = 0; i < MAX_PORTS; i++){
		#ifdef IO_SPI
		writeData[i] = writeData[i] | pulseMask[i];
		#else
		*_portOutputRegister(i) |= pulseMask[i];
		#endif
	}

	#ifdef IO_SPI
	writeOutput();
	#endif

	//Set pulse period into timer
	cli();
	io.pulse_counter = IO_TIMER_REG + io.pulse_length;

	if(io.pulse_counter < 0x100){
		if(io.compareInt == 0){
			IO_TIMER_COMPA_REG = io.pulse_counter;
		}
		else if(IO_TIMER_COMPA_REG > (io.pulse_counter & 0xFF)){
			IO_TIMER_COMPA_REG = io.pulse_counter & 0xFF;
		}
		io.compareInt |= IO_INT_Pulse;
		IO_Enable_COMPA_INT;
	}
	sei();
}
void IO::pulse_low(){
	// uart.transmit("Pl\n",3);
	for(int i = 0; i < MAX_PORTS; i++){
		#ifdef IO_SPI
		writeData[i] = writeData[i] & ~pulseMask[i];
		#else
		*_portOutputRegister(i) &= ~pulseMask[i];
		#endif
		pulseMask[i] = 0;
	}
}

void IO::servo_low(){
	for(int i = 0; i < MAX_PORTS; i++){
		#ifdef IO_SPI
		writeData[i] = writeData[i] & ~(servo1Mask[i] | servo2Mask[i] | servo3Mask[i] | servo4Mask[i]);
		#else
		*_portOutputRegister(i) &= ~(servo1Mask[i] | servo2Mask[i] | servo3Mask[i] | servo4Mask[i]);
		#endif
	}
}

void IO::servo_high(uint8_t * mask){
	for(int i = 0; i < MAX_PORTS; i++){
		#ifdef IO_SPI
		writeData[i] = writeData[i] | mask[i];
		#else
		*_portOutputRegister(i) |= mask[i];
		#endif
	}
}

void IO::set_mask(uint8_t pin, enum IO_event type){
	#ifdef IO_SPI
	switch(type){
		case IO_event_Blink1:
			blink1Mask[pin/8] |= (1 << (pin % 8));
			break;
		case IO_event_Blink2:
			blink2Mask[pin/8] |= (1 << (pin % 8));
			break;
		case IO_event_Pulse:
			pulseMask[pin/8] |= (1 << (pin % 8));
			break;
		case IO_event_Servo1:
			servo1Mask[pin/8] |= (1 << (pin%8));
			break;
		case IO_event_Servo2:
			servo2Mask[pin/8] |= (1 << (pin%8));
			break;
		case IO_event_Servo3:
			servo3Mask[pin/8] |= (1 << (pin%8));
			break;
		case IO_event_Servo4:
			servo4Mask[pin/8] |= (1 << (pin%8));
			break;
		default:
			break;
	}
	#else
	switch(type){
		case IO_event_Blink1:
			blink1Mask[list[pin]/8] |= (1 << (list[pin] % 8));
			break;
		case IO_event_Blink2:
			blink2Mask[list[pin]/8] |= (1 << (list[pin] % 8));
			break;
		case IO_event_Pulse:
			pulseMask[list[pin]/8] |= (1 << (list[pin] % 8));
			break;
		case IO_event_Servo1:
			servo1Mask[list[pin]/8] |= (1 << (list[pin]%8));
			break;
		case IO_event_Servo2:
			servo2Mask[list[pin]/8] |= (1 << (list[pin]%8));
			break;
		case IO_event_Servo3:
			servo3Mask[list[pin]/8] |= (1 << (list[pin]%8));
			break;
		case IO_event_Servo4:
			servo4Mask[list[pin]/8] |= (1 << (list[pin]%8));
			break;
		default:
			break;
	}
	#endif
}

void IO::unset_mask(uint8_t pin, enum IO_event type){
	#ifdef IO_SPI
	switch(type){
		case IO_event_Blink1:
			blink1Mask[pin/8] &= ~(1 << (pin % 8));
			break;
		case IO_event_Blink2:
			blink2Mask[pin/8] &= ~(1 << (pin % 8));
			break;
		case IO_event_Pulse:
			pulseMask[pin/8] &= ~(1 << (pin % 8));
			break;
		case IO_event_Servo1:
			servo1Mask[pin/8] &= ~(1 << (pin%8));
			break;
		case IO_event_Servo2:
			servo2Mask[pin/8] &= ~(1 << (pin%8));
			break;
		case IO_event_Servo3:
			servo3Mask[pin/8] &= ~(1 << (pin%8));
			break;
		case IO_event_Servo4:
			servo4Mask[pin/8] &= ~(1 << (pin%8));
			break;
		default:
			break;
	}
	#else
	switch(type){
		case IO_event_Blink1:
			blink1Mask[list[pin]/8] &= ~(1 << (list[pin] % 8));
			break;
		case IO_event_Blink2:
			blink2Mask[list[pin]/8] &= ~(1 << (list[pin] % 8));
			break;
		case IO_event_Pulse:
			pulseMask[list[pin]/8] &= ~(1 << (list[pin] % 8));
			break;
		case IO_event_Servo1:
			servo1Mask[list[pin]/8] &= ~(1 << (list[pin]%8));
			break;
		case IO_event_Servo2:
			servo2Mask[list[pin]/8] &= ~(1 << (list[pin]%8));
			break;
		case IO_event_Servo3:
			servo3Mask[list[pin]/8] &= ~(1 << (list[pin]%8));
			break;
		case IO_event_Servo4:
			servo4Mask[list[pin]/8] &= ~(1 << (list[pin]%8));
			break;
		default:
			break;
	}
	#endif
}

uint16_t IO::calculateTimer(uint16_t mseconds){
	return ((mseconds * (F_CPU/1000)) / 1024);
}

#ifdef IO_SPI

void IO::writeOutput(){
	cli();
	LATCH_OUT_ENABLE; 

	for(int i = 0; i < MAX_PORTS; i++){
		/* Start transmission */
		SPCR |= (1<<MSTR);
		SPDR = writeData[i];
		// printHex(writeData[i]);
		/* Wait for transmission complete */
		while(!(SPSR & (1<<SPIF)));
	}

	// uart.transmit('\n');
	LATCH_OUT_DISABLE;
	sei();
}

void IO::copyInput(){
	for(int i = 0; i < MAX_PORTS; i++){
		oldreadData[i] = readData[i];
	}
}

void IO::readInput(){
	cli();
	IN_LOAD;
	_delay_ms(10);
	LATCH_IN_ENABLE;

	for(int i = 0; i < MAX_PORTS; i++){
		/* Start transmission */
		SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<SPR1);
		SPDR = 0;
		/* Wait for transmission complete */
		while(!(SPSR & (1<<SPIF)));

		readData[i] = SPDR; 
		// printHex(readData[i]);
	}

	// uart.transmit('\n');
	LATCH_IN_DISABLE;
	sei();
}

#else

void IO::readInput(){

}

#endif


void IO::set(uint8_t pin, enum IO_event func){
	if(func != IO_event_Blink1){
		unset_mask(pin, IO_event_Blink1);
	}
	if(func != IO_event_Blink2){
		unset_mask(pin, IO_event_Blink2);
	}
	switch(func){
		case IO_event_High:
			high(pin);
			break;
		case IO_event_Low:
			low(pin);
			break;
		case IO_event_Blink1:
			set_mask(pin, func);
			break;
		case IO_event_Blink2:
			set_mask(pin, func);
			break;
		case IO_event_Pulse:
			set_mask(pin, func);
			break;

		default:
			break;
	}
}

ISR(IO_TIMER_OVERFLOW_INT){
	if(io.compareInt){
		if(io.compareInt & IO_event_Blink1){
			io.blink1();
			io.compareInt &= ~(IO_event_Blink1);
			io.blink1_counter = 0x100 + io.blink1_period;
		}
		if(io.compareInt & IO_event_Blink2){
			io.blink2();
			io.compareInt &= ~(IO_event_Blink2);
			io.blink2_counter = 0x100 + io.blink2_period;
		}
		if(io.compareInt & IO_event_Pulse){
			io.pulse_low();
			io.compareInt &= ~(IO_event_Pulse);
			io.pulse_counter = 0xFFFF;
		}
	}
	io.blink1_counter -= 0x100;
	io.blink2_counter -= 0x100;

	if(io.pulse_counter != 0xFFFF){
		io.pulse_counter -= 0x100;
	}

	if(io.blink1_counter < 0x100){
		//Enable Comparator and set to counter & 0xFF
		// uart.transmit("B1 SETCOMPA ", 12);
		// printHex(io.blink1_counter);
		// uart.transmit('\n');
		io.compareInt |= IO_INT_BlinkA;
		cli();
		IO_TIMER_COMPA_REG = io.blink1_counter & 0xFF;
		IO_Enable_COMPA_INT;
		sei();
	}
	if(io.blink2_counter < 0x100){
		//Enable Comparator and set to counter & 0xFF
		//Check if Interrupt is allready set
		// uart.transmit("B2 SETCOMPA ", 12);
		// printHex(io.blink2_counter);
		// uart.transmit('\n');
		cli();
		if(io.compareInt == 0){
			IO_TIMER_COMPA_REG = io.blink2_counter & 0xFF;
		}
		else if(IO_TIMER_COMPA_REG > (io.blink2_counter & 0xFF)){
			IO_TIMER_COMPA_REG = io.blink2_counter & 0xFF;
		}
		io.compareInt |= IO_INT_BlinkB;
		IO_Enable_COMPA_INT;
		sei();
	}
	if(io.pulse_counter < 0x100){
		//Check if Interrupt is allready set
		// uart.transmit("P  SETCOMPA ", 12);
		// printHex(io.pulse_counter);
		// uart.transmit('\n');
		cli();
		if(io.compareInt == 0){
			IO_TIMER_COMPA_REG = io.pulse_counter & 0xFF;
		}
		else if(IO_TIMER_COMPA_REG > (io.pulse_counter & 0xFF)){
			IO_TIMER_COMPA_REG = io.pulse_counter & 0xFF;
		}
		io.compareInt |= IO_INT_Pulse;
		IO_Enable_COMPA_INT;
		sei();
	}
}

ISR(IO_TIMER_COMPA_INT){
	// uart.transmit("T1CA", 4);
	// uint8_t x = IO_TIMER_COMPA_REG;
	// printHex(x);
	// uart.transmit('\n');
	// printHex(io.compareInt);
	// uart.transmit("\nBA: ", 5);
	// printHex(io.blink1_counter >> 8);
	// printHex(io.blink1_counter & 0xFF);
	// uart.transmit("\nBB: ", 5);
	// printHex(io.blink2_counter >> 8);
	// printHex(io.blink2_counter & 0xFF);
	// uart.transmit("\nP: ", 4);
	// printHex(io.pulse_counter >> 8);
	// printHex(io.pulse_counter & 0xFF);
	// uart.transmit('\n');

	if(io.compareInt & IO_INT_BlinkA && io.blink1_counter <= IO_TIMER_COMPA_REG){
		io.blink1();
		io.blink1_counter = IO_TIMER_COMPA_REG + io.blink1_period;

		// Disable compa if no other counter is needing compa
		// Otherwise set OCRA0 to the counter
		io.compareInt &= ~(IO_INT_BlinkA);
		if(io.compareInt == 0){
			cli();
			IO_Disable_COMPA_INT;
			IO_TIMER_COMPA_REG = 0xFF;
			sei();
			#ifdef IO_SPI
			// uart.transmit("blnkA\n",6);
			io.writeOutput();
			#endif
			return;
		}
	}
	if(io.compareInt & IO_INT_BlinkB && io.blink2_counter <= IO_TIMER_COMPA_REG){
		io.blink2();
		io.blink2_counter = IO_TIMER_COMPA_REG + io.blink2_period;

		// Disable compa if no other counter is needing compa
		// Otherwise set OCRA0 to the counter
		io.compareInt &= ~(IO_INT_BlinkB);
		if(io.compareInt == 0){
			cli();
			IO_Disable_COMPA_INT;
			IO_TIMER_COMPA_REG = 0xFF;
			sei();
			#ifdef IO_SPI
			// uart.transmit("blnkB\n",6);
			io.writeOutput();
			#endif
			return;
		}
	}

	if(io.compareInt & IO_INT_Pulse && io.pulse_counter <= IO_TIMER_COMPA_REG){
		io.pulse_low();
		io.pulse_counter = 0xFFFF; // Disable Pulse

		// Disable compa if no other counter is needing compa
		// Otherwise set OCRA0 to the counter
		io.compareInt &= ~(IO_INT_Pulse);
		if(io.compareInt == 0){
			cli();
			IO_Disable_COMPA_INT;
			IO_TIMER_COMPA_REG = 0xFF;
			sei();
			#ifdef IO_SPI
			// uart.transmit("pulse\n",6);
			io.writeOutput();
			#endif
			return;
		}
	}

	// #ifdef IO_SPI
	// uart.transmit("compA\n",6);
	// io.writeOutput();
	// #endif

	//Calculate next compare interupt
	IO_TIMER_COMPA_REG = 0xFF;
	cli();
	if(io.compareInt & IO_INT_BlinkA && io.blink1_counter < IO_TIMER_COMPA_REG){
		IO_TIMER_COMPA_REG = io.blink1_counter;
	}
	if(io.compareInt & IO_INT_BlinkB && io.blink2_counter < IO_TIMER_COMPA_REG){
		IO_TIMER_COMPA_REG = io.blink2_counter;
	}
	if(io.compareInt & IO_INT_Pulse && io.pulse_counter < IO_TIMER_COMPA_REG){
		IO_TIMER_COMPA_REG = io.pulse_counter;
	}

	if(io.compareInt == 0){
		IO_Disable_COMPA_INT;
	}
	sei();
}
