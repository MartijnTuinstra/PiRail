#include "pathfinding.h"
#include "switch.h"
#include "mem.h"
#include "logger.h"
#include "websocket_msg.h"
#include "modules.h"

struct paths pathfinding(Block * start, Block * end){
  struct pathfindingconfig c;
  c.start = start;
  c.current = start;
  c.dir = NEXT;
  c.link = Next_link(c.current, c.dir);
  c.end = end;

  struct pathinstruction * final_instruction = 0;
  c.final_instruction = &final_instruction;
  c.length = 0;
  c.steps = 0;

  loggerf(INFO, "Searching path %02i:%02i -> %02i:%02i", start->module, start->id, end->module, end->id);



  c.sw_data = _calloc(1, struct pathfindingswitchdata);
  c.sw_data->sw   = _calloc(unit_len, void *);
  c.sw_data->mssw = _calloc(unit_len, void *);

  for(uint8_t i = 0; i < unit_len; i++){
    if(!Units[i])
      continue;

    if(Units[i]->switch_len)
      c.sw_data->sw[i] = _calloc(Units[i]->switch_len, void *);

    if(Units[i]->msswitch_len)
      c.sw_data->mssw[i] = _calloc(Units[i]->msswitch_len, void *);
  }

  clock_t t;
  t = clock();


  struct pathfindingstep result = _pathfinding_step(c);
  struct pathfindingstep result_backward;
  if(!c.start->oneWay){
  struct pathinstruction * final_instruction = 0;
  c.final_instruction = &final_instruction;

    c.dir = PREV;
    c.link = Next_link(c.current, c.dir);
    c.length = 0;
    c.steps = 0;
    result_backward = _pathfinding_step(c);
  }

  t = clock() - t;
  printf ("It took me %d clicks (%f seconds).\n",t,((float)t)/CLOCKS_PER_SEC);

  if(result.found){
    loggerf(ERROR, "FOUND");
    pathfinding_print(result.instructions, 0);
  }
  else
    result.instructions = 0;

  if(result_backward.found){
    loggerf(ERROR, "Found on other side");
    pathfinding_print(result_backward.instructions, 0);
  }
  else
    result_backward.instructions = 0;


  for(uint8_t i = 0; i < unit_len; i++){
    if(!Units[i])
      continue;

    if(Units[i]->switch_len){

      for(uint8_t j = 0; j < Units[i]->switch_len; j++){
        if(c.sw_data->sw[i][j] && c.sw_data->sw[i][j]->prevcounter == 0){
          c.sw_data->sw[i][j]->prevcounter++;
          free_pathinstructions(c.sw_data->sw[i][j]);
        }
      }

      _free(c.sw_data->sw[i]);
    }
    if(Units[i]->msswitch_len){
      _free(c.sw_data->mssw[i]);
    }
  }
  _free(c.sw_data->sw);
  _free(c.sw_data->mssw);
  _free(c.sw_data);

  printf("Final block counter %i\n", final_instruction->prevcounter);

  struct paths r = {result.instructions, result_backward.instructions};

  return r;
}

// Recursive Function (tree search)
struct pathfindingstep _pathfinding_step(struct pathfindingconfig c){
  loggerf(TRACE, "_pathfinding_step %02i:%02i  %x\t%x", c.current->module, c.current->id, c.final_instruction, c.final_instruction[0]);

  c.steps++;

  // Init return struct
  struct pathfindingstep s;
  s.found = 0;
  s.instructions = 0;
  s.length = c.length;

  if(c.steps > PATHFINDING_MAX_LENGHT){
    // loggerf(ERROR, "ABORTING");
    return s;
  }

  // If finish found return
  if(c.current == c.end){
    s.found = 1;
    if(!c.final_instruction[0]){
      struct pathinstruction * instr = _calloc(1, struct pathinstruction);

      instr->type = RAIL_LINK_R;
      instr->prevcounter = 1;
      instr->p = c.current;

      c.final_instruction[0] = instr;
      s.instructions = instr;
    }
    else{
      c.final_instruction[0]->prevcounter++;
      s.instructions = c.final_instruction[0];
    }

    return s;
  }
  else if(c.current == c.start && c.steps != 1){
    return s;
  }

  if(c.link->type == RAIL_LINK_R){
    if(!dircmp(c.current, c.link->p)){
      c.dir ^= PREV;
    }
    c.current = c.link->p;
    // if(((Block *)c.current)->oneWay)
    //   printf("%02i:%02i %i %i\n", ((Block *)c.current)->module, ((Block *)c.current)->id, ((Block *)c.current)->dir, c.dir);

    if(c.dir == PREV && ((Block *)c.current)->oneWay){
      return s; // Wrongway
    }
    c.length += ((Block *)c.current)->length;

    c.link = Next_link(c.current, c.dir);
    return _pathfinding_step(c);
  }

  else if(c.link->type == RAIL_LINK_S){
    Switch * S = (Switch *)c.link->p;
    if(c.sw_data->sw[S->module][S->id]){
      s.instructions = c.sw_data->sw[S->module][S->id];

      if(!s.instructions->p)
        s.found = 0;

      if(s.instructions->states > 0){
        s.found = 1;
        c.sw_data->sw[S->module][S->id]->prevcounter++;
      }
      return s;
    }

    struct pathinstruction * instr = _calloc(1, struct pathinstruction);
    c.sw_data->sw[S->module][S->id] = instr;
    instr->p = 0;

    struct pathfindingstep str;
    struct pathfindingstep div;
    c.link = &S->str;
    str = _pathfinding_step(c);
    c.link = &S->div;
    div = _pathfinding_step(c);

    instr->type = RAIL_LINK_S;
    instr->p = S;
    instr->prevcounter = 0;
    instr->optionalstates = _calloc(2, uint8_t);
    instr->next_instruction = _calloc(2, struct pathinstruction *);
    instr->lengthstates = _calloc(2, uint16_t);
    instr->states = 0;

    if(str.found || div.found){
      s.found = 1;
      instr->prevcounter++;
    }

    if (str.found){
      instr->optionalstates[instr->states] = 0;
      instr->optionalstates[instr->states] |= (str.found == 3) << 7;
      instr->next_instruction[instr->states] = str.instructions;
      instr->lengthstates[instr->states] = str.length - c.length;
      instr->states++;
    }
    if (div.found){
      instr->optionalstates[instr->states] = 1;
      instr->optionalstates[instr->states] |= (div.found == 3) << 7;
      instr->next_instruction[instr->states] = div.instructions;
      instr->lengthstates[instr->states] = div.length - c.length;
      instr->states++;
    }

    if(str.length > div.length)
      s.length = str.length;
    else
      s.length = div.length;

    s.instructions = instr;
    return s;
  }
  else if(c.link->type == RAIL_LINK_s){
    // loggerf(ERROR, "%i s", c.length);
    c.link = &((Switch *)c.link->p)->app;
    return _pathfinding_step(c);
  }
  else if(c.link->type == RAIL_LINK_MA || c.link->type == RAIL_LINK_MB){
    loggerf(ERROR, "%i M", c.length);
  }
  else if(c.link->type == RAIL_LINK_ma || c.link->type == RAIL_LINK_mb){
    loggerf(ERROR, "%i m", c.length);
  }
  else{
    // loggerf(ERROR, "%i ?", c.length);
  }

  return s;
}

void pathfinding_print(struct pathinstruction * instr, uint8_t level){
  if(level > 10){
    printf("\n");
    return;
  }
  while(instr){
    // for(uint8_t i = 0; i < level; i++)
    //   printf(" ");

    if(instr->type == RAIL_LINK_R){
      printf("   B %02i:%02i\n", ((Block *)instr->p)->module, ((Block *)instr->p)->id);
      return;
    }
    else if(instr->type == RAIL_LINK_S){
      printf("  SW %02i:%02i\t", ((Switch *)instr->p)->module, ((Switch *)instr->p)->id);
    }
    else if(instr->type == RAIL_LINK_MA || instr->type == RAIL_LINK_MB){
      printf("MSSW %02i:%02i\t", ((MSSwitch *)instr->p)->module, ((MSSwitch *)instr->p)->id);
    }
    else{
      printf("adsfjkasdf\n");
      return;
    }

    if(instr->states > 0){
      for(uint8_t i = 0; i < instr->states; i++)
        printf("%i:%i\t", instr->optionalstates[i] & 0x7F, instr->lengthstates[i]);
      printf("\n");

      for(uint8_t i = 0; i < instr->states; i++){

        for(uint8_t j = 0; j < level; j++)
          printf(" ");
        printf(" -> ");
        if(instr->optionalstates[i] & 0x80){
          printf("R\n");
          return;
        }
        else
          pathfinding_print(instr->next_instruction[i], level + 1);
        if(!instr->next_instruction[i])
          printf("\n");
      }
      return;
    }
    else{
      instr = 0;
    }
  }
  printf("\n");
}

void free_pathinstructions(struct pathinstruction * instr){
  if(!instr)
    return;

  if(--instr->prevcounter)
    return;

  if(instr->type == RAIL_LINK_R){
    instr = _free(instr);
    return;
  }

  if(instr->next_instruction){
    for(uint8_t i = 0; i < instr->states; i++){
      free_pathinstructions(instr->next_instruction[i]);
    }
  }

  instr->next_instruction = _free(instr->next_instruction);
  instr->optionalstates = _free(instr->optionalstates);
  instr->lengthstates = _free(instr->lengthstates);
  instr = _free(instr);
}
