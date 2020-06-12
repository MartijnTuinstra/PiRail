#include "switchboard/signals.h"

#include "system.h"
#include "mem.h"

#include "config_data.h"
#include "modules.h"
#include "logger.h"

Signal::Signal(uint8_t module, uint8_t blockId, uint16_t signalId, bool side, char output_len, struct s_IO_port_conf * output, struct s_IO_signal_event_conf * stating){
  memset(this, 0, sizeof(Signal));

  Block * B = U_B(module, blockId);
  
  this->B = B;
  if(side == NEXT)
    B->NextSignal = this;
  else
    B->PrevSignal = this;

  this->id = signalId;
  this->module = module;
  this->state = UNKNOWN;
  this->output_len = output_len;
  
  this->output = (IO_Port **)_calloc(output_len, IO_Port *);
  this->output_stating = (struct s_signal_stating *)_calloc(output_len, struct s_signal_stating);

  for(int i = 0; i<output_len; i++){
    if(Units[module]->Node[output[i].Node].io_ports <= output[i].Adr){
      if(this->output_len == output_len){
        this->output_len = i;
        loggerf(ERROR, "Failed to link IO to Signal %02i:%02i", module, signalId);
      }
      loggerf(WARNING, "IO outside range (Port %02i:%02i:%02i)", module, output[i].Node, output[i].Adr);
      continue;
    }
    this->output[i] = U_IO(module, output[i].Node, output[i].Adr);
    for(int j = 0; j <= UNKNOWN; j++){
      this->output_stating[i].state[j].value = stating[i].event[j];
    }
    struct s_node_adr out;
    out.Node = output[i].Node;
    out.io = output[i].Adr;

    Init_IO(Units[module], out, this);
  }

  if(Units[module]->Sig[this->id]){
    loggerf(WARNING, "Duplicate signal id %02i:%02i, overwriting ... ", module, this->id);
    delete Units[module]->Sig[this->id];
    Units[module]->Sig[this->id] = 0;
  }
  Units[B->module]->Sig[this->id] = this;
  
  char sidos = 0;
  if(side == NEXT)
    sidos = 'N';
  else
    sidos = 'P';
  
  loggerf(DEBUG, "Create signal %02i:%02i, side %c, block %02i:%02i", this->module, this->id, sidos, B->module, B->id);

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
  loggerf(TRACE, "set_signal %x, %i", (unsigned int)this, (unsigned int)state);
  if(this->state != state){
    print_signal_state(this, state);
    // Update state
    this->state = state;
    
    //Update IO
    for(int i = 0; i < this->output_len; i++){
      this->output[i]->w_state = this->output_stating[i].state[this->state];
    }
    Units[this->module]->io_out_changed |= 1;
  }
}

void Signal::check(){
  loggerf(TRACE, "check_Signal %x", (unsigned int)this);
  if(!this->B)
    return;

  if(this->B->NextSignal == this){
    if(this->B->dir == 0 || this->B->dir == 2 || this->B->dir == 3){
      if(this->B->Alg.next == 0){
        set(DANGER);
      }
      else{
        set(this->B->Alg.N[0]->state);
      }
    }
    else{
      if(this->B->Alg.prev == 0){
        set(DANGER);
      }
      else{
        set(this->B->Alg.P[0]->reverse_state);
      }
    }
  }
  else{
    if(this->B->dir == 1 || this->B->dir == 4 || this->B->dir == 6){
      if(this->B->Alg.next == 0){
        set(DANGER);
      }
      else{
        set(this->B->Alg.N[0]->state);
      }
    }
    else{
      if(this->B->Alg.prev == 0){
        set(DANGER);
      }
      else{
        set(this->B->Alg.P[0]->reverse_state);
      }
    }
  }
}
