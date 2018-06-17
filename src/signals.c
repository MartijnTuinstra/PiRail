#include "system.h"

#include "module.h"
#include "signals.h"
#include "logger.h"

void create_signal(Block * B, _Bool side, char length, char * addresses, char * states, char * flash){
  Signal * Z = _calloc(1, Signal);

  Z->B = B;
  if(side)
    B->NextSignal = Z;
  else
    B->PrevSignal = Z;

  Z->io = length;

  for(int i = 0; i<Z->io; i++){
    Z->adr[i] = addresses[i];
    Z->states[i] = states[i];
    Z->flash[i] = flash[i];
  }

  Z->id = find_free_index((void **)Units[B->module],&Units[B->module]->signal_len);
  Z->module = B->module;
  Units[B->module]->Sig[Z->id] = Z;
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
  loggerf(ERROR, "IMPLEMENT set_signal");
}
