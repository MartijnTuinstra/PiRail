#include <string.h>

#include "avr/io.h"
#include "avr/interrupt.h"
#include "IO.h"

#include "RNet.h"
#include "main_node.h"
#include "eeprom_layout.h"

#include "util/delay.h"

IO io;

#if defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)

const uint16_t PROGMEM _PORT[] = {
	0,
	(uint16_t) &PORTB,
	(uint16_t) &PORTC,
	(uint16_t) &PORTD
};

const uint16_t PROGMEM _DDR[] = {
	0,
	(uint16_t) &DDRB,
	(uint16_t) &DDRC,
	(uint16_t) &DDRD
};

const uint16_t PROGMEM _PIN[] = {
	0,
	(uint16_t) &PINB,
	(uint16_t) &PINC,
	(uint16_t) &PIND
};

uint8_t pin_to_Port[] = {
	// 0 - 7
	PD, PD, PD, PD, PD, PD, PC, PC,
	// 8 - 15
	PC, PC, PC, PC, PB
};

uint8_t pin_to_BIT[] = {
	// 0 - 7
	4, 3, 5, 2, 1, 0, 5, 4, 
	// 8 - 15
	3, 2, 1, 0, 5
};

#elif defined(__AVR_ATmega64A__)

const uint16_t PROGMEM _PORT[] = {
	(uint16_t) &PORTA,
	(uint16_t) &PORTB,
	(uint16_t) &PORTC,
	(uint16_t) &PORTD,
	(uint16_t) &PORTE,
	(uint16_t) &PORTF,
	(uint16_t) &PORTG
};

const uint16_t PROGMEM _DDR[] = {
	(uint16_t) &DDRA,
	(uint16_t) &DDRB,
	(uint16_t) &DDRC,
	(uint16_t) &DDRD,
	(uint16_t) &DDRE,
	(uint16_t) &DDRF,
	(uint16_t) &DDRG
};

const uint16_t PROGMEM _PIN[] = {
	(uint16_t) &PINA,
	(uint16_t) &PINB,
	(uint16_t) &PINC,
	(uint16_t) &PIND,
	(uint16_t) &PINE,
	(uint16_t) &PINF,
	(uint16_t) &PING
};

uint8_t pin_to_Port[] = {
	// 0 - 7
	PG, PG, PB, PD, PB, PB, PB, PB,
	// 8 - 15
	PE, PE, PE, PE, PE, PE, PE, PE,
	// 16 - 23
	PF, PF, PF, PF, PF, PF, PF, PF,
	// 24 - 31
	PA, PA, PA, PA, PA, PA, PA, PA,
	// 32 - 39
	PC, PC, PC, PC, PC, PC, PC, PC
};

uint8_t pin_to_BIT[] = {
	// 0 - 7
	4, 3, 7, 0, 6, 5, 4, 0,
	// 8 - 15
	7, 6, 5, 4, 3, 2, 1, 0,
	// 16 - 23
	0, 1, 2, 3, 4, 5, 6, 7,
	// 24 - 31
	0, 1, 2, 3, 4, 5, 6, 7,
	// 32 - 39
	7, 6, 5, 4, 3, 2, 1, 0
};

#elif defined(__AVR_ATmega2560__)

const uint16_t PROGMEM _PORT[] = {
	(uint16_t) &PORTA,
	(uint16_t) &PORTB,
	(uint16_t) &PORTC,
	(uint16_t) &PORTD,
	(uint16_t) &PORTE,
	(uint16_t) &PORTF,
	(uint16_t) &PORTG,
	(uint16_t) &PORTH,
	(uint16_t) &PORTJ,
	(uint16_t) &PORTK,
	(uint16_t) &PORTL
};

const uint16_t PROGMEM _DDR[] = {
	(uint16_t) &DDRA,
	(uint16_t) &DDRB,
	(uint16_t) &DDRC,
	(uint16_t) &DDRD,
	(uint16_t) &DDRE,
	(uint16_t) &DDRF,
	(uint16_t) &DDRG,
	(uint16_t) &DDRH,
	(uint16_t) &DDRJ,
	(uint16_t) &DDRK,
	(uint16_t) &DDRL
};

const uint16_t PROGMEM _PIN[] = {
	(uint16_t) &PINA,
	(uint16_t) &PINB,
	(uint16_t) &PINC,
	(uint16_t) &PIND,
	(uint16_t) &PINE,
	(uint16_t) &PINF,
	(uint16_t) &PING,
	(uint16_t) &PINH,
	(uint16_t) &PINJ,
	(uint16_t) &PINK,
	(uint16_t) &PINL
};

uint8_t pin_to_Port[] = {
	// 0 - 7
	PE, PE, PE, PE, PE, PE, PE, PE,
	// 8 - 15
	PH, PH, PH, PH, PH, PH, PH, PH,
	// 16 - 23
	PD, PD, PD, PB, PB, PB, PB, PB,
	// 24 - 31
	PL, PL, PL, PL, PL, PL, PL, PL,
	// 32 - 39
	PC, PC, PC, PC, PC, PC, PC, PC,
	// 40 - 47
	PJ, PJ, PJ, PJ, PJ, PJ, PJ, PJ,
	// 48 - 55
	PA, PA, PA, PA, PA, PA, PA, PA,
	// 56 - 63
	PK, PK, PK, PK, PK, PK, PK, PK,
	// 64 - 81
	PF, PF, PF, PF, PF, PF, PF, PF
};

uint8_t pin_to_BIT[] = {
	// 0 - 7
	7, 6, 5, 4, 3, 2, 1, 0,
	// 8 - 15
	7, 6, 5, 4, 3, 2, 1, 0,
	// 16 - 23
	7, 6, 5, 0, 4, 5, 6, 7,
	// 24 - 31
	7, 6, 5, 4, 3, 2, 1, 0,
	// 32 - 39
	7, 6, 5, 4, 3, 2, 1, 0,
	// 40 - 47
	7, 6, 5, 4, 3, 2, 1, 0,
	// 48 - 55
	0, 1, 2, 3, 4, 5, 6, 7,
	// 56 - 63
	0, 1, 2, 3, 4, 5, 6, 7,
	// 64 - 81
	0, 1, 2, 3, 4, 5, 6, 7
};
#endif

void set_out(uint8_t pin){
	*portModeRegister(pin_to_Port[pin]) |= (1 << pin_to_BIT[pin]);
	return;
}
void set_in(uint8_t pin){
	*portModeRegister(pin_to_Port[pin]) &= ~(1 << pin_to_BIT[pin]);
	return;
}

void set_high(uint8_t pin){
	*portOutputRegister(pin_to_Port[pin]) |= (1 << pin_to_BIT[pin]);
	return;
}
void set_low(uint8_t pin){
	*portOutputRegister(pin_to_Port[pin]) &= ~(1 << pin_to_BIT[pin]);
	return;
}

void set_toggle(uint8_t pin){
	*portOutputRegister(pin_to_Port[pin]) ^= (1 << pin_to_BIT[pin]);
	return;
}

uint8_t read(uint8_t pin){
	if (portInputRegister(pin_to_Port[pin])) return 1;
	return 0;
}


void IO::high(uint8_t pin){
	#ifdef IO_SPI
	writeData[pin/8] |= 1 << (pin % 8);
	#else
	*portOutputRegister(list[pin]/8) |= 1 << (list[pin] % 8);
	#endif
}

void IO::low(uint8_t pin){
	#ifdef IO_SPI
	writeData[pin/8] &= ~(1 << (pin % 8));
	#else
	*portOutputRegister(list[pin]/8) &= ~(1 << (list[pin] % 8));
	#endif
}

void IO::toggle(uint8_t pin){
	#ifdef IO_SPI
	writeData[pin/8] ^= 1 << (pin % 8);
	#else
	*portOutputRegister(list[pin]/8) ^= 1 << (list[pin] % 8);
	#endif
}


void IO::out(uint8_t pin){
	#ifdef IO_SPI
	return (void)pin;
	#else
	*portModeRegister(list[pin]/8) |= 1 << (list[pin] % 8);
	#endif
}

void IO::in(uint8_t pin){
	#ifdef IO_SPI
	return (void)pin;
	#else
	*portModeRegister(list[pin]/8) &= ~(1 << (list[pin] % 8));
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

#define LATCH_OUT_ENABLE  PORTD &= ~(1 << 3)
#define LATCH_OUT_DISABLE PORTD |=  (1 << 3)

#define LATCH_IN_ENABLE   PORTC &= ~(1 << 0)
#define LATCH_IN_DISABLE  PORTC |=  (1 << 0)
#define IN_LOAD           PORTC &= ~(1 << 2); \
                          _delay_ms(1); \
                          PORTC |=  (1 << 2)

#endif

void IO::init(){
	memset(blink1Mask, 0, MAX_PORTS);
	memset(blink2Mask, 0, MAX_PORTS);

	memset(servo1Mask, 0, MAX_PORTS);
	memset(servo2Mask, 0, MAX_PORTS);
	memset(servo3Mask, 0, MAX_PORTS);
	memset(servo4Mask, 0, MAX_PORTS);

	memset(readMask, 0, MAX_PORTS);
	memset(readData, 0, MAX_PORTS);

	#ifdef IO_SPI

	memset(writeData, 0, MAX_PORTS);

	#if defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)
	// set mosi and sck output
	// DDRB = (1<<DDB2)|(1<<DDB3)|(1<<DDB5);
	_set_out(PORTB, 5); //SCK
	_set_out(PORTB, 3); //MOSI
	_set_in(PORTB, 4);  //MISO
	_set_out(PORTB, 2); //SS

	_set_high(PORTB, 2);
	_set_high(PORTC, 2);

	#endif

	// enable SPI at fsck/128
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<SPR1);

	LATCH_OUT_DISABLE;
	LATCH_IN_DISABLE;

	writeOutput();

	#endif

	//Init blink timer
	blink1_period = calculateTimer(eeprom_read_word(&EE_Mem.settings.blink1));
	blink1_counter = blink1_period;
	blink2_period = calculateTimer(eeprom_read_word(&EE_Mem.settings.blink2));
	blink2_counter = blink1_period;

	pulse_length = calculateTimer((uint16_t)eeprom_read_byte(&EE_Mem.settings.pulse));

	IO_TIMER_REG = 0;
	IO_TIMER = IO_TIMER_PRESCALER;
	IO_TIMER_INT = IO_TIMER_OVERFLOW_INT_REG;
}


void IO::set_blink1(uint8_t pin){
	blink1Mask[list[pin]/8] |= 1 << (list[pin] % 8);
}
void IO::set_blink2(uint8_t pin){
	blink2Mask[list[pin]/8] |= 1 << (list[pin] % 8);
}
void IO::unset_blink1(uint8_t pin){
	blink1Mask[list[pin]/8] &= ~(1 << (list[pin] % 8));
}
void IO::unset_blink2(uint8_t pin){
	blink2Mask[list[pin]/8] &= ~(1 << (list[pin] % 8));
}

void IO::blink1(){
	for(int i = 0; i < MAX_PORTS; i++){
		#ifdef IO_SPI
		writeData[i] = writeData[i] ^ blink1Mask[i];
		#else
		*portOutputRegister(i) ^= blink1Mask[i];
		#endif
	}
}

void IO::blink2(){
	for(int i = 0; i < MAX_PORTS; i++){
		#ifdef IO_SPI
		writeData[i] = writeData[i] ^ blink2Mask[i];
		#else
		*portOutputRegister(i) ^= blink2Mask[i];
		#endif
	}
}


void IO::pulse_set(uint8_t pin){
	pulseMask[list[pin]/8] |= 1 << (list[pin] % 8);
}
void IO::pulse_high(){
	for(int i = 0; i < MAX_PORTS; i++){
		#ifdef IO_SPI
		writeData[i] = writeData[i] ^ pulseMask[i];
		#else
		*portOutputRegister(i) ^= pulseMask[i];
		#endif
	}

	//Set pulse period into timer
	io.pulse_counter = (0xFF - IO_TIMER_REG) + io.pulse_length;

	if(io.pulse_counter < 0x100){
		IO_TIMER_COMPA_REG = io.blink1_counter;
		IO_TIMER_INT |= IO_TIMER_COMPA_INT_REG;
	}
}
void IO::pulse_low(){
	for(int i = 0; i < MAX_PORTS; i++){
		#ifdef IO_SPI
		writeData[i] = writeData[i] ^ pulseMask[i];
		#else
		*portOutputRegister(i) ^= pulseMask[i];
		#endif
		pulseMask[i] = 0;
	}
}

uint16_t IO::calculateTimer(uint16_t mseconds){
	return ((mseconds * (F_CPU/1000)) / 1024);
}

#ifdef IO_SPI

void IO::writeOutput(){
	LATCH_OUT_ENABLE; 

	for(int i = 0; i < MAX_PORTS; i++){
		/* Start transmission */
		SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<SPR1);
		SPDR = writeData[i];
		printHex(writeData[i]);
		/* Wait for transmission complete */
		while(!(SPSR & (1<<SPIF)));
	}

	uart_putchar('\n');
	LATCH_OUT_DISABLE;
}

void IO::readInput(){
	IN_LOAD;
	_delay_ms(1);
	LATCH_IN_ENABLE;

	for(int i = 0; i < MAX_PORTS; i++){
		/* Start transmission */
		SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<SPR1);
		SPDR = 0;
		/* Wait for transmission complete */
		while(!(SPSR & (1<<SPIF)));

		readData[i] = SPDR; 
		printHex(readData[i]);
	}

	uart_putchar('\n');
	LATCH_IN_DISABLE;
}

#else

void IO::readInput(){

}

#endif


void IO::set(uint8_t pin, enum IO_event func){
	if(func != IO_event_Blink1){
		unset_blink1(pin);
	}
	if(func != IO_event_Blink2){
		unset_blink2(pin);
	}
	switch(func){
		case IO_event_High:
			high(pin);
			break;
		case IO_event_Low:
			low(pin);
			break;
		case IO_event_Blink1:
			set_blink1(pin);
			break;
		case IO_event_Blink2:
			set_blink2(pin);
			break;
		case IO_event_Pulse:
			pulse_set(pin);
			break;

		default:
			break;
	}
}

ISR(IO_TIMER_OVERFLOW_INT){
	io.blink1_counter -= 0x100;
	io.blink2_counter -= 0x100;

	if(io.pulse_counter != 0){
		io.pulse_counter -= 0x100;
	}

	if(io.blink1_counter < 0x100){
		//Enable Comparator and set to counter & 0xFF
		IO_TIMER_COMPA_REG = io.blink1_counter;
		IO_TIMER_INT |= IO_TIMER_COMPA_INT_REG;
	}
	if(io.blink2_counter < 0x100){
		//Enable Comparator and set to counter & 0xFF
		//Check if Interrupt is allready set
		if(!(IO_TIMER_INT & IO_TIMER_COMPA_INT_REG)){
			IO_TIMER_COMPA_REG = io.blink2_counter;
		}
		else if(IO_TIMER_COMPA_REG > io.blink2_counter){
			IO_TIMER_COMPA_REG = io.blink2_counter;
		}
		IO_TIMER_INT |= IO_TIMER_COMPA_INT_REG;
	}
	if(io.pulse_counter < 0x100 && io.pulse_counter != 0xFFFF){
		//Check if Interrupt is allready set
		if(!(IO_TIMER_INT & IO_TIMER_COMPA_INT_REG)){
			IO_TIMER_COMPA_REG = io.pulse_counter;
		}
		else if(IO_TIMER_COMPA_REG > io.pulse_counter){
			IO_TIMER_COMPA_REG = io.pulse_counter;
		}
		IO_TIMER_INT |= IO_TIMER_COMPA_INT_REG;
	}
}

ISR(IO_TIMER_COMPA_INT){
	if(io.blink1_counter <= IO_TIMER_REG){
		io.blink1();
		io.blink1_counter = (0xFF - IO_TIMER_REG) + io.blink1_period;

		// Disable compa if no other counter is needing compa
		// Otherwise set OCRA0 to the counter
		if(io.blink2_counter > 0xFF && io.pulse_counter > 0xFF){
			IO_TIMER_INT &= ~(IO_TIMER_COMPA_INT_REG);
		}
		else{
			if(io.blink2_counter < io.pulse_counter){
				IO_TIMER_COMPA_REG = io.blink2_counter;
			}
			else{
				IO_TIMER_COMPA_REG = io.pulse_counter;
			}
		}
		return;
	}
	if(io.blink2_counter <= IO_TIMER_REG){
		io.blink2();
		io.blink2_counter = (0xFF - IO_TIMER_REG) + io.blink2_period;

		// Disable compa if no other counter is needing compa
		// Otherwise set OCRA0 to the counter
		if(io.blink1_counter > 0xFF && io.pulse_counter > 0xFF){
			IO_TIMER_INT &= ~(IO_TIMER_COMPA_INT_REG);
		}
		else{
			if(io.blink1_counter < io.pulse_counter){
				IO_TIMER_COMPA_REG = io.blink1_counter;
			}
			else{
				IO_TIMER_COMPA_REG = io.pulse_counter;
			}
		}
		return;
	}

	if(io.pulse_counter <= IO_TIMER_REG){
		io.pulse_low();
		io.pulse_counter = 0xFFFF;

		// Disable compa if no other counter is needing compa
		// Otherwise set OCRA0 to the counter
		if(io.blink1_counter > 0xFF && io.blink2_counter > 0xFF){
			IO_TIMER_INT &= ~(IO_TIMER_COMPA_INT_REG);
		}
		else{
			if(io.blink1_counter < io.blink2_counter){
				IO_TIMER_COMPA_REG = io.blink1_counter;
			}
			else{
				IO_TIMER_COMPA_REG = io.blink2_counter;
			}
		}
		return;
	}
	IO_TIMER_INT &= ~(IO_TIMER_COMPA_INT_REG);
}