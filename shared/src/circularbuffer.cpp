
#include "circularbuffer.h"

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
