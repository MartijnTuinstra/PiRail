#include "switchboard/manager.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/signals.h"
#include "switchboard/station.h"

#include "config/LayoutStructure.h"

#include "utils/mem.h"
#include "utils/logger.h"
#include "IO.h"
#include "system.h"

#include "algorithm/core.h"
#include "algorithm/queue.h"

using namespace switchboard;

Switch::Switch(uint8_t Module, struct configStruct_Switch * s){
  // struct switch_conf s = read_s_switch_conf(buf_ptr);
  struct s_switch_connect connect;

  connect.module = Module;
  connect.id = s->id;
  connect.app.module = s->App.module; connect.app.id = s->App.id; connect.app.type = (enum link_types)s->App.type;
  connect.str.module = s->Str.module; connect.str.id = s->Str.id; connect.str.type = (enum link_types)s->Str.type;
  connect.div.module = s->Div.module; connect.div.id = s->Div.id; connect.div.type = (enum link_types)s->Div.type;

  // new Switch(connect, s->det_block, s->IO & 0x0f, Adrs, States);
  loggerf(TRACE, "Create Sw %i:%i", connect.module, connect.id);
  // Switch * Z = (Switch *)_calloc(1, Switch);
  memset(this, 0, sizeof(Switch));

  module = connect.module;
  id = connect.id;
  uid = SwManager->addSwitch(this);

  div = connect.div;
  str = connect.str;
  app = connect.app;

  MaxSpeed[0] = s->speed_Str;
  MaxSpeed[1] = s->speed_Div;

  U = Units(connect.module);
  U->insertSwitch(this);

  IO_len = s->IO_length;
  IO = (IO_Port **)_calloc(IO_len, IO_Port *);

  feedback_len = s->feedback_len;


  // ============== IO ==============
  for(int i = 0; i < this->IO_len; i++){
    this->IO[i] = U->linkIO(s->IO_Ports[i], this, IO_Output);
  }

  // IO Stating
  auto arrayIO = (union u_IO_event *)_calloc(2 * IO_len + 10, union u_IO_event);

  IO_events[0] = &arrayIO[0];
  IO_events[1] = &arrayIO[IO_len];

  for(uint8_t i = 0; i < (IO_len * 2); i++){
    IO_events[i % 2][i / 2].value = s->IO_Event[i];
    loggerf(INFO, "SWITCH IO event: %i:%i -> %i", i%2, i/2, IO_events[i % 2][i / 2].value);
  }

  // ============== Feedback ==============
  feedback_en = (s->feedback_len > 0);

  if(!feedback_en)
    feedback = 0;
  else {
    feedback = (IO_Port **)_calloc(feedback_len, IO_Port *);

    for(int i = 0; i < feedback_len; i++){
      if(!U->IO(s->FB_Ports[i]))
        continue;

      this->feedback[i] = U->linkIO(s->FB_Ports[i], this, IO_Input_Switch);
    }

    // Feedback IO Stating
    auto arrayFB = (union u_IO_event *)_calloc(2 * feedback_len + 10, union u_IO_event);

    feedback_events[0] = &arrayFB[0];
    feedback_events[1] = &arrayFB[feedback_len];

    for(uint8_t i = 0; i < (feedback_len * 2); i++){
      feedback_events[i % 2][i / 2].value = s->FB_Event[i];
      loggerf(INFO, "SWITCH IO event: %i:%i -> %i", i%2, i/2, feedback_events[i % 2][i / 2].value);
    }
  }

  // =========== Detection ============
  Detection = U->registerDetection(this, s->det_block);
}

Switch::~Switch(){
  loggerf(MEMORY, "Switch %i:%i Destructor", module, id);
  _free(IO);
  _free(IO_events[0]);

  // if(feedback_en){
  _free(feedback);
  _free(feedback_events[0]);
  // }

  _free(coupled);
  _free(preferences);
}

void Switch::exportConfig(struct configStruct_Switch * cfg){
  if (!this)
    return;

  cfg->id = id;
  cfg->det_block = Detection->id;

  railLinkExport(&cfg->App, app);
  railLinkExport(&cfg->Div, div);
  railLinkExport(&cfg->Str, str);

  cfg->IO_length = IO_len;
  cfg->IO_type = 0;
  cfg->speed_Str = MaxSpeed[0];
  cfg->speed_Div = MaxSpeed[1];
  cfg->feedback_len = feedback_len;


  if (IO){
    cfg->IO_Ports = (struct configStruct_IOport *)_calloc(IO_len, struct configStruct_IOport);
    cfg->IO_Event = (uint8_t *)_calloc(IO_len * 2, uint8_t);
    for(uint8_t i = 0; i < IO_len; i++){
      if (IO[i])
        IO[i]->exportConfig(&cfg->IO_Ports[i]);

      cfg->IO_Event[i * 2]     = IO_events[i][0].value;
      cfg->IO_Event[i * 2 + 1] = IO_events[i][1].value;
    }
  }

  if (feedback){
    cfg->FB_Ports = (struct configStruct_IOport *)_calloc(IO_len, struct configStruct_IOport);
    cfg->FB_Event = (uint8_t *)_calloc(IO_len, uint8_t);

    for(uint8_t i = 0; i < feedback_len; i++){
      if (feedback[i])
        feedback[i]->exportConfig(&cfg->FB_Ports[i]);

      cfg->FB_Event[i * 2]     = feedback_events[i][0].value;
      cfg->FB_Event[i * 2 + 1] = feedback_events[i][1].value;
    }
  }
}

void Switch::addSignal(Signal * Sig){
  loggerf(DEBUG, "AddSignal %i to switch %i", Sig->id, this->id);
  this->Signals.push_back(Sig);
}

bool Switch::approachable(void * p, int flags){
  // Check if the switch is approachable from the div/str side.

  loggerf(TRACE, "Switch::approachable (%x, %x, %x)", (unsigned int)this, (unsigned int)p, flags);
  if((flags & SWITCH_CARE) == 0){
    //No SWITCH_CARE
    return 1;
  }

  loggerf(TRACE, "check s (state: %i, str.p: %x, div.p: %x)", this->state, (unsigned int)this->str.p.p, (unsigned int)this->div.p.p);
  if((this->state == 0 && this->str.p.p == p) || (this->state == 1 && this->div.p.p == p)){
    return 1;
  }

  return 0;
}

Block * Switch::Next_Block(enum link_types type, int flags, int level){
  struct rail_link * next;

  loggerf(TRACE, "Next_Switch_Block %i:%i\t%i", this->module, this->id, level);

  if(type == RAIL_LINK_s){
    next = &this->app;
  }
  else{
    if(this->state == 0){
      next = &this->str;
    }
    else{
      next = &this->div;
    }
  }

  // printf("N%cL%i\t",next.type,level);

  if(next->type == RAIL_LINK_E || next->type == RAIL_LINK_C){
    return 0;
  }

  if(!next->p.p){
    loggerf(ERROR, "NO POINTERS");
    return 0;
  }

  if(next->type == RAIL_LINK_R){
    return next->p.B->Next_Block(flags, level);
  }
  else if(next->type == RAIL_LINK_S){
    return next->p.Sw->Next_Block(next->type, flags, level);
  }
  else if(next->type == RAIL_LINK_s && next->p.Sw->approachable(this, flags)){
    return next->p.Sw->Next_Block(next->type, flags, level);
  }
  else if(next->type == RAIL_LINK_MA && next->p.MSSw->approachableA(this, flags)){
    return next->p.MSSw->Next_Block(next->type, flags, level);
  }
  else if(next->type == RAIL_LINK_MB && next->p.MSSw->approachableB(this, flags)){
    return next->p.MSSw->Next_Block(next->type, flags, level);
  }
  else if(next->type == RAIL_LINK_TT){
    if(next->p.MSSw->approachableA(this, flags)){
      next->type = RAIL_LINK_MA;
    }
    else if(next->p.MSSw->approachableB(this, flags)){
      next->type = RAIL_LINK_MB;
    }
    return next->p.MSSw->Next_Block(next->type, flags, level);
  }
  else if(next->type == RAIL_LINK_E){
    return 0;
  }
  // printf("RET END\n");
  return 0;
}

uint Switch::NextList_Block(Block * Origin, Block ** blocks, uint8_t block_counter, enum link_types type, int flags, int length){
  loggerf(TRACE, "Switch::NextList_Block(%02i:%02i, %i, %2x)", this->module, this->id, block_counter, flags);
  struct rail_link * next;

  if(type == RAIL_LINK_s){
    next = &this->app;
  }
  else{
    if(this->state == 0){
      next = &this->str;
    }
    else{
      next = &this->div;
    }
  }


  if(Detection == Origin){
    uint16_t speed = MaxSpeed[state];
    if (speed != 0 && Origin->MaxSpeed > speed)
      Origin->MaxSpeed = speed;
  }


  // printf("N%cL%i\t",next.type,level);

  if(next->type == RAIL_LINK_E || next->type == RAIL_LINK_C){
    return block_counter;
  }

  if(!next->p.p){
    loggerf(ERROR, "NO POINTERS");
    return block_counter;
  }

  if(next->type == RAIL_LINK_R){
    return next->p.B->_NextList(Origin, blocks, block_counter, flags, length);
  }
  else if(next->type == RAIL_LINK_S){
    return next->p.Sw->NextList_Block(Origin, blocks, block_counter, next->type, flags, length);
  }
  else if(next->type == RAIL_LINK_s && next->p.Sw->approachable(this, flags)){

    return next->p.Sw->NextList_Block(Origin, blocks, block_counter, next->type, flags, length);
  }
  else if(next->type == RAIL_LINK_MA && next->p.MSSw->approachableA(this, flags)){
    return next->p.MSSw->NextList_Block(Origin, blocks, block_counter, next->type, flags, length);
  }
  else if(next->type == RAIL_LINK_MB && next->p.MSSw->approachableB(this, flags)){
    return next->p.MSSw->NextList_Block(Origin, blocks, block_counter, next->type, flags, length);
  }
  else if(next->type == RAIL_LINK_TT){
    if(next->p.MSSw->approachableA(this, flags)){
      next->type = RAIL_LINK_MA;

      // If turntable is turned around
      if(next->p.MSSw->NextLink(flags)->p.p != this){
        flags ^= 0b10;
      }
    }
    else if(next->p.MSSw->approachableB(this, flags)){
      next->type = RAIL_LINK_MB;

      // If turntable is turned around
      if(next->p.MSSw->NextLink(flags)->p.p != this){
        flags ^= 0b10;
      }
    }
    return next->p.MSSw->NextList_Block(Origin, blocks, block_counter, next->type, flags, length);
  }
  // else if(next->type == RAIL_LINK_E){
  //   return 0;
  // }
  // printf("RET END\n");
  return block_counter;
}

void Switch::setState(uint8_t _state){
  loggerf(DEBUG, "Switch setState(%i->%i)", state, _state);
  setState(_state, 1);
}

void Switch::setState(uint8_t _state, uint8_t lock){
  loggerf(TRACE, "Switch::setState (%x, %i, %i)", (unsigned int)this, _state, lock);

  if(Detection && (Detection->state == BLOCKED || Detection->reserved || Detection->switchReserved)){
    loggerf(WARNING, "Failed to setState switch %i to state %i", id, _state);
    return; // Switch is blocked
  }

  if(_state == state){
    loggerf(TRACE, "Switch allready in position");
    return;
  }


  Algorithm::Set_Changed(&Detection->Alg);
  AlQueue.puttemp(&Detection->Alg);

  updateState(_state);

  Detection->AlgorSearch(0);

  Algorithm::Set_Changed(&Detection->Alg);

  Detection->algorchanged = 0; // Block is allready search should not be researched
  
  AlQueue.puttemp(&Detection->Alg);
  AlQueue.puttemp(Detection);
  AlQueue.cpytmp();
}

void Switch::updateState(uint8_t _state){
  state = _state;
  updatedState = true;

  // Update IO
  for(uint8_t i = 0; i < IO_len; i++){
    if(!IO[i])
      continue;

    IO[i]->setOutput(IO_events[state][i]);
  }

  if(feedback_en){
    feedbackWrongState = true;
    Detection->checkSwitchFeedback(true);

  }

  // Update linked Signals
  for(auto Sig: Signals){
    Sig->switchUpdate();
  }

  U->switch_state_changed |= 1;
}

void Switch::updateFeedback(){
  for(uint8_t i = 0; i < feedback_len; i++){
    if(feedback_events[state][i].value != feedback[i]->w_state.value)
      return;
  }

  feedbackWrongState = false;

  Detection->checkSwitchFeedback(false);
}

int throw_multiple_switches(uint8_t len, char * msg){
  struct switchdata {
    uint8_t module;
    uint8_t id:7;
    bool type;
    char state;
  };

  // Check if all switches are non-blocked
  for(int i = 0; i < len; i++){
    char * data = (char *)&msg[i*3];

    struct switchdata p;
    p.module = data[0];
    p.id = data[1] & 0x7F;
    p.type = (data[1] & 0x80) >> 7;
    p.state = data[2];

    if(!(Units(p.module) && (
      (p.type == 0 && U_Sw(p.module, p.id)) ||
      (p.type == 1 && U_MSSw(p.module, p.id)) )) ){
      loggerf(ERROR, "Switch doesnt exist");
      return -1;
    }

    if((p.type == 0 && U_Sw(p.module, p.id)->Detection && (U_Sw(p.module, p.id)->Detection->blocked ||
                                                           U_Sw(p.module, p.id)->Detection->state == RESERVED_SWITCH ||
                                                           U_Sw(p.module, p.id)->Detection->state == RESERVED)) ||
       (p.type == 1 && U_MSSw(p.module, p.id)->Detection && (U_MSSw(p.module, p.id)->Detection->blocked ||
                                                             U_MSSw(p.module, p.id)->Detection->state == RESERVED_SWITCH ||
                                                             U_MSSw(p.module, p.id)->Detection->state == RESERVED))){
      loggerf(INFO, "Switch is blocked");
      return -2;
    }
  }

  //Throw all switches
  for(int i = 0; i < len; i++){
    char * data = (char *)&msg[i*3];

    struct switchdata p;
    p.module = data[0];
    p.id = data[1] & 0x7F;
    p.type = (data[1] & 0x80) >> 7;
    p.state = data[2];

    if(p.type == 0){
      U_Sw(p.module, p.id)->setState(p.state, 0);
    }
    else if(p.type == 1){
      U_MSSw(p.module, p.id)->setState(p.state, 0);
    }
  }

  // algor_queue_enable(1);

  // COM_change_switch(0);
  return 1;
}

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
    if(B && B->reservedBy.size() > 0 && !B->isReservedBy(T)){
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

      loggerf(INFO, "SwitchFindPath: %02i:%02i str: (%i, %i), div: (%i, %i)", Sw->module, Sw->id, str.possible, str.allreadyCorrect, div.possible, div.allreadyCorrect);

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
    if(B && B->reservedBy.size() > 0 && !B->isReservedBy(T))
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
    if(B && B->reservedBy.size() > 0 && !B->isReservedBy(T))
      return f;

    if(Sw->sideB[Sw->state].p.p == p){
      return f;
    }
  }
  else if(link.type == RAIL_LINK_MB){
    loggerf(WARNING, "IMPLEMENT");
    MSSwitch * Sw = link.p.MSSw;

    Block * B = Sw->Detection;
    if(B && B->reservedBy.size() > 0 && !B->isReservedBy(T))
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
      return 1; 
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

  if(link.type == RAIL_LINK_S){
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

    if(DB->isReservedBy(T))
      T->dereserveBlock(DB);
    else if(DB->reservedBy.size() > 0){
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

  if(link.type == RAIL_LINK_S){
    // Go to next switch
    Switch * Sw = link.p.Sw;
    Block * DB = Sw->Detection;

    if(!DB->isReservedBy(T))
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

    if(!DB->isReservedBy(T))
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
