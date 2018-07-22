#include <stdlib.h>
#include <stdint.h>
#include "module.h"

#ifndef INCLUDE_IO
#define INCLUDE_IO

typedef struct s_unit Unit;

typedef struct s_node_adr {
	uint8_t Node;
	uint16_t io;
} Node_adr;

enum IO_type {
	IO_Input,
	IO_Output
};

typedef struct s_IO_Port {
	uint8_t id;
	uint8_t state;
	enum IO_type type;
} IO_Port;

typedef struct s_IO_Node {
	uint8_t id;
	uint16_t io_ports;
	IO_Port ** io;
} IO_Node;

void Add_IO_Node(Unit * U, int Node_nr, int IO);

#endif
