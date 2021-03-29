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

struct _EE_Mem {
	uint8_t ModuleID;
	uint8_t NodeID;
	struct _EE_Settings settings;
	#ifdef IO_SPI
	uint16_t IO[16*MAX_PORTS]; // MSB - 4 bit | 4 bit   | 1 bit    | 7 bit    - LSB
	                           //       type  | default | inverted | reserved
	#else
	uint16_t IO[MAX_PINS]; // MSB - 4 bit | 4 bit   | 1 bit    | 7 bit    - LSB
	                       //       type  | default | inverted | reserved
	#endif
};

extern struct _EE_Mem EEMEM EE_Mem;