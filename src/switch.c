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

void Create_MSSwitch(struct s_msswitch_connect connect, uint8_t block_id, uint8_t output_len, Node_adr * output_pins, uint16_t * output_states){
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

  Algor_search_Blocks(&S->Detection->Alg, 0);

  Algor_Set_Changed(&S->Detection->Alg);
  putList_AlgorQueue(S->Detection->Alg, 0);

  putAlgorQueue(S->Detection, lock);
}

void throw_msswitch(MSSwitch * S, uint8_t state, uint8_t lock){
  loggerf(TRACE, "throw_msswitch");

  Algor_Set_Changed(&S->Detection->Alg);
  putList_AlgorQueue(S->Detection->Alg, 0);

  S->state = (state & 0x0f) | 0x80;

  Units[S->module]->msswitch_state_changed |= 1;

  Algor_search_Blocks(&S->Detection->Alg, 0);

  Algor_Set_Changed(&S->Detection->Alg);
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
