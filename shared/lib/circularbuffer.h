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
    volatile uint8_t read_index;
    volatile uint8_t write_index;

    #ifdef CB_NON_AVR
    uint8_t writefromfd(int fd);
    #endif

    uint8_t read();
    void write(uint8_t data);

    uint8_t size();
    uint8_t peek();
    uint8_t peek(uint8_t offset);
};

#endif