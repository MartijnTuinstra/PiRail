
#ifndef _INCLUDE_Z21_H
  #define _INCLUDE_Z21_H
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

  #define Z21_BROADCAST_FLAGS 0x00010001

  void die(char *s);

  void Z21_boot();
  void * Z21(void * args);
  int Z21_client(char * ip, uint16_t port);
  void * Z21_run(void * args);

  extern char Z21_prio_list[05][30];
  extern char Z21_send_list[10][30];

  struct s_Z21_info {
    int16_t MainCurrent;
    int16_t ProgCurrent;
    int16_t FilteredMainCurrent;
    int16_t Temperature;
    uint16_t SupplyVoltage;
    uint16_t VCCVoltage;
    uint8_t CentralState;
    uint8_t CentralStateEx;
    uint8_t IP[4];
    uint8_t Firmware[2];
  };

  extern struct s_Z21_info Z21_info;

  void Z21_recv(char * data, int length);

  void Z21_send(uint16_t length, uint16_t header, ...);
  void Z21_send_c(uint16_t length, uint16_t header, ...);
  void Z21_send_data(uint8_t * data, uint8_t length);

  void Z21_SET_LOCO_DRIVE(int DCC_Adr,char steps,bool dir,char drive);

  void Z21_SET_LOCO_FUNCTION(int DCC_Adr,char function_nr,char switch_type);


  // Z21 Commands
  // 2 - System, status, version
  #define Z21_GET_SERIAL                  Z21_send(4, 0x10)                                    // 2.1  - LAN_GET_SERIAL_NUMBER
  #define Z21_LOGOUT                      Z21_send(4, 0x30)                                    // 2.2  - LAN_LOGOUT
  #define Z21_GET_VERSION                 Z21_send_c(7, 0x40, 0x21, 0x21)                        // 2.3  - LAN_X_GET_VERSION
  #define Z21_GET_STATUS                  Z21_send_c(7, 0x40, 0x21, 0x24)                        // 2.4  - LAN_X_GET_STATUS
  #define Z21_TRACKPOWER_OFF              Z21_send_c(7, 0x40, 0x21, 0x80)                        // 2.5  - LAN_X_SET_TRACK_POWER_OFF
  #define Z21_TRACKPOWER_ON               Z21_send_c(7, 0x40, 0x21, 0x81)                        // 2.6  - LAN_X_SET_TRACK_POWER_ON
  #define Z21_STOP                        Z21_send_c(6, 0x40, 0x80, 0x80)                        // 2.13 - LAN_X_SET_STOP
  #define Z21_GET_FIRMWARE_VERSION        Z21_send_c(7, 0x40, 0xF1, 0x0A)                        // 2.15 - LAN_X_GET_FIRMARE_VERSION
  #define Z21_SET_BROADCAST_FLAGS(F)      Z21_send(8, 0X50, (F & 0xFF000000) >> 24, \
                                                   (F & 0xFF0000) >> 16, (F & 0xFF00) >> 8, \
                                                   F & 0xFF)                                   // 2.16 - LAN_X_SET_BROADCAST_FLAGS
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
