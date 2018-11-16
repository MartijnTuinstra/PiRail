#ifndef _INCLUDE_SUBMODULES_H
#define _INCLUDE_SUBMODULES_H

#include <pthread.h>


void Algor_start();
void Algor_stop();

void UART_start();
void UART_stop();

void SimA_start();
void SimA_stop();
void SimB_start();
void SimB_stop();

extern pthread_t z21_thread;
extern pthread_t z21_start_thread;

void Z21_start();
void Z21_stop();

#endif