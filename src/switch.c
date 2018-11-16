#include "switch.h"
#include "logger.h"
#include "system.h"
#include "mem.h"
#include "module.h"
#include "websocket_msg.h"
#include "websocket.h"
#include "com.h"
#include "IO.h"
#include "algorithm.h"

void Create_Switch(struct switch_connect connect, uint8_t block_id, uint8_t output_len, Node_adr * output_pins, uint8_t * output_states){
  loggerf(TRACE, "Create Sw %i:%i", connect.module, connect.id);
  Switch * Z = _calloc(1, Switch);

  Z->module = connect.module;
  Z->id = connect.id;

  Z->div = connect.div;
  Z->str = connect.str;
  Z->app = connect.app;

  Z->IO = _calloc(output_len, IO_Port *);

  for(int i = 0; i < output_len; i++){
    Init_IO(Units[connect.module], output_pins[i], IO_Output);

    Z->IO[i] = Units[connect.module]->Node[output_pins[i].Node].io[output_pins[i].io];
  }

  Z->IO_len = output_len;
  Z->IO_states = output_states;

  if(Units[Z->module]->B[block_id]){
    Z->Detection = Units[Z->module]->B[block_id];
    if(Units[Z->module]->B[block_id]->switch_len == 0){
      Units[Z->module]->B[block_id]->Sw = _calloc(1, void *);
      Units[Z->module]->B[block_id]->switch_len = 1;
    }
    
    int id = find_free_index(Units[Z->module]->B[block_id]->Sw, Units[Z->module]->B[block_id]->switch_len);
    Units[Z->module]->B[block_id]->Sw[id] = Z;
  }
  else{
    loggerf(ERROR, "SWITCH %i:%i has no detection block %i", connect.module, connect.id, block_id);
  }


  if(Units[connect.module]->Sw[connect.id]){
    loggerf(INFO, "Duplicate switch %i, overwriting ...", connect.id);
    _free(Units[connect.module]->Sw[connect.id]);
  }
  Units[connect.module]->Sw[connect.id] = Z;
}

void Switch_Add_Feedback(Switch * S, char len, Node_adr * pins, char * state){
  //Enable feedback pins
  S->feedback_en = 1;

  for(int i = 0; i < len; i++){
    Init_IO(Units[S->module], pins[i], IO_Input);

    S->feedback[i] = Units[S->module]->Node[pins[i].Node].io[pins[i].io];
  }
  _free(pins);

  S->feedback_len = len;
  S->feedback_states = state;
}

void Create_MSSwitch(struct msswitch_connect connect, uint8_t block_id, uint8_t output_len, Node_adr * output_pins, uint16_t * output_states){
  loggerf(DEBUG, "Create MSSw %i:%i", connect.module, connect.id);
  MSSwitch * Z = _calloc(1, MSSwitch);

  Z->module = connect.module;
  Z->id = connect.id;

  Z->sideA = connect.sideA;
  Z->sideB = connect.sideB;

  Z->IO = _calloc(output_len, IO_Port *);

  for(int i = 0; i < output_len; i++){
    Init_IO(Units[connect.module], output_pins[i], IO_Output);

    Z->IO[i] = Units[connect.module]->Node[output_pins[i].Node].io[output_pins[i].io];
  }
  _free(output_pins);

  Z->IO_len = output_len;
  Z->IO_states = output_states;

  if(U_B(Z->module, block_id)){
    Z->Detection = Units[Z->module]->B[block_id];
    if(U_B(Z->module, block_id)->msswitch_len == 0){
      U_B(Z->module, block_id)->MSSw = _calloc(2, void *);
      U_B(Z->module, block_id)->msswitch_len = 2;
    }
    int id = find_free_index(U_B(Z->module, block_id)->MSSw, U_B(Z->module, block_id)->msswitch_len);
    U_B(Z->module, block_id)->MSSw[id] = Z;
  }
  else
    loggerf(ERROR, "MSSwitch %i:%i has no detection block %i", connect.module, connect.id, block_id);

  if(U_MSSw(connect.module, connect.id)){
    loggerf(INFO, "Duplicate switch, overwriting ...");
    _free(U_MSSw(connect.module, connect.id));
  }
  U_MSSw(connect.module, connect.id) = Z;
}

int check_linked_switches(Switch * S){
  for(int i = 0;i<S->links_len;i++){
    if(!S->links[i].p)
      continue;

    if(S->links[i].type == RAIL_LINK_S || S->links[i].type == RAIL_LINK_s){
      if(((Switch *)S->links[i].p)->Detection &&
         ( ((Switch *)S->links[i].p)->Detection->state == RESERVED ||
           ((Switch *)S->links[i].p)->Detection->state == RESERVED_SWITCH ||
           ((Switch *)S->links[i].p)->Detection->blocked )){
        return 0;
      }
    }else if(S->links[i].type == RAIL_LINK_M || S->links[i].type == RAIL_LINK_m){
      if(((MSSwitch *)S->links[i].p)->Detection &&
         ( ((MSSwitch *)S->links[i].p)->Detection->state == RESERVED ||
           ((MSSwitch *)S->links[i].p)->Detection->state == RESERVED_SWITCH ||
           ((MSSwitch *)S->links[i].p)->Detection->blocked )){
        return 0;
      }
    }
  }
  return 1;
}

int check_linked_msswitches(MSSwitch * S){
  for(int i = 0;i<S->links_len;i++){
    if(!S->links[i].p)
      continue;

    if(S->links[i].type == RAIL_LINK_S || S->links[i].type == RAIL_LINK_s){
      if(((Switch *)S->links[i].p)->Detection &&
         ( ((Switch *)S->links[i].p)->Detection->state == RESERVED ||
           ((Switch *)S->links[i].p)->Detection->state == RESERVED_SWITCH ||
           ((Switch *)S->links[i].p)->Detection->blocked )){
        return 0;
      }
    }else if(S->links[i].type == RAIL_LINK_M || S->links[i].type == RAIL_LINK_m){
      if(((MSSwitch *)S->links[i].p)->Detection &&
         ( ((MSSwitch *)S->links[i].p)->Detection->state == RESERVED ||
           ((MSSwitch *)S->links[i].p)->Detection->state == RESERVED_SWITCH ||
           ((MSSwitch *)S->links[i].p)->Detection->blocked )){
        return 0;
      }
    }
  }
  return 1;
}

void throw_switch(Switch * S, uint8_t state){
  loggerf(TRACE, "throw_switch");

  Algor_Set_Changed(&S->Detection->Alg);
  putList_AlgorQueue(S->Detection->Alg, 0);

  S->state = (state & 0x0f) | 0x80;

  Units[S->module]->switch_state_changed |= 1;

  process(S->Detection, 3);

  Algor_Set_Changed(&S->Detection->Alg);
  putList_AlgorQueue(S->Detection->Alg, 0);

  putAlgorQueue(S->Detection, 1);
}

void throw_msswitch(MSSwitch * S, uint8_t state){
  loggerf(TRACE, "throw_msswitch");

  Algor_Set_Changed(&S->Detection->Alg);
  putList_AlgorQueue(S->Detection->Alg, 0);

  S->state = (state & 0x0f) | 0x80;

  Units[S->module]->msswitch_state_changed |= 1;

  process(S->Detection, 3);

  Algor_Set_Changed(&S->Detection->Alg);
  putList_AlgorQueue(S->Detection->Alg, 0);

  putAlgorQueue(S->Detection, 1);
}

int set_switch(Switch * S, uint8_t state){

  //Check if linked switches are blocked or reserved
  if(!check_linked_switches(S)){
    loggerf(INFO, "Linked Switches Blocked");
    return 0;
  }

  //Check if switch is blocked or reserved
  if(S->Detection && (S->Detection->state == RESERVED || S->Detection->state == RESERVED_SWITCH || S->Detection->blocked)){
    if(S->Detection->state == RESERVED || S->Detection->state == RESERVED_SWITCH){
      loggerf(INFO, "Switch reserved");
    }
    else{
      loggerf(INFO, "Switch blocked");
    }
    return 0;
  }

  throw_switch(S, state);

  for(int i = 0; i<S->links_len; i++){
    if(!S->links[i].p)
      continue;

    if(S->links[i].type == RAIL_LINK_S || S->links[i].type == RAIL_LINK_s){
      throw_switch(S->links[i].p, S->links[i].states[S->state & 0x7f]);
    }
    else if(S->links[i].type == RAIL_LINK_M || S->links[i].type == RAIL_LINK_m){
      throw_msswitch(S->links[i].p, S->links[i].states[S->state & 0x7f]);
    }
  }

  loggerf(INFO, "Throw Switch %i:%i\n");
  COM_update_switch(S->module);
  return 1;
}

int set_switch_path(void * p, struct rail_link link, int flags){
  loggerf(TRACE, "set_switch_path (%x, %x, %i)", (unsigned int)p, (unsigned int)&link, flags);
  if((flags & 0x80) == 0){
    //No SWITCH_CARE
    return 1;
  }

  //Check if switch is occupied
  if (link.type == RAIL_LINK_S || link.type == RAIL_LINK_s) {
    if(((Switch *)link.p)->Detection && ((Switch *)link.p)->Detection->state == RESERVED_SWITCH)
      return 0;
  }
  else if (link.type == RAIL_LINK_M || link.type == RAIL_LINK_m) {
    if(((MSSwitch *)link.p)->Detection && ((Switch *)link.p)->Detection->state == RESERVED_SWITCH)
      return 0;
  }


  if(link.type == RAIL_LINK_S){
    // Go to next switch
    Switch * Sw = link.p;
    if((Sw->state & 0x7F) == 0 && Sw->str.type != RAIL_LINK_R && Sw->str.type != 'D'){
      return set_switch_path(Sw, Sw->str, flags);
    }
    else if((Sw->state & 0x7F) == 1 && Sw->div.type != RAIL_LINK_R && Sw->div.type != 'D'){
      return set_switch_path(Sw, Sw->div, flags);
    }
  }
  else if(link.type == RAIL_LINK_s){
    // Check if switch is in correct state
    // and continue to next switch
    Switch * N = link.p;
    loggerf(TRACE, "set s (state: %i, str.p: %x, div.p: %x)", (N->state & 0x7F), (unsigned int)N->str.p, (unsigned int)N->div.p);
    if((N->state & 0x7F) == 0){
      if(N->str.p != p)
        set_switch(N, 1);

      return set_switch_path(N, N->app, flags);
    }
    else if((N->state & 0x7F) == 1){
      if(N->div.p != p)
        set_switch(N, 0);

      return set_switch_path(N, N->app, flags);
    }
  }
  else if(link.type == RAIL_LINK_M){
    loggerf(ERROR, "IMPLEMENT");
    MSSwitch * N = link.p;
    if(N->sideB[N->state].p == p){
      return 1;
    }
  }
  else if(link.type == RAIL_LINK_m){
    loggerf(ERROR, "IMPLEMENT");
    MSSwitch * N = link.p;
    if(N->sideA[N->state].p == p){
      return 1;
    }
  }

  else if (link.type == RAIL_LINK_R || link.type == 'D'){
    return 1;
  }
  return 0;
}


int set_msswitch(MSSwitch * S, uint8_t state){

  //Check if linked switches are blocked or reserved
  if(!check_linked_msswitches(S)){
    loggerf(INFO, "Linked Switches Blocked");
    return 0;
  }

  //Check if switch is blocked or reserved
  if(S->Detection && (S->Detection->state == RESERVED || S->Detection->state == RESERVED_SWITCH || S->Detection->blocked)){
    if(S->Detection->state == RESERVED || S->Detection->state == RESERVED_SWITCH){
      loggerf(INFO, "Switch reserved");
    }
    else{
      loggerf(INFO, "Switch blocked");
    }
    return 0;
  }

  throw_msswitch(S, state);

  for(int i = 0; i<S->links_len; i++){
    if(!S->links[i].p)
      continue;

    if(S->links[i].type == RAIL_LINK_S || S->links[i].type == RAIL_LINK_s){
      throw_switch(S->links[i].p, S->links[i].states[S->state & 0x7f]);
    }
    else if(S->links[i].type == RAIL_LINK_M || S->links[i].type == RAIL_LINK_m){
      throw_msswitch(S->links[i].p, S->links[i].states[S->state & 0x7f]);
    }
  }

  loggerf(INFO, "Throw MSSwitch %i:%i\n");
  COM_update_switch(S->module);
  return 1;
}

int throw_multiple_switches(uint8_t len, char * msg){
  struct switchdata {
    uint8_t module;
    uint8_t id:7;
    _Bool type;
    char state;
  };

  // Check if all switches are non-blocked
  for(int i = 0; i < len; i++){
    char * data = (void *)&msg[i*3];

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
    char * data = (void *)&msg[i*3];

    struct switchdata p;
    p.module = data[0];
    p.id = data[1] & 0x7F;
    p.type = (data[1] & 0x80) >> 7;
    p.state = data[2];

    if(p.type == 0){
      throw_switch(U_Sw(p.module, p.id), p.state);
    }
    else if(p.type == 1){
      throw_msswitch(U_MSSw(p.module, p.id), p.state);
    }
  }

  COM_change_switch(0);
  return 1;
}

int check_Switch(struct rail_link link, _Bool pref){
  struct rail_link next;

  if(link.type == 'R'){
    return 1;
  }
  else if(link.type == 'e'){
    return 0;
  }
  else if(link.type == 'S'){
    if(((Switch *)link.p)->state == 0)
      next = ((Switch *)link.p)->str;
    else
      next = ((Switch *)link.p)->div;
  }
  else if(link.type == 's'){
    next = ((Switch *)link.p)->app;
  }
  else if(link.type == 'M'){
    next = ((MSSwitch *)link.p)->sideB[((MSSwitch *)link.p)->state];
  }
  else if(link.type == 'm'){
    next = ((MSSwitch *)link.p)->sideA[((MSSwitch *)link.p)->state];
  }

  if(next.type == 'S' || next.type == 'R' || next.type == 'e'){
    return check_Switch(next, pref);
  }
  else if(next.type == 's'){
    Switch * S = next.p;

    if(S->state == 0){
      //If next switch is straight
      if(S->str.p != link.p){
        return 0;
      }
      else{
        return check_Switch(next, pref);
      }
    }
    else{
      //If next switch is diverging
      if(S->div.p != link.p){
        return 0;
      }
      else{
        return check_Switch(next, pref);
      }
    }
  }
  else if(next.type == 'M'){
    MSSwitch * S = next.p;

    if(S->sideB[S->state].p != link.p){
      return 0;
    }
    else{
      return check_Switch(next, pref);
    }
  }
  else if(next.type == 'm'){
    MSSwitch * S = next.p;

    if(S->sideA[S->state].p != link.p){
      return 0;
    }
    else{
      return check_Switch(next, pref);
    }
  }

  return 0;
}
int check_Switch_State(struct rail_link adr){
  loggerf(ERROR, "Implement check_Switch_State");
  return -1;
}

int free_Switch(Block * B, int dir){
  loggerf(ERROR, "Implement free_Switch");
  return -1;
}

int free_Route_Switch(Block * B, int dir, Trains * T){
  loggerf(ERROR, "Implement free_Route_Switch");
  return -1;
}
