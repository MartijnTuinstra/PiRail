#ifndef _INCLUDE_UART_H
#define _INCLUDE_UART_H

#define Default_Serial_Port "/dev/ttyUSB1"
#define Serial_Baud B500000

#define UART_BUFFER_SIZE 256
#define UART_Msg_NotComplete 0xFF
#define UART_CHECKSUM_SEED 0b10101010

#define UART_COM_t_Length 64

#include "system.h"
#include "circularbuffer.h"

struct COM_t{
  char length;
  char data[UART_COM_t_Length];
};

struct fifobuffer {
  char buffer[UART_BUFFER_SIZE];
  uint8_t read;
  uint8_t write;
};

class UART {
  public:
    char device[50];
    int fd = -1;

    CircularBuffer buffer;

    volatile enum e_SYS_Module_State status = Module_STOP;
    void (*updateStateCb)(enum e_SYS_Module_State);

    bool ACK  = false;
    bool NACK = false;

    UART();
    ~UART();

    int init();
    void close();

    void handle();
    static void * serve(void *);

    void send(struct COM_t *);
    bool recv();
    void parse();
    void parsePacket(uint8_t);

    void setDevice(const char *);
    void resetDevice();

    void setUpdateStatusCb(void (*)(enum e_SYS_Module_State));
    void updateState(enum e_SYS_Module_State);
    void setState(enum e_SYS_Module_State);
};

extern UART uart;

#endif
