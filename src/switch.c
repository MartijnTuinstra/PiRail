#include "switch.h"
#include "logger.h"
#include "system.h"
#include "module.h"

void Create_Switch(struct switch_connect connect, char block_id, char output_len, char * output_pins, _Bool ** output_states){
  Switch * Z = _calloc(1, Switch);

  Z->module = connect.module;
  Z->id = connect.id;

  Z->div = connect.div;
  Z->str = connect.str;
  Z->app = connect.app;

  Z->output_len = output_len;
  Z->output_pins = output_pins;
  Z->output_states = output_states;

  if(Units[Z->module]->B[block_id]){
    Z->Detection = Units[Z->module]->B[block_id];
    loggerf(INFO, "SWITCH %i:%i has detection block %i", connect.module, connect.id, block_id);
  }
  else{
    loggerf(ERROR, "SWITCH %i:%i has no detection block %i", connect.module, connect.id, block_id);
  }


  if(Units[connect.module]->Sw[connect.id]){
    loggerf(INFO, "Duplicate switch, overwriting ...");
    _free(Units[connect.module]->Sw[connect.id]);
  }
  Units[connect.module]->Sw[connect.id] = Z;
}

void Create_MSSwitch(struct msswitch_connect connect, char block_id, char output_len, char * output_pins, _Bool ** output_states){
  MSSwitch * Z = _calloc(1, MSSwitch);

  Z->module = connect.module;
  Z->id = connect.id;

  Z->sideA = connect.sideA;
  Z->sideB = connect.sideB;

  Z->output_len = output_len;
  Z->output_pins = output_pins;
  Z->output_states = output_states;

  if(Units[Z->module]->B[block_id]){
    Z->Detection = Units[Z->module]->B[block_id];
  }
  else
    loggerf(ERROR, "MSSwitch %i:%i has no detection block %i", connect.module, connect.id, block_id);

  if(Units[connect.module]->MSSw[connect.id]){
    loggerf(INFO, "Duplicate switch, overwriting ...");
    _free(Units[connect.module]->MSSw[connect.id]);
  }
  Units[connect.module]->MSSw[connect.id] = Z;
}

int throw_switch(Switch * S){
  loggerf(ERROR, "Implement throw_switch");
}

int throw_msswitch(MSSwitch * S){
  loggerf(ERROR, "Implement throw_msswitch");
}


int set_switch(Switch * S, char state){
  loggerf(ERROR, "Implement set_switch");
}

int set_msswitch(MSSwitch * S, char state){
  loggerf(ERROR, "Implement set_msswitch");
}

int set_multiple_switches(struct switch_list list, char * states){
  loggerf(ERROR, "Implement set_multiple_switches");
}

int check_Switch(Block * B, int dir, _Bool pref){
  loggerf(ERROR, "Implement check_Switch");
}
int check_Switch_State(struct rail_link adr){
  loggerf(ERROR, "Implement check_Switch_State");
}

int free_Switch(Block * B, int dir){
  loggerf(ERROR, "Implement free_Switch");
}

int free_Route_Switch(Block * B, int dir, Trains * T){
  loggerf(ERROR, "Implement free_Route_Switch");
}
