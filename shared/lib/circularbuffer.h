#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <stdint.h>
#include <stdlib.h>

#ifdef CB_NON_AVR
#define CB_MAX_BUFFER 1024
#else
#define CB_MAX_BUFFER 256
#endif

class CircularBuffer {
  public:
    volatile uint8_t buf[CB_MAX_BUFFER];
#ifdef CB_NON_AVR
    volatile uint16_t read_index  = 0;
    volatile uint16_t write_index = 0;
#else
    volatile uint8_t read_index;
    volatile uint8_t write_index;
#endif

    #ifdef CB_NON_AVR
    uint8_t writefromfd(int fd);
    #endif

    uint8_t read();
    void write(uint8_t data);

    void clear();

    uint8_t size();
    uint8_t peek();
    uint8_t peek(uint8_t offset);
};

#endif
