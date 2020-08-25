
#include "mem.h"
#include "logger.h"
#include "com.h"

#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/station.h"
#include "switchboard/signals.h"
#include "switchboard/unit.h"

int unit_len;
Unit ** Units;

Unit::Unit(ModuleConfig * Config){
  memset(this, 0, sizeof(Unit));

  this->module = Config->header.module;
  if(this->module < unit_len){
    Units[this->module] = this;
  }
  else{
    loggerf(CRITICAL, "NEED TO EXPAND UNITS");
    return;
  }


  this->connections_len = Config->header.connections;
  if(this->connections_len > 5){
    loggerf(ERROR, "To many connection for module %i", this->module);
    this->connections_len = 5;
  }

  memset(this->connection, 0, sizeof(struct unit_connector));
  // this->connection = (Unit **)_calloc(Config->header.connections, Unit *);

  this->IO_Nodes = Config->header.IO_Nodes;
  this->block_len = Config->header.Blocks;
  this->switch_len = Config->header.Switches;
  this->msswitch_len = Config->header.MSSwitches;
  this->signal_len = Config->header.Signals;
  this->station_len = Config->header.Stations;

  this->Node = (IO_Node **)_calloc(this->IO_Nodes, IO_Node*);
  this->B = (Block **)_calloc(this->block_len, Block*);
  this->Sw = (Switch **)_calloc(this->switch_len, Switch*);
  this->MSSw = (MSSwitch **)_calloc(this->switch_len, MSSwitch*);
  this->Sig = (Signal **)_calloc(this->signal_len, Signal*);
  this->St = (Station **)_calloc(this->station_len, Station*);

  loggerf(DEBUG, "  Module nodes");

  for(int i = 0; i < Config->header.IO_Nodes; i++){
    new IO_Node(Units[this->module], Config->Nodes[i]);
  }

  loggerf(DEBUG, "  Module Block");

  
  for(int i = 0; i < Config->header.Blocks; i++){
    new Block(this->module, Config->Blocks[i]);
  }

  loggerf(DEBUG, "  Module Switch");

  for(int i = 0; i < Config->header.Switches; i++){
    new Switch(this->module, Config->Switches[i]);
  }

  loggerf(DEBUG, "  Module MSSwitch");

  for(int i = 0; i < Config->header.MSSwitches; i++){
    new MSSwitch(this->module, Config->MSSwitches[i]);
  }

  loggerf(DEBUG, "  Module Signals");
  
  for(int i = 0; i < Config->header.Signals; i++){
    new Signal(this->module, Config->Signals[i]);
  }

  loggerf(DEBUG, "  Module Stations");

  for(int i = 0; i < Config->header.Stations; i++){
    new Station(this->module, i, Config->Stations[i]);
  }

  loggerf(DEBUG, "  Module Layout");

  //Layout
  this->Layout_length = Config->Layout_length;
  this->Layout = (char *)_calloc(Config->Layout_length, char);
  memcpy(this->Layout, Config->Layout, Config->Layout_length);
}

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

  this->IO_Nodes = Nodes;
  this->Node = (IO_Node **)_calloc(this->IO_Nodes, IO_Node *);

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

  //Clear IO
  for(int j = 0; j < this->IO_Nodes; j++){
    delete this->Node[j];
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
  for(int j = 0; j < this->switch_len; j++){
    if(!this->Sw[j])
      continue;

    delete this->Sw[j];
    this->Sw[j] = 0;
  }
  _free(this->Sw);

  //clear Mods
  for(int j = 0; j < this->msswitch_len; j++){
    if(!this->MSSw[j])
        continue;

    delete this->MSSw[j];
    this->MSSw[j] = 0;
  }
  _free(this->MSSw);

  //clear Signals
  for(int j = 0; j < this->signal_len; j++){
    if(!this->Sig[j])
      continue;

    delete this->Sig[j];
    this->Sig[j] = 0;
  }
  _free(this->Sig);

  //clear Stations
  for(int j = 0; j < this->station_len; j++){
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

IO_Port * Unit::linkIO(Node_adr adr, void * pntr, enum e_IO_type type){
  loggerf(INFO, "Linking IO %02d:%02d", adr.Node, adr.io);
  this->Node[adr.Node]->io[adr.io]->link(pntr, type);
  return this->Node[adr.Node]->io[adr.io];
}

IO_Port * Unit::linkIO(struct s_IO_port_conf adr, void * pntr, enum e_IO_type type){
  Node_adr newadr = {adr.Node, adr.Adr};
  return this->linkIO(newadr, pntr, type);
}

IO_Port * Unit::IO(Node_adr adr){
  if(adr.Node >= this->IO_Nodes)
    return 0;
  else if(adr.io >= this->Node[adr.Node]->io_ports)
    return 0;

  return this->Node[adr.Node]->io[adr.io];
}

IO_Port * Unit::IO(struct s_IO_port_conf adr){
  Node_adr newadr = {adr.Node, adr.Adr};
  return this->IO(newadr);
}


void Unit::updateIO(int uart_filestream){
  struct COM_t tx;
  uint8_t check = 0;

  for(int n = 0; n < this->IO_Nodes; n++){
    IO_Node * N = this->Node[n];

    tx.data[0] = module;
    tx.data[1] = COMopc_SetAllOut;
    tx.data[2] = N->io_ports;

    bool data = 0;

    check = UART_CHECKSUM_SEED ^ tx.data[1] ^ tx.data[2];

    memset(&tx.data[3], 0, UART_COM_t_Length - 3);

    for(int io = 0; io < N->io_ports; io++){
      IO_Port * IO = N->io[io];

      if(IO->type == IO_Undefined)
        continue;

      // loggerf(WARNING, "Update io %02i:%02i:%02i %s (%i)", module, n, io, IO_event_string[U_IO(module, n, io)->type][U_IO(module, n, io)->w_state.value], U_IO(module, n, io)->w_state.value);
      if(IO->type <= IO_Output_PWM){
        IO->r_state.value = IO->w_state.value;
        tx.data[io/4 + 3] |= IO->w_state.value << ((io % 4) * 2);
        data = 1;
      }

      if(io%4 == 3)
        check ^= tx.data[io/4 + 2];

      if(IO->type == IO_Output && IO->w_state.output == IO_event_Pulse){ // Reset When pulsing output
        IO->w_state.output = IO_event_Low;
        IO->r_state.value = IO->w_state.value;
      }
    }
    tx.data[N->io_ports/4 + 3] = check;
    tx.length = N->io_ports/4 + 4;

    if(!data){
      loggerf(INFO, "No data");
      continue;
    }

    COM_Send(&tx);
  }
}


void Unit::link_all(){
  link_all_blocks(this);
  link_all_switches(this);
  link_all_msswitches(this);

  for(int i = 0; i < this->station_len; i++){
    if(this->St[i]->blocks && this->St[i]->blocks[0] && !this->St[i]->blocks[0]->path)
      new Path(this->St[i]->blocks[0]);
  }
}

// void Create_Unit(uint16_t M, uint8_t Nodes, char points){
//   new Unit(M, Nodes, points);
// }

// Unit * Clear_Unit(Unit * U){
//   delete U;
//   return 0;
// }
