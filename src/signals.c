#include "signals.h"

#include "system.h"
#include "mem.h"

#include "config_data.h"
#include "module.h"
#include "logger.h"

void create_signal(uint8_t module, uint8_t blockId, uint16_t signalId, _Bool side, char output_len, struct s_IO_port_conf * output, struct s_IO_signal_event_conf * stating){
  Signal * Z = _calloc(1, Signal);

  Block * B = U_B(module, blockId);
  
  Z->B = B;
  if(side == NEXT)
    B->NextSignal = Z;
  else
    B->PrevSignal = Z;

  Z->id = signalId;
  Z->module = module;
  Z->state = PROCEED;
  Z->output_len = output_len;
  
  Z->output = _calloc(output_len, IO_Port *);
  Z->output_stating = _calloc(output_len, struct _signal_stating);

  for(int i = 0; i<Z->output_len; i++){
    Z->output[i] = U_IO(module, output[i].Node, output[i].Adr);
    for(int j = 0; j <= UNKNOWN; j++){
      Z->output_stating[i].state[j] = stating[i].event[j];
    }
  }

  if(Units[module]->Sig[Z->id]){
    loggerf(WARNING, "Duplicate signal id %02i:%02i, overwriting ... ", module, Z->id);
    Units[module]->Sig[Z->id] = clear_Signal(Units[module]->Sig[Z->id]);
  }
  Units[B->module]->Sig[Z->id] = Z;
  
  char sidos = 0;
  if(side == NEXT)
    sidos = 'N';
  else
    sidos = 'P';
  
  loggerf(WARNING, "Create signal %02i:%02i, side %c, block %02i:%02i", Z->module, Z->id, sidos, B->module, B->id);
}

void * clear_Signal(Signal * Sig){

  _free(Sig->output);
  _free(Sig->output_stating);

  _free(Sig);

  return 0;
}

void signal_create_states(char io, enum Rail_states state, char * list, ...){
  va_list args;
  va_start(args, list);

  printf("signal_create_states for state: %i\n", state);

  for(int i = 0; i<io; i++){
    list[state] |= (va_arg(args, int) & 1) << i;
  }

  va_end(args);
}

void set_signal(Signal * Si, enum Rail_states state){
  char out[200];
  if(Si->state != state){
    sprintf(out, "%02i:%02i Sig %i:%i ", Si->B->module, Si->B->id, Si->module, Si->id);
    if(state == BLOCKED || state == DANGER)
      sprintf(out, "%sDANGER ", out);
    else if(state == RESTRICTED)
      sprintf(out, "%sRESTRICTED ", out);
    else if(state == CAUTION)
      sprintf(out, "%sCAUTION ", out);
    else if(state == PROCEED)
      sprintf(out, "%sPROCEED ", out);
    else
      sprintf(out, "%sSTATE %i  ", out, state);
    loggerf(ERROR, "%s", out);
    Si->state = state;
    #warning "IMPLEMENT set_signal"
    // loggerf(WARNING, "IMPLEMENT set_signal");
  }
}
