#ifndef INCLUDED_RNET_H
#define INCLUDED_RNET_H

//#include "IO.h"

#define RNET_BROADCAST_MODULE 0xFF
#define RNET_MASTER 0x0

#define RNET_MAX_BUFFER 40

#if defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)

#define RNET_TX_pin 2
#define RNET_TX_PORT B

#define RNET_RX_pin 0
#define RNET_RX_PORT B

#define RNET_DUPLEX_pin 1
#define RNET_DUPLEX_PORT B

#define RNET_RX_ICP_ISR_vect TIMER1_CAPT_vect
#define RNET_TIMER_ISR_vect  TIMER1_COMPA_vect

#define _TIM_CRA TCCR1A
#define _TIM_CRB TCCR1B
#define _TIM_COUNTER TCNT1
#define _TIM_COMPA OCR1A
#define _TIM_ISR_MSK TIMSK1
#define _TIM_ISR_FLAGS TIFR1
#define _TIM_CTC (1 << WGM12)
#define _TIM_PRESCALER (0 << CS12) | (1 << CS11) | (1 << CS10)
#define _TIM_EN_ISR (1 << OCIE1A) | (1 << ICIE1)

#elif defined(__AVR_ATmega64A__)

#define RNET_TX_pin 2
#define RNET_TX_PORT D

#define RNET_RX_pin 4
#define RNET_RX_PORT D

#define RNET_DUPLEX_pin 3
#define RNET_DUPLEX_PORT D

#define RNET_RX_ICP_ISR_vect TIMER1_CAPT_vect
#define RNET_TIMER_ISR_vect  TIMER1_COMPA_vect

#define _TIM_CRA TCCR1A
#define _TIM_CRB TCCR1B
#define _TIM_COUNTER TCNT1
#define _TIM_COMPA OCR1A
#define _TIM_ISR_MSK TIMSK
#define _TIM_ISR_FLAGS TIFR
#define _TIM_CTC (1 << WGM12)
#define _TIM_PRESCALER (0 << CS12) | (1 << CS11) | (1 << CS10)
#define _TIM_EN_ISR (1 << OCIE1A) | (1 << ICIE1)

#elif defined(__AVR_ATmega2560__)

#define RNET_TX_pin 2
#define RNET_TX_PORT D

#define RNET_RX_pin 4
#define RNET_RX_PORT D

#define RNET_DUPLEX_pin 3
#define RNET_DUPLEX_PORT D

#define RNET_RX_ICP_ISR_vect TIMER1_CAPT_vect
#define RNET_TIMER_ISR_vect  TIMER1_COMPA_vect

#define _TIM_CRA TCCR1A
#define _TIM_CRB TCCR1B
#define _TIM_COUNTER TCNT1
#define _TIM_COMPA OCR1A
#define _TIM_ISR_MSK TIMSK1
#define _TIM_ISR_FLAGS TIFR1
#define _TIM_CTC (1 << WGM12)
#define _TIM_PRESCALER (0 << CS12) | (1 << CS11) | (1 << CS10)
#define _TIM_EN_ISR (1 << OCIE1A) | (1 << ICIE1)

#endif

#define RNET_TX_SET_HIGH _set_high(PORT(RNET_TX_PORT), RNET_TX_pin)
#define RNET_TX_SET_LOW   _set_low(PORT(RNET_TX_PORT), RNET_TX_pin)
#define RNET_DUPLEX_SET_RX  _set_low(PORT(RNET_DUPLEX_PORT), RNET_DUPLEX_pin)
#define RNET_DUPLEX_SET_TX _set_high(PORT(RNET_DUPLEX_PORT), RNET_DUPLEX_pin)
#define RNET_READ_RX _read_pin(PIN(RNET_RX_PORT), RNET_RX_pin)

#define RNET_CHECK_COLLISION (_read_pin(PIN(RNET_RX_PORT), RNET_RX_pin)) & (PORT(RNET_TX_PORT) & _BV(RNET_TX_pin))

#ifdef __AVR_ATmega64A__

#define RNET_ENABLE_ISR_COMPA  _TIM_ISR_MSK |=  (1 << OCIE1A)
#define RNET_ENABLE_ISR_CAPT   _TIM_ISR_MSK |=  (1 << TICIE1)
#define RNET_CLEAR_ISR_CAPT    _TIM_ISR_FLAGS |= ICF1
#define RNET_DISABLE_ISR_COMPA _TIM_ISR_MSK &= ~(1 << OCIE1A)
#define RNET_DISABLE_ISR_CAPT  _TIM_ISR_MSK &= ~(1 << TICIE1)

#else

#define RNET_ENABLE_ISR_COMPA  _TIM_ISR_MSK |=  (1 << OCIE1A)
#define RNET_ENABLE_ISR_CAPT   _TIM_ISR_MSK |=  (1 << ICIE1)
#define RNET_CLEAR_ISR_CAPT    _TIM_ISR_FLAGS |= ICF1
#define RNET_DISABLE_ISR_COMPA _TIM_ISR_MSK &= ~(1 << OCIE1A)
#define RNET_DISABLE_ISR_CAPT  _TIM_ISR_MSK &= ~(1 << ICIE1)

#endif

#define RNET_TX_START_DELAY 200
#define RNET_RX_START_DELAY 400
#define RNET_TX_TICK 400
#define RNET_RX_TICK RNET_TX_TICK
#define RNET_COLLISION_TICKS 20
#define RNET_HOLDOFF_TICKS 25

struct _RNet_buffer {
    uint8_t buf[RNET_MAX_BUFFER];
    uint8_t read_index;
    uint8_t write_index;
};

enum BusState {
	IDLE,      // Doing nothing
	RX,        // Receiving
	TX,        // Transmiting
	HOLDOFF,   // Spacing betwin transmission
	COLLISION  // Collision Happened
};

extern volatile enum BusState BusSt;

enum status {
  OK,
  FAILED,
  BUSY,
  HOFF,
  HOFF_PRIO,
  TOO_MANY_RETRIES,
  UNKNOWN
};

extern struct _RNet_buffer RNet_rx_buffer;
extern struct _RNet_buffer RNet_tx_buffer;

void RNet_add_to_buf(uint8_t * data, uint8_t len, struct _RNet_buffer * buffer);
void RNet_add_char_to_buf(uint8_t data, struct _RNet_buffer * buffer);
void readRXBuf();

void RNet_init();

status TX_try(uint8_t PrioDelay);

#endif
