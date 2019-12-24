#include <stdint.h>
#include <stdlib.h>
#include "avr/interrupt.h"

#include "main_node.h"

#include "IO.h"
#include "RNet.h"
#include "RNet_msg.h"
#include "uart.h"
#include "eeprom_layout.h"

#include "util/delay.h"

struct _RNet_buffer RNet_rx_buffer;
struct _RNet_buffer RNet_tx_buffer;
uint8_t tmp_rx_msg[RNET_MAX_BUFFER];

// #define RNET_DEBUG

// void RNet::add_to_rx_buf(uint8_t data){
//   rx.buf[rx.write_index++] = data;
//   if(rx.write_index >= RNET_MAX_BUFFER){
//     rx.write_index = 0;
//   }
// }

// void RNet::add_to_tx_buf(uint8_t data){
//   tx.buf[tx.write_index++] = data;
//   if(tx.write_index >= RNET_MAX_BUFFER){
//     tx.write_index = 0;
//   }
// }

// void RNet::calculateTxChecksum(){
//   uint8_t checksum = RNET_CHECKSUM_SEED;
//   for(uint8_t i = tx.read_index; i != tx.write_index; i = (i+1)%RNET_MAX_BUFFER){
//     checksum ^= tx.buf[i];
//     printHex(tx.buf[i]);
//   }
//   printHex(checksum);
//   add_to_tx_buf(checksum);
// }

bool RNet::available(){
  if (rx.read_index != rx.write_index){;
    uint8_t size = getMsgSize(&rx);

    if (rx.write_index >= (rx.read_index + size) % RNET_MAX_BUFFER){
      //Copy message and check checksum
      uint8_t checksum = RNET_CHECKSUM_SEED;
      uint8_t i = 0;
      while(rx.read_index != rx.write_index && i != size){
        tmp_rx_msg[i] = rx.buf[rx.read_index++];
        uart.transmit(checksum, HEX, 2);
        uart.transmit(tmp_rx_msg[i], HEX, 2);
        uart.transmit(' ');

        checksum ^= tmp_rx_msg[i++];

        if(rx.read_index >= RNET_MAX_BUFFER)
          rx.read_index = 0;
      }

      uart.transmit(checksum, HEX, 2);

      // If wrong checksum discard message
      // checksum XOR checksum == 0
      if(checksum){
        uart.transmit("WCS\n", 4);
        return false;
      }

      return true;
    }
  }

  return false;
}

#ifndef RNET_MASTER
void RNet::read(){
  if(tmp_rx_msg[0] == RNet_OPC_SetEmergency){
    uart.transmit("SEm\n", 4);
  }
  else if(tmp_rx_msg[0] == RNet_OPC_RelEmergency){
    uart.transmit("REm\n", 4);
  }
  else if(tmp_rx_msg[0] == RNet_OPC_PowerON){
    uart.transmit("POn\n", 4);
  }
  else if(tmp_rx_msg[0] == RNet_OPC_PowerOFF){
    uart.transmit("POff\n", 5);
  }
  else if(tmp_rx_msg[0] == RNet_OPC_ResetALL){
    uart.transmit("RAll\n", 5);
  }

  //Check dev ID in header
  if(tmp_rx_msg[1] != node_id){
    return;
  }

  if(tmp_rx_msg[0] == RNet_OPC_SetOutput){
    if(tmp_rx_msg[2] & 0x80){ // Only one address
      io.set(tmp_rx_msg[2] & 0x7F, (enum IO_event)(tmp_rx_msg[3] & 0xF));
    }
    else{ // More than one
      uint8_t length = tmp_rx_msg[2] & 0x7F;

      uint8_t j = 0;
      for(uint8_t i = 0; i < length; i+= 2, j+=3){
        io.set(tmp_rx_msg[2+j] & 0x7F, (enum IO_event)(tmp_rx_msg[2+j+1] >> 4));

        if(tmp_rx_msg[2+j+2] & 0x80){
          break;
        }

        io.set(tmp_rx_msg[2+j+2] & 0x7F, (enum IO_event)(tmp_rx_msg[2+j+1] & 0xF));
      }

    }
    //End SetOutput message
  }
  else if(tmp_rx_msg[0] == RNet_OPC_SetAllOutput){
    uint8_t j = 0;
    for(uint8_t i = 0; i < tmp_rx_msg[2]; i+=2, j++){
      io.set(i, (enum IO_event)(tmp_rx_msg[3+j] >> 4));

      if(i+1 >= tmp_rx_msg[2]){
        break;
      }

      io.set(i+1, (enum IO_event)(tmp_rx_msg[3+j] & 0xF));
    }
  }

  else if(tmp_rx_msg[0] == RNet_OPC_ChangeID){
    if(tmp_rx_msg[2] != 0 && tmp_rx_msg[2] != 0xFF){
      eeprom_write_byte(&EE_Mem.ModuleID, tmp_rx_msg[2]);

      tx.buf[tx.write_index++] = RNet_OPC_ACK;
      tx.buf[tx.write_index++] = RNet_OPC_ACK ^ RNET_CHECKSUM_SEED;
      txdata = 1;
    }
  }
  else if(tmp_rx_msg[0] == RNet_OPC_ChangeNode){
    if(tmp_rx_msg[2] != 0 && tmp_rx_msg[2] != 0xFF){
      eeprom_write_byte(&EE_Mem.NodeID, tmp_rx_msg[2]);

      tx.buf[tx.write_index++] = RNet_OPC_ACK;
      tx.buf[tx.write_index++] = RNet_OPC_ACK ^ RNET_CHECKSUM_SEED;
      txdata = 1;
    }
  }
  else if(tmp_rx_msg[0] == RNet_OPC_ReadEEPROM){
    uint8_t checksum = RNET_CHECKSUM_SEED ^ RNet_OPC_ReadEEPROM ^ sizeof(struct _EE_Mem);
    tx.buf[tx.write_index++] = RNet_OPC_ReadEEPROM;
    tx.buf[tx.write_index++] = sizeof(struct _EE_Mem);
    for(uint8_t i = 0; i < sizeof(struct _EE_Mem); i++){
      checksum = checksum ^ tx.buf[tx.write_index];
      tx.buf[tx.write_index++] = eeprom_read_byte((&EE_Mem.ModuleID) + i);
    }
    tx.buf[tx.write_index++] = checksum;
    txdata = 1;
  }
}
#endif

uint8_t RNet::getMsgSize(struct _RNet_buffer * msg){
#ifdef RNET_MASTER
  uint8_t r = (msg->read_index + 1) % RNET_MAX_BUFFER;
  if( (uint8_t)(msg->write_index - msg->read_index) % RNET_MAX_BUFFER < 2){
    return RNet_msg_len_NotWhole;
  }
#else
  uint8_t r = msg->read_index;
#endif
  if(msg->buf[r] == RNet_OPC_DEV_ID ||
     msg->buf[r] == RNet_OPC_SetEmergency ||
     msg->buf[r] == RNet_OPC_RelEmergency ||
     msg->buf[r] == RNet_OPC_PowerON ||
     msg->buf[r] == RNet_OPC_PowerOFF ||
     msg->buf[r] == RNet_OPC_ResetALL){
    return 1;
  }
  else if (msg->buf[r] == RNet_OPC_ACK){
   return 2;
  }
  else if(msg->buf[r] == RNet_OPC_ChangeID ||
          msg->buf[r] == RNet_OPC_SetCheck){
    return 4;
  }
  else if (msg->buf[r] == RNet_OPC_SetOutput ||
           msg->buf[r] == RNet_OPC_ChangeNode){
    return 5;
  }
  else if(msg->buf[r] == RNet_OPC_SetBlink){
    return 8;
  }
  else if(msg->buf[r] == RNet_OPC_ReadAll){
    if( (uint8_t)(msg->write_index - msg->read_index) % RNET_MAX_BUFFER < 3){
      return RNet_msg_len_NotWhole; // Not enough bytes
    }
    return 4+msg->buf[(r+1)%RNET_MAX_BUFFER];
  }
  else if(msg->buf[r] == RNet_OPC_SetAllOutput){
    if( (uint8_t)(msg->write_index - msg->read_index) % RNET_MAX_BUFFER < 4){
      return RNet_msg_len_NotWhole; // Not enough bytes
    }
    return (msg->buf[(r+2)%RNET_MAX_BUFFER] + 1) / 2 + 4;
  }
  else if(msg->buf[r] == RNet_OPC_ReadInput){
    if( (uint8_t)(msg->write_index - msg->read_index) % RNET_MAX_BUFFER < 4){
      return RNet_msg_len_NotWhole; // Not enough bytes
    }
    uart.transmit("ORI\n", 4);
    return msg->buf[(r+2)%RNET_MAX_BUFFER] + 4;
  }
  return 0;
}

// uint8_t * currentMsg(struct _RNet_buffer * msg){
//   msg->buf[0] = 'H';
//   msg->buf[1] = 'e';
//   msg->buf[2] = 'l';
//   msg->buf[3] = 'l';
//   msg->buf[4] = 'o';
//   msg->buf[5] = ' ';
//   msg->buf[6] = 'W';
//   msg->buf[7] = 'o';
//   msg->buf[8] = 'r';
//   msg->buf[9] = 'l';
//   msg->buf[10] = 'd';
//   msg->buf[11] = '!';

//   return msg->buf;
// }

#ifdef RNET_MASTER
void RNet::init()
#else
void RNet::init (uint8_t dev, uint8_t node)
#endif
{
#ifdef RNET_MASTER
  for(uint8_t i = 0; i < 32; i++)
    devices_list[i] = 0;

  // uart.transmit("RNet Master Init\n",17);
#else
  dev_id = dev;
  node_id = node;
  txdata = 0;
  uart.transmit("RNet Slave Init\n",16);
#endif

  RNET_TX_SET_HIGH; // Write pull up and will become high output HIGH
  _set_out(DDR(RNET_TX_PORT), RNET_TX_pin); //Set as output

  _set_in(DDR(RNET_RX_PORT), RNET_RX_pin);   //Set as input
  //_set_low(PORT(RNET_RX_PORT), RNET_TX_pin); //Disable pull-resistor
  // _set_high(PORT(RNET_RX_PORT), RNET_RX_pin); //Enable pull-resistor for debug

  _set_out(PORT(RNET_DUPLEX_PORT), RNET_DUPLEX_pin); //Set as output
  RNET_DUPLEX_SET_RX;

  state = IDLE;

  cli(); //Disable interupts

  _TIM_CRA = 0;
  _TIM_CRB = _TIM_CTC;   // Init CTC mode, keep timer halted (no prescaler)
  RNET_ENABLE_ISR_COMPA; // OCIE1A ISR mask

  //Only slaves
  #ifndef RNET_MASTER
    RNET_ENABLE_ISR_CAPT; // TICIE1 ISR mask
  #endif

  sei(); //Enable interupts
}

uint8_t * msg;
uint8_t cAddr = 0;
uint8_t * cMsg = 0; // current message pointer
uint8_t cBy = 0; //currentByte
uint8_t cBi = 0; //currentBit
uint8_t cLen = 0; //current messageLength


// bool RNet::checkTxReady(){
//   uint8_t size = getMsgSize(&tx);
//   if((tx.read_index + size) % RNET_MAX_BUFFER == tx.write_index){
//     //Check checksum
//     uint8_t checksum = RNET_CHECKSUM_SEED;
//     for(uint8_t i = 0; i < (size-1); i++){
//       checksum ^= tx.buf[tx.read_index+i];
//     }
//     if(checksum == tx.buf[tx.write_index-1]){
//       return true;
//     }
//   }
//   return false;
// }
#ifdef RNET_MASTER
void RNet::try_transmit(){
  if(tx.write_index != tx.read_index){
    uint8_t readlen = (uint8_t)(tx.write_index - tx.read_index) % RNET_MAX_BUFFER;
    uint8_t size = getMsgSize(&tx);
    if(size == RNet_msg_len_NotWhole){
      return;
    }
    if(readlen >= size){
      transmit(&tx);
    }
  }
}

void RNet::request_all(){
  //broadcast to enable detection

  for(uint8_t i = 0; i < 32; i++){
    devices_list[i] = 0;
  }


  // scan all addresses (0 master, 255 broadcast)
  for(uint8_t i = 1; i < 20; i++){
    transmit(i);

    if(state != TIMEOUT){
      // Device detected
      #ifdef RNET_DEBUG
      uart.transmit('*');
      uart.transmit(i, HEX);
      uart.transmit('\n');
      #endif
      devices_list[i / 8] |= 1<<(i % 8);
    }
    state = IDLE;
  }

  uart.transmit(0x01);
  for(uint8_t i = 0; i < 32; i++){
    uart.transmit(devices_list[i]);
  }

  rx.write_index = 0;
  rx.read_index = 0;
}

void RNet::request_registered(){
  for(uint8_t i = 1; i < 20; i++){
    if(devices_list[i / 8] & (1<<(i % 8))){
      // Device available
      transmit(i);

      if(state == TIMEOUT){
        state = IDLE;
      }
    }
  }
}

status RNet::transmit(uint8_t addr){
  //Check if Bus is IDLE (Last check)
  if(state != IDLE){
    #ifdef RNET_DEBUG
    uart.transmit('-');
    #endif
    return BUSY;
  }

  cMsg = 0;
  cLen = 0; //current messageLength
  cAddr = addr;
  cBy = 0;
  cBi = 0;

  _transmit();

  while(state != IDLE && state != TIMEOUT){}

  return OK;
}

status RNet::transmit(struct _RNet_buffer * buf){
  //Check if Bus is IDLE (Last check)
  if(state != IDLE){
    #ifdef RNET_DEBUG
    uart.transmit('-');
    #endif
    return BUSY;
  }

  cAddr = buf->buf[buf->read_index];
  cLen = getMsgSize(buf);         //Get size of message

  if(cLen == RNet_msg_len_NotWhole){
    uart.transmit("Message too short ", 17);
    return FAILED;
  }
  else if(cLen == 0){
    uart.transmit("No Message ", 11);
    return FAILED;
  }


  #ifdef RNET_DEBUG
  else{
    uart.transmit(cLen, HEX, 2);
  }
  uart.transmit(buf->read_index, HEX,2);
  uart.transmit("->", 2);
  uart.transmit(buf->write_index, HEX,2);
  uart.transmit('{');
  for(uint8_t i = buf->read_index; i != buf->write_index; i = (i + 1) % RNET_MAX_BUFFER){
    uart.transmit(buf->buf[i], HEX, 2);
  }
  uart.transmit('}');

  uart.transmit("msglen: ", 8);
  uart.transmit((long)cLen, HEX);
  uart.transmit('\n');
  #endif

  cMsg = &buf->buf[(buf->read_index + 1) % RNET_MAX_BUFFER]; // skip addr
  cBy = 0;
  cBi = 0;

  _transmit();

  while(state != IDLE && state != TIMEOUT){}

  return OK;
}

void RNet::_transmit(){

  //Disable ICP ISR
  cli();

  RNET_DISABLE_ISR_CAPT;
  RNET_CLEAR_ISR_CAPT;

  _TIM_COUNTER = 0;
  _TIM_COMPA = RNET_TICK;

  RNET_TIMER_ENABLE;
  RNET_CLEAR_ISR_COMPA;

  //Start sending
  RNET_DUPLEX_SET_TX;
  RNET_TX_SET_LOW; //Start-bit

  state = ADDR;

  sei(); //Enable Interrupts

  // PORTB ^= 0b00100000;
  // PORTB ^= 0b00100000;

  #ifdef RNET_DEBUG
  uart.transmit(':');
  #endif

}
#endif

// void RNet_add_to_buf(uint8_t * data, uint8_t len, struct _RNet_buffer * buffer){
//   for(int i = 0; i < len; i++){
//     RNet_add_char_to_buf(data[i], buffer);
//   }
// }

// void RNet_add_char_to_buf(uint8_t data, struct _RNet_buffer * buffer){
//   // printHex(buffer->write_index);
//   buffer->buf[buffer->write_index++] = data;
//   if(buffer->write_index > RNET_MAX_BUFFER){
//     buffer->write_index = 0;
//   }
// }

// void add_to_RX_buf(uint8_t b){
//   // printHex(RNet_rx_buffer.write_index);
//   RNet_rx_buffer.buf[RNet_rx_buffer.write_index++] = b;
//   if(RNet_rx_buffer.write_index > RNET_MAX_BUFFER){
//     RNet_rx_buffer.write_index = 0;
//   }
// }

// void readRXBuf(){
//   uart.transmit("\nRX buf: ", 9);
//   while(RNet_rx_buffer.read_index != RNet_rx_buffer.write_index){
//     printHex(RNet_rx_buffer.buf[RNet_rx_buffer.read_index]);
//     uart.transmit(' ');
//     RNet_rx_buffer.read_index++;

//     if(RNet_rx_buffer.read_index > RNET_MAX_BUFFER){
//       RNet_rx_buffer.read_index = 0;
//     }
//   }
//   uart.transmit('\n');
// }

uint8_t * RNet::getBufP(struct _RNet_buffer * buf, uint8_t write){
  if(write)
    return &buf->buf[buf->write_index];
  else
    return &buf->buf[buf->read_index];
}

#ifdef RNET_MASTER

ISR(RNET_TIMER_ISR_vect){ //TIMER1_COMPA_vect
  _TIM_COUNTER = 0;
  _TIM_COMPA = RNET_TICK;
  // PORTB ^= 0b00100000;
  if(net.state == RX){
    if(cBi == 0){ // Start-bit (Acknowledge)
      if(!RNET_READ_RX){
        // Start
        #ifdef RNET_DEBUG
          uart.transmit('+');
        #endif
      }
      else{
        #ifdef RNET_DEBUG
          uart.transmit('-');
        #endif
      }
      cBi++;
      cMsg = &net.rx.buf[net.rx.write_index];
      return;
    }
    else if(cBi < 9){ // A data bit
      cMsg[0] >>= 1;
      if(RNET_READ_RX){
        #ifdef RNET_DEBUG
          uart.transmit('1');
        #endif
        cMsg[0] |= 0x80;
      }
      #ifdef RNET_DEBUG
      else{
          uart.transmit('0');
      }
      #endif

      cBi++;

      return;
    }
    else if(cBi == 9){ // Stop-bit
      net.rx.write_index++;
      if(net.rx.write_index >= RNET_MAX_BUFFER){
        net.rx.write_index = 0;
      }
      cBi++;

      // uart.transmit(cMsg[0]);

      if(RNET_READ_RX){
        // Enable input interrupt
        //  for next byte
        RNET_CLEAR_ISR_CAPT;
        RNET_ENABLE_ISR_CAPT;

        // Continue
        #ifdef RNET_DEBUG
          uart.transmit('C');
        #endif

        _TIM_COMPA = RNET_TIMEOUT;
      }
      else{
        // Stop
        #ifdef RNET_DEBUG
          uart.transmit('s');
        #endif
        // End of transmission
        RNET_TIMER_DISABLE;
        net.state = IDLE;
      }

      #ifdef RNET_DEBUG
      uart.transmit('R');
      uart.transmit(cMsg[0], HEX, 2);
      #endif

    }
    else{
      // Timeout
      #ifdef RNET_DEBUG
      uart.transmit('~');
      #endif
      RNET_TIMER_DISABLE;
      net.state = TIMEOUT;
    }
  }
  else if(net.state == TX){
    if(cBi == 0){ // Start-bit
      #ifdef RNET_DEBUG
        uart.transmit('S');
      #endif
      RNET_TX_SET_LOW;
      cBi++;
      return;
    }
    else if(cBi < 9){ // Data
      if(cMsg[0] & _BV(cBi - 1)){
        #ifdef RNET_DEBUG
          uart.transmit('1');
        #endif
        RNET_TX_SET_HIGH;
      }
      else{
        #ifdef RNET_DEBUG
          uart.transmit('0');
        #endif
        RNET_TX_SET_LOW;
      }
      cBi++;
      return;
    }
    else if(cBi == 9){ //Stop bit
      net.tx.read_index = (net.tx.read_index + 1) % RNET_MAX_BUFFER;
      cMsg = &net.tx.buf[net.tx.read_index];
      if(++cBy < cLen){
        //Data available
        cBi = 0;
        RNET_TX_SET_HIGH;
        #ifdef RNET_DEBUG
          uart.transmit('C');
        #endif
      }
      else{
        RNET_TX_SET_LOW;
        #ifdef RNET_DEBUG
          uart.transmit("s\n", 2);
        #endif
        cBi++;
      }
    }
    else if(cBi == 10){
      RNET_TX_SET_HIGH;
      RNET_DUPLEX_SET_RX;

      _TIM_COMPA = RNET_OFFSET;
      cBi++;

      cBy = 0;
      cMsg= 0;
      cLen= 0;
    }
    else{
      cBi = 0;
      net.state = IDLE;
    }
  }
  else if(net.state == ADDR){
    if(cBi < 8){ // Addr
      if(cAddr & _BV(cBi)){
        #ifdef RNET_DEBUG
          uart.transmit('1');
        #endif
        RNET_TX_SET_HIGH;
      }
      else{
        #ifdef RNET_DEBUG
          uart.transmit('0');
        #endif
        RNET_TX_SET_LOW;
      }
      cBi++;
      return;
    }
    else if(cBi == 8){ // R/W bit
      if(cMsg != 0){
        // Write
        #ifdef RNET_DEBUG
          uart.transmit('W');
        #endif
        RNET_TX_SET_HIGH;
        net.state = TX;

        // Address read, increment read_index
        net.tx.read_index = (net.tx.read_index + 1) % RNET_MAX_BUFFER;
        cBi = 0;
        cBy = 0;
        return;
      }
      else{
        // Read
        #ifdef RNET_DEBUG
          uart.transmit('R');
        #endif
        RNET_TX_SET_LOW;
        cBi++;
        return;
      }
    }
    else if(cBi == 9){
      // Wait for ACK
      RNET_TX_SET_HIGH;
      RNET_DUPLEX_SET_RX;

      #ifdef RNET_DEBUG
        uart.transmit('?');
      #endif

      // Enable input interrupt to capture start RX transmission
      RNET_CLEAR_ISR_CAPT;
      RNET_ENABLE_ISR_CAPT;

      _TIM_COMPA = RNET_TIMEOUT;

      cBi++;

      return;
    }
    else{
      // RNET Timeout occured
      RNET_TIMER_DISABLE;
      net.state = TIMEOUT;

      #ifdef RNET_DEBUG
        uart.transmit('X');
      #endif
    }
  }
  else if(net.state == IDLE){
    RNET_TX_SET_HIGH;
    RNET_DUPLEX_SET_RX;
    // uart.transmit('I');
    RNET_TIMER_DISABLE;
  } 
  // PORTB ^= 0b00100000;
}

#else // RNET_SLAVE

ISR(RNET_TIMER_ISR_vect){ //TIMER1_COMPA_vect
  _TIM_COUNTER = 0;
  _TIM_COMPA = RNET_TICK;
  _TIM_ICRn  = RNET_TICK;
  PORTB ^= 0b00100000;
  if(net.state == RX){
    if(cBi == 0){
      #ifdef RNET_DEBUG
      uart.transmit('S');
      #endif
      cMsg = net.getBufP(&net.rx, 1);
    }
    else if(cBi < 9){
      cMsg[0] >>= 1;
      if(RNET_READ_RX){
        cMsg[0] |= 0x80;
        #ifdef RNET_DEBUG
        uart.transmit('1');
        #endif
      }
      #ifdef RNET_DEBUG
      else{
        uart.transmit('0');
      }
      #endif
    }
    else if(cBi == 9){
      net.rx.write_index++;
      if(net.rx.write_index >= RNET_MAX_BUFFER)
        net.rx.write_index = 0;

      if(RNET_READ_RX){
        #ifdef RNET_DEBUG
        uart.transmit('C');
        #endif

        // Enable input interrupt to capture start next RX transmission
        RNET_CLEAR_ISR_CAPT;
        RNET_ENABLE_ISR_CAPT;
        cBi++;

        _TIM_COMPA = RNET_TIMEOUT;
        return;
      }
      else{
        #ifdef RNET_DEBUG
        uart.transmit('S');
        uart.transmit('\n');
        #endif

        // Enable input interrupt to capture start next master transmission
        RNET_CLEAR_ISR_CAPT;
        RNET_ENABLE_ISR_CAPT;

        RNET_TIMER_DISABLE;

        net.state = IDLE;
        return;
      }
    }
    else{
      net.state = IDLE;
    }
    cBi++;
  }
  else if(net.state == TX){
    if(cBi == 0){ // Start-bit
      #ifdef RNET_DEBUG
        uart.transmit('S');
      #endif
      RNET_TX_SET_LOW;
      cBi++;
      return;
    }
    else if(cBi < 9){ // Data
      if(net.tx.buf[net.tx.read_index] & _BV(cBi - 1)){
        #ifdef RNET_DEBUG
          uart.transmit('1');
        #endif
        RNET_TX_SET_HIGH;
      }
      else{
        #ifdef RNET_DEBUG
          uart.transmit('0');
        #endif
        RNET_TX_SET_LOW;
      }
      cBi++;
      return;
    }
    else if(cBi == 9){ //Stop bit
      net.tx.read_index = (net.tx.read_index + 1) % RNET_MAX_BUFFER;
      if(++cBy < cLen){
        //Data available
        RNET_TX_SET_HIGH;
        cBi = 0;
        #ifdef RNET_DEBUG
          uart.transmit('C');
        #endif
      }
      else{
        RNET_TX_SET_LOW;
        #ifdef RNET_DEBUG
          uart.transmit("s\n", 2);
        #endif
        cBi++;
      }
    }
    else{
      RNET_DUPLEX_SET_RX;
      RNET_TX_SET_HIGH;

      cBi = 0;
      cBy = 0;
      cMsg= 0;
      cLen= 0;
      net.state = IDLE;
      // Enable input interrupt to capture start RX transmission
      RNET_CLEAR_ISR_CAPT;
      RNET_ENABLE_ISR_CAPT;

      RNET_TIMER_DISABLE;

      if(net.tx.read_index == net.tx.write_index){
        net.txdata = 0;
      }
    }
  }
  else if(net.state == ADDR){
    if(cBi == 0){
      cBi++;
      return;
    }
    else if(cBi < 9){ // Addr
      if(RNET_READ_RX){
        #ifdef RNET_DEBUG
          uart.transmit('1');
        #endif
        cAddr = (cAddr >> 1) | 0x80;
      }
      else{
        #ifdef RNET_DEBUG
          uart.transmit('0');
        #endif
        cAddr = (cAddr >> 1);
      }
      cBi++;
      return;
    }
    else if(cBi == 9){ // R/W bit
      if(cAddr != net.dev_id && cAddr != RNET_BROADCAST_MODULE){
        if(RNET_READ_RX){
          #ifdef RNET_DEBUG
          uart.transmit('w');
          #endif
        }
        else{
          #ifdef RNET_DEBUG
          uart.transmit('r');
          #endif
          // Enable input interrupt to capture start RX transmission
          RNET_CLEAR_ISR_CAPT;
          RNET_ENABLE_ISR_CAPT;

          cBi = 10;
          _TIM_COMPA = RNET_TIMEOUT;
        }
        net.state = OTHER;
        return;
      }
      if(RNET_READ_RX){
        #ifdef RNET_DEBUG
          uart.transmit('W'); // Master writing
        #endif
        net.state = RX;
        cBi = 0;
        cBy = 0;
        return;
      }
      else{
        #ifdef RNET_DEBUG
          uart.transmit('R');
        #endif
        if(net.txdata){
          cBi = 0; // Slave writing
          cBy = 0;

          // uart.transmit('+');

          cMsg = net.getBufP(&net.tx, 0);
          cLen = net.getMsgSize(&net.tx);

          RNET_TX_SET_HIGH;
          RNET_DUPLEX_SET_TX;
          net.state = TX;

          #ifdef RNET_DEBUG
          uart.transmit('L');
          uart.transmit(cLen, HEX);
          #endif

          net.txdata = 0;
        }
        else{
          net.state = IDLE;

          #ifdef RNET_DEBUG
          uart.transmit('-');
          #endif

          // Enable input interrupt to capture start new master transmission
          RNET_CLEAR_ISR_CAPT;
          RNET_ENABLE_ISR_CAPT;

          RNET_TIMER_DISABLE;
        }
        return;
      }
    }
  }
  else if(net.state == IDLE){
    RNET_TX_SET_HIGH;
    RNET_DUPLEX_SET_RX;

    RNET_CLEAR_ISR_CAPT;
    RNET_ENABLE_ISR_CAPT;

    RNET_TIMER_DISABLE;
    return;
  }
  else if(net.state == OTHER){
    #ifdef RNET_DEBUG
    if(cBi == 0){
      uart.transmit('S');
    }
    else if(cBi < 9){
      if(RNET_READ_RX){
        uart.transmit('1');
      }
      else{
        uart.transmit('0');
      }
    }



    else if(cBi == 9){
    #else
    if(cBi == 9){
    #endif

      // Stop bit

      if(RNET_READ_RX){
        // Continue
        #ifdef RNET_DEBUG
          uart.transmit('C');
        #endif

        // Enable input interrupt to capture start RX transmission
        RNET_CLEAR_ISR_CAPT;
        RNET_ENABLE_ISR_CAPT;
        cBi++;

        _TIM_COMPA = RNET_TIMEOUT;
        return;
      }
      #ifdef RNET_DEBUG
      // Stop
      else
        uart.transmit('S');
      #endif
    }
    else{
      // Timeout or after transmission
      net.state = IDLE;

      RNET_CLEAR_ISR_CAPT;
      RNET_ENABLE_ISR_CAPT;

      #ifdef RNET_DEBUG
      uart.transmit('E');
      #endif
    }
    cBi++;
  }
  PORTB ^= 0b00100000;
}

#endif

ISR(RNET_RX_ICP_ISR_vect){

  RNET_DISABLE_ISR_CAPT;

  _TIM_COUNTER = 0;
  _TIM_COMPA = RNET_OFFSET;

  RNET_TIMER_ENABLE;
  RNET_CLEAR_ISR_COMPA;

  #ifdef RNET_DEBUG
  uart.transmit('%');
  #endif

  // PORTB ^= 0b00100000;

  #ifdef RNET_MASTER
  if(net.state == ADDR){
    net.rx.buf[net.rx.write_index++] = cAddr;
    #ifdef RNET_DEBUG
    uart.transmit('$');
    #endif
    net.state = RX;
  }
  #else
  if(net.state == IDLE){
    net.state = ADDR;
  }
  #endif

  cBi = 0;
  cBy = 0;
}
