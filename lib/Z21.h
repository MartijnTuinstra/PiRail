
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

  #define Z21_BROADCAST_FLAGS 0x00010101

class Z21_Client {
  public:
    struct {
      int16_t MainCurrent;
      int16_t ProgCurrent;
      int16_t FilteredMainCurrent;
      int16_t Temperature;
      uint16_t SupplyVoltage;
      uint16_t VCCVoltage;
      uint8_t CentralState;
      uint8_t CentralStateEx;
    } sensors;

    uint8_t IP[4];
    char IP_string[20];
    uint8_t Firmware[2];
    bool connected;

    pthread_t thread = {0};
    pthread_mutex_t send_mutex;

    struct SchedulerEvent * keepalive_event;

    char buffer[1024];
    uint16_t buffer_write;

    int fd;

    Z21_Client();
    ~Z21_Client();

    void start();
    static void * start_thread(void * args);

    void read_config();
    int connect(uint16_t port);

    static void * serve(void * args);

    int recv(uint8_t length);
    int8_t peek(uint8_t length, uint8_t * tmpbfr);

    bool recv_packet();
    int send(uint16_t length, uint8_t * data);
    int send(uint16_t length, uint16_t header, uint8_t * data);

    void parse();
};

extern class Z21_Client * Z21;

  void die(char *s);

  // void Z21_boot();
  // void * Z21(void * args);
  // int Z21_client(char * ip, uint16_t port);
  // void * Z21_run(void * args);

  extern char Z21_prio_list[05][30];
  extern char Z21_send_list[10][30];

  // struct s_Z21_info {
  //   int16_t MainCurrent;
  //   int16_t ProgCurrent;
  //   int16_t FilteredMainCurrent;
  //   int16_t Temperature;
  //   uint16_t SupplyVoltage;
  //   uint16_t VCCVoltage;
  //   uint8_t CentralState;
  //   uint8_t CentralStateEx;
  //   uint8_t IP[4];
  //   uint8_t Firmware[2];
  // };

  // extern struct s_Z21_info Z21_info;

  // void Z21_recv(char * data, int length);

  // void Z21_send(uint16_t length, uint16_t header, ...);
  // void Z21_send_c(uint16_t length, uint16_t header, ...);
  // void Z21_send_data(uint8_t * data, uint8_t length);

  // void Z21_SET_LOCO_DRIVE(int DCC_Adr,char steps,bool dir,char drive);

  // void Z21_SET_LOCO_FUNCTION(int DCC_Adr,char function_nr,char switch_type);


  // Z21 Commands
  // 2 - System, status, version
  #define Z21_GET_SERIAL                  {uint8_t tmp[4] = {0x04,0,0x10,0}; Z21->send(4, tmp);}              // 2.1  - LAN_GET_SERIAL_NUMBER
  #define Z21_LOGOUT                      {uint8_t tmp[4] = {0x04,0,0x30,0}; Z21->send(4, tmp);}              // 2.2  - LAN_LOGOUT
  #define Z21_GET_VERSION                 {uint8_t tmp[7] = {0x07,0,0x40,0x21,0x21,0}; Z21->send(7, tmp);}    // 2.3  - LAN_X_GET_VERSION
  #define Z21_GET_STATUS                  {uint8_t tmp[7] = {0x07,0,0x40,0x21,0x24,0x05}; Z21->send(7, tmp);} // 2.4  - LAN_X_GET_STATUS
  #define Z21_TRACKPOWER_OFF              {uint8_t tmp[7] = {0x07,0,0x40,0,0x21,0x80,0xA1}; \
                                           Z21->send(7, tmp);}                                                // 2.5  - LAN_X_SET_TRACK_POWER_OFF
  #define Z21_TRACKPOWER_ON               {uint8_t tmp[7] = {0x07,0,0x40,0,0x21,0x81,0xA0}; \
                                           Z21->send(7, tmp);}                                                // 2.6  - LAN_X_SET_TRACK_POWER_ON

  #define Z21_STOP                        {uint8_t tmp[4] = {0x04,0,0x40,0,0x80,0x80}; \
                                           Z21->send(7, tmp);}                                                // 2.13  - LAN_X_SET_STOP
  #define Z21_GET_FIRMWARE_VERSION        {uint8_t tmp[7] = {0x07,0,0x40,0,0xF1,0x0A,0xFB}; \
                                           Z21->send(7, tmp);}                                                // 2.15  - LAN_X_GET_FIRMARE_VERSION
  #define Z21_SET_BROADCAST_FLAGS(F)      {uint8_t tmp[8] = {0x08,0,0x50,0, (F & 0xFF000000) >> 24, (F & 0xFF0000) >> 16, (F & 0xFF00) >> 8, F & 0xFF}; \
                                           Z21->send(8, tmp);}                                                // 2.16  - LAN_X_SET_BROADCAST_FLAGS
  #define Z21_GET_BROADCAST_FLAGS         {uint8_t tmp[4] = {0x04,0,0x51,0}; Z21->send(7, tmp);}              // 2.17  - LAN_X_GET_BROADCAST_FLAGS
  #define Z21_SYSTEMSTATE_GETDATA         {uint8_t tmp[4] = {0x04,0,0x85,0}; Z21->send(7, tmp);}              // 2.19  - LAN_SYSTEMSTATE_GETDATA
  #define Z21_GET_HWINFO                  {uint8_t tmp[4] = {0x04,0,0x1A,0}; Z21->send(7, tmp);}              // 2.20  - LAN_GET_HWINFO

  // // 3 - Settings
  #define Z21_GET_LOCOMODE(Addr)          {uint8_t tmp[6] = {0x06,0,0x60,0, (Addr & 0xFF00) >> 8, Addr & 0xFF}; \
                                           Z21->send(6, tmp);}  // 3.1  - LAN_GET_LOCOMODE
  #define Z21_SET_LOCOMODE(Addr, Mode)    {uint8_t tmp[7] = {0x07,0,0x61,0, (Addr & 0xFF00) >> 8, Addr & 0xFF, Mode}; \
                                           Z21->send(6, tmp);}  // 3.2  - LAN_SET_LOCOMODE

  #define Z21_GET_TURNOUTMODE(Addr)       {uint8_t tmp[6] = {0x06,0,0x70,0, (Addr & 0xFF00) >> 8, Addr & 0xFF}; \
                                           Z21->send(6, tmp);}  // 3.3  - LAN_GET_TURNOUTMODE
  #define Z21_SET_TURNOUTMODE(Addr, Mode) {uint8_t tmp[7] = {0x07,0,0x71,0, (Addr & 0xFF00) >> 8, Addr & 0xFF, Mode}; \
                                           Z21->send(6, tmp);}  // 3.4  - LAN_SET_TURNOUTMODE

  // 4 - Drive
  // Are not define in a macro

  // 5 - Switches
  // 6 - CV Read and Write
  // 7 - Occupancy detectors R-BUS
  // 8 - RailCom
  // 9 - LocoNet
  // Not used




#endif
