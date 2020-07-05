#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/signals.h"

#include "system.h"
#include "mem.h"

#include "config_data.h"
#include "modules.h"
#include "logger.h"

Signal::Signal(uint8_t module, struct signal_conf conf){ //, uint8_t blockId, uint16_t signalId, bool side, char output_len, struct s_IO_port_conf * output, struct s_IO_signal_event_conf * stating){
  // #define create_signal_from_conf(module, data) new Signal(module, data.blockId, data.id, data.side, data.output_len, data.output, data.stating)
  memset(this, 0, sizeof(Signal));

  this->B = U_B(module, conf.blockId);
  this->direction = conf.direction;
  this->state = this->B->addSignal(this);

  this->id = conf.id;
  this->module = module;
  this->output_len = conf.output_len;
  
  this->output = (IO_Port **)_calloc(this->output_len, IO_Port *);
  this->output_stating = (struct s_signal_stating *)_calloc(this->output_len, struct s_signal_stating);

  for(int i = 0; i < this->output_len; i++){
    if(Units[module]->Node[conf.output[i].Node].io_ports <= conf.output[i].Adr){
      // if(this->output_len == output_len){
      //   this->output_len = i;
      //   loggerf(ERROR, "Failed to link IO to Signal %02i:%02i", module, signalId);
      // }
      loggerf(WARNING, "IO outside range (Port %02i:%02i:%02i)", module, conf.output[i].Node, conf.output[i].Adr);
      continue;
    }

    this->output[i] = U_IO(module, conf.output[i].Node, conf.output[i].Adr);
    for(int j = 0; j <= UNKNOWN; j++){
      this->output_stating[i].state[j].value = conf.stating[i].event[j];
    }
    struct s_node_adr out;
    out.Node = conf.output[i].Node;
    out.io = conf.output[i].Adr;

    Init_IO(Units[module], out, this);
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

    struct SignalSwitchLink link = {0,0,0};
    link.MSSw = conf.Switches[i].type;
    link.p.p = p;
    link.state = conf.Switches[i].state;

    this->Switches.push_back(link);
  }

  Units[module]->insertSignal(this);
  
  loggerf(DEBUG, "Create signal %02i:%02i, %s, block %02i:%02i", this->module, this->id, this->direction ? "Forward" : "Reverse", this->B->module, this->B->id);

  set(UNKNOWN);
}

Signal::~Signal(){
  _free(this->output);
  _free(this->output_stating);
}

void print_signal_state(Signal * Si, enum Rail_states state){
  loggerf(INFO, "%02i:%02i Sig %i  %i %s -> %i %s", Si->B->module, Si->B->id, Si->id, Si->state, rail_states_string[Si->state], state, rail_states_string[state]);
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

  for(auto i: this->Switches){
    if((i.MSSw && i.p.MSSw->state != i.state) || (!i.MSSw && i.p.Sw->state != i.state)){
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
    this->output[i]->w_state = this->output_stating[i].state[state];
  }

  Units[this->module]->io_out_changed |= 1;
}

void Signal::check(){
  loggerf(TRACE, "check_Signal %x", (unsigned int)this);
  // if(this->switches)
  this->state = this->B->state;
}
