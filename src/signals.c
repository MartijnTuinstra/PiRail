#include "signals.h"

#include "system.h"
#include "mem.h"

#include "config_data.h"
#include "modules.h"
#include "logger.h"

void Create_Signal(uint8_t module, uint8_t blockId, uint16_t signalId, _Bool side, char output_len, struct s_IO_port_conf * output, struct s_IO_signal_event_conf * stating){
  Signal * Z = _calloc(1, Signal);

  Block * B = U_B(module, blockId);
  
  Z->B = B;
  if(side == NEXT)
    B->NextSignal = Z;
  else
    B->PrevSignal = Z;

  Z->id = signalId;
  Z->module = module;
  Z->state = UNKNOWN;
  Z->output_len = output_len;
  
  Z->output = _calloc(output_len, IO_Port *);
  Z->output_stating = _calloc(output_len, struct s_signal_stating);

  for(int i = 0; i<output_len; i++){
    if(Units[module]->Node[output[i].Node].io_ports <= output[i].Adr){
      if(Z->output_len == output_len){
        Z->output_len = i;
        loggerf(ERROR, "Failed to link IO to Signal %02i:%02i", module, signalId);
      }
      loggerf(WARNING, "IO outside range (Port %02i:%02i:%02i)", module, output[i].Node, output[i].Adr);
      continue;
    }
    Z->output[i] = U_IO(module, output[i].Node, output[i].Adr);
    for(int j = 0; j <= UNKNOWN; j++){
      Z->output_stating[i].state[j].value = stating[i].event[j];
    }
    struct s_node_adr out;
    out.Node = output[i].Node;
    out.io = output[i].Adr;

    Init_IO(Units[module], out, Z);
  }

  if(Units[module]->Sig[Z->id]){
    loggerf(WARNING, "Duplicate signal id %02i:%02i, overwriting ... ", module, Z->id);
    Units[module]->Sig[Z->id] = Clear_Signal(Units[module]->Sig[Z->id]);
  }
  Units[B->module]->Sig[Z->id] = Z;
  
  char sidos = 0;
  if(side == NEXT)
    sidos = 'N';
  else
    sidos = 'P';
  
  loggerf(DEBUG, "Create signal %02i:%02i, side %c, block %02i:%02i", Z->module, Z->id, sidos, B->module, B->id);

  set_signal(Z, UNKNOWN);
}

void * Clear_Signal(Signal * Sig){

  _free(Sig->output);
  _free(Sig->output_stating);

  _free(Sig);

  return 0;
}

void print_signal_state(Signal * Si, enum Rail_states state){
  loggerf(INFO, "%02i:%02i Sig %i  %i %s -> %i %s", Si->B->module, Si->B->id, Si->id, Si->state, rail_states_string[Si->state], state, rail_states_string[state]);
}

void set_signal(Signal * Si, enum Rail_states state){
  loggerf(TRACE, "set_signal %x, %i", (unsigned int)Si, (unsigned int)state);
  if(Si->state != state){
    print_signal_state(Si, state);
    // Update state
    Si->state = state;
    
    //Update IO
    for(int i = 0; i < Si->output_len; i++){
      Si->output[i]->w_state = Si->output_stating[i].state[Si->state];
    }
    Units[Si->module]->io_out_changed |= 1;
  }
}

void check_Signal(Signal * Si){
  loggerf(TRACE, "check_Signal %x", (unsigned int)Si);
  if(!Si->B)
    return;

  if(Si->B->NextSignal == Si){
    if(Si->B->dir == 0 || Si->B->dir == 2 || Si->B->dir == 3){
      if(Si->B->Alg.next == 0){
        set_signal(Si, DANGER);
      }
      else{
        set_signal(Si, Si->B->Alg.N[0]->state);
      }
    }
    else{
      if(Si->B->Alg.prev == 0){
        set_signal(Si, DANGER);
      }
      else{
        set_signal(Si, Si->B->Alg.P[0]->reverse_state);
      }
    }
  }
  else{
    if(Si->B->dir == 1 || Si->B->dir == 4 || Si->B->dir == 6){
      if(Si->B->Alg.next == 0){
        set_signal(Si, DANGER);
      }
      else{
        set_signal(Si, Si->B->Alg.N[0]->state);
      }
    }
    else{
      if(Si->B->Alg.prev == 0){
        set_signal(Si, DANGER);
      }
      else{
        set_signal(Si, Si->B->Alg.P[0]->reverse_state);
      }
    }
  }
}
