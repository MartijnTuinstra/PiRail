#include "uart.h"
#include "main_node.h"
#include "avr/interrupt.h"

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

#if defined(USART_RX_vect)
ISR(USART_RX_vect)
#elif defined(USART0_RX_vect)
  ISR(USART0_RX_vect)
#elif defined(USART_RXC_vect)
  ISR(USART_RXC_vect) // ATmega8
#else
  #error "Don't know what the Data Received vector is called for Serial"
#endif
{
uart._rx_complete_irq();
}


UART uart;

void UART::init(){
  UCSR0B |= (1 << TXEN0) || (1 << RXEN0); // Enable TX and RX
  UCSR0C = (1 << UCSZ00) | (1 << UCSZ01); // 8-bit uart
  uart_38400();
}

// bool UART::available(){
// 	return (UCSR0A & (1<<RXC0));
// }

uint8_t UART::receive(){
	while (!available()){}

	return UDR0;
}

void UART::_rx_complete_irq(){
	buf[w++] = UDR0;

	if(w >= UART_BUF_SIZE){
		w = 0;
	}
}

uint8_t UART::available(){
	return (w != r);
}

char UART::read(){
	uint8_t d = buf[r++];
	if(r > UART_BUF_SIZE){
		r = 0;
	}
	return d;
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

void UART::transmit(long q, enum UART_transmit_type t){
	char buf[5] = {0,0,0,0,0};
	if(t == HEX){
		uint8_t i = 0;
		while(q != 0){
			long r = q % 16;
			if(r < 10)
				buf[i] = (r + 48);
			else
				buf[i] = (r + 55);
			i++;
			q = q / 16;
		}

		for(int8_t j = i - 1; j >= 0; j--){
			uart.transmit(buf[j]);
		}
	}
}


void UART::transmit(long q, enum UART_transmit_type t, uint8_t length){
	char buf[5] = {0,0,0,0,0};
	if(t == HEX){
		uint8_t i = 0;
		while(q != 0){
			long r = q % 16;
			if(r < 10)
				buf[i] = (r + 48);
			else
				buf[i] = (r + 55);
			i++;
			q = q / 16;
		}

		for(uint8_t j = 0; j < length - i; j++){
			uart.transmit('0');
		}

		for(int8_t j = i - 1; j >= 0; j--){
			uart.transmit(buf[j]);
		}
	}
}
