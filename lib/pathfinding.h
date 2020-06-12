#ifndef H_PATHFINDING
#define H_PATHFINDING

#include <stdint.h>

#include "switchboard/rail.h"

#define PATHFINDING_MAX_LENGHT 40

struct pathfindingswitchdata {
	struct pathinstruction *** sw;
	struct pathinstruction *** mssw;
};

struct pathfindingconfig {
	Block * start;
	Block * current;
	struct rail_link * link;
	Block * end;

	struct pathinstruction ** final_instruction;

	uint8_t dir;

	uint8_t steps;
	uint16_t length;

	struct pathfindingswitchdata * sw_data;
};

struct pathinstruction {
	void * p;      // Switch or MSSwitch
	uint8_t type;  //   type of p

	uint8_t prevcounter:4;
	uint8_t states:4;

	uint8_t * optionalstates;
	uint16_t * lengthstates;


	struct pathinstruction ** next_instruction;
};

struct pathfindingstep {
	uint8_t found;
	uint16_t length;
	struct pathinstruction * instructions;
};

struct paths {
	struct pathinstruction * forward;
	struct pathinstruction * reverse;
};

struct paths pathfinding(Block * start, Block * end);
struct pathfindingstep _pathfinding_step(struct pathfindingconfig c);
void pathfinding_print(struct pathinstruction * instr, uint8_t level);
void free_pathinstructions(struct pathinstruction * instr);
void remove_pathinstructions(struct rail_link link, struct pathinstruction * instr);

#endif
