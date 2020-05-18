#include "avr/eeprom.h"
#include "IO.h"

struct _EE_Settings {
	uint16_t blink1;
	uint16_t blink2;
	uint8_t pulse;
	uint8_t poll;
	uint8_t servo1;
	uint8_t servo2;

};

struct _EE_IO_Port {
	uint8_t type;
	uint8_t _default;
};

struct _EE_Mem {
	uint8_t ModuleID;
	uint8_t NodeID;
	struct _EE_Settings settings;
	struct _EE_IO_Port IO[MAX_PINS];
};

extern struct _EE_Mem EEMEM EE_Mem;