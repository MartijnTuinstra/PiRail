#include "switch.h"
#include "logger.h"
#include "system.h"
#include "module.h"
#include "websocket_msg.h"
#include "websocket.h"
#include "com.h"
#include "IO.h"

void Create_Switch(struct switch_connect connect, char block_id, char output_len, Node_adr * output_pins, char * output_states){
  Switch * Z = _calloc(1, Switch);

  Z->module = connect.module;
  Z->id = connect.id;

  Z->div = connect.div;
  Z->str = connect.str;
  Z->app = connect.app;

  Z->IO = _calloc(output_len, IO_Port *);

  for(int i = 0; i < output_len; i++){
    if(output_pins[i].Node > Units[connect.module]->IO_Nodes){
      loggerf(WARNING, "Node not initialized");
      return;
    }
    if(Units[connect.module]->Node[output_pins[i].Node].io[output_pins[i].io]){
      IO_Port * A = Units[connect.module]->Node[output_pins[i].Node].io[output_pins[i].io];

      if(A->type != IO_Undefined){
        loggerf(WARNING, "IO %i:%i already in use", output_pins[i].Node, output_pins[i].io);
      }

      A->type = IO_Output;
      A->state = 0;
      A->id = output_pins[i].io;

      loggerf(DEBUG, "IO %i:%i", output_pins[i].Node, output_pins[i].io);
    }
  }

  Z->IO_len = output_len;
  Z->IO_states = output_states;

  if(Units[Z->module]->B[block_id]){
    Z->Detection = Units[Z->module]->B[block_id];
    if(Units[Z->module]->B[block_id]->switch_len == 0){
      Units[Z->module]->B[block_id]->Sw = _calloc(2, void *);
      Units[Z->module]->B[block_id]->switch_len = 2;
    }
    loggerf(DEBUG, "Block linked switches find index");
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
    if(pins[i].Node > Units[S->module]->IO_Nodes){
      loggerf(WARNING, "Node not initialized");
      return;
    }
    if(Units[S->module]->Node[pins[i].Node].io[pins[i].io]){
      IO_Port * A = Units[S->module]->Node[pins[i].Node].io[pins[i].io];
      A->type = IO_Input;
      A->state = 0;
      A->id = pins[i].io;

      S->feedback[i] = A;
    }
  }
  _free(pins);

  S->feedback_len = len;
  S->feedback_states = state;
}

void Create_MSSwitch(struct msswitch_connect connect, char block_id, char output_len, Node_adr * output_pins, uint16_t * output_states){
  MSSwitch * Z = _calloc(1, MSSwitch);

  Z->module = connect.module;
  Z->id = connect.id;

  Z->sideA = connect.sideA;
  Z->sideB = connect.sideB;

  Z->IO = _calloc(output_len, IO_Port *);

  for(int i = 0; i < output_len; i++){
    if(output_pins[i].Node > Units[connect.module]->IO_Nodes){
      loggerf(WARNING, "Node not initialized");
      return;
    }
    if(Units[connect.module]->Node[output_pins[i].Node].io[output_pins[i].io]){
      IO_Port * A = Units[connect.module]->Node[output_pins[i].Node].io[output_pins[i].io];
      A->type = IO_Output;
      A->state = 0;
      A->id = output_pins[i].io;

      Z->IO[i] = A;
    }
  }
  _free(output_pins);

  Z->IO_len = output_len;
  Z->IO_states = output_states;

  if(Units[Z->module]->B[block_id]){
    Z->Detection = Units[Z->module]->B[block_id];
    if(Units[Z->module]->B[block_id]->msswitch_len == 0){
      Units[Z->module]->B[block_id]->MSSw = _calloc(2, void *);
      Units[Z->module]->B[block_id]->msswitch_len = 2;
    }
    int id = find_free_index(Units[Z->module]->B[block_id]->MSSw, Units[Z->module]->B[block_id]->msswitch_len);
    Units[Z->module]->B[block_id]->MSSw[id] = Z;
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


int set_switch(Switch * S,char state){
  int linked = 0;
  for(int i = 0;i<S->links_len;i++){
    if(S->links[i].p){
      linked = 1;

      if(S->links[i].type == 'S' || S->links[i].type == 's'){
        if(((Switch *)S->links[i].p)->Detection && (((Switch *)S->links[i].p)->Detection->state == RESERVED || 
              ((Switch *)S->links[i].p)->Detection->blocked)){
            printf("Linked switches blocked\n");
          return 0;
        }
      }else if(S->links[i].type == 'M' || S->links[i].type == 'm'){
        loggerf(ERROR, "set_switch linked MSSwitch implement");
      }
    }
  }
  if(S->Detection && (S->Detection->state != RESERVED && S->Detection->state != RESERVED_SWITCH && !S->Detection->blocked) || !S->Detection){
    S->state = state + 0x80;

    char buf[40];
    buf[0] = WSopc_BroadSwitch;
    int index = 1;

    buf[index++] = S->module;
    buf[index++] = S->id;
    buf[index++] = S->state;

    for(int i = 0;i<S->links_len;i++){
      if(S->links[i].p){
        if(S->links[i].type == 'S' || S->links[i].type == 's'){
          Switch * LSw = S->links[i].p;
          LSw->state = S->links[i].states[S->state];

          buf[index++] = LSw->module;
          buf[index++] = LSw->id;
          buf[index++] = LSw->state;
        }
        else if(S->links[i].type == 'M' || S->links[i].type == 'm'){

        }
      }
    }
    printf("Throw Switch %s\n\n",buf);
    COM_change_switch(S->module);
    ws_send_all(buf,index,2);
    return 1;
  }else{
    if(S->Detection->state == RESERVED || S->Detection->state == RESERVED_SWITCH){
      loggerf(INFO, "Switch reserved");
    }
    else{
      loggerf(INFO, "Switch blocked");
    }
    return 0;
  }
}

int set_msswitch(MSSwitch * S, char state){
  loggerf(ERROR, "Implement set_msswitch");
}

int set_multiple_switches(char len, char * msg){
  struct switchdata {
    char module;
    char id:7;
    _Bool type;
    char state;
  };

  char module = 0;

  for(int i = 0; i < len; i++){
    char * data = (void *)&msg[i*3];

    printf("%x %x %x\t",data[0],data[1],data[2]);

    struct switchdata p;
    p.module = data[0];
    p.id = data[1] & 0x7F;
    p.type = (data[1] & 0x80) >> 7;
    p.state = data[2];

    printf("check %d:%d\n",p.module,p.id,p.state);

    if(!(Units[p.module] && (
      (p.type == 0 && Units[p.module]->Sw[p.id]) || 
      (p.type == 1 && Units[p.module]->MSSw[p.id]) )) ){
      printf("Switch doesnt exist\n");
      return -1;
    }

    Unit * U = Units[p.module];

    if((p.type == 0 && U->Sw[p.id]->Detection && U->Sw[p.id]->Detection->blocked) || 
        (p.type == 1 && U->MSSw[p.id]->Detection && U->MSSw[p.id]->Detection->blocked)){
      printf("Switch is blocked\n");
      return -2;
    }
  }

  char buf[40];
  buf[0] = WSopc_BroadSwitch;
  int index = 1;

  for(int i = 0; i < len; i++){
    char * data = (void *)&msg[i*3];

    struct switchdata p;
    p.module = data[0];
    p.id = data[1] & 0x7F;
    p.type = (data[1] & 0x80) >> 7;
    p.state = data[2];

    Unit * U = Units[p.module];

    printf("check %d:%d\n",p.module,p.id,p.state);

    if(p.type == 0){
      Switch * S = U->Sw[p.id];

      printf("throw switch %i:%i to state: \t",p.module,p.id);
      printf("%i->%i",S->state, p.state);

      U->Sw[p.id]->state = p.state | 0x80;

      buf[index++] = S->module;
      buf[index++] = S->id;
      buf[index++] = p.state;
    }
    else if(p.type == 1){
      U->MSSw[p.id];
      printf("Set mulbitple switch msswitch not implemented\n");
      return -3;
    }
  }

  COM_change_switch(module);
  ws_send_all(buf,index,2);
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
