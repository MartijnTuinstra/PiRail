#ifndef _INCLUDE_FLAGS_H
#define _INCLUDE_FLAGS_H

#define FL_DIRECTION_MASK       0x01

#define FL_NEXT_DIRECTION       0x00 // Search in forward direction
#define FL_PREV_DIRECTION       0x01 // Search in reverse direction

#define FL_SWITCH_CARE          0x80 // Stop if switch not approachable
#define FL_DIRECTION_CARE       0x40 // Stop if direction does not match
#define FL_NEXT_FIRST_TIME_SKIP 0x20 // Flag for search Block
#define FL_CONTINUEDANGER       0x10 // Stop if block is DANGER or worse

#define NEXT FL_NEXT_DIRECTION
#define PREV FL_PREV_DIRECTION

#endif