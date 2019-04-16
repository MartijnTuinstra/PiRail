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

// #define RNET_DEBUG

void RNet::RNet (){
  RNET_TX_SET_HIGH; // Write pull up and will become high output HIGH
  _set_out(DDR(RNET_TX_PORT), RNET_TX_pin); //Set as output

  _set_in(DDR(RNET_RX_PORT), RNET_RX_pin);   //Set as input
  //_set_low(PORT(RNET_RX_PORT), RNET_TX_pin); //Disable pull-resistor
  // _set_high(PORT(RNET_RX_PORT), RNET_RX_pin); //Enable pull-resistor for debug

  _set_out(PORT(RNET_DUPLEX_PORT), RNET_DUPLEX_pin); //Set as output
  RNET_DUPLEX_SET_RX;

  this.state = IDLE;

  cli(); //Disable interupts

  _TIM_CRA = 0;
  _TIM_CRB = _TIM_CTC | _TIM_PRESCALER;

  RNET_ENABLE_ISR_CAPT;

  sei(); //Enable interupts
}

status RNet::transmit(uint8_t PrioDelay){
  msgLen = getMsgSize(&RNet_tx_buffer);         //Get size of message
  msg = currentMsg(&RNet_tx_buffer); //Get index of message

  cDBy = msg[0];

  if(state == HOLDOFF && cBi >= PrioDelay){
    cli();
    #ifdef RNET_DEBUG
    uart_putchar('+');
    #endif
    state = IDLE;
    sei();
  }

  if(state == HOLDOFF){
    #ifdef RNET_DEBUG
    uart_putchar('=');
    #endif
    return HOFF;
  }


  //Check if Bus is IDLE (Last check)
  if(state != IDLE){
    #ifdef RNET_DEBUG
    uart_putchar('-');
    #endif
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

  #ifdef RNET_DEBUG
  uart_putchar('S');
  #endif
  state = TX;
  #ifdef RNET_DEBUG
  uart_putchar(state + 0x30);
  uart_putchar('\n');
  #endif

  while(state == TX){}

  return OK;

}

void printHex(uint8_t x){
  if((x >> 4) >= 0xa){
    uart_putchar(0x37 + (x >> 4));
  }
  else{
    uart_putchar(0x30 + (x >> 4));
  }
  if((x & 0xf) >= 0xa){
    uart_putchar(0x37 + (x & 0xf));
  }
  else{
    uart_putchar(0x30 + (x & 0xf));
  }
}

void RNet_add_to_buf(uint8_t * data, uint8_t len, struct _RNet_buffer * buffer){
  for(int i = 0; i < len; i++){
    RNet_add_char_to_buf(data[i], buffer);
  }
}

void RNet_add_char_to_buf(uint8_t data, struct _RNet_buffer * buffer){
  // printHex(buffer->write_index);
  buffer->buf[buffer->write_index++] = data;
  if(buffer->write_index > RNET_MAX_BUFFER){
    buffer->write_index = 0;
  }
}

void RNet_init(){
  RNET_TX_SET_HIGH; // Write pull up and will become high output HIGH
  _set_out(DDR(RNET_TX_PORT), RNET_TX_pin); //Set as output

  _set_in(DDR(RNET_RX_PORT), RNET_RX_pin);   //Set as input
  //_set_low(PORT(RNET_RX_PORT), RNET_TX_pin); //Disable pull-resistor
  // _set_high(PORT(RNET_RX_PORT), RNET_RX_pin); //Enable pull-resistor for debug

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
  // printHex(RNet_rx_buffer.write_index);
  RNet_rx_buffer.buf[RNet_rx_buffer.write_index++] = b;
  if(RNet_rx_buffer.write_index > RNET_MAX_BUFFER){
    RNet_rx_buffer.write_index = 0;
  }
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

  while(BusSt == TX){}

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
  while(RNet_rx_buffer.read_index != RNet_rx_buffer.write_index){
    printHex(RNet_rx_buffer.buf[RNet_rx_buffer.read_index]);
    uart_putchar(' ');
    RNet_rx_buffer.read_index++;

    if(RNet_rx_buffer.read_index > RNET_MAX_BUFFER){
      RNet_rx_buffer.read_index = 0;
    }
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
        #ifdef RNET_DEBUG
          uart_putchar('F');
        #endif
      }
      else{
        #ifdef RNET_DEBUG
          uart_putchar('f');
        #endif
        cBi++;
        return;
      }
    }
    else if(cBi == 9){ // Stop-bit 1
      if(RNET_READ_RX){
        cBi++;
        return;
        #ifdef RNET_DEBUG
          uart_putchar('s');
        #endif
      }
      else{
        #ifdef RNET_DEBUG
          uart_putchar('C');
        #endif
        cBi = 0;
        add_to_RX_buf(cDBy);
        cDBy = 0;
      }
    }
    else if(cBi == 10){ // Stop-bit 2
      if(!RNET_READ_RX){
        //Framing Error
        #ifdef RNET_DEBUG
          uart_putchar('$');
        #endif
      }
      else{
        //Save Byte
        #ifdef RNET_DEBUG
          uart_putchar('#');
        #endif
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
        #ifdef RNET_DEBUG
          uart_putchar('1');
        #endif
        cDBy |= 0x80;
      }
      else{
        #ifdef RNET_DEBUG
	        uart_putchar('0');
        #endif
      }
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
  #ifdef RNET_DEBUG
      uart_putchar('S');
  #endif
      RNET_TX_SET_LOW;
      cBi++;
      return;
    }
    else if(cBi < 9){ // Data
      if(cDBy & _BV(cBi - 1)){
    #ifdef RNET_DEBUG
        uart_putchar('1');
    #endif
        RNET_TX_SET_HIGH;
      }
      else{
    #ifdef RNET_DEBUG
        uart_putchar('0');
    #endif
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
        RNET_TX_SET_LOW;
    #ifdef RNET_DEBUG
        uart_putchar('C');
    #endif
      }
      else{
        RNET_TX_SET_HIGH;
    #ifdef RNET_DEBUG
        uart_putchar('s');
    #endif
        cBi++;
      }
    }
    else{ //Stop-bit 2
      //Whole message transmitted
      RNET_TX_SET_HIGH;
  #ifdef RNET_DEBUG
      uart_putchar('s');
      uart_putchar('\n');
  #endif

      cBi = 0;

      cBy = 0;
      BusSt = HOLDOFF;
    }
  }

  if(BusSt == COLLISION){
#ifdef RNET_DEBUG
    uart_putchar('X');
#endif
    if(cBi++ < RNET_COLLISION_TICKS){
      RNET_TX_SET_LOW;
    }
    else{
      BusSt = HOLDOFF;
    }
  }

  if(BusSt == HOLDOFF){
#ifdef RNET_DEBUG
    uart_putchar('H');
#endif
    if(cBi == 0){
      RNET_TX_SET_HIGH;
      RNET_DUPLEX_SET_RX;

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

  cBi = 0;
  cBy = 0;

  i = 0;


  uart_putchar('\n');
  uart_putchar('R');
  uart_putchar('X');
  uart_putchar('\n');
}
