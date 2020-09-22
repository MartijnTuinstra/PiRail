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

Route * find(Block * start, Block * end){
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

  Route * r = new Route(c, sf, sr);

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
  loggerf(INFO, "findStepSolveSwS: (%2i:%2i) %i, %i", Sw->module, Sw->id, str->found, div->found);

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

Route::Route(struct control c, struct step forward, struct step reverse){
  found_forward = forward.found;
  found_reverse = reverse.found;
  length = (forward.length > reverse.length) ? forward.length : reverse.length;

  Sw_S = c.Sw_S;
  Sw_s = c.Sw_s;
  MSSw_A = c.MSSw_A;
  MSSw_B = c.MSSw_B;
}

Route::~Route(){
  for(uint16_t i = 0; i < SwManager->uniqueSwitch.size; i++){
    if(Sw_S[i])
      _free(Sw_S);

    if(Sw_s[i])
      _free(Sw_s);
  }
  _free(Sw_S);
  _free(Sw_s);

  for(uint16_t i = 0; i < SwManager->uniqueMSSwitch.size; i++){
    if(MSSw_A[i])
      _free(MSSw_A);

    if(MSSw_B[i])
      _free(MSSw_B);
  }
  _free(MSSw_A);
  _free(MSSw_B);
}

};
