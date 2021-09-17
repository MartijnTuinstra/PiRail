
#include "circularbuffer.h"


#ifdef CB_NON_AVR
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include "utils/logger.h"

uint8_t CircularBuffer::writefromfd(int fd){
  if(fd < 0)
    return 0;

  // Wait for available read
  fd_set set;
  FD_ZERO(&set); /* clear the set */
  FD_SET(fd, &set); /* add our file descriptor to the set */
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 10000;

  int rv = select(fd + 1, &set, NULL, NULL, &timeout);

  if (rv <= 0) // Failed or timeout
    return 0;

  //Filestream, buffer to store in, number of bytes to read (max)
  uint8_t readSize = 32;
  if(write_index + readSize > CB_MAX_BUFFER)
    readSize = CB_MAX_BUFFER - write_index;

  int rx_length = ::read(fd, (void*)&buf[write_index], readSize);
  if (rx_length > 0)
  {
    char debug[200];
    char *ptr = debug;

    for(uint8_t i = 0;i<rx_length;i++){
      ptr += sprintf(ptr, "%02x", buf[i+write_index]);
    }

    write_index = (write_index + rx_length) % CB_MAX_BUFFER;

    loggerf(TRACE, "%i/%i-(%i/%i) bytes read, %d available, %s", rx_length, readSize, read_index, write_index, size(), debug);
  }
  return rx_length;
}
#endif

uint8_t CircularBuffer::read(){
  if(read_index != write_index){
    uint8_t byte = buf[read_index];
    read_index = (read_index + 1) % CB_MAX_BUFFER;
    return byte;
  }
  return 0;
}

void CircularBuffer::write(uint8_t data){
  buf[write_index] = data;
  write_index = (write_index + 1) % CB_MAX_BUFFER;
}

uint8_t CircularBuffer::peek(){
  return buf[read_index];
}

uint8_t CircularBuffer::peek(uint8_t offset){
  return buf[(read_index + offset) % CB_MAX_BUFFER];
}

uint8_t CircularBuffer::size(){
  return (uint8_t)(write_index - read_index) % CB_MAX_BUFFER;
}

void CircularBuffer::clear(){
  write_index = 0;
  read_index = 0;
}
