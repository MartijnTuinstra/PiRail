#include "avr/eeprom.h"

struct _EE_Settings {
	uint8_t blink0[4];
	uint8_t blink1[4];
	uint8_t pulse;
	uint8_t poll;
};

struct _EE_IO_Port {
	uint8_t type;
	uint8_t data[4];
};

#if defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)

#define IO_COUNT 12

#elif defined(__AVR_ATmega64A__)

#define IO_COUNT 40

#elif defined(__AVR_ATmega2560__)

#define IO_COUNT 88

#endif

struct _EE_Mem {
	uint8_t ModuleID;
	uint8_t NodeID;
	struct _EE_Settings settings;
	struct _EE_IO_Port IO[IO_COUNT];
};

extern struct _EE_Mem EEMEM EE_Mem;