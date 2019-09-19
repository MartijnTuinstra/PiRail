#include "pathfinding.h"
#include "switch.h"
#include "mem.h"
#include "logger.h"
#include "websocket_msg.h"

struct pathfindingstep pathfinding(Block * start, Block * end){
  struct pathfindingconfig c;
  c.start = start;
  c.current = start;
  c.dir = NEXT;
  c.link = Next_link(c.current, c.dir);
  c.end = end;

  c.length = 0;

  clock_t t;
  t = clock();

  loggerf(INFO, "Searching path %02i:%02i -> %02i:%02i", start->module, start->id, end->module, end->id);

  struct pathfindingstep result = _pathfinding_step(c);

  if(result.found){
    loggerf(ERROR, "FOUND");
    pathfinding_print(result.instructions);
  }

  t = clock() - t;
  printf ("It took me %d clicks (%f seconds).\n",t,((float)t)/CLOCKS_PER_SEC);

  return result;
}

// Recursive Function (tree search)
struct pathfindingstep _pathfinding_step(struct pathfindingconfig c){
  loggerf(WARNING, "_pathfinding_step");
  loggerf(WARNING, "%02i:%02i\n", c.current->module, c.current->id);

  c.length++;

  // Init return struct
  struct pathfindingstep s;
  s.found = 0;
  s.instructions = 0;

  // Visualize
  c.current->state = CAUTION;
  c.current->changed |= State_Changed;
  WS_trackUpdate(0);

  usleep(100000);

  c.current->state = RESTRICTED;
  c.current->changed |= State_Changed;
  WS_trackUpdate(0);

  if(c.length > PATHFINDING_MAX_LENGHT){
    loggerf(ERROR, "ABORTING");
    return s;
  }

  // If finish found return
  if(c.current == c.end){
    s.found = 1;
    loggerf(WARNING, "FOUND\n");
    return s;
  }

  if(c.link->type == RAIL_LINK_R){
    if(dircmp(c.current, c.link->p)){
      c.current = c.link->p;
      c.link = Next_link(c.current, c.dir);
    }
    else{
      c.current = c.link->p;
      c.dir ^= PREV;
      c.link = Next_link(c.current, c.dir);
    }
    loggerf(ERROR, "%i B", c.length);
    return _pathfinding_step(c);
  }
  else if(c.link->type == RAIL_LINK_S){
    loggerf(ERROR, "%i S fork!!", c.length);
    Switch * S = (Switch *)c.link->p;
    struct pathfindingstep str;
    struct pathfindingstep div;
    c.link = &S->str;
    str = _pathfinding_step(c);
    c.link = &S->div;
    div = _pathfinding_step(c);

    struct pathinstruction * instr = _calloc(1, struct pathinstruction);
    instr->type = RAIL_LINK_S;
    instr->p = S;
    instr->optionalstates = _calloc(2, uint8_t);
    instr->next_instruction = _calloc(2, struct pathinstruction *);
    instr->states = 0;

    if(str.found || div.found){
      s.found = 1;
    }

    if (str.found){
      instr->optionalstates[instr->states] = 0;
      instr->next_instruction[instr->states] = str.instructions;
      instr->states++;
    }
    if (div.found){
      instr->optionalstates[instr->states] = 1;
      instr->next_instruction[instr->states] = div.instructions;
      instr->states++;
    }

    s.instructions = instr;
    return s;
  }
  else if(c.link->type == RAIL_LINK_s){
    loggerf(ERROR, "%i s", c.length);
    c.link = &((Switch *)c.link->p)->app;
    return _pathfinding_step(c);
  }
  else if(c.link->type == RAIL_LINK_M){
    loggerf(ERROR, "%i M", c.length);
  }
  else if(c.link->type == RAIL_LINK_m){
    loggerf(ERROR, "%i m", c.length);
  }
  else{
    loggerf(ERROR, "%i ?", c.length);
  }
}

void pathfinding_print(struct pathinstruction * instr){
  while(instr){
    if(instr->type == RAIL_LINK_S){
      printf("  SW %02i:%02i\t", ((Switch *)instr->p)->module, ((Switch *)instr->p)->id);
    }
    else if(instr->type == RAIL_LINK_M){
      printf("MSSW %02i:%02i\t", ((MSSwitch *)instr->p)->module, ((MSSwitch *)instr->p)->id);
    }
    else{
      return;
    }

    if(instr->states > 0){
      printf("%i", instr->optionalstates[0]);
      if(instr->states > 1){
        printf("+");
      }
      printf("\n");
      instr = instr->next_instruction[0];
    }
    else{
      instr = 0;
    }
  }
}

void free_pathinstructions(struct pathinstruction * instr){
  if(instr->next_instruction){
    for(uint8_t i = 0; i < instr->states; i++){
      if(instr->optionalstates[i])
        free_pathinstructions(instr->next_instruction[i]);
    }
    _free(instr->next_instruction);
  }
  _free(instr->optionalstates);
  _free(instr);
}