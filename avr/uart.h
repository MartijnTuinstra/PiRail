#include <stdint.h>
#include <stdlib.h>

#include "avr/io.h"

class UART{
  public:
    void init();
    void transmit(uint8_t byte);
    void transmit(uint8_t * data, uint8_t length);
    void transmit(const char * data, uint8_t length);
    uint8_t receive();
    bool available();
};

extern UART uart;