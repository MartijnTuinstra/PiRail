#ifndef INCLUDED_IO_H
#define INCLUDED_IO_H

#include "prgmem.h"
#include "avr/interrupt.h"

#define PORT_(port) PORT ## port
#define DDR_(port)  DDR  ## port
#define PIN_(port)  PIN  ## port

#define PORT(port) PORT_(port)
#define DDR(port)  DDR_(port)
#define PIN(port)  PIN_(port)

#define _set_in(PORT, PIN) PORT &= ~(1 << PIN)
#define _set_out(PORT, PIN)  PORT |= (1<< PIN)

#define _set_high(PORT, PIN) PORT |= (1 << PIN)
#define _set_low(PORT, PIN)  PORT &= ~(1<< PIN)
#define _set_toggle(PORT, PIN) PORT ^= (1 << PIN)
#define _read_pin(PORT, PIN)     PORT & (1 << PIN)

void set_out(uint8_t pin);
void set_in(uint8_t pin);

void set_high(uint8_t pin);
void set_low(uint8_t pin);
void set_toggle(uint8_t pin);

uint8_t read(uint8_t pin);


enum IO_event {
  IO_event_High,
  IO_event_Low,
  IO_event_Pulse,
  IO_event_Blink1,
  IO_event_Blink2,

  IO_event_Servo1,
  IO_event_Servo2,
  IO_event_Servo3,
  IO_event_Servo4,
  IO_event_PWM1,
  IO_event_PWM2,
  IO_event_PWM3,
  IO_event_PWM4
};

#define PA 0
#define PB 1
#define PC 2
#define PD 3
#define PE 4
#define PF 5
#define PG 6
#define PH 7
#define PJ 8
#define PK 7
#define PL 8

#define pgm_read_word(address_short) __LPM_word((uint16_t)(address_short))

#define portOutputRegister(P) ( (volatile uint8_t *)( pgm_read_word( _PORT + (P))) )
#define portModeRegister(P) ( (volatile uint8_t *)( pgm_read_word( _DDR + (P))) )
#define portInputRegister(P) ( (volatile uint8_t *)( pgm_read_word( _PIN + (P))) )

#define _portOutputRegister(P) ( (volatile uint8_t *)( pgm_read_word( portlist + (P))) )
#define _portModeRegister(P) ( (volatile uint8_t *)( pgm_read_word( ddrlist + (P))) )
#define _portInputRegister(P) ( (volatile uint8_t *)( pgm_read_word( pinlins + (P))) )

extern const uint16_t PROGMEM _PORT[];

extern const uint16_t PROGMEM _DDR[];

extern const uint16_t PROGMEM _PIN[];

extern uint8_t pin_to_Port[];

extern uint8_t pin_to_BIT[];


#if defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)

#define MAX_PINS 13

#elif defined(__AVR_ATmega64A__)

#define MAX_PINS 40

#elif defined(__AVR_ATmega2560__)

#define MAX_PINS 72

#endif
#endif

#if defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)
	#define MAX_PORTS 3

	const uint16_t PROGMEM portlist[] = {
		(uint16_t) &PORTB,
		(uint16_t) &PORTC,
		(uint16_t) &PORTD,
	};
	const uint16_t PROGMEM ddrlist[] = {
		(uint16_t) &DDRB,
		(uint16_t) &DDRC,
		(uint16_t) &DDRD,
	};
	const uint16_t PROGMEM pinlist[] = {
		(uint16_t) &PINB,
		(uint16_t) &PINC,
		(uint16_t) &PIND,
	};
	const uint8_t list[] = {
		// 0, 1, 2, 3, 4, 5
		20, 19, 21, 18, 17, 16,
		// 6, 7, 8, 9, 10, 11
		13, 12, 11, 10, 9, 8,

		// 12 // DEBUG
		5
	};
#elif defined(__AVR_ATmega64A__)
	#define MAX_PORTS 7

	const uint16_t PROGMEM portlist[] = {
		(uint16_t) &PORTA,
		(uint16_t) &PORTB,
		(uint16_t) &PORTC,
		(uint16_t) &PORTD,
		(uint16_t) &PORTE,
		(uint16_t) &PORTF,
		(uint16_t) &PORTG,
	};
	const uint16_t PROGMEM ddrlist[] = {
		(uint16_t) &DDRA,
		(uint16_t) &DDRB,
		(uint16_t) &DDRC,
		(uint16_t) &DDRD,
		(uint16_t) &DDRE,
		(uint16_t) &DDRF,
		(uint16_t) &DDRG,
	};
	const uint16_t PROGMEM pinlist[] = {
		(uint16_t) &PINA,
		(uint16_t) &PINB,
		(uint16_t) &PINC,
		(uint16_t) &PIND,
		(uint16_t) &PINE,
		(uint16_t) &PINF,
		(uint16_t) &PING,
	};
	const uint8_t list[] = {
		// 0, 1 (PG)
		52, 51,
		// 2 (PB)
		15,
		// 3 (PD)
		24,
		// 4, 5, 6, 7 (PB)
		14, 13, 12, 8, 
		// 8, 9, 10, 11, 12, 13, 14, 15 (PE)
		39, 38, 37, 36, 35, 34, 33, 32,
		// 16, 17, 18, 19, 20, 21, 22, 23 (PF)
		40, 41, 42, 43, 44, 45, 46, 47,
		// 24, 25, 26, 27, 28, 29, 30, 31  (PA)
		0, 1, 2, 3, 4, 5, 6, 7,
		// 32, 33, 34, 35, 36, 37, 38, 39 (PC)
		16, 17, 18, 19, 20, 21, 22, 23
	};
#elif defined(__AVR_ATmega2560__)
	#define MAX_PORTS 10

	const uint16_t PROGMEM portlist[] = {
		(uint16_t) &PORTA,
		(uint16_t) &PORTB,
		(uint16_t) &PORTC,
		(uint16_t) &PORTD,
		(uint16_t) &PORTE,
		(uint16_t) &PORTF,
		(uint16_t) &PORTH,
		(uint16_t) &PORTJ,
		(uint16_t) &PORTK,
		(uint16_t) &PORTL,
	};
	const uint16_t PROGMEM ddrlist[] = {
		(uint16_t) &DDRA,
		(uint16_t) &DDRB,
		(uint16_t) &DDRC,
		(uint16_t) &DDRD,
		(uint16_t) &DDRE,
		(uint16_t) &DDRF,
		(uint16_t) &DDRH,
		(uint16_t) &DDRJ,
		(uint16_t) &DDRK,
		(uint16_t) &DDRL,
	};
	const uint16_t PROGMEM pinlist[] = {
		(uint16_t) &PINA,
		(uint16_t) &PINB,
		(uint16_t) &PINC,
		(uint16_t) &PIND,
		(uint16_t) &PINE,
		(uint16_t) &PINF,
		(uint16_t) &PINH,
		(uint16_t) &PINJ,
		(uint16_t) &PINK,
		(uint16_t) &PINL,
	};
	const uint8_t list[] = {
		// 0, 1, 2, 3, 4, 5, 6, 7 (PE)
		39, 38, 37, 36, 35, 34, 33, 32, 
		// 8, 9, 10, 11, 12, 13, 14, 15 (PH)
		55, 54, 53, 52, 51, 50, 49, 48,
		// 16, 17, 18 (PD)
		31, 30, 29,
		// 19, 20, 21, 22, 23 (PB)
		8, 12, 13, 14, 15,
		// 24, 25, 26, 27, 28, 29, 30, 31 (PL)
		79, 78, 77, 76, 75, 74, 73, 72,
		// 32, 33, 34, 35, 36, 37, 38, 39 (PC)
		23, 22, 21, 20, 19, 18, 17, 16,
		// 40, 41, 42, 43, 44, 45, 46, 47 (PJ)
		63, 62, 61, 60, 59, 58, 57, 56,
		// 48, 49, 50, 51, 52, 53, 54, 55 (PA)
		0, 1, 2, 3, 4, 5, 6, 7,
		// 56, 57, 58, 59, 60, 61, 62, 63 (PK)
		64, 65, 66, 67, 68, 69, 70, 71,
		// 64, 65, 66, 67, 68, 69, 70, 71 (PF)
		40, 41, 42, 43, 44, 45, 46, 47,
	};
#endif

#ifdef IO_SPI
	#undef MAX_PORTS
	#define MAX_PORTS 6
#endif

class IO {
	public:
		void set(uint8_t pin, enum IO_event func);

		void high(uint8_t pin);
		void low(uint8_t pin);
		void toggle(uint8_t pin);
		void out(uint8_t pin);
		void in(uint8_t pin);
		uint8_t read(uint8_t pin);

		void init();

		void set_blink1(uint8_t pin);
		void set_blink2(uint8_t pin);
		void unset_blink1(uint8_t pin);
		void unset_blink2(uint8_t pin);

		void blink1();
		void blink2();

		void pulse_set(uint8_t pin);
		void pulse_high();
		void pulse_low();

		void readInput();
		#ifdef IO_SPI
		void writeOutput();
		#endif

		uint16_t calculateTimer(uint16_t mseconds);

		uint16_t blink1_period;
		uint16_t blink1_counter;
		uint16_t blink2_period;
		uint16_t blink2_counter;
		uint16_t pulse_counter;
		uint8_t pulse_length;

	private:
		uint8_t blink1Mask[MAX_PORTS];
		uint8_t blink2Mask[MAX_PORTS];

		uint8_t pulseMask[MAX_PORTS];

		uint8_t servo1Mask[MAX_PORTS];
		uint8_t servo2Mask[MAX_PORTS];
		uint8_t servo3Mask[MAX_PORTS];
		uint8_t servo4Mask[MAX_PORTS];

		uint8_t readMask[MAX_PORTS];
		uint8_t readData[MAX_PORTS];

		#ifdef IO_SPI
		uint8_t writeData[MAX_PORTS];
		#endif
};


extern IO io;

#define IO_TIMER TCCR0B
#define IO_TIMER_PRESCALER (1 << CS02) | (1 << CS00)
#define IO_TIMER_INT TIMSK0
#define IO_TIMER_OVERFLOW_INT_REG 1 //TOIE
#define IO_TIMER_COMPA_INT_REG 2 //OCIEA

#define IO_TIMER_REG TCNT0
#define IO_TIMER_COMPA_REG OCR0A

#define IO_TIMER_OVERFLOW_INT TIMER0_OVF_vect
#define IO_TIMER_COMPA_INT TIMER0_COMPA_vect