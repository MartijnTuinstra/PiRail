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

#include "config.h"
#include "websocket/stc.h"
#include "scheduler/scheduler.h"
// #include "submodule.h"

// pthread_mutex_t z21_send_mutex;
// uint8_t * z21_send_buffer;


Z21_Client * Z21;

Z21_Client::Z21_Client(){
  memset(this, 0, sizeof(Z21_Client));
  read_config();
}

Z21_Client::~Z21_Client(){
  SYS_set_state(&SYS->Z21.state, Module_STOP);

  pthread_join(thread, NULL);
}

void Z21_Client::start(){
  pthread_create(&SYS->Z21.start_th, NULL, &start_thread, this);
}

void * Z21_Client::start_thread(void * args){
  Z21_Client * context = (Z21_Client *)args;

  pthread_join(context->thread, NULL);
  SYS_set_state(&SYS->Z21.state, Module_Init);

  context->connected = 1;
  int8_t ret = context->connect(Z21_PORT);
  if(ret == -2){
    //Retry on second port
    ret = context->connect(Z21_PORT + 1);
  }
  
  if(ret == 1){
    loggerf(INFO, "Connected Succesfully to Z21");

    SYS_set_state(&SYS->Z21.state, Module_Run);

    pthread_create(&context->thread, NULL, &context->serve, context);
  }
  else{
    context->connected = 0;
    SYS_set_state(&SYS->Z21.state, Module_Fail);
  }
  return 0;
}

void Z21_Client::read_config(){
  FILE * fp = fopen("configs/Z21.bin","rb");

  if (!fp){
    loggerf(INFO, "Failed to open Z21 config file.");
    FILE * fp = fopen("configs/Z21.bin", "wb");
    if (!fp){
      loggerf(ERROR, "Failed to create new Z21 config file.");
      SYS_set_state(&SYS->Z21.state, Module_Fail);
      return;
    }
    IP[0] = 192;
    IP[1] = 168;
    IP[2] = 1;
    IP[3] = 111;
    fwrite(IP, 4, 1, fp);
    fclose(fp);
  }
  else{
    fread(IP, 4, 1, fp);
    fclose(fp);
  }

  sprintf(IP_string, "%d.%d.%d.%d", IP[0], IP[1], IP[2], IP[3]);
}

int Z21_Client::connect(uint16_t port){
  loggerf(INFO, "Z21: Connecting %s:%d", IP_string, port);
  struct sockaddr_in z21_addr;

  // Create Socket
  fd = socket(AF_INET, SOCK_DGRAM, 0);

  if(fd == -1){
    loggerf(CRITICAL, "Cannot create Socket");
    return -1;
  }
  
  // Set Timeout
  struct timeval tv;
  tv.tv_sec = 10;
  tv.tv_usec = 0;
  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

  // Connect To Address
  bzero(&z21_addr, sizeof(z21_addr));
  z21_addr.sin_addr.s_addr = inet_addr(IP_string);
  z21_addr.sin_port = htons(port);
  z21_addr.sin_family = AF_INET;

  if( ::connect(fd, (sockaddr *)&z21_addr, sizeof(z21_addr)) < 0){
    loggerf(ERROR, "UDP Connect faild");
    close(fd);
    return -2;
  }

  Z21_GET_SERIAL;

  // Wait for answer
  uint8_t i = 10;
  while(i > 0){
    if(recv_packet()){
      if(buffer[2] == 0x10){
        loggerf(INFO, "Z21 Serial Number:    %02X %02X %02X %02X", buffer[4], buffer[5], buffer[6], buffer[7]);
        break;
      }
    }
    else{
      i = 0;
      break;
    }
    i--;
  }
  if(i == 0){
    loggerf(WARNING, "Z21 failed to receive Serial Number");
    return -2;
  }

  Z21_GET_FIRMWARE_VERSION;

  // If no answer
  i = 10;
  while(i > 0){
    if(recv_packet()){
      if(buffer[0] == 0x09 && buffer[2] == 0x40 && buffer[4] == 0xF3 && buffer[5] == 0x0A){
        break;
      }
    }
    else{
      i = 0;
      break;
    }
    i--;
  }
  if(i == 0){
    loggerf(WARNING, "Z21 failed to receive Firmware Version");
    return -3;
  }

  loggerf(INFO, "Z21 Firmware Version: %02X.%02X", buffer[6], buffer[7]);

  if(buffer[6] < 1 || buffer[7] < 0x20){
    loggerf(ERROR, "Non compatible Firware version");
    return -4;
  }
  else{
    Firmware[0] = buffer[6];
    Firmware[1] = 10*(buffer[7] >> 4) + (buffer[7] & 0xF);
    WS_stc_Z21_IP(0);
  }

  return 1;
}

int Z21_Client::recv(uint8_t length){
  while (length != 0) {
    uint8_t bytes = ::recv(fd, &buffer[buffer_write], length, 0);
    length -= bytes;
    buffer_write += bytes;
  }

  return buffer_write;
}

int8_t Z21_Client::peek(uint8_t length, uint8_t *tmpbfr){
  return ::recv(fd, tmpbfr, length, MSG_PEEK);
}

bool Z21_Client::recv_packet(){
  uint8_t tmp_buffer[2] = {};

  buffer_write = 0;

  int8_t peek_size = peek(2, tmp_buffer);

  if(peek_size > 1){
    uint16_t packet_size = tmp_buffer[0] + (tmp_buffer[1] << 8);
    uint8_t size = recv(packet_size);

    log_hex("Z21 Recv buffer", buffer, packet_size);

    if(size == packet_size){
      return true;
    }

    loggerf(ERROR, "Failed to read packet in one go!!!");
  }

  return false;
}

int Z21_Client::send(uint16_t length, uint8_t * data){
  if(!connected){
    printf("Z21 not connected for sending\n");
    return -1;
  }

  log_hex("Z21 Send buffer", (char *)data, length);

  pthread_mutex_lock(&send_mutex);
  int size = write(fd, data, length);
  pthread_mutex_unlock(&send_mutex);

  return size;
}

int Z21_Client::send(uint16_t length, uint16_t header, uint8_t * data){
  if(!connected)
    return -1;

  uint8_t z21_send_buffer[256] = {};
  
  z21_send_buffer[0] = length & 0x00ff;
  z21_send_buffer[1] = (length & 0xff00) >> 8;
  z21_send_buffer[2] = header & 0x00ff;
  z21_send_buffer[3] = (header & 0xff00) >> 8;

  for(int i = 4; i < length; i++){
    z21_send_buffer[i] = data[i - 4];
  }
  
  return send(length, z21_send_buffer);
}

void * Z21_Client::serve(void * args){
  Z21_Client * context = (Z21_Client *)args;
  pthread_join(SYS->Z21.start_th, NULL);
  
  struct SchedulerEvent * keepalive_event = scheduler->addEvent("Z21_KeepAlive", {20, 0});
  keepalive_event->function = &Z21_KeepAliveFunc;
  scheduler->enableEvent(keepalive_event);

  if(!context->connected){
    return 0;
  }

  Z21_SET_BROADCAST_FLAGS(Z21_BROADCAST_FLAGS);

  while(SYS->Z21.state == Module_Run && !SYS->stop){
    if(context->recv_packet())
      context->parse();
  }

  scheduler->removeEvent(keepalive_event);

  context->connected = 0;

  loggerf(DEBUG, "Z21 LOGOUT");
  Z21_LOGOUT;

  close(context->fd);
  return NULL;
}
/*
char Z21_prio_list[05][30];
char Z21_send_list[10][30];
*/
void Z21_Client::parse(){
  if(buffer_write < 4){
    return; // Invalid size
  }

  char * data = buffer;

  // read header
  uint16_t packet_length = data[0] + (data[1] << 8);
  uint16_t header_code = data[2] + (data[3] << 8);

  // Calculate checksum
  uint8_t checksum = 0;
  for(int i = 4; i < (packet_length - 1); i++){
    checksum ^= data[i];
  }
  bool check = (checksum == data[packet_length-1]);

  loggerf(TRACE, "Header %x", header_code);
  loggerf(TRACE, "Checksum %i | %x != %x", check, checksum, data[packet_length-1]);

  if(header_code == 0x10){ // LAN_GET_SERIAL_NUMBER
    loggerf(DEBUG, "LAN_GET_SERIAL_NUMBER %02x.%02x", data[4], data[5]);
  }
  else if(header_code == 0x40){
    if(!check)
      return;
    uint8_t XHeader = data[4];
    // loggerf(INFO, "XHeader %x", XHeader);
    if(XHeader == 0x63){ // LAN_X_GET_VERSION
      loggerf(TRACE, "LAN_X_GET_VERSION");
    }
    else if(XHeader == 0x61){ // LAN_X_BC_TRACK_POWER_OFF     0x00
                              // LAN_X_BC_TRACK_POWER_ON      0x01
                              // LAN_X_BC_PROGRAMMING_MODE    0x02
                              // LAN_X_BC_TRACK_SHORT_CIRCUIT 0x08
                              // LAN_X_UNKNOWN_COMMAND        0x82
      if(data[5] == 0){
        loggerf(ERROR, "LAN_X_BC_TRACK_POWER_OFF");
        WS_stc_EmergencyStop();
      }
      else if(data[5] == 0x01){
        loggerf(ERROR, "LAN_X_BC_TRACK_POWER_ON");
        WS_stc_ClearEmergency();
      }
      else if(data[5] == 0x02){
        loggerf(ERROR, "LAN_X_BC_PROGRAMMING_MODE");
      }
      else if(data[5] == 0x08){
        loggerf(ERROR, "LAN_X_BC_TRACK_SHORT_CIRCUIT");
        WS_stc_ShortCircuit();
      }
      else if(data[5] == 0x82){
        loggerf(ERROR, "LAN_X_UNKNOWN_COMMAND");
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
      loggerf(DEBUG, "LAN_X_LOCO_INFO");
      Z21_LAN_X_LOCO_INFO(packet_length - 6, &data[5]);
    }
  }
  else if(header_code == 0x51){ //LAN_GET_BROADCASTFLAGS
    loggerf(TRACE, "LAN_GET_BROADCASTFLAGS");
  }
  else if(header_code == 0x84){ //LAN_SYSTEMSTATE_DATACHANGED
    loggerf(DEBUG, "LAN_SYSTEMSTATE_DATACHANGED");
    sensors.MainCurrent = (int16_t)(data[4] + (data[5] << 8));
    sensors.ProgCurrent = (int16_t)(data[6] + (data[7] << 8));
    sensors.FilteredMainCurrent = (int16_t)(data[8] + (data[9] << 8));
    sensors.Temperature = (int16_t)(data[10] + (data[11] << 8));
    sensors.SupplyVoltage = data[12] + (data[13] << 8);
    sensors.VCCVoltage = data[14] + (data[15] << 8);
    sensors.CentralState = data[16];
    sensors.CentralStateEx = data[17];
    WS_stc_Z21_info(0);
  }
  else if(header_code == 0x1A){ //LAN_GET_HWINFO
    loggerf(TRACE, "LAN_GET_HWINFO");
  }
  else if(header_code == 0x60){ //LAN_GET_LOCOMODE
    loggerf(TRACE, "LAN_GET_LOCOMODE");
  }
  else if(header_code == 0x70){ //LAN_GET_TURNOUTMODE
    loggerf(TRACE, "LAN_GET_TURNOUTMODE");
  }
  else{
    loggerf(TRACE, "Z21_UNKNOWN_COMMAND");
  }
}
