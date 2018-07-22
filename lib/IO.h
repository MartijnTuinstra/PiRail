#include <stdlib.h>
#include <stdint.h>
#include "module.h"

#ifndef INCLUDE_IO
#define INCLUDE_IO

typedef struct s_unit Unit;

typedef struct s_IO_Port {
	uint8_t id;
	uint8_t state:4;
	uint8_t type:4;
} IO_Port;

typedef struct s_IO_Node {
	uint8_t id;
	uint8_t io_ports;
	IO_Port ** io;
} IO_Node;

void Add_IO_Node(Unit * U, int Node_nr, int IO);

#endif
