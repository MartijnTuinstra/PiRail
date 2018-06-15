
#ifndef _INCLUDE_Z21_H
  #define _INCLUDE_Z21_H
  #define Z21_IP "192.168.2.92"
  #define Z21_PORT 4129

  /*
      Simple udp client
  */
  #include<stdio.h> //printf
  #include<string.h> //memset
  #include<stdlib.h> //exit(0);
  #include<arpa/inet.h>
  #include<sys/socket.h>

  #define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
  #define BYTE_TO_BINARY(byte)  \
    (byte & 0x80 ? '1' : '0'), \
    (byte & 0x40 ? '1' : '0'), \
    (byte & 0x20 ? '1' : '0'), \
    (byte & 0x10 ? '1' : '0'), \
    (byte & 0x08 ? '1' : '0'), \
    (byte & 0x04 ? '1' : '0'), \
    (byte & 0x02 ? '1' : '0'), \
    (byte & 0x01 ? '1' : '0')

  #define SERVER "127.0.0.1"
  #define BUFLEN 512  //Max length of buffer
  #define PORT 4129   //The port on which to send data

  void die(char *s);

  void Z21_client();

  extern char Z21_prio_list[05][30];
  extern char Z21_send_list[10][30];

  void Z21_recv(char message[100]);

  void Z21_send(int Header,char data[30]);

  void Z21_GET_LOCO_INFO(int DCC_Adr);

  void Z21_SET_LOCO_DRIVE(int DCC_Adr,char steps,_Bool dir,char drive);

  void Z21_SET_LOCO_FUNCTION(int DCC_Adr,char function_nr,char switch_type);
#endif
