#include "uart.h"
#include "main_node.h"

#define USE_2X 0

static void uart_38400(void)
{
#define BAUD 38400
#include <util/setbaud.h>
UBRR0H = UBRRH_VALUE;
UBRR0L = UBRRL_VALUE;
#if USE_2X
UCSR0A |= (1 << U2X0);
#else
UCSR0A &= ~(1 << U2X0);
#endif
}

UART uart;

void UART::init(){
  UCSR0B |= (1 << TXEN0) || (1 << RXEN0); // Enable TX and RX
  UCSR0C = (1 << UCSZ00) | (1 << UCSZ01); // 8-bit uart
  uart_38400();
}

bool UART::available(){
	return (UCSR0A & (1<<RXC0));
}

uint8_t UART::receive(){
	while (!available()){}

	return UDR0;
}

void UART::transmit(uint8_t byte){
    loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
    UDR0 = byte;
}
void UART::transmit(uint8_t * data, uint8_t length){
	for(uint8_t i = 0; i < length; i++){
		transmit(data[i]);
	}
}
void UART::transmit(const char * data, uint8_t length){
	for(uint8_t i = 0; i < length; i++){
		transmit(data[i]);
	}
}
