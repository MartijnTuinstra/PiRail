#ifndef _INCLUDE_FLAGS_H
#define _INCLUDE_FLAGS_H

#define FL_DIRECTION_MASK       0x0001

#define FL_NEXT_DIRECTION       0x0000 // Search in forward direction
#define FL_PREV_DIRECTION       0x0001 // Search in reverse direction

#define FL_BLOCKS_COUNT         0x0100 // Stop if switch not approachable
#define FL_SWITCH_CARE          0x0080 // Stop if switch not approachable
#define FL_DIRECTION_CARE       0x0040 // Stop if direction does not match
#define FL_NEXT_FIRST_TIME_SKIP 0x0020 // Flag for search Block
#define FL_CONTINUEDANGER       0x0010 // Stop if block is DANGER or worse

#define NEXT FL_NEXT_DIRECTION
#define PREV FL_PREV_DIRECTION

#endif