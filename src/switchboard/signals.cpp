#include "system.h"
#include "mem.h"
#include "logger.h"

// #include "modules.h"
#include "config_data.h"

#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/signals.h"
#include "switchboard/unit.h"

Signal::Signal(uint8_t module, struct signal_conf conf){ //, uint8_t blockId, uint16_t signalId, bool side, char output_len, struct s_IO_port_conf * output, struct s_IO_signal_event_conf * stating){
  // #define create_signal_from_conf(module, data) new Signal(module, data.blockId, data.id, data.side, data.output_len, data.output, data.stating)
  memset(this, 0, sizeof(Signal));

  // this->B = U_B(module, conf.blockId);
  this->block_link.module = conf.Block.module;
  this->block_link.id = conf.Block.id;
  this->block_link.type = (enum link_types)conf.Block.type;

  this->direction = conf.direction;

  if(this->block_link.type == RAIL_LINK_R){
    this->B = (Block *)rail_link_pointer(this->block_link);

    this->state = this->B->addSignal(this);
  }
  else if(this->block_link.type != RAIL_LINK_C){
    loggerf(WARNING, "Failed to create Signal, invalid block link.");
    return;
  }

  this->id = conf.id;
  this->module = module;
  this->output_len = conf.output_len;
  
  this->output = (IO_Port **)_calloc(this->output_len, IO_Port *);
  this->output_stating = (struct s_signal_stating *)_calloc(this->output_len, struct s_signal_stating);

  Unit * U = Units[module];

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
      Units[module]->MSSw[conf.Switches[i].Sw]->addSignal(this);
      p = Units[module]->MSSw[conf.Switches[i].Sw];
    }
    else{
      // Switch
      Units[module]->Sw[conf.Switches[i].Sw]->addSignal(this);
      p = Units[module]->Sw[conf.Switches[i].Sw];
    }

    struct SignalSwitchLink * link = (struct SignalSwitchLink *)_calloc(1, struct SignalSwitchLink);
    link->MSSw = conf.Switches[i].type;
    link->p.p = p;
    link->state = conf.Switches[i].state;

    this->Switches.push_back(link);
  }

  this->switchUpdate();

  Units[module]->insertSignal(this);
  
  loggerf(DEBUG, "Create signal %02i:%02i, %s, block %08x", this->module, this->id, this->direction ? "Forward" : "Reverse", this->B);

  set(UNKNOWN);
}

Signal::~Signal(){
  for(auto link: this->Switches){
    _free(link);
  }
  _free(this->output);
  _free(this->output_stating);
}

void print_signal_state(Signal * Si, enum Rail_states state){
  if(Si->B)
    loggerf(INFO, "%02i:%02i Sig %i  %i %s -> %i %s", Si->B->module, Si->B->id, Si->id, Si->state, rail_states_string[Si->state], state, rail_states_string[state]);
  else
    loggerf(INFO, "--:-- Sig %i  %i %s -> %i %s", Si->id, Si->state, rail_states_string[Si->state], state, rail_states_string[state]);
}

void Signal::set(enum Rail_states state){
  loggerf(INFO, "set_signal %x, %i", (unsigned int)this, (unsigned int)state);
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
