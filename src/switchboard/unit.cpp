
#include "mem.h"
#include "logger.h"

#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/station.h"
#include "switchboard/signals.h"
#include "IO.h"

int unit_len;
Unit ** Units;

Unit::Unit(uint16_t M, uint8_t Nodes, char points){
  memset(this, 0, sizeof(Unit));

  if(M < unit_len){
    Units[M] = this;
  }
  else{
    loggerf(CRITICAL, "NEED TO EXPAND UNITS");
    return;
  }

  this->module = M;

  this->connections_len = points;
  this->connection = (Unit **)_calloc(points, Unit *);

  this->IO_Nodes = Nodes;
  this->Node = (IO_Node *)_calloc(this->IO_Nodes, IO_Node);

  this->block_len = 8;
  this->B = (Block **)_calloc(this->block_len, Block*);

  this->switch_len = 8;
  this->Sw = (Switch **)_calloc(this->switch_len, Switch*);

  this->msswitch_len = 8;
  this->MSSw = (MSSwitch **)_calloc(this->switch_len, MSSwitch*);

  this->signal_len = 8;
  this->Sig = (Signal **)_calloc(this->signal_len, Signal*);

  this->station_len = 8;
  this->St = (Station **)_calloc(this->station_len, Station*);
}

Unit::~Unit(){
  loggerf(INFO, "Clearing module %i", this->module);

  //Clear unit connections array
  _free(this->connection);

  //Clear IO
  for(int j = 0; j < this->IO_Nodes; j++){
    for(int k =0; k < this->Node[j].io_ports; k++){
      _free(this->Node[j].io[k]);
    }
    _free(this->Node[j].io);
  }
  _free(this->Node);

  //clear Segments
  for(int j = 0; j < this->block_len; j++){
    if(!this->B[j])
      continue;

    delete this->B[j];
    this->B[j] = 0;
  }
  _free(this->B);

  //clear Switches
  for(int j = 0; j <= this->switch_len; j++){
    if(!this->Sw[j])
      continue;

    delete this->Sw[j];
    this->Sw[j] = 0;
  }
  _free(this->Sw);

  //clear Mods
  for(int j = 0;j<=this->msswitch_len;j++){
    if(!this->MSSw[j])
        continue;

    delete this->MSSw[j];
    this->MSSw[j] = 0;
  }
  _free(this->MSSw);

  //clear Signals
  for(int j = 0;j<=this->signal_len;j++){
    if(!this->Sig[j])
      continue;

    delete this->Sig[j];
    this->Sig[j] = 0;
  }
  _free(this->Sig);

  //clear Stations
  for(int j = 0;j<=this->station_len;j++){
    if(!this->St[j])
      continue;

    delete this->St[j];
    this->St[j] = 0;
  }
  _free(this->St);

  _free(this->raw);
  _free(this->Layout);
}

void Unit::insertBlock(Block * B){
  if(this->block_len <= B->id){
    loggerf(INFO, "Expand block len %i", block_len+8);
    this->B = (Block **)_realloc(this->B, (block_len + 8), Block *);

    memset(&this->B[block_len], 0, 8 * sizeof(Block *));

    block_len += 8;
  }
  
  // If id is already in use
  if(this->B[B->id]){
      loggerf(ERROR, "Duplicate segment %i", B->id);
      delete this->B[B->id];
  }
  this->B[B->id] = B;
}

void Unit::insertSwitch(Switch * Sw){
  if(switch_len <= Sw->id){
    loggerf(INFO, "Expand block len %i", switch_len+8);
    this->Sw = (Switch **)_realloc(this->Sw, (switch_len + 8), Switch *);

    memset(&this->Sw[switch_len], 0, 8 * sizeof(Switch *));

    switch_len += 8;
  }
  
  // If id is already in use
  if(this->Sw[Sw->id]){
      loggerf(ERROR, "Duplicate Switch %i", Sw->id);
      delete this->Sw[Sw->id];
  }
  this->Sw[Sw->id] = Sw;
}

void Unit::insertMSSwitch(MSSwitch * MSSw){
  if(this->msswitch_len <= MSSw->id){
    loggerf(INFO, "Expand block len %i", this->msswitch_len+8);
    this->MSSw = (MSSwitch **)_realloc(this->MSSw, (this->switch_len + 8), MSSwitch *);

    memset(&this->Sw[this->switch_len], 0, 8 * sizeof(MSSwitch *));

    this->switch_len += 8;
  
    // If id is already in use
    if(this->MSSw[MSSw->id]){
        loggerf(ERROR, "Duplicate MSSwitch %i", MSSw->id);
        delete this->MSSw[MSSw->id];
    }
    this->MSSw[MSSw->id] = MSSw;
  }
}


void Create_Unit(uint16_t M, uint8_t Nodes, char points){
  new Unit(M, Nodes, points);
}

Unit * Clear_Unit(Unit * U){
  delete U;
  return 0;
}
