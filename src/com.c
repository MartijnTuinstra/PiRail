#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include <string.h>

#include "com.h"

#include "system.h"

#include "rail.h"
#include "switch.h"
#include "signals.h"
#include "train.h"
#include "logger.h"

#include "modules.h"

#include "submodule.h"
#include "websocket_msg.h"

pthread_mutex_t mutex_UART;

//------------COM PROTOCOL------------//
//Is described in the techincal documentation.

int uart0_filestream = -1;

char COM_ACK = 0;

void * UART(){
  loggerf(INFO, "Starting UART thread");
  //OPEN THE UART

  SYS->UART.state = Module_Init;
  WS_stc_SubmoduleState();

  usleep(200000);
  SYS->UART.state = Module_Run;
  WS_stc_SubmoduleState();

  usleep(3000000);

  // uart0_filestream = open(Serial_Port, O_RDWR | O_NOCTTY);
  if (uart0_filestream == -1)
  {
    //ERROR - CAN'T OPEN SERIAL PORT

    SYS->UART.state = Module_Fail;
    WS_stc_SubmoduleState();
    logger("Unable to open UART",CRITICAL);
    return 0;
  }

  //CONFIGURE THE UART
  struct termios options;
  tcgetattr(uart0_filestream, &options);
  options.c_cflag = Serial_Baud | CS8 | CLOCAL | CREAD;   //<Set baud rate
  options.c_iflag = IGNPAR;
  options.c_oflag = 0;
  options.c_lflag = 0;
  tcflush(uart0_filestream, TCIFLUSH);
  tcsetattr(uart0_filestream, TCSANOW, &options);

  char data[50];
  memset(data,0,50);

  while(SYS->stop == 0){
    // if(COM_Recv(data)){
    //   COM_Parse(data);
    //   memset(data,0,50);
    // }
    usleep(1000);
  }

  //----- CLOSE THE UART -----
  close(uart0_filestream);
  return 0;
}