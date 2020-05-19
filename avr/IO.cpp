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


inline void IO::high(uint8_t pin){
	#ifdef IO_SPI
	writeData[pin/8] |= 1 << (pin % 8);
	#else
	*_portOutputRegister(list[pin]/8) |= 1 << (list[pin] % 8);
	#endif
}

inline void IO::low(uint8_t pin){
	#ifdef IO_SPI
	writeData[pin/8] &= ~(1 << (pin % 8));
	#else
	*_portOutputRegister(list[pin]/8) &= ~(1 << (list[pin] % 8));
	#endif
}

inline void IO::toggle(uint8_t pin){
	#ifdef IO_SPI
	writeData[pin/8] ^= (1 << (pin % 8));
	#else
	*_portOutputRegister(list[pin]/8) ^= 1 << (list[pin] % 8);
	#endif
}


inline void IO::out(uint8_t pin){
	#ifdef IO_SPI
	return (void)pin;
	#else
	*_portModeRegister(list[pin]/8) |= 1 << (list[pin] % 8);
	readMask[list[pin]/8] &= ~(1 << (list[pin] % 8));
	#endif
}

inline void IO::in(uint8_t pin){
	#ifdef IO_SPI
	return (void)pin;
	#else
	*_portModeRegister(list[pin]/8) &= ~(1 << (list[pin] % 8));
	readMask[list[pin]/8] |= (1 << (list[pin] % 8));
	#endif
}

inline uint8_t IO::read(uint8_t pin){
	#ifdef IO_SPI
	return readData[pin/8] & (1 << (pin % 8));
	#else
	uint8_t pinmask = (1 << (list[pin] % 8));
	uint8_t port = list[pin]/8;
	return (*(volatile uint16_t *)pinlist[port] & pinmask);
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

  	uart.transmit("IO_SPI\n",7);

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

	for(uint8_t i = 0; i < MAX_PORTS*8; i++){
		typelist[i] = IO_Output;
	}

	writeOutput();

	#else // not defined IO_SPI

	uart.transmit("IO INIT\n", 8);

	// Init io ports
	for(uint8_t i = 0; i < MAX_PINS; i++){
		enum e_IO_type type = (enum e_IO_type)eeprom_read_byte((const uint8_t *)&EE_Mem.IO[i].type);
		union u_IO_event def;
		def.value = eeprom_read_byte((const uint8_t *)&EE_Mem.IO[i]._default);

		uart.transmit(list[i] / 8, HEX, 2);
		uart.transmit('\t');
		uart.transmit(1 << (list[i] % 8), HEX, 2);
		uart.transmit('\t');
		uart.transmit((uint8_t)type, HEX, 2);
		uart.transmit('\t');
		uart.transmit(def.value, HEX, 2);
		uart.transmit('\n');

		typelist[i] = type;

		if(type == IO_Undefined)
			in(i);
		else if(type <= IO_Output_PWM){
			out(i);
			set(i, def);
		}
		else{
			in(i);
			def.output = IO_event_High;
			set(i, def);
		}
	}

	for(uint8_t i = 0; i < MAX_PORTS; i++){
		uart.transmit(readMask[i], HEX, 2);
	}

	#endif

	//Init blink timer
	blink1_period = calculateTimer(eeprom_read_word(&EE_Mem.settings.blink1));
	blink1_counter = blink1_period;
	blink2_period = calculateTimer(eeprom_read_word(&EE_Mem.settings.blink2));
	blink2_counter = blink2_period;

	pulse_counter = 0xFFFF;
	uint8_t value = eeprom_read_byte(&EE_Mem.settings.pulse);
	pulse_length = calculateTimer((uint16_t)(value*10));

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

	#ifdef IO_SPI
	writeOutput();
	#endif
}

void IO::blink2(){
	for(int i = 0; i < MAX_PORTS; i++){
		#ifdef IO_SPI
		writeData[i] = writeData[i] ^ blink2Mask[i];
		#else
		*_portOutputRegister(i) ^= blink2Mask[i];
		#endif
	}

	#ifdef IO_SPI
	writeOutput();
	#endif
}


void IO::pulse_high(){
	for(int i = 0; i < MAX_PORTS; i++){
		#ifdef IO_SPI
		writeData[i] |= pulseMask[i];
		#else
		*_portOutputRegister(i) |= pulseMask[i];
		#endif
	}

	#ifdef IO_SPI
	writeOutput();
	#endif

	//Set pulse period into timer
	// cli();
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
	// sei();
}
void IO::pulse_low(){
	// uart.transmit("Pl\n",3);
	for(int i = 0; i < MAX_PORTS; i++){
		#ifdef IO_SPI
		writeData[i] &= ~pulseMask[i];
		#else
		*_portOutputRegister(i) &= ~pulseMask[i];
		#endif
		pulseMask[i] = 0;
	}

	#ifdef IO_SPI
	writeOutput();
	#endif
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

void IO::set_mask(uint8_t pin, uint8_t * mask){
	#ifdef IO_SPI
	mask[pin/8] |= (1 << (pin % 8));
	#else
	mask[list[pin]/8] |= (1 << (list[pin] % 8));
	#endif
}

void IO::unset_mask(uint8_t pin, uint8_t * mask){
	#ifdef IO_SPI
	mask[pin/8] &= ~(1 << (pin % 8));
	#else
	mask[list[pin]/8] &= ~(1 << (list[pin] % 8));
	#endif
}

uint16_t IO::calculateTimer(uint16_t mseconds){
	return (uint16_t)((mseconds * (F_CPU/1000)) / 1024);
}

#ifdef IO_SPI

void IO::writeOutput(){
	LATCH_OUT_ENABLE; 

	for(int i = 0; i < MAX_PORTS; i++){
		/* Start transmission */
		SPCR |= (1<<MSTR);
		SPDR = writeData[i];
		/* Wait for transmission complete */
		while(!(SPSR & (1<<SPIF)));
	}

	LATCH_OUT_DISABLE;
}

void IO::readInput(){
	IN_LOAD;
	_delay_us(10);
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
}

#else

void IO::readInput(){
	for(uint8_t i = 0; i < MAX_PORTS; i++){
		readData[i] = (*(volatile uint16_t *)pinlist[i]) & readMask[i];
	}
}

#endif

void IO::copyInput(){
	for(int i = 0; i < MAX_PORTS; i++){
		oldreadData[i] = readData[i];
	}
}


void IO::set(uint8_t pin, union u_IO_event func){
	enum e_IO_type type = typelist[pin];

	uart.transmit("SetIO ", 6);
	uart.transmit(pin, HEX, 2);
	uart.transmit((uint8_t)type, HEX, 2);
	uart.transmit(func.value, HEX, 2);
	uart.transmit('\n');

	if(type == IO_Output){
		if(func.output == IO_event_High)
			high(pin);
		else if(func.output == IO_event_Low)
			low(pin);
		else if(func.output == IO_event_Toggle)
			toggle(pin);
		else if(func.output == IO_event_Pulse){
			set_mask(pin, pulseMask);
		}
	}
	else if(type == IO_Output_Blink){
		if(func.blink != IO_event_Blink1)
			unset_mask(pin, blink1Mask);
		if(func.blink != IO_event_Blink2)
			unset_mask(pin, blink2Mask);

		if(func.blink == IO_event_B_High)
			high(pin);
		else if(func.blink == IO_event_B_Low)
			low(pin);
		else if(func.blink == IO_event_Blink1)
			set_mask(pin, blink1Mask);
		else if(func.blink == IO_event_Blink2)
			set_mask(pin, blink2Mask);

	}
	else if(type == IO_Output_Servo){
		
	}
	else if(type == IO_Output_PWM){
		
	}
}

ISR(IO_TIMER_OVERFLOW_INT, ISR_NOBLOCK){

	// If compare match failed
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
		IO_TIMER_COMPA_REG = io.blink1_counter & 0xFF;
		IO_Enable_COMPA_INT;
	}
	if(io.blink2_counter < 0x100){
		//Enable Comparator and set to counter & 0xFF
		//Check if Interrupt is allready set
		// uart.transmit("B2 SETCOMPA ", 12);
		// printHex(io.blink2_counter);
		// uart.transmit('\n');
		if(io.compareInt == 0){
			IO_TIMER_COMPA_REG = io.blink2_counter & 0xFF;
		}
		else if(IO_TIMER_COMPA_REG > (io.blink2_counter & 0xFF)){
			IO_TIMER_COMPA_REG = io.blink2_counter & 0xFF;
		}
		io.compareInt |= IO_INT_BlinkB;
		IO_Enable_COMPA_INT;
	}
	if(io.pulse_counter < 0x100){
		//Check if Interrupt is allready set
		// uart.transmit("P  SETCOMPA ", 12);
		// printHex(io.pulse_counter);
		// uart.transmit('\n');
		if(io.compareInt == 0){
			IO_TIMER_COMPA_REG = io.pulse_counter & 0xFF;
		}
		else if(IO_TIMER_COMPA_REG > (io.pulse_counter & 0xFF)){
			IO_TIMER_COMPA_REG = io.pulse_counter & 0xFF;
		}
		io.compareInt |= IO_INT_Pulse;
		IO_Enable_COMPA_INT;
	}
}

ISR(IO_TIMER_COMPA_INT, ISR_NOBLOCK){
	if(io.compareInt & IO_INT_BlinkA && io.blink1_counter <= IO_TIMER_COMPA_REG){
		io.blink1();
		io.blink1_counter = IO_TIMER_COMPA_REG + io.blink1_period;

		io.compareInt &= ~(IO_INT_BlinkA);
	}
	if(io.compareInt & IO_INT_BlinkB && io.blink2_counter <= IO_TIMER_COMPA_REG){
		io.blink2();
		io.blink2_counter = IO_TIMER_COMPA_REG + io.blink2_period;

		io.compareInt &= ~(IO_INT_BlinkB);
	}

	if(io.compareInt & IO_INT_Pulse && io.pulse_counter <= IO_TIMER_COMPA_REG){
		io.pulse_low();
		io.pulse_counter = 0xFFFF; // Disable Pulse

		io.compareInt &= ~(IO_INT_Pulse);
	}

	// #ifdef IO_SPI
	if(io.compareInt == 0){
		IO_Disable_COMPA_INT;
		IO_TIMER_COMPA_REG = 0xFF;
		return;
	}
	// uart.transmit("compA\n",6);
	// io.writeOutput();
	// #endif

	//Calculate next compare interupt
	IO_TIMER_COMPA_REG = 0xFF;
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
}
