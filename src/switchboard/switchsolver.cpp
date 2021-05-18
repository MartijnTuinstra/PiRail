#include "switchboard/switchsolver.h"
#include "algorithm/core.h"

#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/station.h"
#include "pathfinding.h"
#include "rollingstock/railtrain.h"

#include "utils/logger.h"
#include "flags.h"

namespace SwitchSolver {

int solve(RailTrain * T, Block * B, Block * tB, struct rail_link link, int flags){
  struct find f = {0, 0};
  bool changed = false;

  PathFinding::Route * r = 0;

  if(T && T->routeStatus && T->route){
    r = T->route;
  }

  loggerf(DEBUG, "SwitchSolver::solve -> findPath %x", (unsigned int)r);
  f = findPath(T, r, tB, link, flags);

  loggerf(DEBUG, "SwitchSolver::solve (%i, %i)", f.possible, f.allreadyCorrect);

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

    changed = true;

    if(f.possible)
      B->recalculate = 1;
  }

  if(!f.possible){
    setWrong(r, tB, link, flags);
  }
  else{
    flags = tB->dir;
    reservePath(T, r, tB, link, flags);
  }

  return changed;
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
    if(B && ((B->reservedBy.size() > 0 && !B->isReservedBy(T)) || B->blocked)){
      return f;
    }

    PathFinding::instruction * instr = r ? r->Sw_S[Sw->uid] : 0;

    struct rail_link * link[2] = {&Sw->str, &Sw->div};
    // int result[2] = {0, 0};

    for(uint8_t i = 0; i < 2; i++){
      uint8_t j;
      if(instr){
        if(i >= instr->nrOptions)
          break;
        
        j = instr->options[i];
      }
      else{
        j = i;
      }
      // loggerf(INFO, "check S %i (state: %i->%i)", Sw->id, Sw->state, j);
      struct find tf = findPath(T, r, Sw, *link[j], flags);
      if(instr) instr->possible[i] = tf.possible;
      f.possible |= tf.possible;
      f.allreadyCorrect |= (tf.allreadyCorrect && (Sw->state == j));
    }

    return f;
  }
  else if(link.type == RAIL_LINK_s){
    Switch * Sw = link.p.Sw;

    Block * B = Sw->Detection;
    if(B && ((B->reservedBy.size() > 0 && !B->isReservedBy(T)) || B->blocked))
      return f;

    // loggerf(INFO, "check s %i (state: %i, str.p: %x, div.p: %x)", Sw->id, Sw->state, (unsigned int)Sw->str.p.p, (unsigned int)Sw->div.p.p);
    f = findPath(T, r, Sw, Sw->app, flags);
    f.allreadyCorrect &= Sw->approachable(p, flags);

    return f;
  }
  else if(link.type == RAIL_LINK_MA){
    MSSwitch * Sw = link.p.MSSw;

    Block * B = Sw->Detection;
    if(B && ((B->reservedBy.size() > 0 && !B->isReservedBy(T)) || B->blocked))
      return f;

    PathFinding::instruction * instr = r ? r->MSSw_A[Sw->uid] : 0;

    for(uint8_t i = 0; i < Sw->state_len; i++){
      uint8_t j;
      if(instr){
        if(i >= instr->nrOptions)
          break;
        
        j = instr->options[i];
      }
      else{
        j = i;
      }
      // loggerf(INFO, "check S %i (state: %i->%i)", Sw->id, Sw->state, j);
      struct find tf = findPath(T, r, Sw, Sw->sideB[j], flags);
      if(instr) instr->possible[i] = tf.possible;
      f.possible |= tf.possible;
      f.allreadyCorrect |= (tf.allreadyCorrect && (Sw->state == j));
    }

    return f;
  }
  else if(link.type == RAIL_LINK_MB){
    MSSwitch * Sw = link.p.MSSw;

    Block * B = Sw->Detection;
    if(B && ((B->reservedBy.size() > 0 && !B->isReservedBy(T)) || B->blocked))
      return f;

    PathFinding::instruction * instr = r ? r->MSSw_B[Sw->uid] : 0;

    for(uint8_t i = 0; i < Sw->state_len; i++){
      uint8_t j;
      if(instr){
        if(i >= instr->nrOptions)
          break;
        
        j = instr->options[i];
      }
      else{
        j = i;
      }
      // loggerf(INFO, "check S %i (state: %i->%i)", Sw->id, Sw->state, j);
      struct find tf = findPath(T, r, Sw, Sw->sideA[j], flags);
      if(instr) instr->possible[i] = tf.possible;
      f.possible |= tf.possible;
      f.allreadyCorrect |= (tf.allreadyCorrect && (Sw->state == j));
    }

    return f;
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
      if(flags & FL_CONTINUEDANGER && B->state <= DANGER){
        return f;
      }
      if(B->isReservedBy(T)){
        if(B->path->Entrance == B){
          f.possible = 1;
          f.allreadyCorrect = 1;
        }
      }
      else if(B->reservedBy.size() == 0){
        f.possible = 1;
        f.allreadyCorrect = 1;
      }

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
    if(B && ((B->reservedBy.size() > 0 && !B->isReservedBy(T)) || B->blocked)) {
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

      if(str)      Sw->setState(0, 0);
      else if(div) Sw->setState(1, 0);
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
          Sw->setState(instr->options[i], 0);

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
    if(B && ((B->reservedBy.size() > 0 && !B->isReservedBy(T)) || B->blocked)) {
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
        Sw->setState(1, 0);
    }
    else if(Sw->state == 1){
      if(Sw->div.p.p != p)
        Sw->setState(0, 0);
    }

    return path;
  }
  else if(link.type == RAIL_LINK_MA || link.type == RAIL_LINK_MB){
    // loggerf(ERROR, "IMPLEMENT");
    // MSSwitch * N = link.p.MSSw;
    // if(N->sideB[N->state].p.p == p){
    //   return 1;
    // }
    /////////////////////////
    
    // Go to next switch
    MSSwitch * Sw = link.p.MSSw;

    Block * B = Sw->Detection;
    if(B && ((B->reservedBy.size() > 0 && !B->isReservedBy(T)) || B->blocked)) {
      // loggerf(INFO, "Switch reserved");
      return 0;
    }

    PathFinding::instruction * instr = 0;
    if(r)
      instr = (link.type == RAIL_LINK_MA) ? r->MSSw_A[Sw->uid] : r->MSSw_B[Sw->uid];

    // No Route or no instruction for this switch => default behaviour
    if(!instr){
      // loggerf(INFO, "Switch S %i No Route", Sw->id);
      bool setArray[256];
      memset(setArray, 0, 256);
      struct rail_link * nextLink = (link.type == RAIL_LINK_MA) ? Sw->sideB : Sw->sideA;

      for(uint8_t i = 0; i < Sw->state_len; i++){
        if(Sw->approachableB(p, FL_SWITCH_CARE)){
          setArray[i] = setPath(T, r, Sw, nextLink[i], flags);
          if(Sw->state == i && setArray[i])
            return 1; // Allready set correctly
        }
      }

      for(uint8_t i = 0; i < Sw->state_len; i++){
        if(setArray[i]){
          Sw->setState(i, 0);
          return 1;
        }
      }

      return 0;
    }

    bool switchSet = 0;

    struct rail_link * nextLink = (link.type == RAIL_LINK_MA) ? Sw->sideB : Sw->sideA;

    for(uint8_t i = 0; i < instr->nrOptions; i++){
      // uint8_t j = instr->options[i]; state
      // loggerf(INFO, "check S %i (%i ? %i)", Sw->id, i, instr->possible[i]);

      if(instr->possible[i] && ((link.type == RAIL_LINK_MA) ? Sw->approachableA(p, FL_SWITCH_CARE) : Sw->approachableB(p, FL_SWITCH_CARE)) ){
        if(Sw->state != instr->options[i])
          Sw->setState(instr->options[i], 0);

        switchSet = setPath(T, r, Sw, nextLink[instr->options[i]], flags);
        break;
      }

    }

    return switchSet;
  }

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
      return (flags & FL_CONTINUEDANGER && B->state <= DANGER) ? 0 : 1;
    }

    if(B->next.p.p == p)
      return setPath(T, r, B, B->prev, flags);
    else if(B->prev.p.p == p)
      return setPath(T, r, B, B->next, flags);
  }

  return 0;
}

void setWrong(PathFinding::Route * r, void * p, struct rail_link link, int flags){
  loggerf(TRACE, "SwitchSolver::setWrong (%x, %x, %i)", (unsigned int)p, (unsigned int)&link, flags);

  if(link.type == RAIL_LINK_S){
    // Go to next switch
    Switch * Sw = link.p.Sw;
    Sw->Detection->switchWrongState = true;
    Algorithm::rail_state(&Sw->Detection->Alg, 0);
  }
  else if(link.type == RAIL_LINK_s){
    // Check if switch is in correct state
    // and continue to next switch
    Switch * Sw = link.p.Sw;

    if(!Sw->approachable(p, flags))
      return;

    setWrong(r, Sw, Sw->app, flags);
  }
  else if(link.type == RAIL_LINK_MA ||
          link.type == RAIL_LINK_MB   ){
    MSSwitch * Sw = link.p.MSSw;
    Sw->Detection->switchWrongState = true;
    Algorithm::rail_state(&Sw->Detection->Alg, 0);
  }
  else if (link.type == RAIL_LINK_R){
    Block * B = link.p.B;
    B->switchWrongState = true;
    Algorithm::rail_state(&B->Alg, 0);
  }

  return;
}

void dereservePath(RailTrain * T, PathFinding::Route * r, void * p, struct rail_link link, int flags){
  loggerf(TRACE, "DEreservePath (%x, %x, %x, %x, %i)", (unsigned int)T, (unsigned int)r, (unsigned int)p, (unsigned int)&link, flags);

  if(link.type == RAIL_LINK_S || link.type == RAIL_LINK_s){
    // Go to next switch
    Switch * Sw = link.p.Sw;
    Block * DB = Sw->Detection;

    if(DB->isReservedBy(T))
      T->dereserveBlock(DB);
    else if(DB->reservedBy.size() > 0){
      loggerf(INFO, "Switch cannot be de-reserved");
      return;
    }

    DB->switchReserved = false;
    DB->switchWrongState = 0;

    // loggerf(TRACE, "Set switch %02i:%02i to deRESERVED", Sw->module, Sw->id);

    if(link.type == RAIL_LINK_S){
      if(Sw->state == 0)
        return dereservePath(T, r, Sw, Sw->str, flags);
      else if(Sw->state == 1)
        return dereservePath(T, r, Sw, Sw->div, flags);
    }
    else // RAIL_LINK_s
      return dereservePath(T, r, Sw, Sw->app, flags);
  }
  else if(link.type == RAIL_LINK_MA || link.type == RAIL_LINK_MB){
    // Check if switch is in correct state
    // and continue to next switch
    MSSwitch * Sw = link.p.MSSw;
    Block * DB = Sw->Detection;

    if(DB->isReservedBy(T))
      T->dereserveBlock(DB);
    else if(DB->reservedBy.size() > 0){
      loggerf(INFO, "Switch cannot be de-reserved");
      return;
    }

    DB->switchReserved = false;
    DB->switchWrongState = 0;

    // loggerf(TRACE, "Set switch %02i:%02i to RESERVED", Sw->module, Sw->id);

    if(link.type == RAIL_LINK_MA)
      return dereservePath(T, r, Sw, Sw->sideB[Sw->state], flags);
    else // RAIL_LINK_MB
      return dereservePath(T, r, Sw, Sw->sideA[Sw->state], flags);
  }

  else if (link.type == RAIL_LINK_R){
    Block * B = link.p.B;
    // loggerf(TRACE, "check B %i", B->id);
    if(B->type != NOSTOP){
      if(B->isReservedBy(T)){
        B->path->dereserve(T);
      }
      return; // Train can stop on the block, so a possible path
    }

    if(B->next.p.p == p)
      return dereservePath(T, r, B, B->prev, flags);
    else if(B->prev.p.p == p)
      return dereservePath(T, r, B, B->next, flags);
  }
}

int reservePath(RailTrain * T, PathFinding::Route * r, void * p, struct rail_link link, int flags){
  loggerf(TRACE, "reservePath (%x, %x, %x, %x, %i)", (unsigned int)T, (unsigned int)r, (unsigned int)p, (unsigned int)&link, flags);

  if(link.type == RAIL_LINK_S || link.type == RAIL_LINK_s){
    // Go to next switch
    Switch * Sw = link.p.Sw;
    Block * DB = Sw->Detection;

    if(!dircmp(DB->dir, flags))
      DB->reverse();
    flags = DB->dir; 

    DB->switchReserved = true;
    if(!DB->isReservedBy(T))
      T->reserveBlock(DB);

    DB->switchWrongState = 0;

    loggerf(TRACE, "Set switch %02i:%02i to RESERVED", Sw->module, Sw->id);

    if(link.type == RAIL_LINK_S){
      if(Sw->state == 0)
        return reservePath(T, r, Sw, Sw->str, flags);
      else if(Sw->state == 1)
        return reservePath(T, r, Sw, Sw->div, flags);
    }
    else{
      return reservePath(T, r, Sw, Sw->app, flags);
    }
  }
  else if(link.type == RAIL_LINK_MA || link.type == RAIL_LINK_MB){
    
    // Go to next switch
    MSSwitch * Sw = link.p.MSSw;
    Block * DB = Sw->Detection;

    if(!dircmp(DB->dir, flags))
      DB->reverse();
    flags = DB->dir; 

    DB->switchReserved = true;
    if(!DB->isReservedBy(T))
      T->reserveBlock(DB);

    DB->switchWrongState = 0;

    loggerf(TRACE, "Set switch %02i:%02i to RESERVED", Sw->module, Sw->id);

    if(link.type == RAIL_LINK_MA){
      return reservePath(T, r, Sw, Sw->sideB[Sw->state], flags);
    }
    else{ // RAIL_LINK_MB
      return reservePath(T, r, Sw, Sw->sideA[Sw->state], flags);
    }
  }

  else if (link.type == RAIL_LINK_R){
    Block * B = link.p.B;
    loggerf(TRACE, "check B %i", B->id);
    if(B->type != NOSTOP){
      if(!B->reserved){
        loggerf(DEBUG, "Path Entrance: %02i:%02i", B->path->Entrance->module, B->path->Entrance->id);
        if(B->path->Entrance != B)
          B->path->reverse();

        B->path->reserve(T);
      }
      return 1; // Train can stop on the block, so a possible path
    }

    if(!dircmp(B->dir, flags)){
      B->reverse();
    }
    flags = B->dir; 

    if(B->next.p.p == p)
      return reservePath(T, r, B, B->prev, flags);
    else if(B->prev.p.p == p)
      return reservePath(T, r, B, B->next, flags);
  }

  return 0;
}

};
