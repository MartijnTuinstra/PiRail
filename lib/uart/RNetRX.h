#ifndef _INCLUDE_UART_RNetTX_H
#define _INCLUDE_UART_RNetRX_H

#include <stdint.h>

void UART_ACK(uint8_t device);
void UART_NACK(uint8_t device);
void UART_DEV_ID(uint8_t device, uint8_t * data);
void UART_ReadInput(uint8_t device, uint8_t * data);

extern void (*UART_RecvCb[256])(uint8_t, uint8_t *);

#endif
