#include "avr/eeprom.h"
#include "eeprom_layout.h"

#include "IO.h"

struct _EE_Mem EEMEM EE_Mem = {10, 5, // Dev ID, Node ID
	                           0, 0, 0, 0, // Blink1, Blink2
	                           0, 0, IO_SERVO_MIN, IO_SERVO_MAX, // Pulse, Poll, Servo1, Servo2
	                           0, 0, 0, 0, 0  // Port1
	                          };