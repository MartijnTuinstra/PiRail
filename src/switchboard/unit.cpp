
#include "utils/mem.h"
#include "utils/logger.h"
#include "uart/RNetTX.h"

#include "config/LayoutStructure.h"

#include "switchboard/manager.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/station.h"
#include "switchboard/signals.h"
#include "switchboard/unit.h"
#include "switchboard/polarityGroup.h"

#include "path.h"

Unit::Unit(ModuleConfig * Config){
  memset(this, 0, sizeof(Unit));

  const struct configStruct_Unit * ConfigHeader = Config->getHeader();

  module = ConfigHeader->Module;

  loggerf(INFO, "Loading Unit %i", module);

  switchboard::SwManager->addUnit(this);

  connections_len = ConfigHeader->Connections;
  if(connections_len > 5){
    loggerf(ERROR, "To many connection for module %i", module);
    connections_len = 5;
  }

  memset(connection, 0, sizeof(struct unit_connector));
  // this->connection = (Unit **)_calloc(ConfigHeader->connections, Unit *);

  IO_Nodes     = ConfigHeader->IO_Nodes;
  block_len    = ConfigHeader->Blocks;
  switch_len   = ConfigHeader->Switches;
  msswitch_len = ConfigHeader->MSSwitches;
  signal_len   = ConfigHeader->Signals;
  station_len  = ConfigHeader->Stations;

  loggerf(INFO, "INIT Unit %i - %d, %d, %d, %d, %d, %d", ConfigHeader->Module, ConfigHeader->IO_Nodes, ConfigHeader->Blocks, ConfigHeader->Switches, ConfigHeader->MSSwitches, ConfigHeader->Signals, ConfigHeader->Stations);

  this->Node = (IO_Node **)_calloc(this->IO_Nodes, IO_Node*);
  this->B = (Block **)_calloc(this->block_len, Block*);
  this->Sw = (Switch **)_calloc(this->switch_len, Switch*);
  this->MSSw = (MSSwitch **)_calloc(this->msswitch_len, MSSwitch*);
  this->Sig = (Signal **)_calloc(this->signal_len, Signal*);
  this->St = (Station **)_calloc(this->station_len, Station*);

  loggerf(DEBUG, "  Module nodes");

  for(int i = 0; i < ConfigHeader->IO_Nodes; i++){
    new IO_Node(this, &Config->Nodes[i]);
  }

  loggerf(DEBUG, "  Module Block");

  
  for(int i = 0; i < ConfigHeader->Blocks; i++){
    new Block(this->module, &Config->Blocks[i]);
  }

  loggerf(DEBUG, "  Module Polarity Group");
  
  for(int i = 0; i < ConfigHeader->PolarityGroup; i++)
    new PolarityGroup(module, &Config->PolarityGroup[i]);

  loggerf(DEBUG, "  Module Switch");

  for(int i = 0; i < ConfigHeader->Switches; i++){
    new Switch(this->module, &Config->Switches[i]);
  }

  loggerf(DEBUG, "  Module MSSwitch");

  for(int i = 0; i < ConfigHeader->MSSwitches; i++){
    new MSSwitch(this->module, &Config->MSSwitches[i]);
  }

  loggerf(DEBUG, "  Module Signals");
  
  for(int i = 0; i < ConfigHeader->Signals; i++){
    new Signal(this->module, &Config->Signals[i]);
  }

  loggerf(DEBUG, "  Module Stations");

  for(int i = 0; i < ConfigHeader->Stations; i++){
    new Station(this->module, i, &Config->Stations[i]);
  }

  loggerf(DEBUG, "  Module Layout");

  //Layout
  Layout_length = Config->Layout->LayoutLength;
  Layout = (char *)_calloc(Config->Layout->LayoutLength, char);
  memcpy(Layout, Config->Layout->Layout, Config->Layout->LayoutLength);
}


Unit::Unit(uint16_t M, uint8_t Nodes, char points){
  memset(this, 0, sizeof(Unit));

  switchboard::SwManager->addUnit(this);

  module = M;

  connections_len = points;

  IO_Nodes = Nodes;
  Node = (IO_Node **)_calloc(IO_Nodes, IO_Node *);

  block_len = 8;
  B = (Block **)_calloc(block_len, Block*);

  switch_len = 8;
  Sw = (Switch **)_calloc(switch_len, Switch*);

  msswitch_len = 8;
  MSSw = (MSSwitch **)_calloc(switch_len, MSSwitch*);

  signal_len = 8;
  Sig = (Signal **)_calloc(signal_len, Signal*);

  station_len = 8;
  St = (Station **)_calloc(station_len, Station*);
}

Unit::~Unit(){
  loggerf(INFO, "Clearing module %i", module);

  //Clear IO
  for(int j = 0; j < IO_Nodes; j++){
    delete Node[j];
  }
  _free(Node);

  //clear Segments
  for(int j = 0; j < block_len; j++){
    if(!B[j])
      continue;

    delete B[j];
    B[j] = 0;
  }
  _free(B);

  //clear Switches
  for(int j = 0; j < switch_len; j++){
    if(!Sw[j])
      continue;

    delete Sw[j];
    Sw[j] = 0;
  }
  _free(Sw);

  //clear Mods
  for(int j = 0; j < msswitch_len; j++){
    if(!MSSw[j])
        continue;

    delete MSSw[j];
    MSSw[j] = 0;
  }
  _free(MSSw);

  //clear Signals
  for(int j = 0; j < signal_len; j++){
    if(!Sig[j])
      continue;

    delete Sig[j];
    Sig[j] = 0;
  }
  _free(Sig);

  //clear Stations
  for(int j = 0; j < station_len; j++){
    if(!St[j])
      continue;

    delete St[j];
    St[j] = 0;
  }
  _free(St);

  _free(raw);
  _free(Layout);
}

void Unit::insertBlock(Block * B){
  if(block_len <= B->id){
    // loggerf(INFO, "Expand block len %i", block_len+8);
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
    // loggerf(INFO, "Expand block len %i", switch_len+8);
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
    loggerf(INFO, "Expand msswitch len %i", this->msswitch_len+8);
    this->MSSw = (MSSwitch **)_realloc(this->MSSw, (this->msswitch_len + 8), MSSwitch *);

    memset(&this->MSSw[this->msswitch_len], 0, 8 * sizeof(MSSwitch *));

    this->msswitch_len += 8;
  }
  // If id is already in use
  if(this->MSSw[MSSw->id]){
      loggerf(ERROR, "Duplicate MSSwitch %i", MSSw->id);
      delete this->MSSw[MSSw->id];
  }
  this->MSSw[MSSw->id] = MSSw;
}

void Unit::insertStation(Station * St){
  // If block array is to small
  if(this->station_len <= St->id){
    loggerf(INFO, "Expand station len %i", this->station_len+8);
    this->St = (Station * *)_realloc(this->St, (this->station_len + 8), Station *);

    int i = this->station_len;
    for(; i < this->station_len+8; i++){
      this->St[i] = 0;
    }
    this->station_len += 8;
  }

  this->St[St->id] = St;
}

void Unit::insertSignal(Signal * Sig){
  if(this->signal_len <= Sig->id){
    loggerf(INFO, "Expand signals len %i", this->signal_len+8);
    this->Sig = (Signal * *)_realloc(this->St, (this->signal_len + 8), Signal *);

    int i = this->signal_len;
    for(; i < this->signal_len+8; i++){
      this->Sig[i] = 0;
    }
    this->signal_len += 8;
  }

  if(this->Sig[Sig->id]){
    loggerf(WARNING, "Duplicate signal id %02i:%02i, overwriting ... ", module, Sig->id);
    delete this->Sig[Sig->id];
    this->Sig[Sig->id] = 0;
  }

  this->Sig[Sig->id] = Sig;
}


Block * Unit::registerDetection(Switch * Sw, uint16_t blockID){
  // Add msswitch to detection block 
  if(block_len > blockID && B[blockID]){
    B[blockID]->addSwitch(Sw);
    return B[blockID];
  }
  
  loggerf(WARNING, "SWITCH %i:%i has no detection block %i", Sw->module, Sw->id, blockID);

  return 0;
}

Block * Unit::registerDetection(MSSwitch * MSSw, uint16_t blockID){
  // Add msswitch to detection block 
  if(block_len > blockID && B[blockID]){

    if(B[blockID]->MSSw)
      loggerf(WARNING, "Block %02i:%02i has duplicate msswitch, overwritting ...", B[blockID]->module, B[blockID]->id);

    B[blockID]->MSSw = MSSw;
    return B[blockID];
  }

  loggerf(WARNING, "MSSWITCH %i:%i has no detection block %i", MSSw->module, MSSw->id, blockID);

  return 0;
}




IO_Port * Unit::linkIO(Node_adr adr, void * pntr, enum e_IO_type type){
  if (adr.Node >= IO_Nodes){
    loggerf(WARNING, "Failed to link IO %02i:%02i", adr.Node, adr.io);
    return 0;
  }
  Node[adr.Node]->io[adr.io]->link(pntr, type);
  return Node[adr.Node]->io[adr.io];
}

IO_Port * Unit::linkIO(struct configStruct_IOport adr, void * pntr, enum e_IO_type type){
  if (adr.Node >= IO_Nodes){
    loggerf(WARNING, "Failed to link IO %02i:%02i", adr.Node, adr.Port);
    return 0;
  }
  Node[adr.Node]->io[adr.Port]->link(pntr, type);
  return Node[adr.Node]->io[adr.Port];
}

IO_Port * Unit::IO(Node_adr adr){
  if(adr.Node >= this->IO_Nodes)
    return 0;
  else if(adr.io >= this->Node[adr.Node]->io_ports)
    return 0;

  return this->Node[adr.Node]->io[adr.io];
}

IO_Port * Unit::IO(struct configStruct_IOport adr){
  Node_adr newadr = {adr.Node, adr.Port};
  return this->IO(newadr);
}


void Unit::updateIO(){
  for(int n = 0; n < IO_Nodes; n++){
    IO_Node * N = Node[n];

    if(n == 0){
      continue;
    }

    if(!N)
      continue;

    COM_change_Output(N);
  }
}


void Unit::link_blocks(){
  for(int i = 0; i < block_len; i++){
    if(!B[i])
      continue;

    Block * tB = B[i];

    tB->next.link();
    tB->prev.link();

    // tB->next.p.p = rail_link_pointer(tB->next);
    // tB->prev.p.p = rail_link_pointer(tB->prev);

    if(!tB->path){
      if(tB->next.type == RAIL_LINK_R && tB->next.p.B->type == NOSTOP)
        new Path(tB);

      else if(tB->prev.type == RAIL_LINK_R && tB->prev.p.B->type == NOSTOP)
        new Path(tB);

      else if(tB->prev.type == RAIL_LINK_MA_inside || tB->prev.type == RAIL_LINK_MB_inside)
        new Path(tB);

      else if(tB->prev.type == RAIL_LINK_E || tB->next.type == RAIL_LINK_E)
        new Path(tB);
    }

  }
}

void Unit::link_switches(){
  for(int i = 0; i < switch_len; i++){
    if(!Sw[i])
      continue;

    Switch * tSw = Sw[i];

    tSw->app.p.p = rail_link_pointer(tSw->app);
    tSw->str.p.p = rail_link_pointer(tSw->str);
    tSw->div.p.p = rail_link_pointer(tSw->div);

    if(tSw->app.type == RAIL_LINK_R && !tSw->app.p.B->path)
      new Path(tSw->app.p.B);
    if(tSw->str.type == RAIL_LINK_R && !tSw->str.p.B->path)
      new Path(tSw->str.p.B);
    if(tSw->div.type == RAIL_LINK_R && !tSw->div.p.B->path)
      new Path(tSw->div.p.B);
  }
}

void Unit::link_msswitches(){
  for(int i = 0; i < msswitch_len; i++){
    if(!MSSw[i])
      continue;

    MSSwitch * tMSSw = MSSw[i];

    for(int s = 0; s < tMSSw->state_len; s++){
      tMSSw->sideA[s].p.p = rail_link_pointer(tMSSw->sideA[s]);
      tMSSw->sideB[s].p.p = rail_link_pointer(tMSSw->sideB[s]);

      if(tMSSw->sideA[s].type == RAIL_LINK_R && !tMSSw->sideA[s].p.B->path)
        new Path(tMSSw->sideA[s].p.B);
      if(tMSSw->sideB[s].type == RAIL_LINK_R && !tMSSw->sideB[s].p.B->path)
        new Path(tMSSw->sideB[s].p.B);
    }
  }
}


void Unit::link_rest(){
  for(int i = 0; i < signal_len; i++)
    Sig[i]->map();

  for(int i = 0; i < station_len; i++){
    if(St[i]->blocks_len > 0 && St[i]->blocks)
      if(St[i]->blocks[0])
        if(!St[i]->blocks[0]->path)
          new Path(St[i]->blocks[0]);
  }
}

void Unit::map_all(){
  for(uint8_t i = 0; i < block_len; i++){
    loggerf(TRACE, "mapping %2i:%2i", B[i]->module, B[i]->id);
    B[i]->next.mapPolarity(B[i], NEXT);
    B[i]->prev.mapPolarity(B[i], PREV);
  }
}

// void Create_Unit(uint16_t M, uint8_t Nodes, char points){
//   new Unit(M, Nodes, points);
// }

// Unit * Clear_Unit(Unit * U){
//   delete U;
//   return 0;
// }
