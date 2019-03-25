#include "Z21.h"
#include "mem.h"
#include "logger.h"
#include "Z21_msg.h"

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

struct s_Z21_info Z21_info;

void Z21_boot(){
  memset(&Z21_info, 0, sizeof(struct s_Z21_info));
  Z21_info.IP[0] = 127;
  Z21_info.IP[3] = 1;
}

void * Z21(){
  pthread_join(z21_thread, NULL);
  z21_connected = 1;
  _SYS->Z21_State = _SYS_Module_Init;
  WS_stc_SubmoduleState();

  loggerf(INFO, "Connecting to Z21");
  int ret = 0;

  char Z21_IP[20];
  sprintf(Z21_IP, "%d.%d.%d.%d", Z21_info.IP[0], Z21_info.IP[1], Z21_info.IP[2], Z21_info.IP[3]);

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

  char * z21_buf = _calloc(1024, 1);
  z21_send_buffer = _calloc(1024, 1);
  int z21_buf_size = 1024;

  Z21_GET_SERIAL;

  int read_length = read(z21_fd, z21_buf, z21_buf_size);
  loggerf(DEBUG, "Z21: got serial, %d", read_length);
  if(read_length <= 0){
    loggerf(WARNING, "Z21 failed to receive data");
    return -2;
  }

  Z21_GET_FIRMWARE_VERSION;
  int i = 10;
  while(i > 0){
    read_length = read(z21_fd, z21_buf, z21_buf_size);
    if(read_length <= 0){
      loggerf(WARNING, "Z21 failed to receive data");
      return -2;
    }
    else if(z21_buf[0] == 0x09 && z21_buf[2] == 0x40 && z21_buf[4] == 0xF3 && z21_buf[5] == 0x0A){
      if(z21_buf[6] < 1 || z21_buf[7] < 0x20){
        loggerf(ERROR, "Non compatible Firware version");
        return -4;
      }
      else{
        loggerf(DEBUG, "Z21: got firmware, %d.%d", z21_buf[6], 10*(z21_buf[7] >> 4) + (z21_buf[7] & 0xF));
        Z21_info.Firmware[0] = z21_buf[6];
        Z21_info.Firmware[1] = 10*(z21_buf[7] >> 4) + (z21_buf[7] & 0xF);
        WS_stc_Z21_IP(0);
        break;
      }
    }
    i--;
    if(i == 0){
      loggerf(ERROR, "No Firmware response packet");
      return -3;
    }
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

  char * z21_buf = _calloc(1024, 1);
  int z21_buf_size = 1024;

  Z21_SET_BROADCAST_FLAGS(Z21_BROADCAST_FLAGS);

  while(_SYS->Z21_State == _SYS_Module_Run){
    int length = read(z21_fd, z21_buf, z21_buf_size);
    Z21_recv(z21_buf, length);
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
  loggerf(WARNING, "Z21 got %d bytes, %d data-size", length, length-4);
  // char * sdata = _calloc(3, length);
  print_hex(data, length);

  uint16_t d_length = data[0] + (data[1] << 8);
  uint16_t header = data[2] + (data[3] << 8);
  uint8_t checksum;
  for(int i = 4; i < (d_length - 1); i++){
    checksum ^= data[i];
  }
  _Bool check = (checksum == data[d_length-1]);

  loggerf(INFO, "Header %i\n", header);
  loggerf(INFO, "Checksum %i | %x != %x\n", check, checksum, data[d_length-1]);

  switch (header){
    case 0x10: // LAN_GET_SERIAL_NUMBER
      loggerf(TRACE, "LAN_GET_SERIAL_NUMBER");
      break;
    case 0x40: ;
      if(!check)
        return;
      uint8_t XHeader = data[4];
      loggerf(INFO, "XHeader %i\n", XHeader);
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
        loggerf(INFO, "LAN_X_LOCO_INFO");
        Z21_LAN_X_LOCO_INFO(length - 6, &data[5]);
      }
      break;
    case 0x51: //LAN_GET_BROADCASTFLAGS
      loggerf(TRACE, "LAN_GET_BROADCASTFLAGS");
      break;

    case 0x84: //LAN_SYSTEMSTATE_DATACHANGED
        loggerf(DEBUG, "LAN_SYSTEMSTATE_DATACHANGED");
        Z21_info.MainCurrent = (int16_t)(data[4] + (data[5] << 8));
        Z21_info.ProgCurrent = (int16_t)(data[6] + (data[7] << 8));
        Z21_info.FilteredMainCurrent = (int16_t)(data[8] + (data[9] << 8));
        Z21_info.Temperature = (int16_t)(data[10] + (data[11] << 8));
        Z21_info.SupplyVoltage = data[12] + (data[13] << 8);
        Z21_info.VCCVoltage = data[14] + (data[15] << 8);
        Z21_info.CentralState = data[16];
        Z21_info.CentralStateEx = data[17];
        WS_stc_Z21_info(0);
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
  
  pthread_mutex_lock(&z21_send_mutex);

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
  
  pthread_mutex_unlock(&z21_send_mutex);
}

void Z21_send_data(uint8_t * data, uint8_t length){
  if(_SYS->Z21_State == _SYS_Module_Stop || _SYS->Z21_State == _SYS_Module_Fail)
    return;

  pthread_mutex_lock(&z21_send_mutex);
  write(z21_fd, data, length);
  pthread_mutex_unlock(&z21_send_mutex);
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
