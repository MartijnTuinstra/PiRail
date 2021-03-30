#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include <string.h>
#include <sys/ioctl.h>

#include "uart/uart.h"
#include "uart/RNetRX.h"

#include "system.h"

#include "utils/logger.h"
#include "utils/mem.h"

#include "submodule.h"

#include "RNet_msg.h"

pthread_mutex_t mutex_UART;

//------------COM PROTOCOL------------//
//Is described in the techincal documentation.

UART uart;

UART::UART(){
  setDevice(Default_Serial_Port);
}

void UART::setDevice(const char * newDevice){
  if(strlen(newDevice) > 49)
    loggerf(WARNING, "UART Device is too long (over 50 characters)");
  strncpy(device, newDevice, 49);
}

UART::~UART(){
  if(status != Module_STOP)
    close();
}

int UART::init(){
  fd = open(device, O_RDWR | O_NOCTTY);
  if (fd == -1){
    //ERROR - CAN'T OPEN SERIAL PORT
    updateState(Module_Fail);

    loggerf(CRITICAL, "Unable to open UART %s", device);
    return 0;
  }

  loggerf(INFO, "UART %s opened", device);

  //CONFIGURE THE UART
  struct termios options;
  tcgetattr(fd, &options);
  options.c_cflag = Serial_Baud | CS8 | CLOCAL | CREAD;   //<Set baud rate
  options.c_iflag = IGNPAR;
  options.c_oflag = 0;
  options.c_lflag = 0;
  options.c_cc[VMIN] = 0;  // Minimum number of characters for noncanonical read
  options.c_cc[VTIME] = 3; // deciseconds
  tcflush(fd, TCIFLUSH);
  tcsetattr(fd, TCSANOW, &options);

  resetDevice();

  updateState(Module_Run);

  return 1;
}

void UART::close(){
  //----- CLOSE THE UART -----
  ::close(fd);
}

void * UART::serve(void * args){
  UART * _uart = (UART *)args;

  _uart->init();
  _uart->handle();
  _uart->close();
}

void UART::handle(){
  while(status == Module_Run){
    if(recv()){
      parse();
    }
    usleep(1000);
  }
}

void UART::resetDevice(){
  buffer.clear();

  uint8_t DTS_FLAG = TIOCM_DTR;
  ioctl(fd,TIOCMBIS,&DTS_FLAG);
  usleep(100000);
  tcflush(fd,TCIOFLUSH);
  ioctl(fd,TIOCMBIC,&DTS_FLAG);
  usleep(100000);
}

void UART::send(struct COM_t * DATA){
  char debug[200];
  char *ptr = debug;

  for(uint8_t i = 0;i<DATA->length;i++){
    ptr += sprintf(ptr, "%02x ", DATA->data[i]);
  }

  if(fd <= 0)
    return;

  loggerf(INFO, "COM TX - %s", debug);

  int count = write(fd, &DATA->data[0], DATA->length);
  if (count < 0)
    printf("UART TX error\n");
}

bool UART::recv(){
  //Check if the filestream is open
  if(fd == -1){
    loggerf(CRITICAL , "UART no filestream");
    return 0;
  }

  return buffer.writefromfd(fd);
}

uint8_t COM_Packet_Length(CircularBuffer * buf){
  uint8_t r = 1;
  uint8_t peekAtR = buf->peek(r);
  uint8_t size = buf->size();
  if( size < 2){
    return UART_Msg_NotComplete;
  }

  if(peekAtR == RNet_OPC_DEV_ID){
    return 35;
  }
  else if(peekAtR == RNet_OPC_SetEmergency ||
          peekAtR == RNet_OPC_RelEmergency ||
          peekAtR == RNet_OPC_PowerON ||
          peekAtR == RNet_OPC_PowerOFF ||
          peekAtR == RNet_OPC_ResetALL || 
          peekAtR == RNet_OPC_ReqReadInput ||
          peekAtR == RNet_OPC_ACK ||
          peekAtR == RNet_OPC_NACK){
    return 2;
  }
  else if(peekAtR == RNet_OPC_ChangeID   ||
          peekAtR == RNet_OPC_ChangeNode ||
          peekAtR == RNet_OPC_SetCheck   ||
          peekAtR == RNet_OPC_SetPulse){
    return 4;
  }
  else if (peekAtR == RNet_OPC_SetOutput){
    return 5;
  }
  else if(peekAtR == RNet_OPC_SetBlink){
    return 9;
  }
  else if(peekAtR == RNet_OPC_ReadAll){
    if( size < 3){
      return UART_Msg_NotComplete; // Not enough bytes
    }
    return 5+buf->peek(r+1);
  }
  else if(peekAtR == RNet_OPC_SetAllOutput){
    if( size < 4){
      return UART_Msg_NotComplete; // Not enough bytes
    }
    return (buf->peek(r+2) + 1) / 2 + 5;
  }
  else if(peekAtR == RNet_OPC_ReadInput ||
          peekAtR == RNet_OPC_ReadEEPROM){
    if( size < 4){
      return UART_Msg_NotComplete; // Not enough bytes
    }
    loggerf(TRACE, "OPC RIN, READEEPROM %x -> len %d", buf->peek(r+2), buf->peek(r+2) + 5);
    return buf->peek(r+2) + 5;
  }

  loggerf(CRITICAL, "Lost frame spacing, opcode: %02x", peekAtR);
  return 1;
}

void UART::parse(){
  uint8_t length = COM_Packet_Length(&buffer);

  if(length == UART_Msg_NotComplete)
    return;

  if(length > buffer.size())
    return;

  uint8_t data[128];

  //Check Checksum
  uint8_t Check = UART_CHECKSUM_SEED;

  char debug[200];
  char *ptr = debug;

  for(uint8_t i = 0;i<length;i++){
    data[i] = buffer.read();
    ptr += sprintf(ptr, "%02x ", data[i]);

    if(i != 0) // Don't copy address in checksum
      Check ^= data[i];
  }

  loggerf(INFO, "COM RECV - (%d) %s", length, debug);


  if(length == 1)
    return;
  else if(length > 2 && Check != 0){
    loggerf(WARNING, "Failed Checksum %x", Check);
    return;
  }


  if(UART_RecvCb[data[1]]){
    UART_RecvCb[data[1]](data[0], &data[2]);
    return;
  }
  else if(data[1] == RNet_OPC_ReadEEPROM){
    loggerf(INFO, "EEPROMDUMP");
    loggerf(INFO, "%s", debug);
  }
}

void UART::setState(enum e_SYS_Module_State s){
  status = s;
}

void UART::updateState(enum e_SYS_Module_State s){
  setState(s);

  if(callback)
    callback(s);
}

void UART::setUpdateStatusCb(void (*f)(enum e_SYS_Module_State)){
  callback = f;
}