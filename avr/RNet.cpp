#include <stdint.h>
#include <stdlib.h>
#include "avr/interrupt.h"

#include "main_node.h"

#include "IO.h"
#include "RNet.h"

#include "util/delay.h"

#if defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)
#define LED 12
#elif defined(__AVR_ATmega64A__)
#define LED 0
#elif defined(__AVR_ATmega2560__)
#define LED 0x17
#endif

struct _RNet_buffer RNet_rx_buffer;
struct _RNet_buffer RNet_tx_buffer;
volatile enum BusState BusSt;

int i = 0;

void RNet_add_to_buf(uint8_t * data, uint8_t len, struct _RNet_buffer * buffer){
  for(int i = 0; i < len; i++){
    RNet_add_char_to_buf(data[i], buffer);
  }
}

void RNet_add_char_to_buf(uint8_t data, struct _RNet_buffer * buffer){
  buffer->buf[buffer->write_index++] = data;
}

void RNet_init(){
  RNET_TX_SET_HIGH; // Write pull up and will become high output HIGH
  _set_out(DDR(RNET_TX_PORT), RNET_TX_pin); //Set as output

  _set_in(DDR(RNET_RX_PORT), RNET_RX_pin);   //Set as input
  //_set_low(PORT(RNET_RX_PORT), RNET_TX_pin); //Disable pull-resistor
  _set_high(PORT(RNET_RX_PORT), RNET_RX_pin); //Enable pull-resistor for debug

  _set_out(PORT(RNET_DUPLEX_PORT), RNET_DUPLEX_pin); //Set as output
  RNET_DUPLEX_SET_RX;

  BusSt = IDLE;

  cli(); //Disable interupts

  _TIM_CRA = 0;
  _TIM_CRB = _TIM_CTC | _TIM_PRESCALER;

  RNET_ENABLE_ISR_CAPT;

  sei(); //Enable interupts
}

void add_to_RX_buf(uint8_t b){
  RNet_rx_buffer.buf[RNet_rx_buffer.write_index++] = b;
}

int getMsgSize(struct _RNet_buffer * msg){
  msg->read_index++;
  return 12;
}

uint8_t * currentMsg(struct _RNet_buffer * msg){
  msg->buf[0] = 'H';
  msg->buf[1] = 'e';
  msg->buf[2] = 'l';
  msg->buf[3] = 'l';
  msg->buf[4] = 'o';
  msg->buf[5] = ' ';
  msg->buf[6] = 'W';
  msg->buf[7] = 'o';
  msg->buf[8] = 'r';
  msg->buf[9] = 'l';
  msg->buf[10] = 'd';
  msg->buf[11] = '!';

  return msg->buf;
}

uint8_t * msg;
uint8_t msgLen = 10;
uint8_t cDBy = 0; //CurrentDataByte
uint8_t cBy = 0; //currentByte
uint8_t cBi = 0; //currentBit
bool cont = false;

status TX_try(uint8_t PrioDelay){
  msgLen = getMsgSize(&RNet_tx_buffer);         //Get size of message
  msg = currentMsg(&RNet_tx_buffer); //Get index of message

  cDBy = msg[0];

  uart_putchar('T');
  uart_putchar('r');
  uart_putchar('y');
  uart_putchar('\n');

  if(BusSt == HOLDOFF && cBi >= PrioDelay){
    cli();
    uart_putchar('+');
    BusSt = IDLE;
    sei();
  }

  if(BusSt == HOLDOFF){
    uart_putchar('=');
    return HOFF;
  }


  //Check if Bus is IDLE (Last check)
  if(BusSt != IDLE){
    uart_putchar('-');
    return BUSY;
  }

  cBy = 0;
  cBi = 0;

  //Disable ICP ISR
  cli();

  RNET_DISABLE_ISR_CAPT;

  _TIM_COUNTER = 0;
  _TIM_COMPA = RNET_TX_START_DELAY;

  RNET_ENABLE_ISR_COMPA;

  //Start sending
  RNET_DUPLEX_SET_TX;
  RNET_TX_SET_LOW; //Start-bit

  sei(); //Enable Interrupts

  uart_putchar('S');
  BusSt = TX;
  uart_putchar(BusSt + 0x30);
  uart_putchar('\n');
  uart_putchar('\n');

  while(BusSt == TX)

  _delay_us(2);
  RNET_DUPLEX_SET_RX;

  uart_putchar('D');
  uart_putchar('o');
  uart_putchar('n');
  uart_putchar('e');

  return OK;
}

void readRXBuf(){
  uart_putchar('\n');
  uart_putchar('R');
  uart_putchar('X');
  uart_putchar(' ');
  uart_putchar('b');
  uart_putchar('u');
  uart_putchar('f');
  uart_putchar(':');
  uart_putchar(' ');
  while(RNet_rx_buffer.read_index < RNet_rx_buffer.read_index){
    if((RNet_rx_buffer.buf[RNet_rx_buffer.read_index] >> 4) > 0xa){
      uart_putchar(0x41 + (RNet_rx_buffer.buf[RNet_rx_buffer.read_index] >> 4));
    }
    else{
      uart_putchar(0x30 + (RNet_rx_buffer.buf[RNet_rx_buffer.read_index] >> 4));
    }
    if((RNet_rx_buffer.buf[RNet_rx_buffer.read_index] & 0xa) > 0xa){
      uart_putchar(0x41 + (RNet_rx_buffer.buf[RNet_rx_buffer.read_index] & 0xa));
    }
    else{
      uart_putchar(0x30 + (RNet_rx_buffer.buf[RNet_rx_buffer.read_index] & 0xa));
    }
    uart_putchar(' ');
    RNet_rx_buffer.read_index++;
  }
  uart_putchar('\n');
}

ISR(RNET_TIMER_ISR_vect){ //TIMER1_COMPA_vect
  _TIM_COMPA = RNET_TX_TICK;
  set_toggle(LED);
  if(BusSt == RX){
    if(cBi == 0){ // Start-bit
      if(!RNET_READ_RX){
        //Framing ERROR
        uart_putchar('F');        
      }
      else
        uart_putchar('f');
      cBi++;
      return;
    }
    else if(cBi == 9){ // Stop-bit 1
      if(RNET_READ_RX){
        uart_putchar('C');
        cBi = 0;
        add_to_RX_buf(cDBy);
        cDBy = 0;
      }
      else
        uart_putchar('s');
      cBi++;
      return;
    }
    else if(cBi == 10){ // Stop-bit 2
      if(!RNET_READ_RX){
        //Framing Error
        uart_putchar('$');
      }
      else{
        //Save Byte
        uart_putchar('s');
        add_to_RX_buf(cDBy);
      }
      cBi = 0;
      cDBy = 0;

      //Stop receiving
      BusSt = HOLDOFF;

      return;
    }
    else{ // A data bit

      cDBy >>= 1;
      if(RNET_READ_RX){
        uart_putchar('1');
        cDBy |= 0x80;
      }
      else
	      uart_putchar('0');
      // if(cDBy & _BV(cBi))
      //   RNET_TX_HIGH;
      // else
      //   RNET_TX_LOW;
      cBi++;

      return;
    }
  }
  else if(BusSt == TX){
    // if(RNET_CHECK_COLLISION){
      // BusSt = COLLISION;
    // }
    if(cBi == 0){ // Start-bit
      uart_putchar('S');
      RNET_TX_SET_LOW;
      cBi++;
      return;
    }
    else if(cBi < 9){ // Data
      if(cDBy & _BV(cBi - 1)){
        uart_putchar('1');
        RNET_TX_SET_HIGH;
      }
      else{
        uart_putchar('0');
        RNET_TX_SET_LOW;
      }
      cBi++;
      return;
    }
    else if(cBi == 9){ //Stop bit 1
      if(++cBy < msgLen){
        //Data available
        cBi = 0;
        cDBy = msg[cBy];
        RNET_TX_SET_HIGH;
        uart_putchar('C');
      }
      else{
        RNET_TX_SET_LOW;
        uart_putchar('s');
      }
      cBi++;
    }
    else{ //Stop-bit 2
      //Whole message transmitted
      RNET_TX_SET_LOW;
      uart_putchar('s');
      uart_putchar('\n');

      cBi = 0;

      cBy = 0;
      BusSt = HOLDOFF;
    }
  }

  if(BusSt == COLLISION){
    uart_putchar('X');
    if(cBi++ < RNET_COLLISION_TICKS){
      RNET_TX_SET_LOW;
    }
    else{
      RNET_TX_SET_HIGH;
    }
  }

  if(BusSt == HOLDOFF){
    uart_putchar('H');
    if(cBi == 0){
      RNET_TX_SET_HIGH;

      RNET_ENABLE_ISR_CAPT;
      RNET_CLEAR_ISR_CAPT;
    }
    else if(cBi > RNET_HOLDOFF_TICKS){
      cBi = 0;
      RNET_DISABLE_ISR_COMPA;

      BusSt = IDLE;
      return;
    }
    cBi++;
  }
}

ISR(RNET_RX_ICP_ISR_vect){
  _TIM_COUNTER = 0;

  RNET_DISABLE_ISR_CAPT;

  _TIM_ISR_FLAGS |= _BV(OCF1A);
  RNET_ENABLE_ISR_COMPA;

  _TIM_COMPA = RNET_RX_START_DELAY;

  BusSt = RX;

  i = 0;


  uart_putchar('\n');
  uart_putchar('R');
  uart_putchar('X');
  uart_putchar('\n');
}
