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

#include "com.h"

#include "system.h"

#include "rail.h"
#include "switch.h"
#include "signals.h"
#include "train.h"
#include "logger.h"
#include "IO.h"
#include "Z21.h"

#include "mem.h"

#include "modules.h"

#include "submodule.h"
#include "websocket_stc.h"

#include "../avr/RNet_msg.h"

pthread_mutex_t mutex_UART;

//------------COM PROTOCOL------------//
//Is described in the techincal documentation.

int uart0_filestream = -1;

char COM_ACK = 0;
char COM_NACK = 0;

void * UART(void * args){
  loggerf(INFO, "Starting UART thread");
  //OPEN THE UART

  SYS_set_state(&SYS->UART.state, Module_Init);

  struct fifobuffer uartbuffer;

  memset(uartbuffer.buffer, 0, UART_BUFFER_SIZE);
  uartbuffer.read = 0;
  uartbuffer.write = 0;

  if(!UART_Serial_Port){
    SYS_set_state(&SYS->UART.state, Module_SIM_State);
    logger("No UART Device", CRITICAL);
    return 0;
  }

  uart0_filestream = open(UART_Serial_Port, O_RDWR | O_NOCTTY);
  if (uart0_filestream == -1)
  {
    //ERROR - CAN'T OPEN SERIAL PORT

    SYS_set_state(&SYS->UART.state, Module_Fail);
    loggerf(CRITICAL, "Unable to open UART %s", UART_Serial_Port);
    return 0;
  }

  loggerf(INFO, "UART %s opened", Serial_Port);

  //CONFIGURE THE UART
  struct termios options;
  tcgetattr(uart0_filestream, &options);
  options.c_cflag = Serial_Baud | CS8 | CLOCAL | CREAD;   //<Set baud rate
  options.c_iflag = IGNPAR;
  options.c_oflag = 0;
  options.c_lflag = 0;
  options.c_cc[VMIN] = 0;  // Minimum number of characters for noncanonical read
  options.c_cc[VTIME] = 3; // deciseconds
  tcflush(uart0_filestream, TCIFLUSH);
  tcsetattr(uart0_filestream, TCSANOW, &options);

  COM_Reset();

  SYS_set_state(&SYS->UART.state, Module_Run);

  while(SYS->UART.state & Module_Run){
    if(COM_Recv(&uartbuffer)){
      COM_Parse(&uartbuffer);
    }
    usleep(1000);
  }

  //----- CLOSE THE UART -----
  close(uart0_filestream);

  SYS_set_state(&SYS->UART.state, Module_STOP);

  return 0;
}

void COM_Reset(){
  uint8_t DTS_FLAG = TIOCM_DTR;
  ioctl(uart0_filestream,TIOCMBIS,&DTS_FLAG);
  usleep(100000); 
  ioctl(uart0_filestream,TIOCMBIC,&DTS_FLAG);
}

void COM_Send(struct COM_t * DATA){
  if (SYS->UART.state == Module_SIM_State){
    return;
  }
  else if (SYS->UART.state != Module_Run){
    loggerf(ERROR, "No UART");
    return;
  }

  tcflush(uart0_filestream, TCIFLUSH);


  char debug[200];
  char *ptr = debug;

  for(uint8_t i = 0;i<DATA->length;i++){
    ptr += sprintf(ptr, "%02x ", DATA->data[i]);
  }
  loggerf(INFO, "COM TX - %s", debug);

  int count = write(uart0_filestream, &DATA->data[0], DATA->length);    //Filestream, bytes to write, number of bytes to write
  if (count < 0)
  {
    printf("UART TX error\n");
  }else{
    //printf("Count: %i\n",count);
  }
  tcdrain(uart0_filestream);
}

int COM_Recv(struct fifobuffer * buf){
  //Check if the filestream is open
  if(uart0_filestream == -1){
    loggerf(CRITICAL , "UART no filestream");
    SYS_set_state(&SYS->UART.state, Module_Fail);
    return 0;
  }

  while(1){
    //Filestream, buffer to store in, number of bytes to read (max)
    uint8_t size = 32;
    if(buf->write + size > UART_BUFFER_SIZE)
      size = UART_BUFFER_SIZE - buf->write;

    int rx_length = read(uart0_filestream, (void*)&buf->buffer[buf->write], size);
    if (rx_length < 0)
    {
      //An error occured (will occur if there are no bytes)
      loggerf(ERROR, "UART received error.");
    }
    else if (rx_length == 0)
    {
      //No data waiting
    }
    else // some data waiting
    {
      char debug[200];
      char *ptr = debug;

      for(uint8_t i = 0;i<rx_length;i++){
        ptr += sprintf(ptr, "%02x", buf->buffer[i+buf->write]);
      }

      buf->write = (buf->write + rx_length) % UART_BUFFER_SIZE;

      loggerf(INFO, "%i/%i-(%i/%i) bytes read, %d available, %s", rx_length, size, buf->read, buf->write, (uint8_t)(((int16_t)buf->write - (int16_t)buf->read) % UART_BUFFER_SIZE), debug);

      if(rx_length < size)
        break;
    }
    return 1;
  }

  return 1;
}

uint8_t COM_Packet_Length(struct fifobuffer * buf){
  uint8_t r = (buf->read + 1) % UART_BUFFER_SIZE;
  if( (uint8_t)(buf->write - buf->read) % UART_BUFFER_SIZE < 2){
    return UART_Msg_NotComplete;
  }

  if(buf->buffer[r] == RNet_OPC_DEV_ID){
    return 34;
  }
  else if(buf->buffer[r] == RNet_OPC_SetEmergency ||
          buf->buffer[r] == RNet_OPC_RelEmergency ||
          buf->buffer[r] == RNet_OPC_PowerON ||
          buf->buffer[r] == RNet_OPC_PowerOFF ||
          buf->buffer[r] == RNet_OPC_ResetALL || 
          buf->buffer[r] == RNet_OPC_ReqReadInput ||
          buf->buffer[r] == RNet_OPC_ACK ||
          buf->buffer[r] == RNet_OPC_NACK){
    return 2;
  }
  else if(buf->buffer[r] == RNet_OPC_ChangeID   ||
          buf->buffer[r] == RNet_OPC_ChangeNode ||
          buf->buffer[r] == RNet_OPC_SetCheck   ||
          buf->buffer[r] == RNet_OPC_SetPulse){
    return 4;
  }
  else if (buf->buffer[r] == RNet_OPC_SetOutput){
    return 5;
  }
  else if(buf->buffer[r] == RNet_OPC_SetBlink){
    return 9;
  }
  else if(buf->buffer[r] == RNet_OPC_ReadAll){
    if( (uint8_t)(buf->write - buf->read) % UART_BUFFER_SIZE < 3){
      return UART_Msg_NotComplete; // Not enough bytes
    }
    return 5+buf->buffer[(r+1)%UART_BUFFER_SIZE];
  }
  else if(buf->buffer[r] == RNet_OPC_SetAllOutput){
    if( (uint8_t)(buf->write - buf->read) % UART_BUFFER_SIZE < 4){
      return UART_Msg_NotComplete; // Not enough bytes
    }
    return (buf->buffer[(r+2)%UART_BUFFER_SIZE] + 1) / 2 + 5;
  }
  else if(buf->buffer[r] == RNet_OPC_ReadInput ||
          buf->buffer[r] == RNet_OPC_ReadEEPROM){
    if( (uint8_t)(buf->write - buf->read) % UART_BUFFER_SIZE < 4){
      return UART_Msg_NotComplete; // Not enough bytes
    }
    loggerf(INFO, "OPC RIN, READEEPROM %x -> len %d", buf->buffer[(r+2) % UART_BUFFER_SIZE], buf->buffer[(r+2) % UART_BUFFER_SIZE] + 5);
    return buf->buffer[(r+2)%UART_BUFFER_SIZE] + 5;
  }

  loggerf(CRITICAL, "Lost frame spacing");
  return 1;
}

void COM_Parse(struct fifobuffer * buf){
  uint8_t length = COM_Packet_Length(buf);

  if(length == UART_Msg_NotComplete)
    return;

  if(length > (uint8_t)(buf->write - buf->read) % UART_BUFFER_SIZE)
    return;

  uint8_t * data = (uint8_t *)_calloc(length, uint8_t);

  //Check Checksum
  uint8_t Check = UART_CHECKSUM_SEED;

  char debug[200];
  char *ptr = debug;

  for(uint8_t i = 0;i<length;i++){
    data[i] = buf->buffer[buf->read];
    ptr += sprintf(ptr, "%02x ", data[i]);

    if(i != 0) // Don't copy address in checksum
      Check ^= data[i];

    buf->read = (buf->read + 1) % UART_BUFFER_SIZE;
  }

  loggerf(INFO, "COM RECV - (%d) %s", length, debug);


  if(data[1] == RNet_OPC_ACK){
    loggerf(INFO, "ACK");
    COM_ACK = 1;
  }
  else if(data[1] == RNet_OPC_NACK){
    loggerf(WARNING, "NACK");
    COM_NACK = 1;
  }
  else if(data[1] == RNet_OPC_DEV_ID){ //Report ID
    //Add device to device list
    
    for(uint16_t i = 0;i<255;i++){
      if(Units[i])
        Units[i]->on_layout = 0;

      if(data[i/8+2] & (1 << (i%8))){
        loggerf(INFO, "UART Found Module %d", i);
        if(Units[i])
          Units[i]->on_layout = 1;
      }
    }

    // Update all clients with the new set of modules
    WS_stc_Track_Layout(0);

    SYS->UART.modules_found = 1;
  }
  else if(data[1] == RNet_OPC_ReadInput){
    loggerf(INFO, "READIN - %s", &debug[6]);

    if(Check != 0){
      loggerf(WARNING, "Failed Checksum %x", Check);
      return;
    }

    uint8_t module = data[0];
    uint8_t node = data[2];

    if(!Units[module] || node >= Units[module]->IO_Nodes)
      return;

    IO_Node * IO = &Units[module]->Node[node];

    //uint8_t node = data[3];
    uint8_t l = data[3];
    for(uint8_t i = 0; i < l*8; i++){
      if(data[i/8+4] & (1 << (i%8))){
        loggerf(INFO, "%d IO %i HIGH", data[0], i);
        IO_set_input(IO->io[i], 1);
      }
      else{
        loggerf(INFO, "%d IO %i LOW", data[0], i);
        IO_set_input(IO->io[i], 0);
      }
    }
  }
  else if(data[1] == RNet_OPC_ReadEEPROM){
    loggerf(INFO, "EEPROMDUMP");
    loggerf(INFO, "%s", debug);
  }
  else if(data[1] == 0x01){ //Set Emergency STOP
    WS_stc_EmergencyStop();
    Z21_TRACKPOWER_OFF;
  }
  else if(data[1] == 0x02){ //Release Emergency STOP
    WS_stc_ClearEmergency();
    Z21_TRACKPOWER_ON;
  }
  /*
  else if(data[1] == 0x03){ //Set Power ON
  }
  else if(data[1] == 0x04){ //Set Power OFF
  }
  else if(data[1] == 0x7F){ //Acknowledge
    COM_ACK = 1;

  }
  else if(data[1] == RNet_OPC_ReadInput){
    //uint8_t module = data[0];
    //uint8_t id = data[2];
    //uint8_t ports = data[3];
    //for(uint8_t i = 0; i < ports*8; i++){
    //  IO_set_input(module, id, i, data[i/8] & (1 << (i%8)));
    //}
  }
  else if(data[1] == 0x17){ //Post Blink Mask
    COM_ACK = 1;//Response uses same flag
  }
  else if(data[1] == 0x05){ //Post Single Input
  }
  else if(data[1] == 0x06){ //Post Multiple Input
  }
  else if(data[1] == 0x07){ //Post All input

  }
  else if(data[1] == 0x55){ //Post EEPROM Values
    COM_ACK = 1;//Response uses same flag
  }

  _free(data);*/
}

void COM_change_A_signal(int M){
  COM_change_Output(M);
}

void COM_DevReset(){
  struct COM_t Tx;
  Tx.data[0] = 0xFF;  //Broadcast
  Tx.data[1] = RNet_OPC_DEV_ID;
  Tx.length  = 2;
  COM_Send(&Tx);
}

void COM_set_single_Output_output(int M, int io, enum e_IO_output_event event){
  union u_IO_event type;
  type.output = event;
  COM_set_single_Output(M, io, type);
}

void COM_set_single_Output(int M, int io, union u_IO_event type){
  struct COM_t TX;
  TX.data[0] = M;
  TX.data[1] = RNet_OPC_SetOutput;
  TX.data[2] = io + 48;
  TX.data[3] = type.value;
  TX.data[4] = UART_CHECKSUM_SEED ^ RNet_OPC_SetOutput ^ (io + 48) ^ type.value;
  TX.length = 5;
  COM_Send(&TX);
}

void COM_change_Output(int M){
  loggerf(ERROR, "FIX COM_change_Output (%i)", M);

  // uint8_t * OutRegs   = (uint8_t *)malloc(((Units[M]->Out_length-1)/8)+1);
  // uint8_t * BlinkMask = (uint8_t *)malloc(((Units[M]->Out_length-1)/8)+1);
  // uint8_t * PulseMask = (uint8_t *)malloc(((Units[M]->Out_length-1)/8)+1);

  // memcpy(OutRegs  ,Units[M]->OutRegs  ,((Units[M]->Out_length-1)/8)+1);
  // memcpy(BlinkMask,Units[M]->BlinkMask,((Units[M]->Out_length-1)/8)+1);

  // char Out = 0,Blink = 0,Pulse = 0, Toggle = 0;

  // uint8_t * PulseAdr = (uint8_t *)malloc(1);
  // uint8_t * BlinkAdr = (uint8_t *)malloc(1);
  // uint8_t * ToggleAdr = (uint8_t *)malloc(1);

  // uint8_t byte,offset;

  // //Signals
  // for(int i = 0;i<Units[M]->Si_L;i++){
  //   if(!Units[M]->Signals[i]){continue;}
  //   for(int j = 0;j<Units[M]->Signals[i]->length;j++){
  //     if(Units[M]->Signals[i]->state & 0x80){ //If output needs to be updated
  //       Units[M]->Signals[i]->state &= 0x3F; //Output state is updated

  //       byte   = Units[M]->Signals[i]->adr[Units[M]->Signals[i]->state] / 8;
  //       offset = Units[M]->Signals[i]->adr[Units[M]->Signals[i]->state] % 8;

  //       if(Units[M]->Signals[i]->states[Units[M]->Signals[i]->state] == (1 << j)){
  //         //Enable Output
  //         if(!(OutRegs[byte] & (1<<offset))){
  //           //if output is not enabled yet, add to address list
  //           ToggleAdr[Toggle] = Units[M]->Signals[i]->adr[Units[M]->Signals[i]->state];
  //           Toggle++;
  //           ToggleAdr = realloc(ToggleAdr,Toggle+1);
  //         }

  //         OutRegs[byte] |= (1 << offset);
  //       }else{
  //         //Disable Output
  //         if((OutRegs[byte] & (1<<offset))){
  //           //if output is still enabled, add to address list
  //           ToggleAdr[Toggle] = Units[M]->Signals[i]->adr[Units[M]->Signals[i]->state];
  //           Toggle++;
  //           ToggleAdr = realloc(ToggleAdr,Toggle+1);
  //         }

  //         OutRegs[byte] |= (1 << offset);
  //       }
  //       if(Units[M]->Signals[i]->flash[Units[M]->Signals[i]->state] == (1 << j)){
  //         //Enable blink
  //         if(!(OutRegs[byte] & (1<<offset))){
  //           //if blink is not enabled yet, add to address list to enable
  //           BlinkAdr[Blink] = Units[M]->Signals[i]->adr[Units[M]->Signals[i]->state];
  //           Blink++;
  //           BlinkAdr = realloc(BlinkAdr,Blink+1);
  //         }

  //         OutRegs[byte] |= (1 << offset);
  //       }else{
  //         //Disable Output
  //         if((OutRegs[byte] & (1<<offset))){
  //           //if blink is still enabled, add to address list to disable
  //           BlinkAdr[Blink] = Units[M]->Signals[i]->adr[Units[M]->Signals[i]->state];
  //           Blink++;
  //           BlinkAdr = realloc(BlinkAdr,Blink+1);
  //         }

  //         OutRegs[byte] |= (1 << offset);
  //       }
        
  //     }
  //   }
  // }


  // //Switches
  // for(int i = 0;i<Units[M]->S_L;i++){
  //   //for(int j = 0;j<Units[M]->Sw[i]->length;j++){
  //   if(Units[M]->Sw[i]->len & 0xC0 == 0){ //Pulse Address
  //     if(Units[M]->Sw[i]->state & 0x80){
  //       Units[M]->Sw[i]->state &= 0x3F;
  //       byte   = Units[M]->Sw[i]->Out[Units[M]->Sw[i]->state & 0x3F] / 8;
  //       offset = Units[M]->Sw[i]->Out[Units[M]->Sw[i]->state & 0x3F] % 8;
  //       PulseMask[byte] |= (1 << offset);
  //       Pulse++;
  //     }
  //   }else if(Units[M]->Sw[i]->len & 0xC0 == 0x40){// Hold a single Address--------------------------------------------------------------------------- Roadmap: Toggle outputs, pulse multiple
  //     if(Units[M]->Sw[i]->state & 0x80){
  //       Units[M]->Sw[i]->state &= 0x3F;
  //       byte   = Units[M]->Sw[i]->Out[Units[M]->Sw[i]->state & 0x3F] / 8;
  //       offset = Units[M]->Sw[i]->Out[Units[M]->Sw[i]->state & 0x3F] % 8;
  //       OutRegs[byte] |= (1 << offset);
  //       Out++;
  //     }
  //   }
  // }

  // if(Out > 0){
  //   printf("Set All Out Addresses:\n");
  //   struct COM_t TxPacket;
  //   TxPacket.data[0] = COMopc_SetAllOut;
  //   TxPacket.data[1] = (Units[M]->Out_length/8)+4;
  //   for(int i = 0;i<(Units[M]->Out_length/8)+1;i++){
  //     TxPacket.data[2+i] = OutRegs[i];
  //   }
  //   for(int i = 0;i<(Units[M]->Out_length/8)+4;i++){
  //     printf("%02X ",TxPacket.data[i]);
  //   }
  //   printf("\n");
  //   memcpy(Units[M]->OutRegs,OutRegs,((Units[M]->Out_length-1)/8)+1);
  // }
  // if(Blink > 0){
  //   printf("Set Blink Mask:\n");
  //   struct COM_t TxPacket;
  //   TxPacket.data[0] = COMopc_SetBlOut;
  //   TxPacket.data[1] = (Units[M]->Out_length/8)+4;
  //   for(int i = 0;i<(Units[M]->Out_length/8)+1;i++){
  //     TxPacket.data[2+i] = BlinkMask[i];
  //   }
  //   for(int i = 0;i<(Units[M]->Out_length/8)+4;i++){
  //     printf("%02X ",TxPacket.data[i]);
  //   }
  //   printf("\n");
  //   memcpy(Units[M]->BlinkMask,BlinkMask,((Units[M]->Out_length-1)/8)+1);
  // }
  // if(Pulse > 0){
  //   printf("Set Pulse Mask: \n");

  // }

  // free(OutRegs);
  // free(BlinkMask);
  // free(PulseMask);

  // free(PulseAdr);
  // free(BlinkAdr);
  // free(ToggleAdr);
}

void COM_change_signal(Signal * Si){
  if (uart0_filestream != -1){
    loggerf(ERROR, "FIX Signal (%x)", (unsigned int)Si);

    // COM_change_A_signal(Si->MAdr);
    /*
    int M = Si->MAdr;
    struct COM_t C;
    memset(C.Data,0,32);
    C.Adr = M;
    C.Opcode = 0b1010;
    C.Length = 2;
    int nr_signals = Units[M]->Si_L;
    int location = 0;
    int position = 0;
    int Signal_Adr = 0;
    int empty = 0;
    int loc = -1;
    char data;
    printf("Finding Signal 0%o\n",Si->id);
    for(int i = 0;i<=nr_signals;i++){
      if(Units[M]->Signals[i] != NULL){
        printf("Signal found on 0%o\tlocation: %i\tPosition %i\n",Units[M]->Signals[i]->UAdr,location+2,position);
        if(Units[M]->Signals[i]->UAdr == Si->UAdr){
          loc = location;
        }
        out[2+location] += Units[M]->Signals[i]->state << position;
        position += (Units[M]->Signals[i]->type+1);
      }else{
        printf("No Signal\t\tlocation: %i\tPosition %i\n",location+2,position);
        position += 3;
        empty++;
      }
      if(Units[M]->Signals[i+1] != NULL && position >= (8 - Units[M]->Signals[i+1]->type - 1)){
        position = 0;
        if(empty == 2){
          printf("empty set\n");
          //nr_signals--;
        }else{
          location++;
          Signal_Adr++;
        }
        if(loc != -1){
          break;
        }
        empty = 0;
      }else if(position >= 5){
        position = 0;
        if(empty == 2){
          printf("empty set\n");
          //nr_signals--;
        }else{
          location++;
          Signal_Adr++;
        }
        if(loc != -1){
          break;
        }
        empty = 0;
      }
    }
    out[3] = out[loc+2];
    out[2] = loc;
    printf("Sending: [%i][%i]",out[0],out[1]);
    for(int i = 0;i<2;i++){
      printf("[%i]",out[i+2]);
    }
    printf("\n\n");
    int count = write(uart0_filestream, &out[0], 4);    //Filestream, bytes to write, number of bytes to write*/
  }
}

void COM_change_switch(int M){
  loggerf(DEBUG, "COM_change_switch (%i)", M);
  if (uart0_filestream != -1){
    printf("ch\n");
    uint8_t * PulseAdr = (uint8_t *)malloc(1);
    uint8_t * ToggleAdr = (uint8_t *)malloc(1);
    char Pulse = 0;

    for(int i = 0;i<Units[M]->switch_len;i++){
      if(!Units[M]->Sw[i]){
        continue;
      }
      //for(int j = 0;j<Units[M]->Sw[i]->length;j++){
      printf("Switch: %i\t",Units[M]->Sw[i]->id);
      loggerf(ERROR, "FIX Switch len");
      // if((Units[M]->Sw[i]->len & 0xC0) == 0){ //Pulse One Addresses
      //   printf("P%x\t",Units[M]->Sw[i]->state);
      //   if((Units[M]->Sw[i]->state & 0x80) > 0){
      //     Units[M]->Sw[i]->state &= 0x3F;
      //     PulseAdr[Pulse] = Units[M]->Sw[i]->Out[Units[M]->Sw[i]->state];
      //     Pulse++;
      //     PulseAdr = realloc(PulseAdr,Pulse+1);
      //   }
      // }else{
      //   printf("Weird Length bit\n")  ;
      // }// --------------------------------------------------------------------------- Roadmap: Toggle outputs, pulse multiple
      printf("\n");
    }

    printf("%i addresses\n",Pulse);
    
    struct COM_t TxPacket;
    if(Pulse == 1){
      TxPacket.data[0] = 0x11;
    }else if(Pulse > 1){
      TxPacket.data[0] = 0x14;
    }else{
      return;
    }
    TxPacket.data[1] = Pulse+3;
    for(int i = 0;i<Pulse;i++){
      TxPacket.data[i+2] = PulseAdr[i];
    }

    printf("COM Sending: ");
    for(int i = 0;i<(TxPacket.data[1]-1);i++){
      printf("%02X ",TxPacket.data[i]);
    }
    printf("\n\n");
    //Send via UART and get send bytes back
    write(uart0_filestream, TxPacket.data, TxPacket.data[1]-1);
    //Check if all bytes were send

    free(ToggleAdr);
    free(PulseAdr);
  }
}

void COM_update_switch(int M){
  if(M == 0)
    loggerf(INFO, "COM update ALL switches");
  else
    loggerf(INFO, "COM update switches of module %i", M);

  loggerf(ERROR, "IMPLEMENT");
}
