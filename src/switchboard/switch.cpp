#include "switchboard/manager.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/signals.h"
#include "switchboard/station.h"

#include "mem.h"
#include "logger.h"
#include "IO.h"
#include "modules.h"
#include "system.h"
#include "algorithm.h"

using namespace switchboard;

Switch::Switch(uint8_t Module, struct switch_conf s){
  // struct switch_conf s = read_s_switch_conf(buf_ptr);
  struct s_switch_connect connect;

  connect.module = Module;
  connect.id = s.id;
  connect.app.module = s.App.module; connect.app.id = s.App.id; connect.app.type = (enum link_types)s.App.type;
  connect.str.module = s.Str.module; connect.str.id = s.Str.id; connect.str.type = (enum link_types)s.Str.type;
  connect.div.module = s.Div.module; connect.div.id = s.Div.id; connect.div.type = (enum link_types)s.Div.type;

  // new Switch(connect, s.det_block, s.IO & 0x0f, Adrs, States);
  loggerf(MEMORY, "Create Sw %i:%i", connect.module, connect.id);
  // Switch * Z = (Switch *)_calloc(1, Switch);
  memset(this, 0, sizeof(Switch));

  this->module = connect.module;
  this->id = connect.id;

  uid = SwManager->addSwitch(this);

  this->div = connect.div;
  this->str = connect.str;
  this->app = connect.app;

  this->IO_len = s.IO & 0x0F;
  this->IO = (IO_Port **)_calloc(this->IO_len, IO_Port *);

  U = Units(connect.module);


  // ============== IO ==============
  Node_adr * Adrs = (Node_adr *)_calloc(this->IO_len, Node_adr);

  for(int i = 0; i < (this->IO_len); i++){
    Adrs[i].Node = s.IO_Ports[i].Node;
    Adrs[i].io = s.IO_Ports[i].Adr;
  }
  for(int i = 0; i < this->IO_len; i++){
    if(!U->IO(s.IO_Ports[i]))
      continue;

    this->IO[i] = U->linkIO(Adrs[i], this, IO_Output);
  }

  _free(Adrs);

  uint8_t * States = (uint8_t *)_calloc(2, uint8_t);
  States[0] = 1 + (0 << 1); //State 0 - Address 0 high, address 1 low
  States[1] = 0 + (1 << 1); //State 1 - Address 1 high, address 0 low

  this->IO_states = (uint8_t *)_calloc(this->IO_len, uint8_t);
  memcpy(this->IO_states, States, this->IO_len * sizeof(uint8_t));
  _free(States);

  // =========== Detection ============

  U->insertSwitch(this);

  if(U->block_len > s.det_block && U->B[s.det_block]){
    this->Detection = U->B[s.det_block];
    this->Detection->addSwitch(this);
  }
  else{
    loggerf(WARNING, "SWITCH %i:%i has no detection block %i", connect.module, connect.id, s.det_block);
  }
}

// Switch::Switch(struct s_switch_connect connect, uint8_t block_id, uint8_t output_len, Node_adr * output_pins, uint8_t * output_states){
//   loggerf(MEMORY, "Create Sw %i:%i", connect.module, connect.id);
//   // Switch * Z = (Switch *)_calloc(1, Switch);
//   memset(this, 0, sizeof(Switch));

//   this->module = connect.module;
//   this->id = connect.id;

//   this->div = connect.div;
//   this->str = connect.str;
//   this->app = connect.app;

//   this->IO = (IO_Port **)_calloc(output_len, IO_Port *);

//   for(int i = 0; i < output_len; i++){
//     Init_IO(Units[connect.module], output_pins[i], this);

//     this->IO[i] = Units[connect.module]->Node[output_pins[i].Node].io[output_pins[i].io];
//   }

//   this->IO_len = output_len;
//   this->IO_states = output_states;

//   Units[this->module]->insertSwitch(this);

//   if(Units[this->module]->block_len > block_id && U_B(this->module, block_id)){
//     this->Detection = U_B(this->module, block_id);
//     this->Detection->addSwitch(this);
//   }
//   else{
//     loggerf(WARNING, "SWITCH %i:%i has no detection block %i", connect.module, connect.id, block_id);
//   }
// }

Switch::~Switch(){
  loggerf(MEMORY, "Switch %i:%i Destructor", module, id);
  _free(this->feedback);
  _free(this->IO);
  _free(this->IO_states);
  _free(this->coupled);
  _free(this->preferences);
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
    return next->p.B->_Next(flags, level);
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

uint Switch::NextList_Block(Block ** blocks, uint8_t block_counter, enum link_types type, int flags, int length){
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

  // printf("N%cL%i\t",next.type,level);

  if(next->type == RAIL_LINK_E || next->type == RAIL_LINK_C){
    return block_counter;
  }

  if(!next->p.p){
    loggerf(ERROR, "NO POINTERS");
    return block_counter;
  }

  if(next->type == RAIL_LINK_R){
    return next->p.B->_NextList(blocks, block_counter, flags, length);
  }
  else if(next->type == RAIL_LINK_S){
    return next->p.Sw->NextList_Block(blocks, block_counter, next->type, flags, length);
  }
  else if(next->type == RAIL_LINK_s && next->p.Sw->approachable(this, flags)){
    return next->p.Sw->NextList_Block(blocks, block_counter, next->type, flags, length);
  }
  else if(next->type == RAIL_LINK_MA && next->p.MSSw->approachableA(this, flags)){
    return next->p.MSSw->NextList_Block(blocks, block_counter, next->type, flags, length);
  }
  else if(next->type == RAIL_LINK_MB && next->p.MSSw->approachableB(this, flags)){
    return next->p.MSSw->NextList_Block(blocks, block_counter, next->type, flags, length);
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
    return next->p.MSSw->NextList_Block(blocks, block_counter, next->type, flags, length);
  }
  // else if(next->type == RAIL_LINK_E){
  //   return 0;
  // }
  // printf("RET END\n");
  return block_counter;
}

void Switch::setState(uint8_t _state){
  setState(_state, 1);
}

void Switch::setState(uint8_t state, uint8_t lock){
  loggerf(TRACE, "Switch::setState (%x, %i, %i)", (unsigned int)this, state, lock);

  if(Detection && (Detection->state == BLOCKED || Detection->state == RESERVED_SWITCH))
    return; // Switch is blocked

  Algor_Set_Changed(&Detection->Alg);
  putList_AlgorQueue(Detection->Alg, 0);

  updateState(state);

  Detection->AlgorSearch(0);

  Algor_Set_Changed(&Detection->Alg);

  Detection->algorchanged = 0; // Block is allready search should not be researched
  
  putList_AlgorQueue(Detection->Alg, 0);

  putAlgorQueue(Detection, lock);
}

void Switch::updateState(uint8_t _state){
  state = _state;
  updatedState = true;

  for(auto Sig: Signals){
    Sig->switchUpdate();
  }

  U->switch_state_changed |= 1;
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

int Switch_Set_Free_Path(void * p, struct rail_link link, int flags){
  loggerf(DEBUG, "Switch_Set_Path (%x, %x, %i)", (unsigned int)p, (unsigned int)&link, flags);
  if((flags & SWITCH_CARE) == 0){
    return 1;
  }

  //Check if switch is occupied
  if (link.type == RAIL_LINK_S || link.type == RAIL_LINK_s) {
    if((link.p.Sw)->Detection && (link.p.Sw)->Detection->state == RESERVED_SWITCH){
      loggerf(ERROR, "Switch allready Reserved");
      return 0;
    }
  }
  else if (link.type >= RAIL_LINK_MA && link.type <= RAIL_LINK_MB_inside) {
    if((link.p.MSSw)->Detection && (link.p.Sw)->Detection->state == RESERVED_SWITCH){
      loggerf(ERROR, "Switch allready Reserved");
      return 0;
    }
  }


  if(link.type == RAIL_LINK_S){
    // Go to next switch
    Switch * Sw = link.p.Sw;
    loggerf(INFO, "Switch S %i", Sw->id);
    bool str, div;
    str = Switch_Set_Free_Path(Sw, Sw->str, flags);
    div = Switch_Set_Free_Path(Sw, Sw->div, flags);

    loggerf(INFO, "SwitchSetFreePath: str: %i, div: %i", str, div);

    if((Sw->state == 0 && str) || (Sw->state == 1 && div))
      return 1;

    if(str)
      Sw->setState(0);
    else if(div)
      Sw->setState(1);
    else
      return 0;

    return 1;
  }
  else if(link.type == RAIL_LINK_s){
    // Check if switch is in correct state
    // and continue to next switch
    Switch * Sw = link.p.Sw;
    loggerf(INFO, "Switch s %i", Sw->id);
    bool path = Switch_Set_Free_Path(Sw, Sw->app, flags);

    if(!path)
      return path;

    loggerf(INFO, "set s %i (state: %i, str.p: %x, div.p: %x)", Sw->id, Sw->state, (unsigned int)Sw->str.p.p, (unsigned int)Sw->div.p.p);
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
    loggerf(INFO, "check B %i", B->id);

    // Block is part of station that is blocked by a train stopped on it. Not a solution
    if(B->type == STATION && B->station && (B->station->stoppedTrain || 
                                            (B->station->parent && B->station->parent->stoppedTrain)) ){
      loggerf(INFO, "STATION FAIL");
      return 0;
    }

    // If Block is the exit of a path
    else if(B->path && B != B->path->Entrance && B->path->reserved){
      loggerf(INFO, "PATH FAIL");
      return 0;
    }

    // Train can stop on the block, so a solution
    else if(B->type != NOSTOP){
      loggerf(INFO, "NOSTOP SUCCESS");
      return 1; 
    }

    if(B->next.p.p == p)
      return Switch_Set_Free_Path(B, B->prev, flags);
    else if(B->prev.p.p == p)
      return Switch_Set_Free_Path(B, B->next, flags);
  }

  return 0;
}

int Switch_Reserve_Path(RailTrain * T, void * p, struct rail_link link, int flags){
  loggerf(TRACE, "reserve_switch_path (%x, %x, %x, %i)", (unsigned int)T, (unsigned int)p, (unsigned int)&link, flags);
  if((flags & SWITCH_CARE) == 0){
    return 1;
  }


  if(link.type == RAIL_LINK_S){
    // Go to next switch
    Switch * Sw = link.p.Sw;
    Block * DB = Sw->Detection;

    DB->state = RESERVED_SWITCH;
    if(DB->reservedBy != T)
      T->reserveBlock(DB);
    DB->statechanged = 1;
    Units(DB->module)->block_state_changed = 1;
    loggerf(TRACE, "Set switch %02i:%02i to RESERVED", Sw->module, Sw->id);

    if(Sw->state == 0)
      return Switch_Reserve_Path(T, Sw, Sw->str, flags);
    else if(Sw->state == 1)
      return Switch_Reserve_Path(T, Sw, Sw->div, flags);
  }
  else if(link.type == RAIL_LINK_s){
    // Check if switch is in correct state
    // and continue to next switch
    Switch * Sw = link.p.Sw;
    Block * DB = Sw->Detection;

    DB->state = RESERVED_SWITCH;
    if(DB->reservedBy != T)
      T->reserveBlock(DB);
    DB->statechanged = 1;
    Units(DB->module)->block_state_changed = 1;
    loggerf(TRACE, "Set switch %02i:%02i to RESERVED", Sw->module, Sw->id);

    return Switch_Reserve_Path(T, Sw, Sw->app, flags);
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
    if(B->type != NOSTOP)
      return 1; // Train can stop on the block, so a possible path

    if(B->next.p.p == p)
      return Switch_Reserve_Path(T, B, B->prev, flags);
    else if(B->prev.p.p == p)
      return Switch_Reserve_Path(T, B, B->next, flags);
  }
  else if(link.type == 'D'){
    return 1;
  }
  return 0;
}

int Switch_Check_Path(void * p, struct rail_link link, int flags){
  // Check if switches are set to a good path

  loggerf(INFO, "Switch_Check_Path (%x, %x, %i)", (unsigned int)p, (unsigned int)&link, flags);
  if(!link.p.p){
    loggerf(ERROR, "Empty LINK {%i:%i - %i}", link.module, link.id, link.type);
    return 0;
  }

  if((flags & SWITCH_CARE) == 0){
    return 1;
  }
  else if(link.type == RAIL_LINK_S){
    Switch * Sw = link.p.Sw;
    loggerf(INFO, "check S %i (state: %i)", Sw->id, Sw->state);
    if(Sw->state == 0){
      return Switch_Check_Path(Sw, Sw->str, flags);
    }
    else if(Sw->state == 1){
      return Switch_Check_Path(Sw, Sw->div, flags);
    }
  }
  else if(link.type == RAIL_LINK_s){
    Switch * N = link.p.Sw;
    loggerf(INFO, "check s %i (state: %i, str.p: %x, div.p: %x)", N->id, N->state, (unsigned int)N->str.p.p, (unsigned int)N->div.p.p);
    if(N->state == 0 && N->str.p.p == p){
      return Switch_Check_Path(N, N->app, flags);
    }
    else if(N->state == 1 && N->div.p.p == p){
      return Switch_Check_Path(N, N->app, flags);
    }
    loggerf(ERROR, "wrong State %x", N->state);
  }
  else if(link.type == RAIL_LINK_MA){
    loggerf(WARNING, "IMPLEMENT");
    MSSwitch * N = link.p.MSSw;
    if(N->sideB[N->state].p.p == p){
      return 1;
    }
  }
  else if(link.type == RAIL_LINK_MB){
    loggerf(WARNING, "IMPLEMENT");
    MSSwitch * N = link.p.MSSw;
    if(N->sideA[N->state].p.p == p){
      return 1;
    }
  }

  else if (link.type == RAIL_LINK_R){

    Block * B = link.p.B;

    loggerf(INFO, "check B %i", B->id);

    // Block is part of station that is blocked by a train stopped on it. Not a solution
    if(B->type == STATION && B->station && (B->station->stoppedTrain || 
                                            (B->station->parent && B->station->parent->stoppedTrain)) )
      return 0;

    // If Block is the exit of a path
    else if(B->path && B != B->path->Entrance && B->path->reserved)
      return 0;

    // Train can stop on the block, so a solution
    else if(B->type != NOSTOP)
      return 1; 


    if(B->next.p.p == p)
      return Switch_Check_Path(B, B->prev, flags);
    else if(B->prev.p.p == p)
      return Switch_Check_Path(B, B->next, flags);
  }
  else if(link.type == 'D'){
    return 1;
  }
  loggerf(ERROR, "Done checking");
  return 0;
}

