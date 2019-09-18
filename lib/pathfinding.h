#ifndef H_PATHFINDING
#define H_PATHFINDING

#include "rail.h"

#define PATHFINDING_MAX_LENGHT 30

struct pathfindingconfig {
	Block * start;
	Block * current;
	struct rail_link * link;
	Block * end;

	uint8_t dir;

	uint8_t length;
};

struct pathinstruction {
	void * p;      // Switch or MSSwitch
	uint8_t type;  //   type of p

	uint8_t states;
	uint8_t * optionalstates;

	struct pathinstruction ** next_instruction;
};

struct pathfindingstep {
	uint8_t found;
	struct pathinstruction * instructions;
};

struct pathfindingstep pathfinding(Block * start, Block * end);
struct pathfindingstep _pathfinding_step(struct pathfindingconfig c);
void pathfinding_print(struct pathinstruction * instr);

#endif
