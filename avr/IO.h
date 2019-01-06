#ifndef INCLUDED_IO_H
#define INCLUDED_IO_H

#include "prgmem.h"

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
