#include "pathfinding.h"
#include "switchboard/manager.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "mem.h"
#include "logger.h"
#include "websocket/stc.h"
#include "modules.h"

using namespace switchboard;

namespace PathFinding {

struct route find(Block * start, Block * end){
  struct control c = {
    .start = start,
    .end = end,
    .prev = 0,
    .dir = NEXT,
    .searchDepth = 0
  };

  c.link = start->NextLink(c.dir);

  c.Sw_S = (struct instruction **)_calloc(SwManager->uniqueSwitch.size, void *);
  c.Sw_s = (struct instruction **)_calloc(SwManager->uniqueSwitch.size, void *);
  c.MSSw_A = (struct instruction **)_calloc(SwManager->uniqueMSSwitch.size, void *);
  c.MSSw_B = (struct instruction **)_calloc(SwManager->uniqueMSSwitch.size, void *);

  loggerf(WARNING, "Searching Forward");
  auto sf = findStep(c);

  c.dir = PREV;
  c.link = start->NextLink(c.dir);
  c.prev = 0;
  loggerf(WARNING, "Searching Reverse");
  auto sr = findStep(c);

  struct route r = {
    .found_forward = sf.found,
    .found_reverse = sr.found,
    .length = (sf.length > sr.length) ? sf.length : sr.length,

    .Sw_S = c.Sw_S,
    .Sw_s = c.Sw_s,
    .MSSw_A = c.MSSw_A,
    .MSSw_B = c.MSSw_B,
  };

  return r;

  // if(s.found){
  //   loggerf(INFO, "FOUND\nlenght: %i \t%x", s.length, (unsigned int)s.next);
  // }
  // else
  //   loggerf(WARNING, "NOT FOUND");


}

struct step findStep(struct control c){
  struct step s = {
    .length = 0,
    .found = false,
    .next = 0
  };

  char t[6][4] = {"B  ", "SwS", "Sws", "MA ", "MB "};

  if(c.link->type <= RAIL_LINK_MB)
    loggerf(INFO, "PF::findStep \t %s %02i:%02i", t[c.link->type], c.link->module, c.link->id);
  else if(c.link->type == RAIL_LINK_E)
    loggerf(INFO, "PF::findStep \t E", c.link->module, c.link->id);

  if(c.prev == c.start){
    loggerf(WARNING, "Returned back to start");
    return s;
  }
  else if(c.prev == c.end){
    loggerf(WARNING, "FOUND!!! :)");
    s.found = true;
    return s;
  }

  if(c.link->type == RAIL_LINK_R){
    Block * B = c.link->p.B;

    if(!c.prev)
      c.prev = c.start;

    loggerf(INFO, "%02i:%02i %i %i\n", c.prev->module, c.prev->id, c.prev->dir, c.dir);

    if(!dircmp(c.prev, B)){
      c.dir ^= PREV;
    }
    uint16_t length = B->length;
    c.link = B->NextLink(c.dir);
    c.prev = B;
    
    if(c.dir == PREV && B->oneWay){
      return s; // Wrongway
    }

    struct step ns = findStep(c);

    ns.length += length;

    return ns;
  }
  else if(c.link->type == RAIL_LINK_S){
    Switch * Sw = c.link->p.Sw;

    // If switch allready has an instruction
    if(c.Sw_S[Sw->uid]){
      s.next = c.Sw_S[Sw->uid];

      // If looped back to this switch (No switch assigned)
      if(!s.next->p.p)
        s.found = 0;

      // If a solution exists
      if(s.next->nrOptions > 0){
        s.found = 1;
        // s.next->prevcounter++;
      }
      return s;
    }


    // Else, no instruction exists
    struct instruction * instr = (struct instruction *)_calloc(1, struct instruction);
    c.Sw_S[Sw->uid] = instr;
    instr->p.p = 0;

    // Check solution for each side
    struct step str, div;
    c.link = &Sw->str;
    loggerf(INFO, " SwS %2i:%2i Str", Sw->module, Sw->id);
    str = findStep(c);
    c.link = &Sw->div;
    loggerf(INFO, " SwS %2i:%2i Div", Sw->module, Sw->id);
    div = findStep(c);

    findStepSolveSwS(Sw, instr, &str, &div);

    s.found = str.found || div.found;

    if(str.length > div.length)
      s.length = str.length;
    else
      s.length = div.length;

    s.next = instr;
    return s;
  }
  else if(c.link->type == RAIL_LINK_s){
    Switch * Sw = c.link->p.Sw;

    // If switch allready has an instruction
    if(c.Sw_s[Sw->uid]){
      s.next = c.Sw_s[Sw->uid];

      // If looped back to this switch (No switch assigned)
      if(!s.next->p.p)
        s.found = 0;

      // If a solution exists
      if(s.next->nrOptions > 0){
        s.found = 1;
        // s.next->prevcounter++;
      }
      return s;
    }

    // Else, no instruction exists
    struct instruction * instr = (struct instruction *)_calloc(1, struct instruction);
    c.Sw_s[Sw->uid] = instr;
    instr->p.p = 0;

    c.link = &Sw->app;
    loggerf(INFO, " Sws %2i:%2i App", Sw->module, Sw->id);
    struct step app = findStep(c);

    instr->type = RAIL_LINK_s;
    instr->p.Sw = Sw;
    // instr->prevcounter = 0;
    instr->options = (uint8_t *)_calloc(1, uint8_t);
    instr->next = (struct instruction **)_calloc(1, struct instruction *);
    instr->length = (uint16_t *)_calloc(1, uint16_t);
    instr->nrOptions = 0;

    if(app.found){
      instr->next[instr->nrOptions] = app.next;
      instr->length[instr->nrOptions] = app.length;
      instr->nrOptions = 1;
    }

    s.found = app.found;
    s.length = app.length;
    s.next = instr;

    return s;
  }

  return s;
}

void findStepSolveSwS(Switch * Sw, struct instruction * instr, struct step * str, struct step * div){
  loggerf(INFO, "findStepSolveSwS: %i, %i", str->found, div->found);

  instr->type = RAIL_LINK_S;
  instr->p.Sw = Sw;
  // instr->prevcounter = 0;
  instr->options = (uint8_t *)_calloc(2, uint8_t);
  instr->next = (struct instruction **)_calloc(2, struct instruction *);
  instr->length = (uint16_t *)_calloc(2, uint16_t);
  instr->nrOptions = 0;

  // if(str.found || div.found){
  //   instr->prevcounter++;
  // }

  if (str->found){
    instr->options[instr->nrOptions] = 0;
    instr->next[instr->nrOptions] = str->next;
    instr->length[instr->nrOptions] = str->length;
    instr->nrOptions++;
  }
  if (div->found){
    instr->options[instr->nrOptions] = 1;
    instr->next[instr->nrOptions] = div->next;
    instr->length[instr->nrOptions] = div->length;
    instr->nrOptions++;
  }
}

};

struct paths pathfinding(Block * start, Block * end){
  struct pathfindingconfig c;
  c.start = start;
  c.current = start;
  c.dir = NEXT;
  c.link = c.current->NextLink(c.dir);
  c.end = end;

  struct pathinstruction * final_instruction = 0;
  c.final_instruction = &final_instruction;
  c.length = 0;
  c.steps = 0;

  loggerf(INFO, "Searching path %02i:%02i -> %02i:%02i", start->module, start->id, end->module, end->id);



  c.sw_data = (struct pathfindingswitchdata *)_calloc(1, struct pathfindingswitchdata);
  c.sw_data->sw   = (pathinstruction **)_calloc(SwManager->uniqueSwitch.size, void *);
  c.sw_data->mssw = (pathinstruction **)_calloc(SwManager->uniqueMSSwitch.size, void *);

  clock_t t;
  t = clock();


  struct pathfindingstep result = _pathfinding_step(c);
  struct pathfindingstep result_backward;
  if(!c.start->oneWay){
    struct pathinstruction * final_instruction = 0;
    c.final_instruction = &final_instruction;

    loggerf(INFO, "Backwards");

    c.dir = PREV;
    c.link = c.current->NextLink(c.dir);
    c.length = 0;
    c.steps = 0;
    result_backward = _pathfinding_step(c);
  }

  t = clock() - t;
  printf ("It took me %ld clicks (%f seconds).\n",t,((float)t)/CLOCKS_PER_SEC);

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


  for(uint8_t i = 0; i < SwManager->uniqueSwitch.size; i++){
    struct pathinstruction * instr = c.sw_data->sw[i];
    if(!instr)
      continue;

    if(instr->prevcounter == 0){
      instr->prevcounter++;
      free_pathinstructions(instr);
    }
  }

  _free(c.sw_data->sw);
  _free(c.sw_data->mssw);

  _free(c.sw_data->sw);
  _free(c.sw_data->mssw);
  _free(c.sw_data);

  if(final_instruction)
    printf("Final block counter %i\n", final_instruction->prevcounter);

  struct paths r = {result.instructions, result_backward.instructions};

  return r;
}

// Recursive Function (tree search)
struct pathfindingstep _pathfinding_step(struct pathfindingconfig c){
  loggerf(INFO, "_pathfinding_step %02i:%02i:%2i  %x\t%x", c.current->module, c.current->id, c.link->type, c.final_instruction, c.final_instruction[0]);

  c.steps++;

  // Init return struct
  struct pathfindingstep s = {
    .found = 0,
    .length = c.length,
    .instructions = 0,
  };

  if(c.steps > PATHFINDING_MAX_LENGHT){
    loggerf(ERROR, "ABORTING");
    return s;
  }

  // If finish found return
  if(c.current == c.end){
    s.found = 1;
    if(!c.final_instruction[0]){
      struct pathinstruction * instr = (struct pathinstruction *)_calloc(1, struct pathinstruction);

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

  loggerf(INFO, "next link type %i", c.link->type);

  if(c.link->type == RAIL_LINK_R){
    loggerf(INFO, "%02i:%02i %i %i\n", c.current->module, c.current->id, c.current->dir, c.dir);

    if(!dircmp(c.current, c.link->p.B)){
      c.dir ^= PREV;
    }
    c.current = c.link->p.B;
    // if(c.current->oneWay)

    if(c.dir == PREV && c.current->oneWay){
      return s; // Wrongway
    }
    c.length += c.current->length;

    c.link = c.current->NextLink(c.dir);

    loggerf(INFO, "  next %02i:%02i:%02i", c.link->module, c.link->id, c.link->type);
    return _pathfinding_step(c);
  }

  else if(c.link->type == RAIL_LINK_S){
    Switch * S = c.link->p.Sw;
    if(c.sw_data->sw[S->uid]){
      s.instructions = c.sw_data->sw[S->uid];

      if(!s.instructions->p)
        s.found = 0;

      if(s.instructions->states > 0){
        s.found = 1;
        c.sw_data->sw[S->uid]->prevcounter++;
      }
      return s;
    }

    struct pathinstruction * instr = (struct pathinstruction *)_calloc(1, struct pathinstruction);
    c.sw_data->sw[S->uid] = instr;
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
    instr->optionalstates = (uint8_t *)_calloc(2, uint8_t);
    instr->next_instruction = (struct pathinstruction **)_calloc(2, struct pathinstruction *);
    instr->lengthstates = (uint16_t *)_calloc(2, uint16_t);
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
    loggerf(ERROR, "%i s", c.length);
    c.link = &c.link->p.Sw->app;
    return _pathfinding_step(c);
  }
  else if(c.link->type == RAIL_LINK_MA || c.link->type == RAIL_LINK_MB){
    loggerf(ERROR, "%i M", c.length);
  }
  // else if(c.link->type == RAIL_LINK_ma || c.link->type == RAIL_LINK_mb){
  //   loggerf(ERROR, "%i m", c.length);
  // }
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
    instr = (struct pathinstruction*)_free(instr);
    return;
  }

  if(instr->next_instruction){
    for(uint8_t i = 0; i < instr->states; i++){
      free_pathinstructions(instr->next_instruction[i]);
    }
  }

  instr->next_instruction = (struct pathinstruction **)_free(instr->next_instruction);
  instr->optionalstates = (uint8_t *)_free(instr->optionalstates);
  instr->lengthstates = (uint16_t *)_free(instr->lengthstates);
  instr = (pathinstruction *)_free(instr);
}
