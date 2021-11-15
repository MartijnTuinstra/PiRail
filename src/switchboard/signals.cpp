#include "system.h"
#include "utils/mem.h"
#include "utils/logger.h"

#include "config/LayoutStructure.h"

#include "switchboard/manager.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/signals.h"
#include "switchboard/unit.h"

using namespace switchboard;

Signal::Signal(uint8_t _module, struct configStruct_Signal * conf):
  block_link(conf->block)
{
  direction = conf->direction;

  module = _module;
  id = conf->id;

  uid = SwManager->addSignal(this);
  U = Units(_module);

  tmpP = (Block *)rail_link_pointer(conf->tmp_block);
  block_link.p.p = nullptr;

  output_len = conf->output_len;
  
  output = (IO_Port **)_calloc(output_len, IO_Port *);
  output_stating = (struct s_signal_stating *)_calloc(output_len, struct s_signal_stating);

  for(int i = 0; i < output_len; i++){
    if(!U->IO(conf->output[i])){
      loggerf(WARNING, "IO outside range (Port %02i:%02i:%02i)", module, conf->output[i].Node, conf->output[i].Port);
      continue;
    }

    output[i] = U->linkIO(conf->output[i], this, IO_Output);

    for(int j = 0; j <= UNKNOWN; j++){
      output_stating[i].state[j].value = conf->stating[i].event[j];
    }
  }

  // for(uint8_t i = 0; i < conf->Switch_len; i++){
  //   void * p = 0;
  //   if(conf->Switches[i].type){
  //     // MSSwitch
  //     U->MSSw[conf->Switches[i].Sw]->addSignal(this);
  //     p = U->MSSw[conf->Switches[i].Sw];
  //   }
  //   else{
  //     // Switch
  //     U->Sw[conf->Switches[i].Sw]->addSignal(this);
  //     p = U->Sw[conf->Switches[i].Sw];
  //   }

  //   struct SignalSwitchLink * link = (struct SignalSwitchLink *)_calloc(1, struct SignalSwitchLink);
  //   link->MSSw = conf->Switches[i].type;
  //   link->p.p = p;
  //   link->state = conf->Switches[i].state;

  //   Switches.push_back(link);
  // }

  switchUpdate();

  U->insertSignal(this);
  
  loggerf(DEBUG, "Create signal %02i:%02i, %s, block %08x", module, id, direction ? "Forward" : "Reverse", B);

  set(UNKNOWN);
}

Signal::~Signal(){
  _free(this->output);
  _free(this->output_stating);
}

void Signal::map(){
  loggerf(INFO, "Signal::map  %i:%i uid %i", module, id, uid);
  RailLink * L = &block_link;

  while(L->type != RAIL_LINK_R){
    loggerf(INFO, " %02i:%02i:%02x", L->module, L->id, L->type);
    if(L->type == RAIL_LINK_s){
      Switch * Sw = (Switch *)rail_link_pointer(*L);
      Sw->addSignal(this);

      loggerf(INFO, " %x str %x div %x", tmpP, Sw->str.p.p, Sw->div.p.p);

      bool state = (Sw->str.p.p == tmpP ? STRAIGHT_SWITCH : DIVERGING_SWITCH);

      loggerf(INFO, " state = %i", state);
      SignalSwitchLink link = {.MSSw=SIGNAL_LINK___SWITCH, .p=Sw, .state=state};
      Switches.push_back(link);

      loggerf(INFO, " %02i-%02i:%02i-%02i", link.MSSw, link.p.Sw->module, link.p.Sw->id, link.state);

      L = &Sw->app;
    }
    else if(L->type == RAIL_LINK_MA || L->type == RAIL_LINK_MB){
      MSSwitch * Sw = (MSSwitch *)rail_link_pointer(*L);
      RailLink * next = L->type == RAIL_LINK_MA ? Sw->sideB : Sw->sideA;
      RailLink * prev = L->type == RAIL_LINK_MA ? Sw->sideA : Sw->sideB;
      Sw->addSignal(this);

      bool stateFound = false;
      uint8_t state = 0;

      for(uint8_t i = 0; i < Sw->state_len; i++){
        if(prev[i].p.p == tmpP){
          stateFound = true;
          state = i;
          break;
        }
      }

      if(!stateFound){
        loggerf(WARNING, "Signal state not found");
        return;
      }

      Switches.push_back({SIGNAL_LINK_MSSWITCH, (Switch *)Sw, state});

      L = &next[state];
    }
    else if(L->type == RAIL_LINK_C){
      loggerf(ERROR, " Signal still has Connector link");
      return;
    }
  }

  B = (Block *)rail_link_pointer(*L);
  memcpy(&block_link, L, sizeof(RailLink));
  block_link.p.p = rail_link_pointer(block_link);

  if(!B){
    loggerf(ERROR, "Failed to retrieve block (%2i:%2i) connected to signal %2i:%2i", L->module, L->id, module, id);
    return;
  }
  else{
    loggerf(INFO, "Signal::map  %i:%i to B %02i:%02i / %x", module, id, uid, block_link.module, block_link.id, block_link.p.p);
  }

  state = B->addSignal(this);
}

void Signal::exportConfig(struct configStruct_Signal * cfg){

// struct configStruct_SignalDependentSwitch
// {
//   uint8_t type;
//   uint8_t Sw;
//   uint8_t state;
// };

// struct configStruct_SignalEvent
// {
//   uint8_t event[8];
// };

// struct configStruct_Signal
// {
//   uint8_t direction;
//   uint16_t id;
//   struct configStruct_RailLink block;
//   uint8_t output_len;
//   uint8_t Switch_len;
//   struct configStruct_IOport * output;
//   struct configStruct_SignalEvent * stating;
//   struct configStruct_SignalDependentSwitch * Switches;
// };
// struct configStruct_SignalDependentSwitch
// {
//   uint8_t type;
//   uint8_t Sw;
//   uint8_t state;
// };
  cfg->direction = direction;
  cfg->id = id;
  railLinkExport(&cfg->block, block_link);

  cfg->output_len = output_len;
  cfg->Switch_len = Switches.size();

  if(output_len){
    cfg->output  = (struct configStruct_IOport *)_calloc(output_len, struct configStruct_IOport);
    cfg->stating = (struct configStruct_SignalEvent *)_calloc(output_len, struct configStruct_SignalEvent);
    for(uint8_t i = 0; i < output_len; i++){
      if(output[i])
        output[i]->exportConfig(&cfg->output[i]);

      for(uint8_t j = 0; j < 8; j++){
        cfg->stating[i].event[j] = output_stating[i].state[j].value;
      }
    }
  }
}

void print_signal_state(Signal * Si, enum Rail_states state){
  if(Si->B)
    loggerf(INFO, "%02i:%02i Sig %i  %i %s -> %i %s", Si->B->module, Si->B->id, Si->id, Si->state, rail_states_string[Si->state], state, rail_states_string[state]);
  else
    loggerf(DEBUG, "--:-- Sig %i  %i %s -> %i %s", Si->id, Si->state, rail_states_string[Si->state], state, rail_states_string[state]);
}

void Signal::set(enum Rail_states _state){
  loggerf(TRACE, "set_signal %x, %i", (unsigned long)this, (unsigned long)_state);
  if(state != _state){
    print_signal_state(this, _state);
    // Update state
    state = _state;
    setIO();
  }
}

void Signal::switchUpdate(){
  switchDanger = false;

  for(auto link: Switches){
    if((link.MSSw && link.p.MSSw->state != link.state) || (!link.MSSw && link.p.Sw->state != link.state)){
      switchDanger = true;
      break;
    }
  }

  this->setIO();
}

void Signal::setIO(){
  //Update IO
  if(switchDanger)
    state = DANGER;

  for(int i = 0; i < output_len; i++){
    output[i]->setOutput(output_stating[i].state[state]);
  }
}

void Signal::check(){
  loggerf(TRACE, "check_Signal %x", (unsigned long)this);
  // if(this->switches)
  this->state = this->B->state;
}
