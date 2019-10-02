#include "switch.h"
#include "logger.h"
#include "system.h"
#include "mem.h"
// #include "modules.h"
// #include "websocket_msg.h"
// #include "websocket.h"
// #include "com.h"
#include "IO.h"
#include "algorithm.h"

void Create_Switch(struct s_switch_connect connect, uint8_t block_id, uint8_t output_len, Node_adr * output_pins, uint8_t * output_states){
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

void create_msswitch_from_conf(uint8_t module, struct ms_switch_conf conf){

  // uint8_t id;
  // uint8_t det_block;

  // uint8_t nr_states;
  // uint8_t IO;

  // struct s_ms_switch_state_conf * states;
  // struct s_IO_port_conf * IO_Ports;  

  struct s_msswitch_connect connect;
  connect.module = module;
  connect.id = conf.id;
  connect.states = conf.nr_states;

  connect.sideA = _calloc(conf.nr_states, struct rail_link);
  connect.sideB = _calloc(conf.nr_states, struct rail_link);

  for(uint8_t i = 0; i < conf.nr_states; i++){
    connect.sideA[i].module = conf.states[i].sideA.module;
    connect.sideA[i].id = conf.states[i].sideA.id;
    connect.sideA[i].type = conf.states[i].sideA.type;

    connect.sideB[i].module = conf.states[i].sideB.module;
    connect.sideB[i].id = conf.states[i].sideB.id;
    connect.sideB[i].type = conf.states[i].sideB.type;
  }

  Create_MSSwitch(connect, conf.det_block, 0, 0, 0);
}

void Create_MSSwitch(struct s_msswitch_connect connect, uint8_t block_id, uint8_t output_len, Node_adr * output_pins, uint16_t * output_states){
  loggerf(DEBUG, "Create MSSw %i:%i", connect.module, connect.id);
  MSSwitch * Z = _calloc(1, MSSwitch);

  Z->module = connect.module;
  Z->id = connect.id;

  Z->sideA = connect.sideA;
  Z->sideB = connect.sideB;

  Z->state_len = connect.states;

  Z->IO = _calloc(output_len, IO_Port *);

  for(int i = 0; i < output_len; i++){
    Init_IO(Units[connect.module], output_pins[i], IO_Output);

    Z->IO[i] = Units[connect.module]->Node[output_pins[i].Node].io[output_pins[i].io];
  }
  if(output_pins)
    _free(output_pins);

  Z->IO_len = output_len;
  Z->IO_states = output_states;

  if(U_B(Z->module, block_id)){
    Z->Detection = U_B(Z->module, block_id);
    if(U_B(Z->module, block_id)->msswitch_len == 0){
      U_B(Z->module, block_id)->MSSw = _calloc(1, void *);
      U_B(Z->module, block_id)->msswitch_len = 1;
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

void * Clear_Switch(Switch * Sw){
  _free(Sw->feedback);
  _free(Sw->IO);
  _free(Sw->IO_states);
  _free(Sw->links);
  _free(Sw->preferences);

  _free(Sw);
  return NULL;
}

void * Clear_MSSwitch(MSSwitch * MSSw){
  _free(MSSw);
  return NULL;
}


void throw_switch(Switch * S, uint8_t state, uint8_t lock){
  loggerf(TRACE, "throw_switch");

  Algor_Set_Changed(&S->Detection->Alg);
  putList_AlgorQueue(S->Detection->Alg, 0);

  S->state = (state & 0x0f) | 0x80;

  Units[S->module]->switch_state_changed |= 1;

  Algor_search_Blocks(S->Detection, 0);

  Algor_Set_Changed(&S->Detection->Alg);

  S->Detection->algorchanged = 0; // Block is allready search should not be researched
  
  putList_AlgorQueue(S->Detection->Alg, 0);

  putAlgorQueue(S->Detection, lock);
}

void throw_msswitch(MSSwitch * S, uint8_t state, uint8_t lock){
  loggerf(TRACE, "throw_msswitch");

  Algor_Set_Changed(&S->Detection->Alg);
  putList_AlgorQueue(S->Detection->Alg, 0);

  S->state = (state & 0x0f) | 0x80;

  Units[S->module]->msswitch_state_changed |= 1;

  Algor_search_Blocks(S->Detection, 0);

  Algor_Set_Changed(&S->Detection->Alg);

  S->Detection->algorchanged = 0; // Block is allready search should not be researched

  putList_AlgorQueue(S->Detection->Alg, 0);

  putAlgorQueue(S->Detection, lock);
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
      throw_switch(U_Sw(p.module, p.id), p.state, 0);
    }
    else if(p.type == 1){
      throw_msswitch(U_MSSw(p.module, p.id), p.state, 0);
    }
  }

  algor_queue_enable(1);

  // COM_change_switch(0);
  return 1;
}

int Next_check_Switch(void * p, struct rail_link link, int flags){
  loggerf(TRACE, "Next_check_Switch (%x, %x, %i)", (unsigned int)p, (unsigned int)&link, flags);
  if((flags & 0x80) == 0){
    //No SWITCH_CARE
    return 1;
  }
  else if(link.type == RAIL_LINK_S){
    return 1;
  }
  else if(link.type == RAIL_LINK_s){
    Switch * N = link.p;
    loggerf(TRACE, "check s (state: %i, str.p: %x, div.p: %x)", (N->state & 0x7F), (unsigned int)N->str.p, (unsigned int)N->div.p);
    if(((N->state & 0x7F) == 0 && N->str.p == p) || ((N->state & 0x7F) == 1 && N->div.p == p)){
      return 1;
    }
    // else
    //   printf("str: %i  %x==%x\tdiv: %i  %x==%x\t",N->state, N->str.p, p, N->state, N->div.p, p);
  }
  else if(link.type == RAIL_LINK_M){
    MSSwitch * N = link.p;
    if(N->sideB[N->state].p == p){
      return 1;
    }
  }
  else if(link.type == RAIL_LINK_m){
    MSSwitch * N = link.p;
    if(N->sideA[N->state].p == p){
      return 1;
    }
  }
  return 0;
}


int Switch_Set_Path(void * p, struct rail_link link, int flags){
  loggerf(TRACE, "Switch_Set_Path (%x, %x, %i)", (unsigned int)p, (unsigned int)&link, flags);
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
      return Switch_Set_Path(Sw, Sw->str, flags);
    }
    else if((Sw->state & 0x7F) == 1 && Sw->div.type != RAIL_LINK_R && Sw->div.type != 'D'){
      return Switch_Set_Path(Sw, Sw->div, flags);
    }
  }
  else if(link.type == RAIL_LINK_s){
    // Check if switch is in correct state
    // and continue to next switch
    Switch * N = link.p;
    loggerf(TRACE, "set s (state: %i, str.p: %x, div.p: %x)", (N->state & 0x7F), (unsigned int)N->str.p, (unsigned int)N->div.p);
    if((N->state & 0x7F) == 0){
      if(N->str.p != p)
        throw_switch(N, 1, 1);

      if(N->str.p != p)
        return 0; // Failed to set switch

      return Switch_Set_Path(N, N->app, flags);
    }
    else if((N->state & 0x7F) == 1){
      if(N->div.p != p)
        throw_switch(N, 0, 1);

      if(N->div.p != p)
        return 0; // Failed to set switch

      return Switch_Set_Path(N, N->app, flags);
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

int Switch_Reserve_Path(void * p, struct rail_link link, int flags){
  loggerf(TRACE, "reserve_switch_path (%x, %x, %i)", (unsigned int)p, (unsigned int)&link, flags);
  if((flags & 0x80) == 0){
    //No SWITCH_CARE
    return 1;
  }


  if(link.type == RAIL_LINK_S){
    // Go to next switch
    Switch * Sw = link.p;
    Block * DB = Sw->Detection;

    Block_reserve(DB);
    DB->state = RESERVED_SWITCH;
    DB->reserved = 1;
    DB->statechanged = 1;
    Units[DB->module]->block_state_changed = 1;
    loggerf(INFO, "Set switch %02i:%02i to RESERVED", Sw->module, Sw->id);

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
    Switch * Sw = link.p;
    Block * DB = Sw->Detection;

    Block_reserve(DB);
    DB->state = RESERVED_SWITCH;
    DB->reserved = 1;
    DB->statechanged = 1;
    Units[DB->module]->block_state_changed = 1;
    loggerf(INFO, "Set switch %02i:%02i to RESERVED", Sw->module, Sw->id);

    return Switch_Reserve_Path(Sw, Sw->app, flags);
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

int Switch_Check_Path(void * p, struct rail_link link, int flags){
  // Check if switches are set to a good path

  loggerf(TRACE, "Switch_Check_Path (%x, %x, %i)", (unsigned int)p, (unsigned int)&link, flags);
  if(!link.p){
    loggerf(ERROR, "Empty LINK {%i:%i - %i}", link.module, link.id, link.type);
    return 1;
  }

  if((flags & 0x80) == 0){
    //No SWITCH_CARE
    return 1;
  }
  else if(link.type == RAIL_LINK_S){
    Switch * Sw = link.p;
    if((Sw->state & 0x7F) == 0 && Sw->str.type != RAIL_LINK_R && Sw->str.type != 'D'){
      return Switch_Check_Path(Sw, Sw->str, flags);
    }
    else if((Sw->state & 0x7F) == 1 && Sw->div.type != RAIL_LINK_R && Sw->div.type != 'D'){
      return Switch_Check_Path(Sw, Sw->div, flags);
    }
  }
  else if(link.type == RAIL_LINK_s){
    Switch * N = link.p;
    loggerf(TRACE, "check s (state: %i, str.p: %x, div.p: %x)", (N->state & 0x7F), (unsigned int)N->str.p, (unsigned int)N->div.p);
    if((N->state & 0x7F) == 0 && N->str.p == p){
      return Switch_Check_Path(N, N->app, flags);
    }
    else if((N->state & 0x7F) == 1 && N->div.p == p){
      return Switch_Check_Path(N, N->app, flags);
    }
    loggerf(TRACE, "wrong State");
  }
  else if(link.type == RAIL_LINK_M){
    loggerf(WARNING, "IMPLEMENT");
    MSSwitch * N = link.p;
    if(N->sideB[N->state].p == p){
      return 1;
    }
  }
  else if(link.type == RAIL_LINK_m){
    loggerf(WARNING, "IMPLEMENT");
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
