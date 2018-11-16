#include "Z21.h"
#include "mem.h"
#include "logger.h"

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<pthread.h>
#include<arpa/inet.h>
#include<sys/socket.h>

#include "websocket_msg.h"
#include "submodule.h"

int z21_fd = -1;
_Bool z21_connected = 0;

pthread_mutex_t z21_send_mutex;
char * z21_send_buffer;

void * Z21(){
  pthread_join(z21_thread, NULL);
  z21_connected = 1;
  _SYS->Z21_State = _SYS_Module_Init;
  WS_stc_SubmoduleState();

  loggerf(INFO, "Connecting to Z21");
  int ret = 0;
  ret = Z21_client(Z21_IP, Z21_PORT);
  if(ret == -2){
    //Retry on second port
    ret = Z21_client(Z21_IP, Z21_PORT+1);
  }
  
  if(ret == 1){
    loggerf(INFO, "Connected Succesfully to Z21");
    _SYS->Z21_State = _SYS_Module_Run;
  }
  else{
    loggerf(WARNING, "Failed to connect to Z21");
    z21_connected = 0;
    _SYS->Z21_State = _SYS_Module_Fail;
  }
  WS_stc_SubmoduleState();
  pthread_create(&z21_thread, NULL, Z21_run, NULL);
  return 0;
}

int Z21_client(char * ip, uint16_t port){
  loggerf(DEBUG, "Z21: Connecting %s:%d", ip, port);
  struct sockaddr_in z21_addr;

  z21_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if(z21_fd == -1){
    loggerf(CRITICAL, "Cannot create Socket");
    return -1;
  }
  
  struct timeval tv;
  tv.tv_sec = 10;
  tv.tv_usec = 0;
  setsockopt(z21_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

  bzero(&z21_addr, sizeof(z21_addr));
  z21_addr.sin_addr.s_addr = inet_addr(ip);
  z21_addr.sin_port = htons(port);
  z21_addr.sin_family = AF_INET;

  if( connect(z21_fd, (void *)&z21_addr, sizeof(z21_addr)) < 0){
    loggerf(ERROR, "UDP Connect faild");
    close(z21_fd);
    return -2;
  }

  char * z21_buf = _calloc(1, 1024);
  z21_send_buffer = _calloc(1, 1024);
  int z21_buf_size = 1024;

  Z21_GET_SERIAL;

  int read_length = read(z21_fd, z21_buf, z21_buf_size);
  loggerf(DEBUG, "Z21: got serial, %d", read_length);
  if(read_length <= 0){
    // Failed to read data
    return -2;
  }

  // tv.tv_sec = 30;
  // setsockopt(z21_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

  _free(z21_buf);
  return 1;
}

void * Z21_run(){
  pthread_join(z21_start_thread, NULL);

  if(!z21_connected){
    return 0;
  }

  char * z21_buf = _calloc(1, 1024);
  int z21_buf_size = 1024;

  Z21_SET_BROADCAST_FLAGS(Z21_BROADCAST_FLAGS);

  while(_SYS->Z21_State == _SYS_Module_Run){
    Z21_recv(z21_buf, read(z21_fd, z21_buf, z21_buf_size));
  }

  z21_connected = 0;

  loggerf(DEBUG, "Z21 LOGOUT");
  Z21_LOGOUT;
    
  _free(z21_buf);
  close(z21_fd);
  return NULL;
}

char Z21_prio_list[05][30];
char Z21_send_list[10][30];

void Z21_recv(char * data, int length){
  if(length < 4){
    return; // Invalid size
  }
  // TODO Implement
  loggerf(DEBUG, "Z21 got %d bytes, %d data-size", length, length-4);
  // char * sdata = _calloc(3, length);
  for(int i = 0; i < length; i++){
    printf("%02X ", data[i]);
  }
  uint16_t d_length = data[0] + (data[1] << 8);
  uint16_t header = data[2] + (data[3] << 8);
  uint8_t checksum;
  for(int i = 4; i < (d_length - 1); i++){
    checksum ^= data[i];
  }
  if(checksum != data[d_length-1]){
    loggerf(INFO, "Z21 wrong checksum");
  }

  switch (header){
    case 0x10: // LAN_GET_SERIAL_NUMBER
      loggerf(TRACE, "LAN_GET_SERIAL_NUMBER");
      break;
    case 0x40: ;
      uint8_t XHeader = data[4];
      if(XHeader == 0x63){ // LAN_X_GET_VERSION
        loggerf(TRACE, "LAN_X_GET_VERSION");
      }
      else if(XHeader == 0x61){ // LAN_X_BC_TRACK_POWER_OFF     0x00
                                // LAN_X_BC_TRACK_POWER_ON      0x01
                                // LAN_X_BC_PROGRAMMING_MODE    0x02
                                // LAN_X_BC_TRACK_SHORT_CIRCUIT 0x08
                                // LAN_X_UNKNOWN_COMMAND        0x82
        if(data[5] == 0){
          loggerf(TRACE, "LAN_X_BC_TRACK_POWER_OFF");
          WS_EmergencyStop();
        }
        else if(data[5] == 0x01){
          loggerf(TRACE, "LAN_X_BC_TRACK_POWER_ON");
          WS_ClearEmergency();
        }
        else if(data[5] == 0x02){
          loggerf(TRACE, "LAN_X_BC_PROGRAMMING_MODE");
        }
        else if(data[5] == 0x08){
          loggerf(TRACE, "LAN_X_BC_TRACK_SHORT_CIRCUIT");
          WS_ShortCircuit();
        }
        else if(data[5] == 0x82){
          loggerf(TRACE, "LAN_X_UNKNOWN_COMMAND");
        }
      }
      else if(XHeader == 0x62){ // LAN_X_STATUS_CHANGED
        loggerf(TRACE, "LAN_X_STATUS_CHANGED");
      }
      else if(XHeader == 0x81){ // LAN_X_BC_STOPPED
        loggerf(TRACE, "LAN_X_BC_STOPPED");
      }
      else if(XHeader == 0xF3){ // LAN_X_GET_FIRMWARE_VERSION
        loggerf(TRACE, "LAN_X_GET_FIRMWARE_VERSION");
      }
      else if(XHeader == 0xEF){ // LAN_X_LOCO_INFO
        loggerf(TRACE, "LAN_X_LOCO_INFO");
      }
      break;
    case 0x51: //LAN_GET_BROADCASTFLAGS
      loggerf(TRACE, "LAN_GET_BROADCASTFLAGS");
      break;

    case 0x84: //LAN_SYSTEMSTATE_DATACHANGED
        loggerf(TRACE, "LAN_SYSTEMSTATE_DATACHANGED");
      break;

    case 0x1A: //LAN_GET_HWINFO
        loggerf(TRACE, "LAN_GET_HWINFO");
      break;

    case 0x60: //LAN_GET_LOCOMODE
        loggerf(TRACE, "LAN_GET_LOCOMODE");
      break;

    case 0x70: //LAN_GET_TURNOUTMODE
        loggerf(TRACE, "LAN_GET_TURNOUTMODE");
      break;

    default:
        loggerf(TRACE, "Z21_UNKNOWN_COMMAND");
      break;
  }

  if(d_length < length){
    length -= d_length;
  }
}

void Z21_send(uint16_t length, uint16_t header, ...){
  // Check if not connected
  if(_SYS->Z21_State == _SYS_Module_Stop || _SYS->Z21_State == _SYS_Module_Fail)
    return;
  // TODO lock z21 mutex
  z21_send_buffer[0] = length & 0x00ff;
  z21_send_buffer[1] = (length & 0xff00) >> 8;
  z21_send_buffer[2] = header & 0x00ff;
  z21_send_buffer[3] = (header & 0xff00) >> 8;
  uint8_t checksum = 0;
  if(length != 4){
    va_list arglist;
    va_start(arglist, header);

    for(int i = 4; i<(length - 1); i++){
      z21_send_buffer[i] = (uint8_t)va_arg(arglist, int);
      checksum ^= z21_send_buffer[i];
    }
    z21_send_buffer[length - 1] = checksum;

    va_end(arglist);
  }
  
  write(z21_fd, z21_send_buffer, length);
  // TODO unlock z21 mutex
}

void Z21_get_train(Trains * T){
  loggerf(ERROR, "Implement Z21_get_train (%x)", (unsigned int)T);
  Z21_LOGOUT;
  Z21_GET_SERIAL;
  Z21_GET_STATUS;
  Z21_TRACKPOWER_ON;
  Z21_TRACKPOWER_OFF;
  Z21_STOP;
  Z21_GET_FIRMWARE_VERSION;
}

void Z21_get_engine(int dcc){
  loggerf(ERROR, " Implement Z21_get_engine (%x)", dcc);
}

void Z21_SET_LOCO_DRIVE(int DCC_Adr,char steps,_Bool dir,char drive){
  loggerf(ERROR, "Implement Z21_SET_LOCO_DRIVE (%x, %x, %x, %x)", DCC_Adr, steps, dir, drive);
}

void Z21_SET_LOCO_FUNCTION(int DCC_Adr,char function_nr,char switch_type){
  loggerf(ERROR, "Implement Z21_SET_LOCO_Function (%x, %x, %x)", DCC_Adr, function_nr, switch_type);
}
