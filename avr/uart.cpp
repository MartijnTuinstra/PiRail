#include "uart.h"
#include "RNet.h"
#include "main_node.h"
#include "avr/interrupt.h"

#define USE_2X 0

static void uart_38400(void)
{
#define BAUD 500000
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

#if defined(USART_RX_vect)
  SIGNAL(USART_RX_vect)
#elif defined(USART0_RX_vect)
  SIGNAL(USART0_RX_vect) // ATMega2560
#elif defined(USART_RXC_vect)
  SIGNAL(USART_RXC_vect) // ATmega8
#else
  #error "Don't know what the Data Received vector is called for Serial"
#endif
{
uart._rx_complete_irq();
}


#if defined(USART_UDRE_vect)
  ISR(USART_UDRE_vect)
#elif defined(USART0_UDRE_vect)
  ISR(USART0_UDRE_vect) // ATMega2560
#elif defined(USART_UDREC_vect)
  ISR(USART_UDREC_vect) // ATmega8
#else
  #error "Don't know what the Data Received vector is called for Serial"
#endif
{
uart._tx_complete_irq();
}

void UART::init(){
  UCSR0B = (1 << TXEN0) | (1 << RXEN0); // Enable TX and RX
  // UART defaults to 8N1
  UCSR0C = (1 << UCSZ00) | (1 << UCSZ01); // 8N1 mode
  uart_38400();

  #ifdef RNET_MASTER
  cli();
  // Enable interrupts
  UCSR0B |= (1 << RXCIE0);
  sei();
  #endif
}

// bool UART::available(){
// 	return (UCSR0A & (1<<RXC0));
// }

uint8_t UART::receive(){
	while (!available()){}

	return UDR0;
}

void UART::_rx_complete_irq(){
	net.tx.buf[net.tx.write_index] = UDR0;

	if(++net.tx.write_index >= RNET_MAX_BUFFER){
    net.tx.write_index = 0;
  }
}

void UART::start_tx(){
	UDR0 = net.rx.buf[net.rx.read_index];

	if(++net.rx.read_index >= RNET_MAX_BUFFER){
    net.rx.read_index = 0;
  }

  UCSR0B |= (1<<UDRIE0);
}

void UART::_tx_complete_irq(){
	#ifdef RNET_MASTER
    if(net.rx.read_index != net.rx.write_index){
		UDR0 = net.rx.buf[net.rx.read_index];

		if(++net.rx.read_index >= RNET_MAX_BUFFER){
          net.rx.read_index = 0;
        }

    	UCSR0B |= (1<<UDRIE0);
    	UCSR0A &= ~(1<<UDRE0); // Clear interrupt
    }
    else{
    	UCSR0B &= ~(1<<UDRIE0);
    }
	#endif
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
