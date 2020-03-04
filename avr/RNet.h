#ifndef INCLUDED_RNET_H
#define INCLUDED_RNET_H

//#include "IO.h"

#define RNET_BROADCAST_MODULE 0xFF
// #define RNET_MASTER 0x0

#define RNET_MAX_BUFFER 128

#if defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)

#ifndef IO_SPI
#define RNET_TX_pin 2
#define RNET_TX_PORT B
#else
#define RNET_TX_pin 7
#define RNET_TX_PORT D
#endif

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
#define _TIM_PRESCALER (0 << CS12) | (1 << CS11) | (0 << CS10)
#define _TIM_ICRn ICR1

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

#elif defined(__AVR_ATmega2560__)

#define RNET_TX_pin 4
#define RNET_TX_PORT B

#define RNET_RX_pin 1
#define RNET_RX_PORT L

#define RNET_DUPLEX_pin 6
#define RNET_DUPLEX_PORT H

#define RNET_RX_ICP_ISR_vect TIMER5_CAPT_vect
#define RNET_TIMER_ISR_vect  TIMER5_COMPA_vect

#define _TIM_CRA TCCR5A
#define _TIM_CRB TCCR5B
#define _TIM_COUNTER TCNT5
#define _TIM_COMPA OCR5A
#define _TIM_ISR_MSK TIMSK5
#define _TIM_ISR_FLAGS TIFR5
#define _TIM_CTC (1 << WGM52)
#define _TIM_PRESCALER (1 << CS52) | (0 << CS51) | (1 << CS50)

#endif

#define RNET_TX_SET_HIGH _set_high(PORT(RNET_TX_PORT), RNET_TX_pin)
#define RNET_TX_SET_LOW   _set_low(PORT(RNET_TX_PORT), RNET_TX_pin)
#define RNET_DUPLEX_SET_RX  _set_low(PORT(RNET_DUPLEX_PORT), RNET_DUPLEX_pin)
#define RNET_DUPLEX_SET_TX _set_high(PORT(RNET_DUPLEX_PORT), RNET_DUPLEX_pin)
#define RNET_READ_RX _read_pin(PIN(RNET_RX_PORT), RNET_RX_pin)

#define RNET_TIMER_ENABLE _TIM_CRB |= _TIM_PRESCALER;
#define RNET_TIMER_DISABLE _TIM_CRB &= ~(0x7);

#define RNET_CHECK_COLLISION (_read_pin(PIN(RNET_RX_PORT), RNET_RX_pin)) & (PORT(RNET_TX_PORT) & _BV(RNET_TX_pin))

#ifdef __AVR_ATmega64A__

#define RNET_ENABLE_ISR_COMPA  _TIM_ISR_MSK |=  (1 << OCIE1A)
#define RNET_ENABLE_ISR_CAPT   _TIM_ISR_MSK |=  (1 << TICIE1)
#define RNET_CLEAR_ISR_CAPT    _TIM_ISR_FLAGS |= (1 << ICF1)
#define RNET_CLEAR_ISR_COMPA   _TIM_ISR_FLAGS |= (1 << OCFA1)
#define RNET_DISABLE_ISR_COMPA _TIM_ISR_MSK &= ~(1 << OCIE1A)
#define RNET_DISABLE_ISR_CAPT  _TIM_ISR_MSK &= ~(1 << TICIE1)

#elif __AVR_ATmega2560__

#define RNET_ENABLE_ISR_COMPA  _TIM_ISR_MSK |=  (1 << OCF5A)
#define RNET_ENABLE_ISR_CAPT   _TIM_ISR_MSK |=  (1 << ICF5)
#define RNET_CLEAR_ISR_CAPT    _TIM_ISR_FLAGS |= (1 << ICF5)
#define RNET_CLEAR_ISR_COMPA   _TIM_ISR_FLAGS |= (1 << OCFA5)
#define RNET_DISABLE_ISR_COMPA _TIM_ISR_MSK &=  ~(1 << OCF5A)
#define RNET_DISABLE_ISR_CAPT  _TIM_ISR_MSK &=  ~(1 << ICF5)

#else

#define RNET_ENABLE_ISR_COMPA  _TIM_ISR_MSK |=  (1 << OCIE1A)
#define RNET_ENABLE_ISR_CAPT   _TIM_ISR_MSK |=  (1 << ICIE1)
#define RNET_CLEAR_ISR_CAPT    _TIM_ISR_FLAGS |= (1 << ICF1)
#define RNET_CLEAR_ISR_COMPA   _TIM_ISR_FLAGS |= (1 << OCF1A)
#define RNET_DISABLE_ISR_COMPA _TIM_ISR_MSK &= ~(1 << OCIE1A)
#define RNET_DISABLE_ISR_CAPT  _TIM_ISR_MSK &= ~(1 << ICIE1)

#endif

#if __AVR_ATmega2560__

#define RNET_TX_TICK 40

#else

#define RNET_TX_TICK 2000

#endif

#define RNET_TICK RNET_TX_TICK
#define RNET_OFFSET RNET_TX_TICK / 4

#ifdef RNET_MASTER
#define RNET_TIMEOUT RNET_TX_TICK * 4
#else
#define RNET_TIMEOUT RNET_TX_TICK * 1.5
#endif

struct _RNet_buffer {
    uint8_t buf[RNET_MAX_BUFFER];
    volatile uint8_t read_index;
    volatile uint8_t write_index;
};

#ifdef RNET_MASTER

enum BusState {
  IDLE,      // Doing nothing
  ADDR,      // Sending Address
  TX,        // Transmiting
  RX,        // Receiving
  TIMEOUT
};

#else

enum BusState {
	IDLE,      // Doing nothing
  ADDR,      // Receiving Address
	RX,        // Receiving
	TX,        // Transmiting
	OTHER      // Other transmit
};

#endif

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
extern uint8_t tmp_rx_msg[RNET_MAX_BUFFER];

class RNet {
  public:
    volatile enum BusState state;
    #ifdef RNET_MASTER
    void init();
    #else
    void init(uint8_t dev, uint8_t node);
    #endif
    void reset_bus();
    uint8_t getMsgSize(struct _RNet_buffer * msg);
    uint8_t getMsgSize(uint8_t * buf);

    bool available();
    void read();

    void calculateTxChecksum();

    uint8_t * getBufP(struct _RNet_buffer * buf, uint8_t write);

    #ifdef RNET_MASTER
    void try_transmit();
    status transmit(uint8_t addr);
    status transmit(struct _RNet_buffer * buf);
    void _transmit();

    void request_all();
    void request_registered();
    #else
    #endif

    struct _RNet_buffer rx;
    struct _RNet_buffer tx;
    #ifdef RNET_MASTER
    uint8_t devices_list[32]; // 256-bits

    #define RNET_GET_DEVICE(dev) devices_list[dev / 8] & (1 << (dev % 8))
    #else
    uint8_t dev_id;
    uint8_t node_id;
    volatile uint8_t txdata;
    #endif
};

extern RNet net;

void RNet_add_to_buf(uint8_t * data, uint8_t len, struct _RNet_buffer * buffer);
void RNet_add_char_to_buf(uint8_t data, struct _RNet_buffer * buffer);
void readRXBuf();
void printHex(uint8_t x);

#endif
