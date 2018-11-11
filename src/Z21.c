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

void die(char *s){
  loggerf(CRITICAL, "Crashed: %s", s);
  exit(1);
}

int z21_fd = -1;

pthread_mutex_t z21_send_mutex;
char * z21_send_buffer;

void Z21(pthread_t * thread){
  loggerf(INFO, "Connecting to Z21");
  int ret = 0;
  ret = Z21_client(Z21_IP, Z21_PORT);
  if(ret == -2){
    //Retry on second port
    ret = Z21_client(Z21_IP, Z21_PORT+1);

    if(ret != 1){
      return;
    }
  }
  
  if(ret == 1){
    loggerf(INFO, "Connected Succesfully to Z21");
    pthread_create(thread, NULL, Z21_run, NULL);
  }
  else{
    return;
  }
}

int Z21_client(char * ip, uint16_t port){
  loggerf(DEBUG, "Z21: Connecting %s:%d", ip, port);
  struct sockaddr_in z21_addr;

  z21_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if(z21_fd == -1){
    loggerf(CRITICAL, "Cannot create Socket");
    _SYS_change(STATE_RUN | STATE_Client_Accept, 3);
    return -1;
  }
  
  struct timeval tv;
  tv.tv_sec = 30;
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

  _free(z21_buf);
  return 1;
}

void * Z21_run(){
  char * z21_buf = _calloc(1, 1024);
  int z21_buf_size = 1024;

  while(_SYS->_STATE & STATE_RUN){
    Z21_recv(z21_buf, read(z21_fd, z21_buf, z21_buf_size));
  }

  Z21_LOGOUT;
    
  _free(z21_buf);
  close(z21_fd);
  return NULL;
}

char Z21_prio_list[05][30];
char Z21_send_list[10][30];

void Z21_recv(char * data, int length){
  // TODO Implement
  char * sdata = _calloc(3, length);
  for(int i = 0; i < length*3; i++){
    switch(i % 3){
      case 0: 
        if((data[i/3] >> 4) < 0xA){
          sdata[i] = (data[i/3] >> 4) + 0x30;
        }
        else{
          sdata[i] = (data[i/3] >> 4) + 0x37;
        }
        break;
      case 1:
        if((data[i/3] >> 4) < 0xA){
          sdata[i] = (data[i/3] & 0x0F) + 0x30;
        }
        else{
          sdata[i] = (data[i/3] & 0x0F) + 0x37;
        }
        break;
      case 2:
        if(i != (length * 3) - 1){
          sdata[i] = ' ';
        }
        break;
    }
  }
  loggerf(INFO, "Z21 got data %s", sdata);
}

void Z21_send(uint16_t length, uint16_t header, ...){
  // TODO lock z21 mutex
  z21_send_buffer[0] = length & 0x00ff;
  z21_send_buffer[1] = (length & 0xff00) >> 8;
  z21_send_buffer[2] = header & 0x00ff;
  z21_send_buffer[3] = (header & 0xff00) >> 8;
  if(length != 4){
    va_list arglist;
    va_start(arglist, header);

    for(int i = 4; i<length; i++){
      z21_send_buffer[i] = (uint8_t)va_arg(arglist, int);
    }

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
