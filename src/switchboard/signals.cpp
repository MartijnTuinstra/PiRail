#include "system.h"
#include "utils/mem.h"
#include "utils/logger.h"

// #include "modules.h"
#include "config_data.h"
#include "config/LayoutStructure.h"

#include "switchboard/manager.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/signals.h"
#include "switchboard/unit.h"

using namespace switchboard;

Signal::Signal(uint8_t module, struct signal_conf conf){ //, uint8_t blockId, uint16_t signalId, bool side, char output_len, struct s_IO_port_conf * output, struct s_IO_signal_event_conf * stating){
  // #define create_signal_from_conf(module, data) new Signal(module, data.blockId, data.id, data.side, data.output_len, data.output, data.stating)
  memset(this, 0, sizeof(Signal));

  // this->B = U_B(module, conf.blockId);
  this->block_link.module = conf.Block.module;
  this->block_link.id = conf.Block.id;
  this->block_link.type = (enum link_types)conf.Block.type;

  this->direction = conf.direction;

  this->module = module;
  this->id = conf.id;

  uid = SwManager->addSignal(this);
  U = Units(module);

  if(this->block_link.type == RAIL_LINK_R){
    this->B = (Block *)rail_link_pointer(this->block_link);

    if(!this->B){
      loggerf(ERROR, "Failed to retrieve block (%2i:%2i) connected to signal %2i:%2i", block_link.module, block_link.id, module, id);
      return;
    }

    this->state = this->B->addSignal(this);
  }
  else if(this->block_link.type != RAIL_LINK_C){
    loggerf(WARNING, "Failed to create Signal, invalid block link.");
    return;
  }

  this->output_len = conf.output_len;
  
  this->output = (IO_Port **)_calloc(this->output_len, IO_Port *);
  this->output_stating = (struct s_signal_stating *)_calloc(this->output_len, struct s_signal_stating);

  for(int i = 0; i < this->output_len; i++){
    if(!U->IO(conf.output[i])){
      loggerf(WARNING, "IO outside range (Port %02i:%02i:%02i)", module, conf.output[i].Node, conf.output[i].Adr);
      continue;
    }

    this->output[i] = U->linkIO(conf.output[i], this, IO_Output);

    for(int j = 0; j <= UNKNOWN; j++){
      this->output_stating[i].state[j].value = conf.stating[i].event[j];
    }
  }

  for(uint8_t i = 0; i < conf.Switch_len; i++){
    void * p = 0;
    if(conf.Switches[i].type){
      // MSSwitch
      U->MSSw[conf.Switches[i].Sw]->addSignal(this);
      p = U->MSSw[conf.Switches[i].Sw];
    }
    else{
      // Switch
      U->Sw[conf.Switches[i].Sw]->addSignal(this);
      p = U->Sw[conf.Switches[i].Sw];
    }

    struct SignalSwitchLink * link = (struct SignalSwitchLink *)_calloc(1, struct SignalSwitchLink);
    link->MSSw = conf.Switches[i].type;
    link->p.p = p;
    link->state = conf.Switches[i].state;

    this->Switches.push_back(link);
  }

  this->switchUpdate();

  U->insertSignal(this);
  
  loggerf(DEBUG, "Create signal %02i:%02i, %s, block %08x", this->module, this->id, this->direction ? "Forward" : "Reverse", this->B);

  set(UNKNOWN);
}

Signal::Signal(uint8_t _module, struct configStruct_Signal * conf){ //, uint8_t blockId, uint16_t signalId, bool side, char output_len, struct s_IO_port_conf * output, struct s_IO_signal_event_conf * stating){
  // #define create_signal_from_conf(module, data) new Signal(module, data.blockId, data.id, data.side, data.output_len, data.output, data.stating)
  memset(this, 0, sizeof(Signal));

  // this->B = U_B(module, conf.blockId);
  block_link.module = conf->block.module;
  block_link.id = conf->block.id;
  block_link.type = (enum link_types)conf->block.type;

  direction = conf->direction;

  module = _module;
  id = conf->id;

  uid = SwManager->addSignal(this);
  U = Units(_module);

  if(block_link.type == RAIL_LINK_R){
    B = (Block *)rail_link_pointer(block_link);

    if(!B){
      loggerf(ERROR, "Failed to retrieve block (%2i:%2i) connected to signal %2i:%2i", block_link.module, block_link.id, module, id);
      return;
    }

    state = B->addSignal(this);
  }
  else if(block_link.type != RAIL_LINK_C){
    loggerf(WARNING, "Failed to create Signal, invalid block link.");
    return;
  }

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

  for(uint8_t i = 0; i < conf->Switch_len; i++){
    void * p = 0;
    if(conf->Switches[i].type){
      // MSSwitch
      U->MSSw[conf->Switches[i].Sw]->addSignal(this);
      p = U->MSSw[conf->Switches[i].Sw];
    }
    else{
      // Switch
      U->Sw[conf->Switches[i].Sw]->addSignal(this);
      p = U->Sw[conf->Switches[i].Sw];
    }

    struct SignalSwitchLink * link = (struct SignalSwitchLink *)_calloc(1, struct SignalSwitchLink);
    link->MSSw = conf->Switches[i].type;
    link->p.p = p;
    link->state = conf->Switches[i].state;

    Switches.push_back(link);
  }

  switchUpdate();

  U->insertSignal(this);
  
  loggerf(DEBUG, "Create signal %02i:%02i, %s, block %08x", module, id, direction ? "Forward" : "Reverse", B);

  set(UNKNOWN);
}

Signal::~Signal(){
  for(auto link: this->Switches){
    _free(link);
  }
  _free(this->output);
  _free(this->output_stating);
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

  if(cfg->Switch_len){
    cfg->Switches = (struct configStruct_SignalDependentSwitch *)_calloc(cfg->Switch_len, struct configStruct_SignalDependentSwitch);
    for(uint8_t i = 0; i < cfg->Switch_len; i++){
      cfg->Switches[i].type  = Switches[i]->MSSw;
      if(Switches[i]->MSSw)
        cfg->Switches[i].Sw    = Switches[i]->p.MSSw->id;
      else
        cfg->Switches[i].Sw    = Switches[i]->p.Sw->id;
      cfg->Switches[i].state = Switches[i]->state;
    }
  }
}

void print_signal_state(Signal * Si, enum Rail_states state){
  if(Si->B)
    loggerf(DEBUG, "%02i:%02i Sig %i  %i %s -> %i %s", Si->B->module, Si->B->id, Si->id, Si->state, rail_states_string[Si->state], state, rail_states_string[state]);
  else
    loggerf(DEBUG, "--:-- Sig %i  %i %s -> %i %s", Si->id, Si->state, rail_states_string[Si->state], state, rail_states_string[state]);
}

void Signal::set(enum Rail_states state){
  loggerf(TRACE, "set_signal %x, %i", (unsigned int)this, (unsigned int)state);
  if(this->state != state){
    print_signal_state(this, state);
    // Update state
    this->state = state;
    this->setIO();
  }
}

void Signal::switchUpdate(){
  this->switchDanger = false;

  for(auto link: this->Switches){
    if((link->MSSw && link->p.MSSw->state != link->state) || (!link->MSSw && link->p.Sw->state != link->state)){
      this->switchDanger = true;
      break;
    }
  }

  this->setIO();
}

void Signal::setIO(){
  //Update IO
  enum Rail_states state = this->state;

  if(this->switchDanger)
    state = DANGER;

  for(int i = 0; i < this->output_len; i++){
    this->output[i]->setOutput(this->output_stating[i].state[state]);
  }
}

void Signal::check(){
  loggerf(TRACE, "check_Signal %x", (unsigned int)this);
  // if(this->switches)
  this->state = this->B->state;
}
