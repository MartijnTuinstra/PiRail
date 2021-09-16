#ifndef _INCLUDE_UART_RNetRX_H
#define _INCLUDE_UART_RNetRX_H

#include <stdint.h>
#include "switchboard/declares.h"
#include "IO.h"

void COM_DevReset();

void COM_set_single_Output(int M, int io, union u_IO_event type);
void COM_set_single_Output_output(int M, int io, enum e_IO_output_event event);

void COM_change_Output(int);
void COM_change_Output(IO_Node *);
void COM_request_Inputs(uint8_t M);
void COM_Configure_IO(uint8_t, uint8_t, uint16_t);
void COM_change_signal(Signal *);

#endif