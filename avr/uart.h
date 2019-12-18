#include <stdint.h>
#include <stdlib.h>

#include "avr/io.h"

enum UART_transmit_type {
	DEC,
	HEX,
	BIN
};

#define UART_BUF_SIZE 64

class UART{
  public:
    void init();
    void transmit(uint8_t byte);
    void transmit(uint8_t * data, uint8_t length);
    void transmit(const char * data, uint8_t length);
    void transmit(long i, enum UART_transmit_type t);
    void transmit(long q, enum UART_transmit_type t, uint8_t length);
    uint8_t receive();
    // bool available();


    void  _rx_complete_irq();
    uint8_t available();
    char  read();

  private:
    char buf[UART_BUF_SIZE];
    uint8_t w;
    uint8_t r;
};

extern UART uart;
