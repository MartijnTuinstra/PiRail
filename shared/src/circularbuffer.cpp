
#include "circularbuffer.h"


#ifdef CB_NON_AVR
uint8_t writefromfd(int fd){
  if(fd < 0)
    return;

  //Filestream, buffer to store in, number of bytes to read (max)
  uint8_t size = 32;
  if(write_index + size > CB_MAX_BUFFER)
    size = CB_MAX_BUFFER - write_index;

  int rx_length = read(fd, (void*)&buf[write_index], size);
  if (rx_length > 0)
  {
    char debug[200];
    char *ptr = debug;

    for(uint8_t i = 0;i<rx_length;i++){
      ptr += sprintf(ptr, "%02x", buf[i+write_index]);
    }

    write_index = (write_index + rx_length) % CB_MAX_BUFFER;

    // loggerf(INFO, "%i/%i-(%i/%i) bytes read, %d available, %s", rx_length, size, buf->read, buf->write, (uint8_t)(((int16_t)buf->write - (int16_t)buf->read) % UART_BUFFER_SIZE), debug);
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
