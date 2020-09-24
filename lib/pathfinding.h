#ifndef H_PATHFINDING
#define H_PATHFINDING

#include <stdint.h>

#include "switchboard/rail.h"

#define PATHFINDING_MAX_LENGHT 40

namespace PathFinding {

class Route {
  public:
    bool found_forward;
    bool found_reverse;

    uint16_t length;

    struct instruction ** Sw_S;
    struct instruction ** Sw_s;
    struct instruction ** MSSw_A;
    struct instruction ** MSSw_B;

    Route(struct control, struct step, struct step);
    ~Route();

    void print(char * str);
};

struct control {
  Block * start;
  Block * end;

  Block * prev;
  void * prevPtr;

  uint8_t dir;
  struct rail_link * link;

  uint8_t searchDepth;

  struct instruction ** Sw_S;
  struct instruction ** Sw_s;
  struct instruction ** MSSw_A;
  struct instruction ** MSSw_B;
};

struct instruction {
  uint8_t type;
  union {
    void * p;
    Switch * Sw;
    MSSwitch * MSSw;
  } p;

  uint8_t nrOptions;
  uint8_t * options;
  uint16_t * length;
  uint8_t * possible;

  struct instruction ** next;
};

struct step {
  uint16_t length;
  bool found;

  struct instruction * next;
};

Route * find(Block * start, Block * end);
struct step findStep(struct control c);

void findStepSolveSwS(Switch * Sw, struct instruction * instr, struct step * str, struct step * div);
void findStepSolveMSSw(MSSwitch *, struct instruction *, struct step *);

}; // namespace PathFinding

struct pathfindingswitchdata {
  struct pathinstruction ** sw;
  struct pathinstruction ** mssw;
};

struct pathfindingconfig {
  Block * start;
  Block * end;

  Block * current;
  struct rail_link * link;

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
