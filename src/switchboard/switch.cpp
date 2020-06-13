#include "switchboard/switch.h"
#include "switchboard/msswitch.h"

#include "mem.h"
#include "logger.h"
#include "IO.h"
#include "modules.h"
#include "system.h"
#include "algorithm.h"

// Switch::Switch(uint8_t module, struct s_switch_conf config){

// }

Switch::Switch(struct s_switch_connect connect, uint8_t block_id, uint8_t output_len, Node_adr * output_pins, uint8_t * output_states){
  loggerf(MEMORY, "Create Sw %i:%i", connect.module, connect.id);
  // Switch * Z = (Switch *)_calloc(1, Switch);
  memset(this, 0, sizeof(Switch));

  this->module = connect.module;
  this->id = connect.id;

  this->div = connect.div;
  this->str = connect.str;
  this->app = connect.app;

  this->IO = (IO_Port **)_calloc(output_len, IO_Port *);

  for(int i = 0; i < output_len; i++){
    Init_IO(Units[connect.module], output_pins[i], this);

    this->IO[i] = Units[connect.module]->Node[output_pins[i].Node].io[output_pins[i].io];
  }

  this->IO_len = output_len;
  this->IO_states = output_states;

  Units[this->module]->insertSwitch(this);

  if(Units[this->module]->block_len > block_id && U_B(this->module, block_id)){
    this->Detection = U_B(this->module, block_id);
    this->Detection->addSwitch(this);
  }
  else{
    loggerf(WARNING, "SWITCH %i:%i has no detection block %i", connect.module, connect.id, block_id);
  }
}

Switch::~Switch(){
  loggerf(MEMORY, "Switch %i:%i Destructor", module, id);
  _free(this->feedback);
  _free(this->IO);
  _free(this->IO_states);
  _free(this->coupled);
  _free(this->preferences);
}

bool Switch::approachable(void * p, int flags){
  // Check if the switch is approachable from the div/str side.

  loggerf(TRACE, "Switch::approachable (%x, %x, %x)", (unsigned int)this, (unsigned int)p, flags);
  if((flags & 0x80) == 0){
    //No SWITCH_CARE
    return 1;
  }

  loggerf(TRACE, "check s (state: %i, str.p: %x, div.p: %x)", (this->state & 0x7F), (unsigned int)this->str.p.p, (unsigned int)this->div.p.p);
  if(((this->state & 0x7F) == 0 && this->str.p.p == p) || ((this->state & 0x7F) == 1 && this->div.p.p == p)){
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
    if((this->state & 0x7F) == 0){
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
    //if(this->Detection != next->p){
    //  level--;
    //}
    return next->p.B->_Next(flags, level);
  }
  else if(next->type == RAIL_LINK_S){
    return next->p.Sw->Next_Block(next->type, flags, level);
  }
  else if(next->type == RAIL_LINK_s && next->p.Sw->approachable(this, flags)){
    return next->p.Sw->Next_Block(next->type, flags, level);
  }
  else if(next->type == RAIL_LINK_MA || next->type == RAIL_LINK_MB){
    // TODO
    // MSSwitch * N = next->p.MSSw;
    // if(N->Detection && this->Detection != N->Detection && level == 1){
    //   return N->Detection;
    // }
    // else if(Next_check_Switch(S, next, flags)){
    //   return Next_MSSwitch_Block(N, next->type, flags, level);
    // }
  }
  else if(next->type == RAIL_LINK_E){
    return 0;
  }
  // printf("RET END\n");
  return 0;
}

void Switch::setState(uint8_t state, uint8_t lock){
  loggerf(TRACE, "Switch::setState (%x, %i, %i)", (unsigned int)this, state, lock);

  if(this->Detection && (this->Detection->state == BLOCKED || this->Detection->state == RESERVED_SWITCH))
    return; // Switch is blocked

  Algor_Set_Changed(&this->Detection->Alg);
  putList_AlgorQueue(this->Detection->Alg, 0);

  this->state = (state & 0x0f) | 0x80;

  Units[this->module]->switch_state_changed |= 1;

  this->Detection->AlgorSearch(0);

  Algor_Set_Changed(&this->Detection->Alg);

  this->Detection->algorchanged = 0; // Block is allready search should not be researched
  
  putList_AlgorQueue(this->Detection->Alg, 0);

  putAlgorQueue(this->Detection, lock);
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

    if(!(Units[p.module] && (
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

  algor_queue_enable(1);

  // COM_change_switch(0);
  return 1;
}

int Switch_Set_Path(void * p, struct rail_link link, int flags){
  loggerf(TRACE, "Switch_Set_Path (%x, %x, %i)", (unsigned int)p, (unsigned int)&link, flags);
  if((flags & 0x80) == 0){
    //No SWITCH_CARE
    return 1;
  }

  //Check if switch is occupied
  if (link.type == RAIL_LINK_S || link.type == RAIL_LINK_s) {
    if((link.p.Sw)->Detection && (link.p.Sw)->Detection->state == RESERVED_SWITCH){
      loggerf(ERROR, "Switch allready Reserved");
      return 0;
    }
  }
  else if (link.type >= RAIL_LINK_MA && link.type <= RAIL_LINK_MB) {
    if((link.p.MSSw)->Detection && (link.p.Sw)->Detection->state == RESERVED_SWITCH){
      loggerf(ERROR, "Switch allready Reserved");
      return 0;
    }
  }


  if(link.type == RAIL_LINK_S){
    // Go to next switch
    Switch * Sw = link.p.Sw;
    if((Sw->state & 0x7F) == 0 && Sw->str.type != RAIL_LINK_R && Sw->str.type != 'D'){
      return Switch_Set_Path(Sw, Sw->str, flags);
    }
    else if((Sw->state & 0x7F) == 1 && Sw->div.type != RAIL_LINK_R && Sw->div.type != 'D'){
      return Switch_Set_Path(Sw, Sw->div, flags);
    }
  }
  else if(link.type == RAIL_LINK_s){
    // Check if switch is in correct state
    // and continue to next switch
    Switch * N = link.p.Sw;
    loggerf(TRACE, "set s %i (state: %i, str.p: %x, div.p: %x)", N->id, (N->state & 0x7F), (unsigned int)N->str.p.p, (unsigned int)N->div.p.p);
    if((N->state & 0x7F) == 0){
      if(N->str.p.p != p)
        N->setState(1, 1);

      // if(N->str.p != p)
      //   return 0; // Failed to set switch

      return Switch_Set_Path(N, N->app, flags);
    }
    else if((N->state & 0x7F) == 1){
      if(N->div.p.p != p)
        N->setState(0, 1);

      // if(N->div.p != p)
      //   return 0; // Failed to set switch

      return Switch_Set_Path(N, N->app, flags);
    }
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
    if(N->sideA.p.p == p){
      return 1;
    }
  }

  else if (link.type == RAIL_LINK_R){
    Block * B = link.p.B;
    loggerf(TRACE, "check B %i", B->id);
    if(B->type != NOSTOP)
      return 1; // Train can stop on the block, so a possible path

    if(B->next.p.p == p)
      return Switch_Set_Path(B, B->prev, flags);
    else if(B->prev.p.p == p)
      return Switch_Set_Path(B, B->next, flags);
  }
  else if(link.type == 'D'){
    return 1;
  }
  return 0;
}

int Switch_Reserve_Path(void * p, struct rail_link link, int flags){
  loggerf(TRACE, "reserve_switch_path (%x, %x, %i)", (unsigned int)p, (unsigned int)&link, flags);
  if((flags & 0x80) == 0){
    //No SWITCH_CARE
    return 1;
  }


  if(link.type == RAIL_LINK_S){
    // Go to next switch
    Switch * Sw = link.p.Sw;
    Block * DB = Sw->Detection;

    DB->state = RESERVED_SWITCH;
    DB->reserved = 1;
    DB->statechanged = 1;
    Units[DB->module]->block_state_changed = 1;
    loggerf(TRACE, "Set switch %02i:%02i to RESERVED", Sw->module, Sw->id);

    if((Sw->state & 0x7F) == 0 && Sw->str.type != RAIL_LINK_R && Sw->str.type != 'D'){
      return Switch_Reserve_Path(Sw, Sw->str, flags);
    }
    else if((Sw->state & 0x7F) == 1 && Sw->div.type != RAIL_LINK_R && Sw->div.type != 'D'){
      return Switch_Reserve_Path(Sw, Sw->div, flags);
    }
  }
  else if(link.type == RAIL_LINK_s){
    // Check if switch is in correct state
    // and continue to next switch
    Switch * Sw = link.p.Sw;
    Block * DB = Sw->Detection;

    DB->state = RESERVED_SWITCH;
    DB->reserved = 1;
    DB->statechanged = 1;
    Units[DB->module]->block_state_changed = 1;
    loggerf(TRACE, "Set switch %02i:%02i to RESERVED", Sw->module, Sw->id);

    return Switch_Reserve_Path(Sw, Sw->app, flags);
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
    if(N->sideA.p.p == p){
      return 1;
    }
  }

  else if (link.type == RAIL_LINK_R){
    Block * B = link.p.B;
    loggerf(TRACE, "check B %i", B->id);
    if(B->type != NOSTOP)
      return 1; // Train can stop on the block, so a possible path

    if(B->next.p.p == p)
      return Switch_Set_Path(B, B->prev, flags);
    else if(B->prev.p.p == p)
      return Switch_Set_Path(B, B->next, flags);
  }
  else if(link.type == 'D'){
    return 1;
  }
  return 0;
}

int Switch_Check_Path(void * p, struct rail_link link, int flags){
  // Check if switches are set to a good path

  loggerf(TRACE, "Switch_Check_Path (%x, %x, %i)", (unsigned int)p, (unsigned int)&link, flags);
  if(!link.p.p){
    loggerf(ERROR, "Empty LINK {%i:%i - %i}", link.module, link.id, link.type);
    return 1;
  }

  if((flags & 0x80) == 0){
    //No SWITCH_CARE
    return 1;
  }
  else if(link.type == RAIL_LINK_S){
    Switch * Sw = link.p.Sw;
    loggerf(TRACE, "check S %i (state: %i)", Sw->id, (Sw->state & 0x7F));
    if((Sw->state & 0x7F) == 0){
      return Switch_Check_Path(Sw, Sw->str, flags);
    }
    else if((Sw->state & 0x7F) == 1){
      return Switch_Check_Path(Sw, Sw->div, flags);
    }
  }
  else if(link.type == RAIL_LINK_s){
    Switch * N = link.p.Sw;
    loggerf(TRACE, "check s %i (state: %i, str.p: %x, div.p: %x)", N->id, (N->state & 0x7F), (unsigned int)N->str.p.p, (unsigned int)N->div.p.p);
    if((N->state & 0x7F) == 0 && N->str.p.p == p){
      return Switch_Check_Path(N, N->app, flags);
    }
    else if((N->state & 0x7F) == 1 && N->div.p.p == p){
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
    if(N->sideA.p.p == p){
      return 1;
    }
  }

  else if (link.type == RAIL_LINK_R){

    Block * B = link.p.B;

    loggerf(TRACE, "check B %i", B->id);
    if(B->type != NOSTOP)
      return 1; // Train can stop on the block, so a possible path

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

