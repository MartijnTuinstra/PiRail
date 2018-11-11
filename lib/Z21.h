
#ifndef _INCLUDE_Z21_H
  #define _INCLUDE_Z21_H
  #define Z21_IP "192.168.2.8"
  #define Z21_PORT 21105

  /*
      Simple udp client
  */

  #include "system.h"
  #include "train.h"

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

  void Z21(pthread_t * thread);
  int Z21_client(char * ip, uint16_t port);
  void * Z21_run();

  extern char Z21_prio_list[05][30];
  extern char Z21_send_list[10][30];

  void Z21_recv(char * data, int length);

  void Z21_send(uint16_t length, uint16_t header, ...);

  void Z21_get_train(Trains * T);
  void Z21_get_engine(int dcc);

  void Z21_SET_LOCO_DRIVE(int DCC_Adr,char steps,_Bool dir,char drive);

  void Z21_SET_LOCO_FUNCTION(int DCC_Adr,char function_nr,char switch_type);


  // Z21 Commands
  // 2 - System, status, version
  #define Z21_GET_SERIAL                  Z21_send(4, 0x10)                                    // 2.1  - LAN_GET_SERIAL_NUMBER
  #define Z21_LOGOUT                      Z21_send(4, 0x30)                                    // 2.2  - LAN_LOGOUT
  #define Z21_GET_VERSION                 Z21_send(7, 0x40, 0x21, 0x21, 0x00)                  // 2.3  - LAN_X_GET_VERSION
  #define Z21_GET_STATUS                  Z21_send(7, 0x40, 0x21, 0x24, 0x05)                  // 2.4  - LAN_X_GET_STATUS
  #define Z21_TRACKPOWER_OFF              Z21_send(7, 0x40, 0x21, 0x80, 0xA1)                  // 2.5  - LAN_X_SET_TRACK_POWER_OFF
  #define Z21_TRACKPOWER_ON               Z21_send(7, 0x40, 0x21, 0x81, 0xA0)                  // 2.6  - LAN_X_SET_TRACK_POWER_ON
  #define Z21_STOP                        Z21_send(6, 0x40, 0x80, 0x80)                        // 2.13 - LAN_X_SET_STOP
  #define Z21_GET_FIRMWARE_VERSION        Z21_send(7, 0x40, 0xF1, 0x0A, 0xFB)                  // 2.15 - LAN_X_GET_FIRMARE_VERSION
  #define Z21_SET_BROADCAST_FLAGS(F)      Z21_send(8, 0X50, (A & 0xFF000000) >> 24, \
                                                   (B & 0xFF0000) >> 16, (C & 0xFF00) >> 8, \
                                                   D & 0xFF)                                   // 2.16 - LAN_X_SET_BROADCAST_FLAGS
  #define Z21_GET_BROADCAST_FLAGS          Z21_send(4, 0X51)                                   // 2.17 - LAN_X_GET_BROADCAST_FLAGS
  #define Z21_SYSTEMSTATE_GETDATA          Z21_send(4, 0x85)                                   // 2.19 - LAN_SYSTEMSTATE_GETDATA
  #define Z21_GET_HWINFO                   Z21_send(4, 0x1A)                                   // 2.20 - LAN_GET_HWINFO

  // 3 - Settings
  #define Z21_GET_LOCOMODE(Addr)          Z21_send(6, 0x60, (Addr & 0xFF00) >> 8, Addr & 0xFF) // 3.1  - LAN_GET_LOCOMODE
  #define Z21_SET_LOCOMODE(Addr, Mode)    Z21_send(6, 0x61, (Addr & 0xFF00) >> 8, \
                                                   Addr & 0xFF, Mode)                          // 3.2  - LAN_SET_LOCOMODE
  #define Z21_GET_TURNOUTMODE(Addr)       Z21_send(6, 0x70, (Addr & 0xFF00) >> 8, Addr & 0xFF) // 3.3  - LAN_GET_TURNOUTMODE
  #define Z21_SET_TURNOUTMODE(Addr, Mode) Z21_send(6, 0x71, (Addr & 0xFF00) >> 8, \
                                                   Addr & 0xFF, Mode)                          // 3.4  - LAN_SET_TURNOUTMODE

  // 4 - Drive
  // Are not define in a macro

  // 5 - Switches
  // 6 - CV Read and Write
  // 7 - Occupancy detectors R-BUS
  // 8 - RailCom
  // 9 - LocoNet
  // Not used




#endif
