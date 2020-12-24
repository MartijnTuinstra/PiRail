#include "switchboard/manager.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/switchSolver.h"
#include "switchboard/station.h"

#include "rollingstock/railtrain.h"

#include "utils/mem.h"
#include "utils/logger.h"
#include "modules.h"
#include "system.h"

#include "algorithm/core.h"
#include "algorithm/queue.h"

namespace SwitchSolver {

int solve(RailTrain * T, Block * B, Block * tB, struct rail_link link, int flags){
  struct find f = {0, 0};

  PathFinding::Route * r = 0;

  if(T && T->onroute && T->route){
    r = T->route;
  }

  loggerf(WARNING, "SwitchSolver::solve -> findPath");
  f = findPath(T, r, tB, link, flags);

  loggerf(WARNING, "SwitchSolver::solve (%i, %i)", f.possible, f.allreadyCorrect);

  if(r){
    char debug[1000];
    r->print(debug);
    loggerf(INFO, "%s", debug);
  }

  if(!r)
    f.possible = 1;

  // If route exists check if possible and if allready correct
  // If no route exists only check if allready correct
  if(!(r && !f.possible) && !f.allreadyCorrect){
    // Path needs changes
    dereservePath(T, r, tB, link, flags);

    f.possible &= setPath(T, r, tB, link, flags);

    if(f.possible)
      B->recalculate = 1;
  }

  if(!f.possible){
    setWrong(r, tB, link, flags);
  }
  else{
    reservePath(T, r, tB, link, flags);
  }

  return f.possible;
}

struct find findPath(RailTrain * T, PathFinding::Route * r, void * p, struct rail_link link, int flags){
  // Check if switches are set to a good path
  loggerf(DEBUG, "SwitchSolver::findPath (%x, %x, %x, %i)", (unsigned int)r, (unsigned int)p, (unsigned int)&link, flags);

  struct find f = {0, 0};

  if(!link.p.p){
    return f;
  }
  else if(link.type == RAIL_LINK_S){
    Switch * Sw = link.p.Sw;

    Block * B = Sw->Detection;
    if(B && B->reservedBy && B->reservedBy != T){
      return f;
    }

    PathFinding::instruction * instr = 0;
    if(r)
      instr = r->Sw_S[Sw->uid];

    if(!instr){
      // loggerf(INFO, "check S %i (state: %i) No Route", Sw->id, Sw->state);
      //Default behaviour
      struct find str, div;
      str = findPath(T, r, Sw, Sw->str, flags);
      div = findPath(T, r, Sw, Sw->div, flags);

      loggerf(INFO, "SwitchFindPath: str: (%i, %i), div: (%i, %i)", str.possible, str.allreadyCorrect, div.possible, div.allreadyCorrect);

      f.possible = (str.possible | div.possible);

      if(Sw->state == 0){
        f.allreadyCorrect |= str.allreadyCorrect;
      }
      else if(Sw->state == 1){
        f.allreadyCorrect |= div.allreadyCorrect;
      }

      return f;
    }

    struct rail_link * link[2] = {&Sw->str, &Sw->div};
    // int result[2] = {0, 0};

    for(uint8_t i = 0; i < instr->nrOptions; i++){
      uint8_t j = instr->options[i];
      // loggerf(INFO, "check S %i (state: %i->%i)", Sw->id, Sw->state, j);
      struct find tf = findPath(T, r, Sw, *link[j], flags);
      instr->possible[i] = tf.possible;
      f.possible |= tf.possible;
      f.allreadyCorrect |= (tf.allreadyCorrect && (Sw->state == j));
    }

    return f;
  }
  else if(link.type == RAIL_LINK_s){
    Switch * Sw = link.p.Sw;

    Block * B = Sw->Detection;
    if(B && B->reservedBy && B->reservedBy != T)
      return f;

    // loggerf(INFO, "check s %i (state: %i, str.p: %x, div.p: %x)", Sw->id, Sw->state, (unsigned int)Sw->str.p.p, (unsigned int)Sw->div.p.p);
    f = findPath(T, r, Sw, Sw->app, flags);
    f.allreadyCorrect &= Sw->approachable(p, flags);

    return f;
  }
  else if(link.type == RAIL_LINK_MA){
    loggerf(WARNING, "IMPLEMENT");
    MSSwitch * Sw = link.p.MSSw;

    Block * B = Sw->Detection;
    if(B && B->reservedBy && B->reservedBy != T)
      return f;

    if(Sw->sideB[Sw->state].p.p == p){
      return f;
    }
  }
  else if(link.type == RAIL_LINK_MB){
    loggerf(WARNING, "IMPLEMENT");
    MSSwitch * Sw = link.p.MSSw;

    Block * B = Sw->Detection;
    if(B && B->reservedBy && B->reservedBy != T)
      return f;

    if(Sw->sideA[Sw->state].p.p == p){
      return f;
    }
  }

  else if (link.type == RAIL_LINK_R){

    Block * B = link.p.B;

    // loggerf(INFO, "check B %i", B->id);

    // Block is part of station that is blocked by a train stopped on it. Not a solution
    if(B->type == STATION && B->station && (B->station->stoppedTrain || 
                                            (B->station->parent && B->station->parent->stoppedTrain)) ){
      loggerf(INFO, "FOUND BLOCKED STATION %i (%s)", B->station->id, B->station->name);
      return f;
    }

    // If Block is the exit of a path
    else if(B->path && B != B->path->Entrance && B->path->reserved){
      // loggerf(INFO, "FOUND WRONG PATH DIRECTION");
      return f;
    }

    // Train can stop on the block, so a solution
    else if(B->type != NOSTOP){
      // loggerf(INFO, "FOUND FREE BLOCK :)");
      f.possible = 1;
      f.allreadyCorrect = 1;
      return f; 
    }


    if(B->next.p.p == p)
      return findPath(T, r, B, B->prev, flags);
    else if(B->prev.p.p == p)
      return findPath(T, r, B, B->next, flags);
  }

  // loggerf(ERROR, "Done checking");
  return f;
}

int setPath(RailTrain * T, PathFinding::Route * r, void * p, struct rail_link link, int flags){
  loggerf(TRACE, "setPath (%x, %x, %x, %i)", (unsigned int)r, (unsigned int)p, (unsigned int)&link, flags);
  // //Check if switch is occupied
  // if (link.type == RAIL_LINK_S || link.type == RAIL_LINK_s) {
  //   if((link.p.Sw)->Detection && (link.p.Sw)->Detection->state == RESERVED_SWITCH){
  //     loggerf(ERROR, "Switch allready Reserved");
  //     return 0;
  //   }
  // }
  // else if (link.type >= RAIL_LINK_MA && link.type <= RAIL_LINK_MB_inside) {
  //   if((link.p.MSSw)->Detection && (link.p.MSSw)->Detection->state == RESERVED_SWITCH){
  //     loggerf(ERROR, "Switch allready Reserved");
  //     return 0;
  //   }
  // }


  if(link.type == RAIL_LINK_S){
    // Go to next switch
    Switch * Sw = link.p.Sw;

    Block * B = Sw->Detection;
    if(B && ((B->reservedBy && B->reservedBy != T) || B->blocked)) {
      // loggerf(INFO, "Switch reserved");
      return 0;
    }

    PathFinding::instruction * instr = 0;
    if(r)
      instr = r->Sw_S[Sw->uid];

    // No Route or no instruction for this switch => default behaviour
    if(!instr){
      // loggerf(INFO, "Switch S %i No Route", Sw->id);
      bool str, div;
      str = setPath(T, r, Sw, Sw->str, flags);
      div = setPath(T, r, Sw, Sw->div, flags);

      // loggerf(INFO, "SwitchSetFreePath: str: %i, div: %i", str, div);

      if((Sw->state == 0 && str) || (Sw->state == 1 && div))
        return 1;

      if(str)      Sw->setState(0);
      else if(div) Sw->setState(1);
      else         return 0;

      return 1;
    }

    bool switchSet = 0;

    struct rail_link * links[2] = {&Sw->str, &Sw->div};

    for(uint8_t i = 0; i < instr->nrOptions; i++){
      // uint8_t j = instr->options[i]; state
      // loggerf(INFO, "check S %i (%i ? %i)", Sw->id, i, instr->possible[i]);

      if(instr->possible[i]){
        if(Sw->state != instr->options[i])
          Sw->setState(instr->options[i]);

        switchSet = setPath(T, r, Sw, *links[instr->options[i]], flags);
        break;
      }

    }

    return switchSet;

  }
  else if(link.type == RAIL_LINK_s){
    // Check if switch is in correct state
    // and continue to next switch
    Switch * Sw = link.p.Sw;

    Block * B = Sw->Detection;
    if(B && ((B->reservedBy && B->reservedBy != T) || B->blocked)) {
      // loggerf(INFO, "Switch reserved");
      return 0;
    }

    // loggerf(INFO, "Switch s %i", Sw->id);
    bool path = setPath(T, r, Sw, Sw->app, flags);

    if(!path)
      return path;

    // loggerf(INFO, "set s %i (state: %i, str.p: %x, div.p: %x)", Sw->id, Sw->state, (unsigned int)Sw->str.p.p, (unsigned int)Sw->div.p.p);
    if(Sw->state == 0){
      if(Sw->str.p.p != p)
        Sw->setState(1, 1);
    }
    else if(Sw->state == 1){
      if(Sw->div.p.p != p)
        Sw->setState(0, 1);
    }

    return path;
  }
  // else if(link.type == RAIL_LINK_MA){
  //   loggerf(ERROR, "IMPLEMENT");
  //   MSSwitch * N = link.p.MSSw;
  //   if(N->sideB[N->state].p.p == p){
  //     return 1;
  //   }
  // }
  // else if(link.type == RAIL_LINK_MB){
  //   loggerf(ERROR, "IMPLEMENT");
  //   MSSwitch * N = link.p.MSSw;
  //   if(N->sideA[N->state].p.p == p){
  //     return 1;
  //   }
  // }

  else if (link.type == RAIL_LINK_R){
    Block * B = link.p.B;
    // loggerf(INFO, "check B %i", B->id);

    // Block is part of station that is blocked by a train stopped on it. Not a solution
    if(B->type == STATION && B->station && (B->station->stoppedTrain || 
                                            (B->station->parent && B->station->parent->stoppedTrain)) ){
      // loggerf(INFO, "STATION FAIL");
      return 0;
    }

    // If Block is the exit of a path
    else if(B->path && B != B->path->Entrance && B->path->reserved){
      // loggerf(INFO, "PATH FAIL");
      return 0;
    }

    // Train can stop on the block, so a solution
    else if(B->type != NOSTOP){
      // loggerf(INFO, "NOSTOP SUCCESS");
      return 1; 
    }

    if(B->next.p.p == p)
      return setPath(T, r, B, B->prev, flags);
    else if(B->prev.p.p == p)
      return setPath(T, r, B, B->next, flags);
  }

  return 0;
}

int setWrong(PathFinding::Route * r, void * p, struct rail_link link, int flags){
  loggerf(TRACE, "SwitchSolver::setWrong (%x, %x, %i)", (unsigned int)p, (unsigned int)&link, flags);

  if(link.type == RAIL_LINK_S){
    // Go to next switch
    Switch * Sw = link.p.Sw;

    // loggerf(INFO, "Switch S %i", Sw->id);

    if(Sw->state == 0)
      setWrong(r, Sw, Sw->str, flags);
    else
      setWrong(r, Sw, Sw->div, flags);

    if(Sw->Detection)
      Sw->Detection->switchWrongState = true;

    return 1;
  }
  else if(link.type == RAIL_LINK_s){
    // Check if switch is in correct state
    // and continue to next switch
    Switch * Sw = link.p.Sw;

    if(!Sw->approachable(p, flags))
      return 0;

    if(Sw->Detection)
      Sw->Detection->switchWrongState = true;

    // loggerf(INFO, "Switch s %i", Sw->id);
    setWrong(r, Sw, Sw->app, flags);

    return 0;
  }
  // else if(link.type == RAIL_LINK_MA){
  //   loggerf(ERROR, "IMPLEMENT");
  //   MSSwitch * N = link.p.MSSw;
  //   if(N->sideB[N->state].p.p == p){
  //     return 1;
  //   }
  // }
  // else if(link.type == RAIL_LINK_MB){
  //   loggerf(ERROR, "IMPLEMENT");
  //   MSSwitch * N = link.p.MSSw;
  //   if(N->sideA[N->state].p.p == p){
  //     return 1;
  //   }
  // }

  else if (link.type == RAIL_LINK_R){
    Block * B = link.p.B;
    // loggerf(INFO, "check B %i", B->id);

    // Block is part of station that is blocked by a train stopped on it. Not a solution
    if(B->type == STATION && B->station && (B->station->stoppedTrain || 
                                            (B->station->parent && B->station->parent->stoppedTrain)) ){
      // loggerf(INFO, "STATION FAIL");
      return 0;
    }

    // If Block is the exit of a path
    else if(B->path && B != B->path->Entrance && B->path->reserved){
      // loggerf(INFO, "PATH FAIL");
      return 0;
    }

    // Train can stop on the block, so a solution
    else if(B->type != NOSTOP){
      // loggerf(INFO, "NOSTOP SUCCESS");
      return 1; 
    }

    if(B->next.p.p == p)
      return setWrong(r, B, B->prev, flags);
    else if(B->prev.p.p == p)
      return setWrong(r, B, B->next, flags);
  }

  return 0;
}

void dereservePath(RailTrain * T, PathFinding::Route * r, void * p, struct rail_link link, int flags){
  loggerf(TRACE, "DEreservePath (%x, %x, %x, %x, %i)", (unsigned int)T, (unsigned int)r, (unsigned int)p, (unsigned int)&link, flags);

  if(link.type == RAIL_LINK_S){
    // Go to next switch
    Switch * Sw = link.p.Sw;
    Block * DB = Sw->Detection;

    if(DB->reservedBy == T)
      T->dereserveBlock(DB);
    else if(DB->reservedBy){
      loggerf(INFO, "Switch cannot be de-reserved");
      return;
    }

    DB->switchWrongState = 0;

    // loggerf(TRACE, "Set switch %02i:%02i to deRESERVED", Sw->module, Sw->id);

    if(Sw->state == 0)
      return dereservePath(T, r, Sw, Sw->str, flags);
    else if(Sw->state == 1)
      return dereservePath(T, r, Sw, Sw->div, flags);
  }
  else if(link.type == RAIL_LINK_s){
    // Check if switch is in correct state
    // and continue to next switch
    Switch * Sw = link.p.Sw;
    Block * DB = Sw->Detection;

    if(DB->reservedBy == T)
      T->reserveBlock(DB);
    else if(DB->reservedBy){
      loggerf(INFO, "Switch cannot be de-reserved");
      return;
    }

    DB->switchWrongState = 0;

    // loggerf(TRACE, "Set switch %02i:%02i to RESERVED", Sw->module, Sw->id);

    return dereservePath(T, r, Sw, Sw->app, flags);
  }
  else if(link.type == RAIL_LINK_MA){
    // loggerf(ERROR, "IMPLEMENT");
    MSSwitch * N = link.p.MSSw;
    if(N->sideB[N->state].p.p == p){
      return;
    }
  }
  else if(link.type == RAIL_LINK_MB){
    // loggerf(ERROR, "IMPLEMENT");
    MSSwitch * N = link.p.MSSw;
    if(N->sideA[N->state].p.p == p){
      return;
    }
  }

  else if (link.type == RAIL_LINK_R){
    Block * B = link.p.B;
    // loggerf(TRACE, "check B %i", B->id);
    if(B->type != NOSTOP)
      return; // Train can stop on the block, so a possible path

    if(B->next.p.p == p)
      return dereservePath(T, r, B, B->prev, flags);
    else if(B->prev.p.p == p)
      return dereservePath(T, r, B, B->next, flags);
  }
}

int reservePath(RailTrain * T, PathFinding::Route * r, void * p, struct rail_link link, int flags){
  loggerf(TRACE, "reservePath (%x, %x, %x, %x, %i)", (unsigned int)T, (unsigned int)r, (unsigned int)p, (unsigned int)&link, flags);

  if(link.type == RAIL_LINK_S){
    // Go to next switch
    Switch * Sw = link.p.Sw;
    Block * DB = Sw->Detection;

    if(DB->reservedBy != T)
      T->reserveBlock(DB);

    DB->switchWrongState = 0;

    loggerf(TRACE, "Set switch %02i:%02i to RESERVED", Sw->module, Sw->id);

    if(Sw->state == 0)
      return reservePath(T, r, Sw, Sw->str, flags);
    else if(Sw->state == 1)
      return reservePath(T, r, Sw, Sw->div, flags);
  }
  else if(link.type == RAIL_LINK_s){
    // Check if switch is in correct state
    // and continue to next switch
    Switch * Sw = link.p.Sw;
    Block * DB = Sw->Detection;

    if(DB->reservedBy != T)
      T->reserveBlock(DB);

    DB->switchWrongState = 0;

    loggerf(TRACE, "Set switch %02i:%02i to RESERVED", Sw->module, Sw->id);

    return reservePath(T, r, Sw, Sw->app, flags);
  }
  else if(link.type == RAIL_LINK_MA){
    loggerf(ERROR, "IMPLEMENT");
    MSSwitch * N = link.p.MSSw;
    if(N->sideB[N->state].p.p == p){
      return 1;
    }
  }
  else if(link.type == RAIL_LINK_MB){
    loggerf(ERROR, "IMPLEMENT");
    MSSwitch * N = link.p.MSSw;
    if(N->sideA[N->state].p.p == p){
      return 1;
    }
  }

  else if (link.type == RAIL_LINK_R){
    Block * B = link.p.B;
    loggerf(TRACE, "check B %i", B->id);
    if(B->type != NOSTOP){
      T->reserveBlock(B);
      return 1; // Train can stop on the block, so a possible path
    }

    if(B->next.p.p == p)
      return reservePath(T, r, B, B->prev, flags);
    else if(B->prev.p.p == p)
      return reservePath(T, r, B, B->next, flags);
  }

  return 0;
}

};
