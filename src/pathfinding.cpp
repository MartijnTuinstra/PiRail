#include <algorithm>

#include "pathfinding.h"

#include "switchboard/manager.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/station.h"

#include "utils/mem.h"
#include "utils/logger.h"
#include "websocket/stc.h"

using namespace switchboard;

namespace PathFinding {

Route * find(Block * start, Block * end){
  struct control c = {
    .start = start,
    .end = {end, 0},
    .nr_Stations = 0,
    .prev = 0,
    .prevPtr = start,
    .dir = NEXT,
    .searchDepth = 0
  };

  c.link = start->NextLink(c.dir);

  c.Sw_S = (struct instruction **)_calloc(SwManager->uniqueSwitch.size, void *);
  c.Sw_s = (struct instruction **)_calloc(SwManager->uniqueSwitch.size, void *);
  c.MSSw_A = (struct instruction **)_calloc(SwManager->uniqueMSSwitch.size, void *);
  c.MSSw_B = (struct instruction **)_calloc(SwManager->uniqueMSSwitch.size, void *);

  // loggerf(WARNING, "Searching Forward");
  auto sf = findStep(c);

  c.dir = PREV;
  c.link = start->NextLink(c.dir);
  c.prev = 0;
  c.prevPtr = start;
  // loggerf(WARNING, "Searching Reverse");
  auto sr = findStep(c);

  Route * r = new Route(c, sf, sr);

  return r;
}


Route * find(Block * start, Station * end){
  struct control c = {
    .start = start,
    .nr_Stations = end->childs.size(),
    .destination = end,
    .prev = 0,
    .prevPtr = start,
    .dir = NEXT,
    .searchDepth = 0
  };

  c.link = start->NextLink(c.dir);

  loggerf(INFO, "Pathfinding station %s (%i childs)", end->name, c.nr_Stations);

  if(c.nr_Stations){
    c.endStations = (Station **)_calloc(c.nr_Stations, void *);

    uint8_t i = 0; // Copy stations into list
    for(auto st: end->childs){ c.endStations[i++] = st; };
    
    // Find blocks on the edge of the station(s)
    uint8_t i_Block = 0;

    for(uint8_t i = 0; i < c.nr_Stations; i++){
      for(uint8_t j = 0; j < c.endStations[i]->blocks_len; j++){
        auto B = c.endStations[i]->blocks[j];
        auto nB = B->Next_Block(NEXT, 1);
        auto pB = B->Next_Block(PREV, 1);

        bool nSt = true;
        bool pSt = true;

        if(nB){
          for(uint8_t k = 0; k < c.nr_Stations; k++){ nSt &= (nB->station != c.endStations[k]); }

          if(nSt)
            c.end[i_Block++] = B;
        }
        else{
          c.end[i_Block++] = B;
        }

        if(pB){
          for(uint8_t k = 0; k < c.nr_Stations; k++){ pSt &= (pB->station != c.endStations[k]); }

          if(pSt)
            c.end[i_Block++] = B;
        }
        else{
          c.end[i_Block++] = B;;
        }
      }
    }

    if(i_Block == 1){
      c.end[1] = c.end[0];
    }
    else if(i_Block == 0 || i_Block > 2){
      loggerf(ERROR, "Failed to initialize Pathfinding (Block, Station)  i = %i", i_Block);
      return 0;
    }
  }
  else{
    c.nr_Stations = 1;
    c.endStations = (Station **)_calloc(c.nr_Stations, void *);
    c.endStations[0] = end;

    uint8_t i_Block = 0;

    for(uint8_t i = 0; i < end->blocks_len; i++){
      auto B = end->blocks[i];
      auto nB = B->Next_Block(NEXT, 1);
      auto pB = B->Next_Block(PREV, 1);

      if((nB && nB->station != end) || !nB){
        c.end[i_Block++] = B;
      }

      if((pB && pB->station != end) || !pB){
        c.end[i_Block++] = B;
      }
    }

    if(i_Block == 1){
      c.end[1] = c.end[0];
    }
    else if(i_Block == 0){
      loggerf(ERROR, "Failed to initialize Pathfinding (Block, Station)");
      return 0;
    }
  }


  c.Sw_S = (struct instruction **)_calloc(SwManager->uniqueSwitch.size, void *);
  c.Sw_s = (struct instruction **)_calloc(SwManager->uniqueSwitch.size, void *);
  c.MSSw_A = (struct instruction **)_calloc(SwManager->uniqueMSSwitch.size, void *);
  c.MSSw_B = (struct instruction **)_calloc(SwManager->uniqueMSSwitch.size, void *);

  // loggerf(WARNING, "Searching Forward");
  auto sf = findStep(c);

  c.dir = PREV;
  c.link = start->NextLink(c.dir);
  c.prev = 0;
  c.prevPtr = start;
  // loggerf(WARNING, "Searching Reverse");
  auto sr = findStep(c);

  Route * r = new Route(c, sf, sr);

  return r;
}


struct step findStep(struct control c){
  struct step s = {
    .length = 0,
    .found = false,
    .next = 0
  };

  // char t[6][4] = {"B  ", "SwS", "Sws", "MA ", "MB "};

  // if(c.link->type <= RAIL_LINK_MB)
  //   loggerf(INFO, "PF::findStep \t %s %02i:%02i  %x", t[c.link->type], c.link->module, c.link->id, (unsigned long)c.link->p.p);
  // else if(c.link->type == RAIL_LINK_E)
  //   loggerf(INFO, "PF::findStep \t E", c.link->module, c.link->id);

  if(c.link->type == RAIL_LINK_R){
    Block * B = c.link->p.B;

    if(!c.prev)
      c.prev = c.start;

    // loggerf(INFO, "%02i:%02i %i %i\n", c.prev->module, c.prev->id, c.prev->dir, c.dir);

    if(!dircmp(c.prev, B)){
      // loggerf(WARNING, "Changin direction dircmp");
      c.dir ^= PREV;
    }
    if((!c.prev->MSSw && !B->cmpPolarity(c.prev)) ||
      (( c.prev->MSSw && !c.prev->MSSw->cmpPolarity(B, c.prevMSSwState)))
    ){
      // loggerf(WARNING, "Changing direction MSSw");
      c.dir ^= PREV;

      if(B->polarity_type == 0 && c.prev->polarity_type == 0){
        loggerf(WARNING, "Failed - wrong polarity");
        return s; // failed no way to toggle polarity
      }
    }
    uint16_t length = B->length;
    c.link = B->NextLink(c.dir);
    c.prev = B;
    
    if(B->oneWay){
      if(B->oneWayDir == B->dir){
        if(c.dir == PREV){
          loggerf(WARNING, "Failed - wrongway");
          return s;
        }
      }
      else{
        if(c.dir == NEXT){
          loggerf(WARNING, "Failed - wrongway");
          return s;
        }
      }
    }
    
    if(B == c.start){
      loggerf(WARNING, "Returned back to start");
      return s; // return failed
    }
    else if(B == c.end[0] || B == c.end[1]){
      loggerf(WARNING, "FOUND!!! :)");
      s.found = true;
      return s; // return found
    }
    else if(c.nr_Stations && B->station){
      for(uint8_t i = 0; i < c.nr_Stations; i++){
        if(c.endStations[i] == B->station){
          loggerf(WARNING, "Found path entering station from the middle");
          return s; // return failed
        }
      }
    }

    c.prevPtr = B;

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

    // void * tmpPtr = c.prevPtr;
    c.prevPtr = Sw;

    // Check solution for each side
    struct step str, div;
    c.link = &Sw->str;
    // loggerf(INFO, " SwS %2i:%2i Str", Sw->module, Sw->id);
    str = findStep(c);
    c.link = &Sw->div;
    // loggerf(INFO, " SwS %2i:%2i Div", Sw->module, Sw->id);
    div = findStep(c);

    loggerf(INFO, " SwS %2i:%2i Result %i|%i", Sw->module, Sw->id, str.found, div.found);

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

    struct instruction * instr = c.Sw_s[Sw->uid];

    // If switch allready has an instruction
    if(instr){
      s.next = instr;

      // If looped back to this switch (No switch assigned)
      if(!s.next->p.p)
        s.found = 0;

      // If a solution exists
      if(s.next->nrOptions > 0){
        s.found = 1;
        s.length = instr->length[0];
      }

      return s;
    }

    // Else, no instruction exists
    instr = (struct instruction *)_calloc(1, struct instruction);
    c.Sw_s[Sw->uid] = instr;
    instr->p.p = 0;

    // void * tmpPtr = c.prevPtr;
    c.prevPtr = Sw;

    c.link = &Sw->app;
    // loggerf(INFO, " Sws %2i:%2i App", Sw->module, Sw->id);
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
  else if(c.link->type == RAIL_LINK_MA || c.link->type == RAIL_LINK_MB){
    MSSwitch * Sw = c.link->p.MSSw;

    struct instruction * instr;
    if(c.link->type == RAIL_LINK_MA)
      instr = c.MSSw_A[Sw->uid];
    else
      instr = c.MSSw_B[Sw->uid];

    // loggerf(INFO, "MSSw %2i:%2i -> instr %x", Sw->module, Sw->id, (unsigned long)instr);

    // If switch allready has an instruction
    if(instr){
      s.next = instr;

      // If looped back to this switch (No switch assigned)
      if(!s.next->p.p)
        s.found = 0;


      // If a solution exists
      for(uint8_t i = 0; i < instr->nrOptions; i++){
        uint8_t state = instr->options[i];
        if(c.link->type == RAIL_LINK_MA)
          c.link = &Sw->sideA[state];
        else
          c.link = &Sw->sideB[state];

        if(c.link->p.p == c.prevPtr){
          s.found = true;

          if(!s.length)
            s.length = instr->length[i];
        }
      }

      return s;
    }

    if(Sw->type != MSSW_TYPE_CROSSING)
      return s;

    // Else, no instruction exists
    instr = (struct instruction *)_calloc(1, struct instruction);

    if(c.link->type == RAIL_LINK_MA)
       c.MSSw_A[Sw->uid] = instr;
    else
      c.MSSw_B[Sw->uid] = instr;

    void * tmpPtr = c.prevPtr;
    c.prevPtr = Sw;

    instr->p.p = 0;

    // Check solution for each side
    struct step * steps = (struct step *)_calloc(Sw->state_len, struct step);

    RailLink * tmpLink = c.link;

    for(uint8_t i = 0; i < Sw->state_len; i++){
      RailLink * opposite;
      if(tmpLink->type == RAIL_LINK_MA){
        c.link = &Sw->sideB[i];
        opposite = &Sw->sideA[i];
      }
      else{
        c.link = &Sw->sideA[i];
        opposite = &Sw->sideB[i];
      }

      c.prev = Sw->Detection;
      c.prevMSSwState = i;

      loggerf(INFO, " MSSw%c %2i:%2i state %i", tmpLink->type == RAIL_LINK_MA ? 'A':'B', Sw->module, Sw->id, i);

      steps[i] = findStep(c);

      if(steps[i].found && opposite->p.p == tmpPtr)
        s.found = true;
    }

    if(tmpLink->type == RAIL_LINK_MA)
      instr->type = RAIL_LINK_MA;
    else
      instr->type = RAIL_LINK_MB;

    findStepSolveMSSw(Sw, instr, steps);

    loggerf(INFO, " MSSw%c %2i:%2i result %i solutions", tmpLink->type == RAIL_LINK_MA ? 'A':'B', Sw->module, Sw->id, instr->nrOptions);

    s.next = instr;
    return s;
  }

  return s;
}

void findStepSolveSwS(Switch * Sw, struct instruction * instr, struct step * str, struct step * div){
  instr->type = RAIL_LINK_S;
  instr->p.Sw = Sw;
  // instr->prevcounter = 0;
  instr->options = (uint8_t *)_calloc(2, uint8_t);
  instr->next = (struct instruction **)_calloc(2, struct instruction *);
  instr->length = (uint16_t *)_calloc(2, uint16_t);
  instr->possible = (uint8_t *)_calloc(2, uint8_t);
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

  if(str->found && div->found && div->length < str->length){
    loggerf(WARNING, "SWAP DUE TO LENGTH");
    std::swap(instr->options[0], instr->options[1]);
    std::swap(instr->next[0], instr->next[1]);
    std::swap(instr->length[0], instr->length[1]);
  }
}

void findStepSolveMSSw(MSSwitch * Sw, struct instruction * instr, struct step * steps){
  instr->p.MSSw = Sw;
  instr->options = (uint8_t *)_calloc(Sw->state_len, uint8_t);
  instr->next = (struct instruction **)_calloc(Sw->state_len, struct instruction *);
  instr->length = (uint16_t *)_calloc(Sw->state_len, uint16_t);
  instr->possible = (uint8_t *)_calloc(Sw->state_len, uint8_t);
  instr->nrOptions = 0;

  for(uint8_t i = 0; i < Sw->state_len; i++){
    if(!steps[i].found)
      continue;

    // Insert before larger length:
    //  find position

    uint8_t pos = 0;
    for(; pos < instr->nrOptions; pos++){
      if(instr->length[pos] > steps[i].length)
        break;
    }

    //  Add an option
    instr->nrOptions++;
  
    //  Shifting all entries after position
    for (int j = instr->nrOptions; j > pos; j--) {
      instr->options[j] = instr->options[j - 1];
      instr->next[j] = instr->next[j - 1];
      instr->length[j] = instr->length[j - 1];
    }

    instr->options[pos] = i;
    instr->next[pos] = steps[i].next;
    instr->length[pos] = steps[i].length;
  }

  _free(steps);
}

Route::Route(struct control c, struct step forward, struct step reverse){
  found_forward = forward.found;
  found_reverse = reverse.found;
  length = (forward.length > reverse.length) ? forward.length : reverse.length;

  routeType = (c.nr_Stations > 0);
  if(routeType){
    destination = c.destination->uid;
  }
  else{
    destination = (c.end[0]->module << 8) + c.end[0]->id;
  }
  destinationBlocks[0] = c.end[0];
  destinationBlocks[1] = c.end[1];

  Sw_S = c.Sw_S;
  Sw_s = c.Sw_s;
  MSSw_A = c.MSSw_A;
  MSSw_B = c.MSSw_B;
}

Route::~Route(){
  for(uint16_t i = 0; i < SwManager->uniqueSwitch.size; i++){
    if(Sw_S[i]){
      _free(Sw_S[i]->options);
      _free(Sw_S[i]->next);
      _free(Sw_S[i]->length);
      _free(Sw_S[i]->possible);

      _free(Sw_S[i]);
    }

    if(Sw_s[i]){
      _free(Sw_s[i]->options);
      _free(Sw_s[i]->next);
      _free(Sw_s[i]->length);

      _free(Sw_s[i]);
    }
  }
  _free(Sw_S);
  _free(Sw_s);

  for(uint16_t i = 0; i < SwManager->uniqueMSSwitch.size; i++){
    if(MSSw_A[i]){
      _free(MSSw_A[i]->options);
      _free(MSSw_A[i]->next);
      _free(MSSw_A[i]->length);
      _free(MSSw_A[i]->possible);

      _free(MSSw_A[i]);
    }

    if(MSSw_B[i]){
      _free(MSSw_B[i]->options);
      _free(MSSw_B[i]->next);
      _free(MSSw_B[i]->length);
      _free(MSSw_B[i]->possible);

      _free(MSSw_B[i]);
    }
  }
  _free(MSSw_A);
  _free(MSSw_B);
}

void Route::print(char * str){
  for(uint16_t i = 0; i < SwManager->uniqueSwitch.size; i++){
    if(Sw_S[i]){
      str += sprintf(str, " SwS %2i:%2i\t", SwManager->uniqueSwitch[i]->module, SwManager->uniqueSwitch[i]->id);
      for(uint8_t j = 0; j < Sw_S[i]->nrOptions; j++){
        str += sprintf(str, "%c s%i l%5i ", Sw_S[i]->possible[j] ? 'p':' ', Sw_S[i]->options[j], Sw_S[i]->length[j]);
      }
      str[0] = '\n';
      str++;
    }

    if(Sw_s[i]){
      str += sprintf(str, " Sws %2i:%2i\t", SwManager->uniqueSwitch[i]->module, SwManager->uniqueSwitch[i]->id);
      for(uint8_t j = 0; j < Sw_s[i]->nrOptions; j++){
        str += sprintf(str, "  s%i l%5i ", Sw_s[i]->options[j], Sw_s[i]->length[j]);
      }
      str[0] = '\n';
      str++;
    }
  }

  for(uint16_t i = 0; i < SwManager->uniqueMSSwitch.size; i++){
    if(MSSw_A[i]){
      str += sprintf(str, " MSSwA %2i:%2i\t", SwManager->uniqueMSSwitch[i]->module, SwManager->uniqueMSSwitch[i]->id);
      for(uint8_t j = 0; j < MSSw_A[i]->nrOptions; j++){
        str += sprintf(str, "%c s%i l%5i ", MSSw_A[i]->possible[j] ? 'p':' ', MSSw_A[i]->options[j], MSSw_A[i]->length[j]);
      }
      str[0] = '\n';
      str++;
    }

    if(MSSw_B[i]){
      str += sprintf(str, " MSSwB %2i:%2i\t", SwManager->uniqueMSSwitch[i]->module, SwManager->uniqueMSSwitch[i]->id);
      for(uint8_t j = 0; j < MSSw_B[i]->nrOptions; j++){
        str += sprintf(str, "%c s%i l%5i ", MSSw_B[i]->possible[j] ? 'p':' ', MSSw_B[i]->options[j], MSSw_B[i]->length[j]);
      }
      str[0] = '\n';
      str++;
    }
  }
  str[0] = 0;
  str++;
}

};
