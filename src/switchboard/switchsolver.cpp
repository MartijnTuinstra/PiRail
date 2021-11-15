#include "switchboard/switchsolver.h"
#include "algorithm/core.h"

#include "switchboard/manager.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/station.h"

#include "rollingstock/train.h"

#include "utils/logger.h"
#include "flags.h"
#include "pathfinding.h"
#include "path.h"

namespace SwitchSolver {

struct switchFind * switches;
struct msswitchFind * msswitches;

void init(){
  uint16_t uniqueSwitches = switchboard::SwManager->uniqueSwitch.items;
  switches = (struct switchFind *)_calloc(uniqueSwitches, struct switchFind);
  
  uint16_t uniqueMSSwitches = switchboard::SwManager->uniqueMSSwitch.items;
  msswitches = (struct msswitchFind *)_calloc(uniqueMSSwitches, struct msswitchFind);
}

void free(){
  _free(switches);
  _free(msswitches);
}

int solve(Train * T, Block * B, Block * tB, RailLink link, int flags){
  struct find f = {0};
  bool changed = false;

  PathFinding::Route * r = 0;

  if(T && T->routeStatus && T->route){
    r = T->route;
  }

  struct SwSolve SolveControl = {
    .train = T,
    .trainLength = T->length,
    .route = r,
    .prevBlock = tB,
    .prevPtr   = tB,
    .link  = &link,
    .flags = flags
  };

  loggerf(DEBUG, "SwitchSolver::solve -> findPath %x", (unsigned long)r);
  f = findPath(SolveControl);

  loggerf(DEBUG, "SwitchSolver::solve (%i, %i, %i)", f.possible, f.allreadyCorrect, f.polarityWrong);

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

    f.possible &= setPath(SolveControl);

    changed = true;

    if(f.possible)
      B->recalculate = 1;
  }

  if(!f.possible)
    setWrong(r, tB, link, flags);
  else
    reservePath(SolveControl);

  return changed;
}

struct find findPath(struct SwSolve SwS){
  // Check if switches are set to a good path
  loggerf(DEBUG, "SwitchSolver::findPath (%x, %x, %2i:%2i:%2x, %i)", (unsigned long)SwS.route, (unsigned long)SwS.prevPtr,
                                                                     SwS.link->module, SwS.link->id, (uint8_t)SwS.link->type,
                                                                    SwS.flags);

  struct find f = {0};

  if(!SwS.link->p.p){
    return f;
  }
  else if(SwS.link->type == RAIL_LINK_S){
    Switch * Sw = SwS.link->p.Sw;

    Block * B = Sw->Detection;
    if(B && ((B->reservedBy.size() > 0 && !B->isReservedBy(SwS.train)) || B->blocked)){
      return f;
    }

    PathFinding::instruction * instr = SwS.route ? SwS.route->Sw_S[Sw->uid] : 0;

    RailLink * link[2] = {&Sw->str, &Sw->div};
    // int result[2] = {0, 0};

    memset(&switches[Sw->uid], 0, sizeof(struct switchFind));

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
      SwS.prevPtr = Sw;
      SwS.link = link[j];
      struct find tf = findPath(SwS);

      memcpy(&switches[Sw->uid].f[j], &tf, sizeof(struct find));

      if(instr) instr->possible[i] = tf.possible;
      f.possible |= tf.possible;
      f.polarityWrong |= tf.polarityWrong;
      f.allreadyCorrect |= (tf.allreadyCorrect && (Sw->state == j));

      switches[Sw->uid].f[j].allreadyCorrect = (tf.possible && (Sw->state == j));
    }

    loggerf(INFO, "%2i:%2i (%3i) checked S (str: p:%c, aC:%c, pW:%c) / (div: p:%c, aC:%c, pW:%c)", Sw->module, Sw->id, Sw->uid,
            switches[Sw->uid].f[0].possible ? 'Y':'N', switches[Sw->uid].f[0].allreadyCorrect ? 'Y':'N', switches[Sw->uid].f[0].polarityWrong ? 'Y':'N',
            switches[Sw->uid].f[1].possible ? 'Y':'N', switches[Sw->uid].f[1].allreadyCorrect ? 'Y':'N', switches[Sw->uid].f[1].polarityWrong ? 'Y':'N');


    return f;
  }
  else if(SwS.link->type == RAIL_LINK_s){
    Switch * Sw = SwS.link->p.Sw;

    Block * B = Sw->Detection;
    if(B && ((B->reservedBy.size() > 0 && !B->isReservedBy(SwS.train)) || B->blocked))
      return f;

    loggerf(INFO, "check s %i (state: %i, str.p: %x, div.p: %x, == p: %x)", Sw->id, Sw->state, (unsigned long)Sw->str.p.p, (unsigned long)Sw->div.p.p, SwS.prevPtr);
    void * p = SwS.prevPtr;
    SwS.prevPtr = Sw;
    SwS.link = &Sw->app;
    f = findPath(SwS);
    bool allreadyCorrect = Sw->approachable(p, SwS.flags);
    f.allreadyCorrect &= allreadyCorrect;
    
    memset(&switches[Sw->uid], 0, sizeof(struct switchFind));

    uint8_t correctState = allreadyCorrect ? Sw->state : (Sw->state ^ 1);

    loggerf(INFO, "correct state: %i, Possible: %i, aC: %i, pW: %i", correctState, f.possible, allreadyCorrect, f.polarityWrong);

    switches[Sw->uid].f[correctState].possible = f.possible;
    switches[Sw->uid].f[correctState].allreadyCorrect = allreadyCorrect;
    switches[Sw->uid].f[correctState].polarityWrong = f.polarityWrong;

    loggerf(INFO, "%2i:%2i (%3i) checked s (str: p:%c, aC:%c, pW:%c) / (div: p:%c, aC:%c, pW:%c)", Sw->module, Sw->id, Sw->uid,
            switches[Sw->uid].f[0].possible ? 'Y':'N', switches[Sw->uid].f[0].allreadyCorrect ? 'Y':'N', switches[Sw->uid].f[0].polarityWrong ? 'Y':'N',
            switches[Sw->uid].f[1].possible ? 'Y':'N', switches[Sw->uid].f[1].allreadyCorrect ? 'Y':'N', switches[Sw->uid].f[1].polarityWrong ? 'Y':'N');

    return f;
  }
  else if(SwS.link->type == RAIL_LINK_MA){
    MSSwitch * Sw = SwS.link->p.MSSw;

    Block * B = Sw->Detection;
    if(B && ((B->reservedBy.size() > 0 && !B->isReservedBy(SwS.train)) || B->blocked))
      return f;

    PathFinding::instruction * instr = SwS.route ? SwS.route->MSSw_A[Sw->uid] : 0;
    
    memset(&msswitches[Sw->uid], 0, sizeof(struct msswitchFind));
    void * prevPtr = SwS.prevPtr;

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
      SwS.prevPtr = Sw;
      SwS.link = &Sw->sideB[j];
      struct find tf = findPath(SwS);

      if(Sw->sideA[j].p.p != prevPtr){
        loggerf(INFO, "MSSwitch in wrong state");
        tf.possible = 0;
        tf.allreadyCorrect = 0;
      }

      memcpy(&msswitches[Sw->uid].f[j], &tf, sizeof(struct find));

      if(instr) instr->possible[i] = tf.possible;
      f.possible        |= tf.possible;
      f.allreadyCorrect |= tf.allreadyCorrect && (tf.possible && (Sw->state == j));

      msswitches[Sw->uid].f[j].allreadyCorrect = (tf.possible && (Sw->state == j));
    }

    char debug[4000] = "";
    char * debugPtr = &debug[0];

    debugPtr += sprintf(debugPtr, "%2i:%2i (%3i) checked MSSwA", Sw->module, Sw->id, Sw->uid);
    for(uint8_t i = 0; i < Sw->state_len; i++)
      debugPtr += sprintf(debugPtr, "\n\t(s%i: p:%c, aC:%c, pW:%c)", i, msswitches[Sw->uid].f[i].possible ? 'Y':'N', msswitches[Sw->uid].f[i].allreadyCorrect ? 'Y':'N', msswitches[Sw->uid].f[i].polarityWrong ? 'Y':'N');

    loggerf(INFO, "%s", debug);

    return f;
  }
  else if(SwS.link->type == RAIL_LINK_MB){
    MSSwitch * Sw = SwS.link->p.MSSw;

    Block * B = Sw->Detection;
    if(B && ((B->reservedBy.size() > 0 && !B->isReservedBy(SwS.train)) || B->blocked))
      return f;

    PathFinding::instruction * instr = SwS.route ? SwS.route->MSSw_B[Sw->uid] : 0;

    memset(&msswitches[Sw->uid], 0, sizeof(struct msswitchFind));
    void * prevPtr = SwS.prevPtr;

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
      SwS.prevPtr = Sw;
      SwS.link = &Sw->sideA[j];
      struct find tf = findPath(SwS);
      
      if(Sw->sideB[j].p.p != prevPtr){
        loggerf(INFO, "MSSwitch in wrong state");
        tf.possible = 0;
        tf.allreadyCorrect = 0;
      }

      memcpy(&msswitches[Sw->uid].f[j], &tf, sizeof(struct find));

      if(instr) instr->possible[i] = tf.possible;
      f.possible        |= tf.possible;
      f.allreadyCorrect |= tf.allreadyCorrect && (tf.possible && (Sw->state == j));

      msswitches[Sw->uid].f[j].allreadyCorrect = (tf.possible && (Sw->state == j));
    }

    char debug[4000] = "";
    char * debugPtr = &debug[0];

    debugPtr += sprintf(debugPtr, "%2i:%2i (%3i) checked MSSwB", Sw->module, Sw->id, Sw->uid);
    for(uint8_t i = 0; i < Sw->state_len; i++)
      debugPtr += sprintf(debugPtr, "\n\t(s%i: p:%c, aC:%c, pW:%c)", i, msswitches[Sw->uid].f[i].possible ? 'Y':'N', msswitches[Sw->uid].f[i].allreadyCorrect ? 'Y':'N', msswitches[Sw->uid].f[i].polarityWrong ? 'Y':'N');

    loggerf(INFO, "%s", debug);


    return f;
  }

  else if (SwS.link->type == RAIL_LINK_R){

    Block * B = SwS.link->p.B;

    loggerf(INFO, "check B %i", B->id);

    if(!B->checkPolarity(SwS.prevBlock) || SwS.flags & FL_WRONGPOLARITY){
      loggerf(INFO, "Polarity Compare  %2i:%2i <> %2i:%2i => %i", SwS.prevBlock->module, SwS.prevBlock->id, B->module, B->id, B->checkPolarity(SwS.prevBlock));
      SwS.flags |= FL_WRONGPOLARITY;
      f.polarityWrong = 1;
    }

    // Block is part of station that is blocked by a train stopped on it. Not a solution
    if(B->type == STATION && B->station && (B->station->stoppedTrain || 
                                           (B->station->parent && B->station->parent->stoppedTrain)) ){
      loggerf(INFO, "FOUND BLOCKED STATION %i (%s)", B->station->id, B->station->name);
      return f;
    }

    // If Block is the exit of a path
    else if(B->path && B != B->path->Entrance && B->path->reserved){
      loggerf(INFO, "FOUND WRONG PATH DIRECTION");
      return f;
    }

    // Train can stop on the block, so a solution
    else if(B->type != NOSTOP){
      if(SwS.flags & FL_CONTINUEDANGER && B->state <= DANGER){
        return f;
      }
      if(B->isReservedBy(SwS.train)){
        if(B->path->Entrance == B){
          loggerf(INFO, "A");
          f.possible = 1;
          f.allreadyCorrect = 1;
        }
      }
      else if(B->reservedBy.size() == 0){
        loggerf(INFO, "B");
        f.possible = 1;
        f.allreadyCorrect = 1;

        Path * P = B->path;

        if(P->Entrance != B && SwS.train->length > P->maxLength){
          loggerf(INFO, "Train Does not fit");
          // Train cannot fit in reverseable section
          f.possible = 0;
          f.allreadyCorrect = 0;
        }
      }

      return f; 
    }

    SwS.prevBlock = B;
    if(B->next.p.p == SwS.prevPtr){
      SwS.prevPtr = B;
      SwS.link = &B->prev;
      return findPath(SwS);
    }
    else if(B->prev.p.p == SwS.prevPtr){
      SwS.prevPtr = B;
      SwS.link = &B->next;
      return findPath(SwS);
    }
  }

  // loggerf(ERROR, "Done checking");
  return f;
}


int setPath(struct SwSolve SwS){
  loggerf(INFO, "setPath (%x, %x, %2i:%2i %2x, %i)", (unsigned long)SwS.route, (unsigned long)SwS.prevPtr,
                                                      SwS.link->module, SwS.link->id, SwS.link->type,
                                                      SwS.flags);
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


  if(SwS.link->type == RAIL_LINK_S || SwS.link->type == RAIL_LINK_s){
    // Go to next switch
    Switch * Sw = SwS.link->p.Sw;

    Block * B = Sw->Detection;
    if(B && ((B->reservedBy.size() > 0 && !B->isReservedBy(SwS.train)) || B->blocked)) {
      // loggerf(INFO, "Switch reserved");
      return 0;
    }

    struct find * swF = switches[Sw->uid].f;
    bool str = false, div = false;

    str = swF[0].allreadyCorrect;
    div = swF[1].allreadyCorrect;

    if(!str && !div){
      if(swF[0].possible){
        str = true;
      }
      else if(swF[1].possible){
        div = true;
      }
    }

    SwS.prevPtr = Sw;

    // Set switch without override, and without mutexLock
    if(SwS.link->type == RAIL_LINK_S){
      if(str){
        SwS.link = &Sw->str;
        Sw->setState(0, false, false);
        setPath(SwS);
      }
      else if(div){
        SwS.link = &Sw->div;
        Sw->setState(1, false, false);
        setPath(SwS);
      }
      else
        return 0;
    }
    else{
      SwS.link = &Sw->app;
      if(str){
        Sw->setState(0, false, false);
        setPath(SwS);
      }
      else if(div){
        Sw->setState(1, false, false);
        setPath(SwS);
      }
      else
        return 0;
    }

    return 1;
  }
  else if(SwS.link->type == RAIL_LINK_MA || SwS.link->type == RAIL_LINK_MB){
    // loggerf(ERROR, "IMPLEMENT");
    // MSSwitch * N = SwS.link->p.MSSw;
    // if(N->sideB[N->state].p.p == p){
    //   return 1;
    // }
    /////////////////////////

    // TODO/FIXME
    
    // Go to next switch
    MSSwitch * Sw = SwS.link->p.MSSw;

    Block * B = Sw->Detection;
    if(B && ((B->reservedBy.size() > 0 && !B->isReservedBy(SwS.train)) || B->blocked)) {
      // loggerf(INFO, "Switch reserved");
      return 0;
    }

    PathFinding::instruction * instr = 0;
    if(SwS.route)
      instr = (SwS.link->type == RAIL_LINK_MA) ? SwS.route->MSSw_A[Sw->uid] : SwS.route->MSSw_B[Sw->uid];

    void * p = SwS.prevPtr;

    // No Route or no instruction for this switch => default behaviour
    if(!instr){
      // loggerf(INFO, "Switch S %i No Route", Sw->id);
      uint8_t setState = 0xFF;

      struct find * swF = msswitches[Sw->uid].f;
      bool found = false;
      bool correctSearched = false;

      uint8_t i = 0;
      while(!found){
        loggerf(INFO, " %i :: %i, %i, %i", i, swF[i].allreadyCorrect, correctSearched, swF[i].possible);
        if((swF[i].allreadyCorrect || correctSearched) && swF[i].possible){
          found = true;
          setState = i;
        }

        i++;
        if(i >= Sw->state_len){
          i = 0;
          if(correctSearched)
            break;

          correctSearched = true;
        }
      }

      if(found){
        Sw->setState(setState, false);
        return 1;
      }

      return 0;
    }

    bool switchSet = 0;

    RailLink * nextLink = (SwS.link->type == RAIL_LINK_MA) ? Sw->sideB : Sw->sideA;

    for(uint8_t i = 0; i < instr->nrOptions; i++){
      // uint8_t j = instr->options[i]; state
      // loggerf(INFO, "check S %i (%i ? %i)", Sw->id, i, instr->possible[i]);

      if(instr->possible[i] && ((SwS.link->type == RAIL_LINK_MA) ? Sw->approachableA(p, FL_SWITCH_CARE) : Sw->approachableB(p, FL_SWITCH_CARE)) ){
        if(Sw->state != instr->options[i])
          Sw->setState(instr->options[i], false);

        SwS.prevPtr = Sw;
        SwS.link = &nextLink[instr->options[i]];

        switchSet = setPath(SwS);
        break;
      }

    }

    return switchSet;
  }

  else if (SwS.link->type == RAIL_LINK_R){
    Block * B = SwS.link->p.B;
    // loggerf(INFO, "check B %i", B->id);

    /* FIXME
    if(!B->checkPolarity(SwS.prevBlock)){
      loggerf(INFO, "Polarity Compare  %2i:%2i <> %2i:%2i => %i", SwS.prevBlock->module, SwS.prevBlock->id, B->module, B->id, B->checkPolarity(SwS.prevBlock));
      if(B->path)
        B->path->flipPolarity(0);
      else
        B->flipPolarity(0);
    }
    */

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
      return (SwS.flags & FL_CONTINUEDANGER && B->state <= DANGER) ? 0 : 1;
    }

    SwS.prevBlock = B;
    if(B->next.p.p == SwS.prevPtr){
      SwS.prevPtr = B;
      SwS.link = &B->prev;
      return setPath(SwS);
    }
    else if(B->prev.p.p == SwS.prevPtr){
      SwS.prevPtr = B;
      SwS.link = &B->next;
      return setPath(SwS);
    }
  }

  return 0;
}

void setWrong(PathFinding::Route * r, void * p, RailLink link, int flags){
  loggerf(TRACE, "SwitchSolver::setWrong (%x, %x, %i)", (unsigned long)p, (unsigned long)&link, flags);

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

void dereservePath(Train * T, PathFinding::Route * r, void * p, RailLink link, int flags){
  loggerf(TRACE, "DEreservePath (%x, %x, %x, %x, %i)", (unsigned long)T, (unsigned long)r, (unsigned long)p, (unsigned long)&link, flags);

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

int reservePath(struct SwSolve SwS){
  loggerf(DEBUG, "reservePath (%x, %x, %2i:%2i:%2x, %i)", (unsigned long)SwS.route, (unsigned long)SwS.prevPtr,
                                                          SwS.link->module, SwS.link->id, (uint8_t)SwS.link->type,
                                                          SwS.flags);

  if(SwS.link->type == RAIL_LINK_S || SwS.link->type == RAIL_LINK_s){
    // Go to next switch
    Switch * Sw = SwS.link->p.Sw;
    Block * DB = Sw->Detection;

    if(dircmp(DB, SwS.prevBlock) != DB->cmpPolarity(SwS.prevBlock)){
      loggerf(DEBUG, "Block reversing %2i:%2i<>%2i:%2i, %i, %i", DB->module, DB->id,
                                                                 SwS.prevBlock->module, SwS.prevBlock->id,
                                                                 dircmp(DB, SwS.prevBlock), DB->cmpPolarity(SwS.prevBlock));
      DB->reverse();
    }
    SwS.prevBlock = DB;

    if(!DB->isReservedBy(SwS.train))
      SwS.train->reserveBlock(DB);

    DB->switchWrongState = 0;

    loggerf(TRACE, "Set switch %02i:%02i to RESERVED", Sw->module, Sw->id);

    SwS.prevPtr = Sw;
    if(SwS.link->type == RAIL_LINK_S)
      SwS.link = Sw->state ? &Sw->div : &Sw->str;
    else
      SwS.link = &Sw->app;
    
    return reservePath(SwS);
  }
  else if(SwS.link->type == RAIL_LINK_MA || SwS.link->type == RAIL_LINK_MB){
    
    // Go to next switch
    MSSwitch * Sw = SwS.link->p.MSSw;
    Block * DB = Sw->Detection;

    if(dircmp(DB, SwS.prevBlock) != DB->cmpPolarity(SwS.prevBlock)){
      loggerf(DEBUG, "Block reversing %2i:%2i<>%2i:%2i, %i, %i", DB->module, DB->id,
                                                                 SwS.prevBlock->module, SwS.prevBlock->id,
                                                                 dircmp(DB, SwS.prevBlock), DB->cmpPolarity(SwS.prevBlock));
      
      DB->reverse();
    }
    SwS.prevBlock = DB;

    if(!DB->isReservedBy(SwS.train))
      SwS.train->reserveBlock(DB);

    DB->switchWrongState = 0;

    loggerf(TRACE, "Set switch %02i:%02i to RESERVED", Sw->module, Sw->id);

    SwS.prevPtr = Sw;
    if(SwS.link->type == RAIL_LINK_MA)
      SwS.link = &Sw->sideB[Sw->state];
    else // RAIL_LINK_MB
      SwS.link = &Sw->sideA[Sw->state];

    return reservePath(SwS);
  }

  else if (SwS.link->type == RAIL_LINK_R){
    Block * B = SwS.link->p.B;
    loggerf(TRACE, "check B %i", B->id);
    if(B->type != NOSTOP){
      if(!B->reserved){
        loggerf(DEBUG, "Path Entrance: %02i:%02i", B->path->Entrance->module, B->path->Entrance->id);
        if(B->path->Entrance != B)
          B->path->reverse();

        B->path->reserve(SwS.train);
      }
      return 1; // Train can stop on the block, so a possible path
    }

    if(dircmp(B, SwS.prevBlock) != B->cmpPolarity(SwS.prevBlock))
      B->reverse();

    SwS.prevBlock = B;
    if(B->next.p.p == SwS.prevPtr){
      SwS.prevPtr = B;
      SwS.link = &B->prev;
      return reservePath(SwS);
    }
    else if(B->prev.p.p == SwS.prevPtr){
      SwS.prevPtr = B;
      SwS.link = &B->next;
      return reservePath(SwS);
    }
  }

  return 0;
}

};
