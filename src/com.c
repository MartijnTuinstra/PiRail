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

#include "module.h"

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

  _SYS->UART_State = _SYS_Module_Init;
  WS_stc_SubmoduleState();

  usleep(200000);
  _SYS->UART_State = _SYS_Module_Run;
  WS_stc_SubmoduleState();

  usleep(3000000);

  // uart0_filestream = open(Serial_Port, O_RDWR | O_NOCTTY);
  if (uart0_filestream == -1)
  {
    //ERROR - CAN'T OPEN SERIAL PORT

    _SYS->UART_State = _SYS_Module_Fail;
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

  _SYS_change(STATE_COM_FLAG,0);

  while(_SYS->_STATE & STATE_RUN){
    if(COM_Recv(data)){
      COM_Parse(data);
      memset(data,0,50);
    }
    usleep(1000);
  }

  //----- CLOSE THE UART -----
  close(uart0_filestream);
  return 0;
}

void COM_Send(struct COM_t DATA){
  if (uart0_filestream == -1){
    loggerf(ERROR, "No UART");
    return;
  }

  tcflush(uart0_filestream, TCIFLUSH);

  int count = write(uart0_filestream, &DATA.data[0], DATA.length);    //Filestream, bytes to write, number of bytes to write
  if (count < 0)
  {
    printf("UART TX error\n");
  }else{
    //printf("Count: %i\n",count);
  }
  tcdrain(uart0_filestream);
}

int COM_Recv(char * OUT_Data){
  //Check if the filestream is open
  if(uart0_filestream == -1){
    return 0;
  }
  int index = 0;

  //Create buffer and clear it
  unsigned char data_buffer[256] = {0};
  memset(data_buffer,0,256);
  while(1){
    // Read up to 255 characters from the port if they are there
      unsigned char rx_buffer[255];

      //Filestream, buffer to store in, number of bytes to read (max)
    int rx_length = read(uart0_filestream, (void*)rx_buffer, 255);
    if (rx_length < 0)
    {
      //An error occured (will occur if there are no bytes)
    }
    else if (rx_length == 0)
    {
      //No data waiting
    }
    else if (rx_length == 8)
    {
      rx_buffer[rx_length] = '\0';
      //printf("%i bytes read : %s\n", rx_length, rx_buffer);
      for(int i = 0;i<8;i++){
        data_buffer[index++] = rx_buffer[i];
      }
    }
    else
    {
      //Bytes received
      rx_buffer[rx_length] = '\0';
      //printf("%i bytes read : %s\n", rx_length, rx_buffer);
      for(int i = 0;i<rx_length;i++){
        data_buffer[index++] = rx_buffer[i];
      }
      break;
    }
    return index;
  }

  for(int i = 0;i<index;i++){
    OUT_Data[i] = data_buffer[i];
  }
  return index;
}

uint8_t COM_Packet_Length(char * Data){
  uint8_t Opcode = Data[0];
  if(Opcode == COMopc_EmergencyEn ||
      Opcode == COMopc_EmergencyDis ||
      Opcode == COMopc_PowerON ||
      Opcode == COMopc_PowerOFF   ){
    return 2;
  }else if(Opcode == COMopc_ACK || // Set Acknowledge
      Opcode == COMopc_ReportID      ||
      Opcode == COMopc_ReqOut_Bl    ||
      Opcode == COMopc_ReqIn     ||
      Opcode == COMopc_ReqEEPROM    ){
    return 3;
  }else if(Opcode == COMopc_ChangeDevID  ){ 
    return 4;
  }else if(Opcode == COMopc_SetIN_OUT || // Change input and output
      Opcode == COMopc_TogSinAdr || // Toggle Single Address
      Opcode == COMopc_PulSinAdr || // Pulse Single Address
      Opcode == COMopc_TogBlSinAdr || // Blink Single Address
      Opcode == COMopc_PostSinAdr    // Post Single Input Address
      ){ 
    return 5;
  }else{
    return Data[1];
  }
}

void COM_Parse(char * Data){
  uint8_t length = COM_Packet_Length(Data);

  //Check Checksum
  uint8_t Check = 0xFF;
  for(uint8_t i = 0;i<(length-1);i++){
    Check ^= Data[i];
  }

  if(Check != Data[length]){
    printf("COM - Checksum doesn't match\n");
    return;
  }


  switch (Data[0]){
    case 0x00: //Report ID
      //Add device to device list
      for(uint8_t i = 0;i<unit_len;i++){
        loggerf(ERROR, "FIX DEVICELIST");
        // if(DeviceList[i] != 0){
        //   DeviceList[i] = Data[1];
        // }
      }
      break;
    case 0x01: //Set Emergency STOP
    case 0x02: //Release Emergency STOP
    case 0x03: //Set Power ON
    case 0x04: //Set Power OFF
      break;
    case 0x7F: //Acknowledge
      COM_ACK = 1;
      break;

    case 0x16: //Post Output
    case 0x17: //Post Blink Mask
      COM_ACK = 1;//Response uses same flag
      break;


    case 0x05: //Post Single Input
    case 0x06: //Post Multiple Input
    case 0x07: //Post All input
      break;

    case 0x55: //Post EEPROM Values
      COM_ACK = 1;//Response uses same flag
      break;
  }
}

void COM_change_A_signal(int M){
  COM_change_Output(M);
}

void COM_DevReset(){
  struct COM_t Tx;
  Tx.data[0] = COMopc_RESET;
  Tx.length  = 2;
  Tx.data[1] = 0xFF ^ COMopc_RESET;
  COM_Send(Tx);
}

void COM_set_Output(int M){
  loggerf(ERROR, "FIX COM_set_Output (%i)", M);

  // uint8_t * OutRegs   = (uint8_t *)malloc(((Units[M]->Out_length-1)/8)+1);
  // uint8_t * BlinkMask = (uint8_t *)malloc(((Units[M]->Out_length-1)/8)+1);
  // uint8_t * PulseMask = (uint8_t *)malloc(((Units[M]->Out_length-1)/8)+1);
  // char Out = 0,Blink = 0,Pulse = 0;
  // uint8_t byte,offset;

  // for(int i = 0;i<Units[M]->S_L;i++){
  //   for(int j = 0;j<Units[M]->Signals[i]->length;j++){
  //     byte   = Units[M]->Signals[i]->adr[Units[M]->Signals[i]->state&0x3F] / 8;
  //     offset = Units[M]->Signals[i]->adr[Units[M]->Signals[i]->state&0x3F] % 8;
  //     if(Units[M]->Signals[i]->states[Units[M]->Signals[i]->state&0x3F] == (1 << j)){
  //       OutRegs[byte] |= (1 << offset);
  //       Out++;
  //     }
  //     if(Units[M]->Signals[i]->flash[Units[M]->Signals[i]->state&0x3F] == (1 << j)){
  //       BlinkMask[byte] |= (1 << offset);
  //       Blink++;
  //     }
  //     Units[M]->Signals[i]->state &= 0x3F;
  //   }
  // }

  // for(int i = 0;i<Units[M]->S_L;i++){
  //   //for(int j = 0;j<Units[M]->Sw[i]->length;j++){
  //   if(Units[M]->Sw[i]->len & 0xC0 == 0){ //Pulse Address
  //     Units[M]->Sw[i]->state &= 0x3F;
  //     byte   = Units[M]->Sw[i]->Out[Units[M]->Sw[i]->state & 0x3F] / 8;
  //     offset = Units[M]->Sw[i]->Out[Units[M]->Sw[i]->state & 0x3F] % 8;
  //     PulseMask[byte] |= (1 << offset);
  //     Pulse++;
  //   }else if(Units[M]->Sw[i]->len & 0xC0 == 0x40){// Hold a single Address--------------------------------------------------------------------------- Roadmap: Toggle outputs, pulse multiple
  //     Units[M]->Sw[i]->state &= 0x3F;
  //     byte   = Units[M]->Sw[i]->Out[Units[M]->Sw[i]->state & 0x3F] / 8;
  //     offset = Units[M]->Sw[i]->Out[Units[M]->Sw[i]->state & 0x3F] % 8;
  //     OutRegs[byte] |= (1 << offset);
  //     Out++;
  //   }
  // }

  // if(Out > 0){
  //   printf("Set All Out Addresses:\n");
  //   struct COM_t TxPacket;
  //   TxPacket.data[0] = COMopc_SetAllOut;
  //   TxPacket.data[1] = (Units[M]->Out_length/8)+4;
  //   TxPacket.length = TxPacket.data[1];
  //   TxPacket.data[2] = M;
  //   for(int i = 0;i<(Units[M]->Out_length/8)+1;i++){
  //     TxPacket.data[3+i] = OutRegs[i];
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
  //   TxPacket.length = TxPacket.data[1];
  //   TxPacket.data[2] = M;
  //   for(int i = 0;i<(Units[M]->Out_length/8)+1;i++){
  //     TxPacket.data[3+i] = BlinkMask[i];
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
