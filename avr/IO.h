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

// enum IO_type {
// 	IO_InputToggle,
// 	IO_InputRising,
// 	IO_InputFalling,
// 	IO_Output
// };

enum e_IO_type {
  IO_Undefined,
  IO_Output,
  IO_Output_Blink,
  IO_Output_Servo,
  IO_Output_PWM,
  IO_Input,
  IO_Input_Block,
  IO_Input_Switch,
  IO_Input_MSSwitch,
};

enum e_IO_output_event {
  IO_event_Low,
  IO_event_High,
  IO_event_Pulse,
  IO_event_Toggle
};

enum e_IO_blink_event {
  IO_event_B_Low,
  IO_event_B_High,
  IO_event_Blink1,
  IO_event_Blink2
};

enum e_IO_servo_event {
  IO_event_Servo1,
  IO_event_Servo2,
  IO_event_Servo3,
  IO_event_Servo4
};

enum e_IO_PWM_event {
  IO_event_PWM1,
  IO_event_PWM2,
  IO_event_PWM3,
  IO_event_PWM4
};

union u_IO_event {
  enum e_IO_output_event output;
  enum e_IO_blink_event blink;
  enum e_IO_servo_event servo;
  enum e_IO_PWM_event pwm;

  uint8_t value;
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
#define _portInputRegister(P) ( (volatile uint8_t *)( pgm_read_word( pinlist + (P))) )

// extern const uint16_t PROGMEM _PORT[];

// extern const uint16_t PROGMEM _DDR[];

// extern const uint16_t PROGMEM _PIN[];

// extern uint8_t pin_to_Port[];

// extern uint8_t pin_to_BIT[];

#ifndef IO_SPI

const uint16_t PROGMEM portlist[] = {
	#if defined(__AVR_ATmega64A__) || defined(__AVR_ATmega2560__)
	(uint16_t) &PORTA,
	#endif
	(uint16_t) &PORTB,
	(uint16_t) &PORTC,
	(uint16_t) &PORTD,
	#if defined(__AVR_ATmega64A__) || defined(__AVR_ATmega2560__)
	(uint16_t) &PORTE,
	(uint16_t) &PORTF,
	#endif
	#if defined(__AVR_ATmega64A__)
	(uint16_t) &PORTG,
	#elif defined(__AVR_ATmega2560__)
	(uint16_t) &PORTH,
	(uint16_t) &PORTJ,
	(uint16_t) &PORTK,
	(uint16_t) &PORTL,
	#endif
};
const uint16_t PROGMEM ddrlist[] = {
	#if defined(__AVR_ATmega64A__) || defined(__AVR_ATmega2560__)
	(uint16_t) &DDRA,
	#endif
	(uint16_t) &DDRB,
	(uint16_t) &DDRC,
	(uint16_t) &DDRD,
	#if defined(__AVR_ATmega64A__) || defined(__AVR_ATmega2560__)
	(uint16_t) &DDRE,
	(uint16_t) &DDRF,
	#endif
	#if defined(__AVR_ATmega64A__)
	(uint16_t) &DDRG,
	#elif defined(__AVR_ATmega2560__)
	(uint16_t) &DDRH,
	(uint16_t) &DDRJ,
	(uint16_t) &DDRK,
	(uint16_t) &DDRL,
	#endif
};
const uint16_t PROGMEM pinlist[] = {
	#if defined(__AVR_ATmega64A__) || defined(__AVR_ATmega2560__)
	(uint16_t) &PINA,
	#endif
	(uint16_t) &PINB,
	(uint16_t) &PINC,
	(uint16_t) &PIND,
	#if defined(__AVR_ATmega64A__) || defined(__AVR_ATmega2560__)
	(uint16_t) &PINE,
	(uint16_t) &PINF,
	#endif
	#if defined(__AVR_ATmega64A__)
	(uint16_t) &PING,
	#elif defined(__AVR_ATmega2560__)
	(uint16_t) &PINH,
	(uint16_t) &PINJ,
	(uint16_t) &PINK,
	(uint16_t) &PINL,
	#endif
};

#if defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)
	#define MAX_PORTS 3
	#define MAX_PINS 13
	// RNet uses B0,B1,B2 -> 0,1,2
	const uint8_t list[] = {
		// 0, 1, 2, 3, 4, 5
		// PD4, PD3, PD5, PD6, PD7, PD2
		20, 19, 21, 22, 23, 18,
		// 6, 7, 8, 9, 10, 11
		// PC5, PC4, PC3, PC2, PC1, PC0
		13, 12, 11, 10, 9, 8,
		// 12 // DEBUG
		// PB5
		5
	};
#elif defined(__AVR_ATmega64A__)
	#define MAX_PORTS 7
	#define MAX_PINS 40
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
	#define MAX_PINS 72

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

#else // IO_SPI
	#undef MAX_PORTS
	#define MAX_PORTS 6
	#define MAX_PINS 48*2
#endif

class IO {
	public:
		void set(uint8_t pin, union u_IO_event func);

		inline void high(uint8_t pin);
		inline void low(uint8_t pin);
		inline void toggle(uint8_t pin);
		inline void out(uint8_t pin);
		inline void in(uint8_t pin);
		inline uint8_t read(uint8_t pin);

		void init();
		void initPin(uint8_t pin);

		void blink1();
		void blink2();

		void pulse_high();
		void pulse_low();

		void servo_low();
		void servo_high(uint8_t * mask);

		void set_mask(uint8_t pin, uint8_t * mask);
		void unset_mask(uint8_t pin, uint8_t * mask);

		void readInput();
		void copyInput();
		#ifdef IO_SPI
		void writeOutput();
		void readwrite();
		#endif

		uint16_t calculateTimer(uint16_t mseconds);

		uint16_t blink1_period;
		volatile uint16_t blink1_counter;
		uint16_t blink2_period;
		volatile uint16_t blink2_counter;
		volatile uint16_t pulse_counter;
		uint16_t pulse_length;

		uint8_t servo1Pulse;
		volatile uint16_t servo1counter;
		uint8_t servo2Pulse;
		volatile uint16_t servo2counter;

		volatile uint8_t compareInt;
		// uint8_t servo3Pulse;
		// uint16_t servo3counter;
		// uint8_t servo4Pulse;
		// uint16_t servo4counter;
		
		uint8_t readData[MAX_PORTS];
		uint8_t oldreadData[MAX_PORTS];
		uint8_t readMask[MAX_PORTS];

	// private:
		uint8_t blink1Mask[MAX_PORTS];
		uint8_t blink2Mask[MAX_PORTS];

		uint8_t pulseMask[MAX_PORTS];

		uint8_t invertedMask[MAX_PORTS];

		uint8_t servo1Mask[MAX_PORTS];
		uint8_t servo2Mask[MAX_PORTS];
		uint8_t servo3Mask[MAX_PORTS];
		uint8_t servo4Mask[MAX_PORTS];


		#ifdef IO_SPI
		uint8_t writeData[MAX_PORTS];
		enum e_IO_type typelist[MAX_PORTS*16];
		#else
		enum e_IO_type typelist[MAX_PINS];
		#endif
};


extern IO io;

#define IO_TIMER_REG TCNT0
#define IO_TIMER_PRESCALER (1 << CS02) | (1 << CS00)
#define IO_TIMER_OVERFLOW_INT TIMER0_OVF_vect

#if defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)

#define IO_TIMER TCCR0B
#define IO_TIMER_INT TIMSK0
#define IO_TIMER_OVERFLOW_INT_REG (1<<TOIE0) //TOIE
#define IO_TIMER_COMPA_INT_REG (1<<OCIE0A) //OCIEA


#define IO_TIMER_COMPA_REG OCR0A

#define IO_TIMER_COMPA_INT TIMER0_COMPA_vect

#else // __AVR_ATmega64A

#define IO_TIMER TCCR0
#define IO_TIMER_INT TIMSK
#define IO_TIMER_OVERFLOW_INT_REG (1 << TOIE0)
#define IO_TIMER_COMPA_INT_REG (1 << OCIE0)

#define IO_TIMER_COMPA_REG OCR0

#define IO_TIMER_COMPA_INT TIMER0_COMP_vect

#endif

#define IO_Enable_COMPA_INT IO_TIMER_INT |= IO_TIMER_COMPA_INT_REG
#define IO_Disable_COMPA_INT IO_TIMER_INT &= ~(IO_TIMER_COMPA_INT_REG)

#define IO_SERVO_MIN 15
#define IO_SERVO_MAX 32
#define IO_SERVO_PERIOD 313

#define IO_INT_BlinkA 0x01
#define IO_INT_BlinkB 0x02
#define IO_INT_Pulse  0x04
#define IO_INT_Servo  0x08
#define IO_INT_Servo1 0x10
#define IO_INT_Servo2 0x20
#define IO_INT_Servo3 0x40
#define IO_INT_Servo4 0x80

#endif
