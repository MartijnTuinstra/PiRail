#include <stdint.h>
#include <stdlib.h>
#include "avr/interrupt.h"

#include "main_node.h"

#include "IO.h"
#include "RNet.h"
#include "RNet_msg.h"
#include "RNet_messages.h"
#include "uart.h"
#include "eeprom_layout.h"

#include "util/delay.h"

struct _RNet_buffer RNet_rx_buffer;
struct _RNet_buffer RNet_tx_buffer;
uint8_t tmp_rx_msg[RNET_MAX_BUFFER];

// #define RNET_DEBUG

bool RNet::available(){
  if (state != IDLE)
    return;

  if (rx.read_index != rx.write_index){;
    uint8_t size = getMsgSize(&rx);

#ifdef DEBUG
    uart.transmit(rx.read_index, HEX, 2);
    uart.transmit("->", 2);
    uart.transmit(rx.write_index, HEX, 2);
    uart.transmit('\t');
    uart.transmit(size, HEX, 2);
    uart.transmit('\n');
#endif

    if(size == 0){
      #ifdef DEBUG
      uart.transmit("NOPC\n", 5);
      #endif
      rx.read_index++;
      return false;
    }
    else if(size == RNet_msg_len_NotWhole){
      // Message not complete
      return false;
    }

    if (rx.size() >= size){
      //Copy message and check checksum
      uint8_t checksum = RNET_CHECKSUM_SEED;
      uint8_t i = 0;
      while(rx.read_index != rx.write_index && i != size){
        tmp_rx_msg[i] = rx.read();
        #ifdef DEBUG
        uart.transmit(checksum, HEX, 2);
        uart.transmit(tmp_rx_msg[i], HEX, 2);
        uart.transmit(' ');
        #endif

        checksum ^= tmp_rx_msg[i++];
      }

      #ifdef DEBUG
      uart.transmit(checksum, HEX, 2);
      #endif

      // If wrong checksum discard message
      // checksum XOR checksum == 0
      if(checksum && size != 1){
        #ifdef DEBUG
        uart.transmit("WCS\n", 4);
        #endif
        return false;
      }

      return true;
    }
  }

  return false;
}

#ifndef RNET_MASTER
void writeACK(CircularBuffer * b){
  b->write(RNet_OPC_ACK);
  net.txdata++;
}

void writeNACK(CircularBuffer * b){
  b->write(RNet_OPC_NACK);
  net.txdata++;
}

void RNet::read(){
  if(tmp_rx_msg[0] == RNet_OPC_SetEmergency){
    uart.transmit("SEm\n", 4);

    return;
  }
  else if(tmp_rx_msg[0] == RNet_OPC_RelEmergency){
    uart.transmit("REm\n", 4);

    return;
  }
  else if(tmp_rx_msg[0] == RNet_OPC_PowerON){
    uart.transmit("POn\n", 4);

    return;
  }
  else if(tmp_rx_msg[0] == RNet_OPC_PowerOFF){
    uart.transmit("POff\n", 5);

    return;
  }
  else if(tmp_rx_msg[0] == RNet_OPC_ResetALL){
    uart.transmit("RAll\n", 5);

    return;
  }
  else if(tmp_rx_msg[0] == RNet_OPC_DEV_ID){
    uart.transmit("DEVID\t", 6);
    uart.transmit(net.dev_id);
    uart.transmit('\n');
    reset_bus();

    uart.transmit('!');

    while(tx.read_index < 1){
      _delay_us(100);
    }
    
    uart.transmit('!');

    return;
  }

  else if(tmp_rx_msg[0] == RNet_OPC_ReqReadInput){
    uart.transmit("RQRI\n", 5);
    NotifyInputChange();
    return;
  }

  //Check dev ID in header
  // if(tmp_rx_msg[1] != node_id){
  //   return;
  // }
  
  if(tmp_rx_msg[0] == RNet_OPC_SetOutput){
    uart.transmit("SOut\n", 5);
    union u_IO_event event;
    event.value = tmp_rx_msg[2] & 0x3;
    io.set(tmp_rx_msg[1], event);
    io.pulse_high();
  }
  else if(tmp_rx_msg[0] == RNet_OPC_SetAllOutput){
    uart.transmit("SAOut\n", 6);
    uint8_t i = 0;
    union u_IO_event event;
    for(uint8_t j = 0; j < (tmp_rx_msg[1] * 4); j++){
      event.value = (tmp_rx_msg[2+i] >> ((j%4)*2)) & 0x3;
      io.set(j, event);

      if(j%4==3)
        i++;

      // if(i+1 >= tmp_rx_msg[2]){
      //   break;
      // }
    }

    io.pulse_high();
  }
  else if(tmp_rx_msg[0] == RNet_OPC_ChangeID){
    uart.transmit("Chng\n", 5);
    if(tmp_rx_msg[1] != 0 && tmp_rx_msg[1] != 0xFF){
      eeprom_write_byte(&EE_Mem.ModuleID, tmp_rx_msg[1]);
      dev_id = tmp_rx_msg[1];

      writeACK(&tx);
    }
    else
      writeNACK(&tx);
  }
  else if(tmp_rx_msg[0] == RNet_OPC_ChangeNode){
    uart.transmit("CHNG\n", 5);
    if(tmp_rx_msg[1] != 0 && tmp_rx_msg[1] != 0xFF){
      eeprom_write_byte(&EE_Mem.NodeID, tmp_rx_msg[1]);
      node_id = tmp_rx_msg[1];

      writeACK(&tx);
    }
    else
      writeNACK(&tx);
  }
  else if(tmp_rx_msg[0] == RNet_OPC_SetIO){
    uart.transmit("SetIO\n", 6);

    uint8_t ioPort = tmp_rx_msg[1];
    uint16_t conf  = (tmp_rx_msg[3] << 8) + tmp_rx_msg[2];

    if(ioPort >= MAX_PINS){
      writeNACK(&tx);
      return;
    }

    eeprom_write_word(&EE_Mem.IO[ioPort], conf);

    io.initPin(ioPort);

    writeACK(&tx);
  }
  else if(tmp_rx_msg[0] == RNet_OPC_SetBlink){
    uart.transmit("SetB\n", 4);
    uint16_t value = (tmp_rx_msg[1] << 8) + tmp_rx_msg[2];
    eeprom_write_word(&EE_Mem.settings.blink1, value);
    value = (tmp_rx_msg[3] << 8) + tmp_rx_msg[4];
    eeprom_write_word(&EE_Mem.settings.blink2, value);

    writeACK(&tx);
  }
  else if(tmp_rx_msg[0] == RNet_OPC_SetPulse){
    uart.transmit("SetP\n", 4);
    eeprom_write_byte(&EE_Mem.settings.pulse, tmp_rx_msg[1]);

    writeACK(&tx);
  }
  else if(tmp_rx_msg[0] == RNet_OPC_SetCheck){
    uart.transmit("SetC\n", 4);
    eeprom_write_byte(&EE_Mem.settings.poll, tmp_rx_msg[1]);
    io.pulse_length = io.calculateTimer(tmp_rx_msg[1]*10);

    writeACK(&tx);
  }
  else if(tmp_rx_msg[0] == RNet_OPC_ReadEEPROM){
    uart.transmit("RDEE\n", 5);
    if(sizeof(struct _EE_Mem) > 100){
      uint8_t parts = sizeof(struct _EE_Mem) / 100;

      for (uint8_t i = 0; i < parts; i++){
        uint8_t checksum = RNET_CHECKSUM_SEED ^ (i + 1) ^ parts ^ RNet_OPC_ReadEEPROM ^ sizeof(struct _EE_Mem);
        net.tx.write(RNet_OPC_ReadEEPROM);
        net.tx.write(sizeof(struct _EE_Mem));
        net.tx.write(i + 1);
        net.tx.write(parts);

        for(uint8_t j = 0; j < 100; j++){
          if((uint16_t)(i * 100 + j) > sizeof(struct _EE_Mem)){
            break;
          }

          uint8_t data = eeprom_read_byte((&EE_Mem.ModuleID) + (i*100) + j);
          checksum = checksum ^ data;
          net.tx.write(data);
        }

        net.tx.write(checksum);
        txdata++;

        // Wait untill packet is fully transmitted
        while(tx.write_index != tx.read_index){}
      }
    }
    else{
      uint8_t checksum = RNET_CHECKSUM_SEED ^ RNet_OPC_ReadEEPROM ^ sizeof(struct _EE_Mem);
      net.tx.write(RNet_OPC_ReadEEPROM);
      net.tx.write(sizeof(struct _EE_Mem));
      net.tx.write(1);
      net.tx.write(1);

      for(uint8_t i = 0; i < sizeof(struct _EE_Mem); i++){
        uint8_t data = eeprom_read_byte((&EE_Mem.ModuleID) + i);
        checksum = checksum ^ data;
        net.tx.write(data);
      }
      
      net.tx.write(checksum);
      txdata++;
    }
  }
}
#endif

uint8_t RNet::getMsgSize(CircularBuffer * msg){
#ifdef RNET_MASTER
  uint8_t r = (msg->read_index + 1) % CB_MAX_BUFFER;
  if( (uint8_t)(msg->write_index - msg->read_index) % CB_MAX_BUFFER < 2){
    return RNet_msg_len_NotWhole;
  }
#else
  uint8_t r = msg->read_index;
#endif
  return getMsgSize(msg, r);
}

inline uint8_t RNet::getMsgSize(CircularBuffer * msg, uint8_t offset){
  if(msg->buf[offset] == RNet_OPC_DEV_ID ||
     msg->buf[offset] == RNet_OPC_SetEmergency ||
     msg->buf[offset] == RNet_OPC_RelEmergency ||
     msg->buf[offset] == RNet_OPC_PowerON ||
     msg->buf[offset] == RNet_OPC_PowerOFF ||
     msg->buf[offset] == RNet_OPC_ResetALL ||
     msg->buf[offset] == RNet_OPC_ReqReadInput ||
     msg->buf[offset] == RNet_OPC_ACK ||
     msg->buf[offset] == RNet_OPC_NACK){
    return 1;
  }
  else if(msg->buf[offset] == RNet_OPC_ChangeID   ||
          msg->buf[offset] == RNet_OPC_ChangeNode ||
          msg->buf[offset] == RNet_OPC_SetCheck   ||
          msg->buf[offset] == RNet_OPC_SetPulse){
    return 3;
  }
  else if (msg->buf[offset] == RNet_OPC_SetOutput){
    return 4;
  }
  else if(msg->buf[offset] == RNet_OPC_SetIO){
    return 5;
  }
  else if(msg->buf[offset] == RNet_OPC_SetBlink){
    return 8;
  }
  else if(msg->buf[offset] == RNet_OPC_ReadAll){
    if( (uint8_t)(msg->write_index - offset) % CB_MAX_BUFFER < 3){
      return RNet_msg_len_NotWhole; // Not enough bytes
    }
    return 4+msg->buf[(offset+1)%CB_MAX_BUFFER];
  }
  else if(msg->buf[offset] == RNet_OPC_SetAllOutput){
    if( (uint8_t)(msg->write_index - offset) % CB_MAX_BUFFER < 2){
      return RNet_msg_len_NotWhole; // Not enough bytes
    }
    return msg->buf[(offset+1)%CB_MAX_BUFFER] + 3;
  }
  else if(msg->buf[offset] == RNet_OPC_ReadInput  || 
          msg->buf[offset] == RNet_OPC_ReadEEPROM){
    if( (uint8_t)(msg->write_index - offset) % CB_MAX_BUFFER < 4){
      return RNet_msg_len_NotWhole; // Not enough bytes
    }
    return msg->buf[(offset+2)%CB_MAX_BUFFER] + 4;
  }
  return 0;
}

#ifdef RNET_MASTER
void RNet::init()
#else
void RNet::init (uint8_t dev, uint8_t node)
#endif
{
#ifdef RNET_MASTER
  for(uint8_t i = 0; i < 32; i++)
    devices_list[i] = 0;

  #ifdef RNET_DEBUG
  uart.transmit("RNet Master Init\n",17);
  #endif
#else
  dev_id = dev;
  node_id = node;
  txdata = 0;
  #ifdef RNET_DEBUG
  uart.transmit("RNet Slave Init\t",16);
  uart.transmit(dev, HEX, 2);
  uart.transmit('\n');
  #endif
#endif

  RNET_TX_SET_HIGH; // Write pull up and will become high output HIGH
  _set_out(DDR(RNET_TX_PORT), RNET_TX_pin); //Set as output

  _set_in(DDR(RNET_RX_PORT), RNET_RX_pin);   //Set as input
  _set_high(PORT(RNET_RX_PORT), RNET_RX_pin); //Enable pull-resistor for debug

  _set_out(DDR(RNET_DUPLEX_PORT), RNET_DUPLEX_pin); //Set as output
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


void RNet::reset_bus(){
#ifdef RNET_MASTER
  tx.buf[0] = 0xFF;
  tx.buf[1] = RNet_OPC_DEV_ID;

  tx.write_index = 2;
  tx.read_index = 0;
  _delay_ms(500);
  try_transmit(); // includes request all

  tx.buf[0] = 0;
  tx.buf[1] = 0;
  tx.write_index = 0;
  tx.read_index = 0;
#else // Slave
  net.tx.buf[0] = RNet_OPC_DEV_ID;
  net.tx.write_index = 1;
  net.tx.read_index = 0;
  net.txdata = 1;

  net.rx.write_index = 0;
  net.rx.read_index = 0;
#endif
}

uint8_t * msg;
uint8_t cAddr = 0;
uint8_t * cMsg = 0; // current message pointer
uint8_t cdataByte = 0; // current message pointer
uint8_t cBy = 0; //currentByte
uint8_t cBi = 0; //currentBit
uint8_t cLen = 0; //current messageLength
uint8_t cStart = 0;


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
    uint8_t readlen = tx.size();
    uint8_t size = getMsgSize(&tx);
    if(size == RNet_msg_len_NotWhole){
      return;
    }
    else if(size == 0){
      return;
    }

    // uart.transmit("tt: ", 4);
    // uart.transmit(size, HEX, 2);
    // uart.transmit(readlen, HEX, 2);
    // uart.transmit('\n');

    if(readlen > 1 && tx.peek(0) == 0xFF && tx.peek(1) == 0x01){
      transmit(&tx);
      _delay_ms(1000);
      request_all();
    }
    else if(readlen > size){
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
      #endif
      devices_list[i / 8] |= 1<<(i % 8);
    }
    #ifdef RNET_DEBUG
    uart.transmit(i, HEX);
    uart.transmit('|');
    #endif
    state = IDLE;
    _delay_ms(10);
  }

  uart.transmit(0xFF);
  uart.transmit(0x01);

  uint8_t checksum = RNET_CHECKSUM_SEED ^ 0x01;
  for(uint8_t i = 0; i < 32; i++){
    uart.transmit(devices_list[i]);
    checksum ^= devices_list[i];
  }
  uart.transmit(checksum);

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
  cdataByte = 0;

  _transmit();

  while(state != IDLE && state != TIMEOUT){}

  return OK;
}

status RNet::transmit(CircularBuffer * buf){
  //Check if Bus is IDLE (Last check)
  if(state != IDLE){
    #ifdef RNET_DEBUG
    uart.transmit('-');
    #endif
    return BUSY;
  }

  cLen = getMsgSize(buf);         //Get size of message
  cAddr = buf->read();
  
  if(!(devices_list[cAddr / 8] & (1 << (cAddr % 8))) && cAddr != 0xFF){
    for(uint8_t i = 0; i < cLen; i++)
      buf->read();
    // uart.transmit('X');
    return FAILED;
  }

  if(cLen == RNet_msg_len_NotWhole){
    // uart.transmit("Message too short ", 17);
    return FAILED;
  }
  else if(cLen == 0){
    // uart.transmit("No Message ", 11);
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
  uint8_t size = buf->size();
  for(uint8_t i = 0; i != size; i++){
    uart.transmit(buf->peek(i), HEX, 2);
  }
  uart.transmit('}');

  uart.transmit("msglen: ", 8);
  uart.transmit((long)cLen, HEX);
  uart.transmit('\n');
  #endif

  cdataByte = buf->read(); // skip addr
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

volatile uint8_t * RNet::getBufP(CircularBuffer * buf, uint8_t write){
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
      cBi++;
      cdataByte = 0;
      return;
    }
    else if(cBi < 9){ // A data bit
      cdataByte >>= 1;
      if(RNET_READ_RX){
        #ifdef RNET_DEBUG
          uart.transmit('1');
        #endif
        cdataByte |= 0x80;
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
      net.rx.write(cdataByte);
      cBi++;
      cBy++;

      if(RNET_READ_RX){
        // Enable input interrupt
        //  for next byte
        RNET_CLEAR_ISR_CAPT;
        RNET_ENABLE_ISR_CAPT;

        // Continue
        #ifdef RNET_DEBUG
          uart.transmit('C');
        #endif

        // _TIM_COMPA = RNET_TIMEOUT;
        cBi++; // Goto to timeout if no signal is captured

        if(cLen == RNet_msg_len_NotWhole){
          cLen = net.getMsgSize(&net.rx, cStart);
          if(cLen == 0)
            cLen = RNet_msg_len_NotWhole;
        }
        else if(cLen <= cBy){
          cBy = 0;
          cLen = RNet_msg_len_NotWhole;

          // net.rx.buf[net.rx.write_index] = cAddr;
          // net.rx.write_index = (net.rx.write_index + 1) % RNET_MAX_BUFFER;
          // cStart = net.rx.write_index;
        }
      }

      #ifdef RNET_DEBUG
      uart.transmit('R');
      uart.transmit(cdataByte, HEX, 2);
      #endif

    }
    else if(cBi == 10){
      // End of Message
      #ifdef RNET_DEBUG
      uart.transmit('*');
      #endif
      RNET_TIMER_DISABLE;
      net.state = IDLE;
    }
    else{
      #ifdef RNET_DEBUG
      uart.transmit('~');
      #endif
      if(!RNET_READ_RX){
        // ICP Trigger failed
        //  but start bit found

        RNET_DISABLE_ISR_CAPT;
        RNET_CLEAR_ISR_COMPA;
        cBi = 1;
        cdataByte = 0;
        return;
      }

      // Timeout
      #ifdef RNET_DEBUG
      uart.transmit('T');
      #endif
      // net.tx.read_index = (net.tx.read_index + 1) % RNET_MAX_BUFFER;
      // net.rx.buf[net.rx.write_index] = 0xAB;
      // net.tx.read_index = (net.tx.read_index + 1) % RNET_MAX_BUFFER;
      // net.rx.buf[net.rx.write_index] = 0xDC;
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
      if(cdataByte & _BV(cBi - 1)){
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
      if(++cBy < cLen){
        //Data available
        cBi = 0;
        cdataByte = net.tx.read();
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
      cdataByte = 0;
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
      if(cdataByte != 0){
        // Write
        RNET_TX_SET_HIGH;
        #ifdef RNET_DEBUG
          uart.transmit('W');
        #endif
        net.state = TX;

        cBi = 0;
        cBy = 0;
        return;
      }
      else{
        // Read
        RNET_TX_SET_LOW;
        #ifdef RNET_DEBUG
          uart.transmit('R');
        #endif
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
  // PORTC ^= 0b1;
  if(net.state == RX){
    if(cBi == 0){
      #ifdef RNET_DEBUG
      uart.transmit('S');
      #endif
      cdataByte = 0;
    }
    else if(cBi < 9){
      cdataByte >>= 1;
      if(RNET_READ_RX){
        cdataByte |= 0x80;
        #ifdef RNET_DEBUG
        //uart.transmit('1');
        #endif
      }
      #ifdef RNET_DEBUG
      else{
        //uart.transmit('0');
      }
      #endif
    }
    else if(cBi == 9){
      net.rx.write(cdataByte);

      if(RNET_READ_RX){
        #ifdef RNET_DEBUG
        //uart.transmit('C');
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
        //uart.transmit('S');
        //uart.transmit('\n');
	      uart.transmit(cdataByte, HEX, 2);
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
      //uart.transmit('S');
      #endif
      RNET_TX_SET_LOW;
      cBi++;
      cdataByte = net.tx.read();
      return;
    }
    else if(cBi < 9){ // Data
      if(cdataByte & _BV(cBi - 1)){
        #ifdef RNET_DEBUG
        //uart.transmit('1');
        #endif
        RNET_TX_SET_HIGH;
      }
      else{
        #ifdef RNET_DEBUG
        //uart.transmit('0');
        #endif
        RNET_TX_SET_LOW;
      }
      cBi++;
      return;
    }
    else if(cBi == 9){ //Stop bit
      if(++cBy < cLen){
        //Data available
        RNET_TX_SET_HIGH;
        cBi = 0;
        #ifdef RNET_DEBUG
        //uart.transmit('C');
        #endif
      }
      else{
        RNET_TX_SET_LOW;
        #ifdef RNET_DEBUG
        //uart.transmit("s\n", 2);
        #endif
        cBi++;
      }
      #ifdef RNET_DEBUG
      uart.transmit(cdataByte, HEX, 2);
      #endif

      return;
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
        //uart.transmit('1');
        #endif
        cAddr = (cAddr >> 1) | 0x80;
      }
      else{
        #ifdef RNET_DEBUG
        //uart.transmit('0');
        #endif
        cAddr = (cAddr >> 1);
      }
      cBi++;
      return;
    }
    else if(cBi == 9){ // R/W bit
      bool forThisNode  = (cAddr == net.dev_id) || (cAddr == RNET_BROADCAST_ID);

      if(!forThisNode){
        if(RNET_READ_RX){
          #ifdef RNET_DEBUG
          uart.transmit('w');
          #endif
          cBi = 0;
          cBy = 0;
        }
        else{
          #ifdef RNET_DEBUG
          uart.transmit('r');
          #endif
          // Enable input interrupt to capture start RX transmission
          RNET_CLEAR_ISR_CAPT;
          RNET_ENABLE_ISR_CAPT;

          cBi = 10;
          cBy = 0;
          _TIM_COMPA = RNET_TIMEOUT;
        }
        net.state = OTHER;
        return;
      }

      // For this node
      if(RNET_READ_RX){ // Master writing to node
        #ifdef RNET_DEBUG
        uart.transmit('W');
        #endif
        net.state = RX;
        cBi = 0;
        cBy = 0;
        return;
      }
      else{ // Master request Reading from node (wait one tick extra)
        #ifdef RNET_DEBUG
        uart.transmit('R');
        #endif
        if(net.txdata){
          cBi++;

          net.txdata--;
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
    else if(cBi == 10){ // Master Reading from node
      cBi = 0; // Slave writing
      cBy = 0;

      #ifdef RNET_DEBUG
      uart.transmit('+');
      #endif

      cLen = net.getMsgSize(&net.tx);

      RNET_TX_SET_HIGH;
      RNET_DUPLEX_SET_TX;
      net.state = TX;

      #ifdef RNET_DEBUG
      //uart.transmit('L');
      //uart.transmit(cLen, HEX);
      #endif
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
    if(cBi == 0){}else if(cBi < 9){
      cBy >>= 1;
      if(RNET_READ_RX){
        cBy |= 0x80;
      }
    }
    else if(cBi == 9){
      #ifdef RNET_DEBUG
      uart.transmit(cBy, HEX, 2);
      #endif
      cBy = 0;
      if(RNET_READ_RX){
        // Enable input interrupt to capture start next slave transmission
        RNET_CLEAR_ISR_CAPT;
        RNET_ENABLE_ISR_CAPT;
        cBi++;

        _TIM_COMPA = RNET_TIMEOUT;
        return;
      }
      else{
        // Enable input interrupt to capture start next master transmission
        RNET_CLEAR_ISR_CAPT;
        RNET_ENABLE_ISR_CAPT;

        RNET_TIMER_DISABLE;

        net.state = IDLE;
        return;
      }
    }
    else{
      #ifdef RNET_DEBUG
      uart.transmit('T');
      #endif
      RNET_TIMER_DISABLE;
      net.state = IDLE;
    }
    cBi++;
  }
  // PORTB ^= 0b00100000;
}

#endif

ISR(RNET_RX_ICP_ISR_vect){
  RNET_DISABLE_ISR_CAPT;

  _TIM_COUNTER = 0;
  _TIM_COMPA = RNET_OFFSET;

  RNET_TIMER_ENABLE;
  RNET_CLEAR_ISR_COMPA;

  // PORTB ^= 0b00100000;

  #ifdef RNET_MASTER
  if(net.state == ADDR){
    net.rx.write(cAddr);
    cStart = net.rx.write_index;
    #ifdef RNET_DEBUG
    uart.transmit('$');
    #endif
    net.state = RX;
    cLen = RNet_msg_len_NotWhole;
    cBy = 0;
  }
  #else
  if(net.state == IDLE){
    net.state = ADDR;
    #ifdef RNET_DEBUG
    uart.transmit('|');
    #endif
    cBy = 0;
  }
  #endif

  #ifdef RNET_DEBUG
  uart.transmit('%');
  #endif

  cBi = 0;
}
