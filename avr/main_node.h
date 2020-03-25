#ifndef INCLUDE_MAIN_NODE_H
#define INCLUDE_MAIN_NODE_H

#if defined(__AVR_ATmega328__)
#define F_CPU 16000000U
#elif defined(__AVR_ATmega328P__)
#define F_CPU 16000000U
#elif defined(__AVR_ATmega64A__)
#define F_CPU 16000000U
#elif defined(__AVR_ATmega2560__)
#define F_CPU 16000000U
#else
#error "Device not supported"
#endif

#endif
