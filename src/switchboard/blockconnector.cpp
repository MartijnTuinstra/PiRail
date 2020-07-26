#include "switchboard/blockconnector.h"
#include "logger.h"
#include "mem.h"

#include "websocket/stc.h"

BlockConnector::BlockConnector(uint8_t unit, uint16_t connector){
  memset(this, 0, sizeof(BlockConnector));

  this->unit = unit;
  this->connector = connector;
};

inline void BlockConnector::update(uint8_t port, Block * B){
  this->B[port - 1] = B;

  if(this->ports <= port)
    this->ports = port;
};

inline void BlockConnector::update(uint8_t port, Switch * Sw){
  this->Sw[port - 1] = Sw;
  this->update(port, Sw->Detection);
};

inline void BlockConnector::update(uint8_t port, MSSwitch * Sw){
  this->MSSw[port - 1] = Sw;
  this->update(port, Sw->Detection);
};

inline void BlockConnector::update(uint8_t port, Signal * Sig){
  this->Sig[port - 1] = Sig;
};

void BlockConnector::connect(BlockConnector * BC, bool crossover){
  for(uint8_t i = 0; i < this->ports; i++){
    uint8_t A = (this->Sw[i] != 0) + (this->MSSw[i] != 0) * 2;
    uint8_t j = (crossover * (this->ports - 1)) + (crossover * -i) + (!crossover * i);
    uint8_t B = (BC->Sw[j] != 0) + (BC->MSSw[j] != 0) * 2;

    if(this->Sig[i])
      BlockConnectorMatrix[3][B](this, BC, i, j);

    if(BC->Sig[j])
      BlockConnectorMatrix[3][A](BC, this, j, i);

    loggerf(INFO, "MATRIX %i<>%i", A, B);
    BlockConnectorMatrix[A][B](this, BC, i, j);
  }

  Units[this->unit]->connection[this->connector - 1] = Units[BC->unit];
  Units[BC->unit]->connection[BC->connector - 1] = Units[this->unit];

  WS_stc_Partial_Layout(this->unit);
  WS_stc_Partial_Layout(BC->unit);
}

BlockConnectorMatrixFunc BlockConnectorMatrix[4][3] = {
  /*Blocks -> x*/
  {BlockConnectBlockBlock, BlockConnectBlockSwitch, BlockConnectBlockMSSwitch},
  /*Switch -> x*/
  {BlockConnectSwitchBlock, BlockConnectSwitchSwitch, BlockConnectSwitchMSSwitch},
  /*MSSwitch -> x*/
  {BlockConnectMSSwitchBlock, BlockConnectMSSwitchSwitch, BlockConnectMSSwitchMSSwitch},
  /*Signal -> x*/
  {BlockConnectSignalBlock, BlockConnectSignalSwitch, BlockConnectSignalMSSwitch}
};

void BlockConnectBlockBlock(BlockConnector * A, BlockConnector * B, uint8_t portA, uint8_t portB){
  Block * bA = A->B[portA];
  Block * bB = B->B[portB];

  if(bA->next.type == RAIL_LINK_C){
    bA->next.module = bB->module;
    bA->next.id = bB->id;
    bA->next.type = RAIL_LINK_R;
  }
  else{
    bA->prev.module = bB->module;
    bA->prev.id = bB->id;
    bA->prev.type = RAIL_LINK_R;
  }

  if(bB->next.type == RAIL_LINK_C){
    bB->next.module = bA->module;
    bB->next.id = bA->id;
    bB->next.type = RAIL_LINK_R;
  }
  else{
    bB->prev.module = bA->module;
    bB->prev.id = bA->id;
    bB->prev.type = RAIL_LINK_R;
  }
}

void BlockConnectBlockSwitch(BlockConnector * A, BlockConnector * B, uint8_t portA, uint8_t portB){
  Block * bA = A->B[portA];
  Switch * bB = B->Sw[portB];

  enum link_types switchsideB;

  if(bB->str.type == RAIL_LINK_C){
    switchsideB = RAIL_LINK_s;
    bB->str.module = bA->module;
    bB->str.id = bA->id;
    bB->str.type = RAIL_LINK_R;
  }
  else if(bB->div.type == RAIL_LINK_C){
    switchsideB = RAIL_LINK_s;
    bB->div.module = bA->module;
    bB->div.id = bA->id;
    bB->div.type = RAIL_LINK_R;
  }
  else{ // App
    switchsideB = RAIL_LINK_S;
    bB->str.module = bA->module;
    bB->str.id = bA->id;
    bB->str.type = RAIL_LINK_R;
  }

  if(bA->next.type == RAIL_LINK_C){
    bA->next.module = bB->module;
    bA->next.id = bB->id;
    bA->next.type = switchsideB;
  }
  else{
    bA->prev.module = bB->module;
    bA->prev.id = bB->id;
    bA->prev.type = switchsideB;
  }
}
void BlockConnectBlockMSSwitch(BlockConnector * A, BlockConnector * B, uint8_t portA, uint8_t portB){
  // Block * bA = A->B[portA];
  // Block * bB = B->B[portB];

  loggerf(ERROR, "TODO, implement BlockConnectBlockMSSwitch");
}
void BlockConnectSwitchBlock(BlockConnector * A, BlockConnector * B, uint8_t portA, uint8_t portB){
  BlockConnectBlockSwitch(B, A, portB, portA);
  // Switch * bA = A->Sw[portA];
  // Block * bB = B->B[portB];

  // enum link_types switchsideA;

  // if(bA->str.type == RAIL_LINK_C){
  //   switchsideA = RAIL_LINK_s;
  //   bA->str.module = bB->module;
  //   bA->str.id = bB->id;
  //   bA->str.type = RAIL_LINK_R;
  // }
  // else if(bA->div.type == RAIL_LINK_C){
  //   switchsideA = RAIL_LINK_s;
  //   bA->div.module = bB->module;
  //   bA->div.id = bB->id;
  //   bA->div.type = RAIL_LINK_R;
  // }
  // else{ // App
  //   switchsideA = RAIL_LINK_S;
  //   bA->str.module = bB->module;
  //   bA->str.id = bB->id;
  //   bA->str.type = RAIL_LINK_R;
  // }

  // if(bB->next.type == RAIL_LINK_C){
  //   bB->next.module = bA->module;
  //   bB->next.id = bA->id;
  //   bB->next.type = switchsideA;
  // }
  // else{
  //   bB->prev.module = bA->module;
  //   bB->prev.id = bA->id;
  //   bB->prev.type = switchsideA;
  // }
}

void BlockConnectSwitchSwitch(BlockConnector * A, BlockConnector * B, uint8_t portA, uint8_t portB){
  Switch * bA = A->Sw[portA];
  Switch * bB = B->Sw[portB];

  struct rail_link * linkA;
  struct rail_link * linkB;

  enum link_types switchsideA;
  enum link_types switchsideB;

  if(bA->str.type == RAIL_LINK_C){
    switchsideA = RAIL_LINK_s;
    linkA = &bA->str;
  }
  else if(bA->div.type == RAIL_LINK_C){
    switchsideA = RAIL_LINK_s;
    linkA = &bA->div;
  }
  else{ // App
    switchsideA = RAIL_LINK_S;
    linkA = &bA->app;
  }

  if(bB->str.type == RAIL_LINK_C){
    switchsideB = RAIL_LINK_s;
    linkB = &bB->str;
  }
  else if(bB->div.type == RAIL_LINK_C){
    switchsideB = RAIL_LINK_s;
    linkB = &bB->div;
  }
  else{ // App
    switchsideB = RAIL_LINK_S;
    linkB = &bB->app;
  }

  linkA->module = bB->module;
  linkA->id = bB->id;
  linkA->type = switchsideB;

  linkB->module = bA->module;
  linkB->id = bA->id;
  linkB->type = switchsideA;
}
void BlockConnectSwitchMSSwitch(BlockConnector * A, BlockConnector * B, uint8_t portA, uint8_t portB){
  // Block * bA = A->B[portA];
  // Block * bB = B->B[portB];
  loggerf(ERROR, "TODO, implement BlockConnectSwitchMSSwitch");
}
void BlockConnectMSSwitchBlock(BlockConnector * A, BlockConnector * B, uint8_t portA, uint8_t portB){
  BlockConnectBlockMSSwitch(B, A, portB, portA);
}
void BlockConnectMSSwitchSwitch(BlockConnector * A, BlockConnector * B, uint8_t portA, uint8_t portB){
  BlockConnectSwitchMSSwitch(B, A, portB, portA);
}
void BlockConnectMSSwitchMSSwitch(BlockConnector * A, BlockConnector * B, uint8_t portA, uint8_t portB){
  // Block * bA = A->B[portA];
  // Block * bB = B->B[portB];
  loggerf(ERROR, "TODO, implement BlockConnectMSSwitchMSSwitch");
}
void BlockConnectSignalBlock(BlockConnector * A, BlockConnector * B, uint8_t portA, uint8_t portB){
  Signal * bA = A->Sig[portA];
  Block * bB = B->B[portB];

  bA->block_link.module = bB->module;
  bA->block_link.id = bB->id;
  bA->block_link.type = RAIL_LINK_R;

  bA->B = bB;
  bA->state = bB->addSignal(bA);
}
void BlockConnectSignalSwitch(BlockConnector * A, BlockConnector * B, uint8_t portA, uint8_t portB){
  Signal * bA = A->Sig[portA];
  Block * bB = B->B[portB];
  Switch * SwB = B->Sw[portB];

  bA->block_link.module = bB->module;
  bA->block_link.id = bB->id;
  bA->block_link.type = RAIL_LINK_R;

  bA->B = bB;
  bA->state = bB->addSignal(bA);

  Switch * tmpSw = 0;
  MSSwitch * tmpMSSw = 0;

  struct SignalSwitchLink * link = (struct SignalSwitchLink *)_calloc(1, struct SignalSwitchLink);
  link->MSSw = 0;
  link->p.p = SwB;
  SwB->addSignal(bA);

  if(SwB->div.type == RAIL_LINK_C && SwB->div.id == portB){
    link->state = 1;
  }
  else if(SwB->str.type == RAIL_LINK_C && SwB->str.id == portB){ // str
    link->state = 0;
  }
  else{
    loggerf(INFO, "BlockConnectSignalSwitch link not found");
  }

  bA->Switches.push_back(link);

  void * prevptr = SwB;

  if(SwB->app.type == RAIL_LINK_s)
    tmpSw = SwB->app.p.Sw;
  else if(SwB->app.type == RAIL_LINK_MA || SwB->app.type == RAIL_LINK_MB)
    tmpMSSw = SwB->app.p.MSSw;

  while(tmpSw || tmpMSSw){
    link = (struct SignalSwitchLink *)_calloc(1, struct SignalSwitchLink);

    if(tmpSw){
      link->MSSw = 0;
      link->p.p = tmpSw;
      tmpSw->addSignal(bA);

      if(tmpSw->div.p.p == prevptr){
        link->state = 1;
      }
      else if(tmpSw->str.p.p == prevptr){ // str
        link->state = 0;
      }
      else{
        loggerf(INFO, "BlockConnectSignalSwitch link not found");
      }

      prevptr = tmpSw;
      if(tmpSw->app.type == RAIL_LINK_s){
        tmpSw = tmpSw->app.p.Sw;
        tmpMSSw = 0;
      }
      else if(tmpSw->app.type == RAIL_LINK_MA || tmpSw->app.type == RAIL_LINK_MB){
        tmpMSSw = tmpSw->app.p.MSSw;
        tmpSw = 0;
      }
    }
    else{
      link->MSSw = 1;
      link->p.p = tmpMSSw;
      tmpMSSw->addSignal(bA);

      bool statefound = false;

      for(uint8_t i = 0; i < tmpMSSw->state_len; i++){
        if(tmpMSSw->sideA[i].p.p == prevptr || tmpMSSw->sideB[i].p.p == prevptr){
          link->state = i;
          statefound = true;
          break;
        }
      }

      if(!statefound){
        loggerf(INFO, "BlockConnectSignalSwitch link not found");
      }

      tmpMSSw = 0;
    }

    bA->Switches.push_back(link);
  }
}
void BlockConnectSignalMSSwitch(BlockConnector * A, BlockConnector * B, uint8_t portA, uint8_t portB){
  // Block * bA = A->B[portA];
  // Block * bB = B->B[portB];
  loggerf(ERROR, "TODO, implement BlockConnectSignalMSSwitch");
}

// typedef std::vector<BlockConnector *> BlockConnectors;
BlockConnectors Algorithm_find_connectors(){
  BlockConnectors Connectors;

  for(uint8_t i = 0;i < unit_len;i++){
    if(!Units[i])
      continue;

    if(!Units[i]->on_layout)
      continue;

    for(uint16_t j = 0;j < Units[i]->block_len; j++){
      if(!Units[i]->B[j])
        continue;

      Block * B = Units[i]->B[j];

      if(B->next.type == RAIL_LINK_C || B->prev.type == RAIL_LINK_C){
        uint8_t connector;
        uint8_t port;
        if(B->next.type == RAIL_LINK_C){
          connector = B->next.module;
          port = B->next.id;
        }
        else{
          connector = B->prev.module;
          port = B->prev.id;
        }

        bool found = false;
        for(auto Connector: Connectors){
          if(Connector->unit == i && Connector->connector == connector){
            found = true;

            Connector->update(port, B);
            break;
          }
        }
        if(!found){
          auto newconnector = new BlockConnector(i, connector);
          newconnector->update(port, B);

          Connectors.push_back(newconnector);
        }
      }
    } // Block Loop

    for(uint16_t j = 0;j < Units[i]->switch_len; j++){
      if(!Units[i]->Sw[j])
        continue;

      Switch * Sw = Units[i]->Sw[j];

      if(Sw->app.type == RAIL_LINK_C || Sw->div.type == RAIL_LINK_C || Sw->str.type == RAIL_LINK_C){
        uint8_t connector;
        uint8_t port;
        if(Sw->app.type == RAIL_LINK_C){
          connector = Sw->app.module;
          port = Sw->app.id;
        }
        else if(Sw->div.type == RAIL_LINK_C){
          connector = Sw->div.module;
          port = Sw->div.id;
        }
        else{ // str
          connector = Sw->str.module;
          port = Sw->str.id;
        }

        bool found = false;
        for(auto Connector: Connectors){
          if(Connector->unit == i && Connector->connector == connector){
            found = true;

            Connector->update(port, Sw);
            break;
          }
        }
        if(!found){
          auto newconnector = new BlockConnector(i, connector);
          newconnector->update(port, Sw);

          Connectors.push_back(newconnector);
        }
      }
    } // Switch loop

    for(uint16_t j = 0;j < Units[i]->signal_len; j++){
      if(!Units[i]->Sig[j])
        continue;

      Signal * Sig = Units[i]->Sig[j];

      if(Sig->block_link.type == RAIL_LINK_C){
        uint8_t connector = Sig->block_link.module;
        uint8_t port = Sig->block_link.id;

        loggerf(INFO,"found Signal %i:%i\n",i,j);
        bool found = false;
        for(auto Connector: Connectors){
          if(Connector->unit == i && Connector->connector == connector){
            found = true;

            Connector->update(port, Sig);
            break;
          }
        }
        if(!found){
          auto newconnector = new BlockConnector(i, connector);
          newconnector->update(port, Sig);

          Connectors.push_back(newconnector);
        }
      }
    } // Signal loop
  } // unit loop

  return Connectors;
}

uint8_t * Algorithm_find_connectable(BlockConnectors * Connectors){
  uint8_t * blockedConnector = (uint8_t *)_calloc(2, uint8_t);
  uint8_t found = 0;

  for(uint8_t j = 0; j < Connectors->size(); j++){ //auto Connector: Connectors){
    loggerf(INFO, "New Connector %i", j);
    BlockConnector * C = Connectors->operator[](j);

    for(uint8_t i = 0; i < 10; i++){
      if(!C->B[i])
        continue;

      loggerf(INFO, "Check Block %i:%i", C->unit, C->B[i]->id);

      if(!C->B[i]->blocked)
        continue;

      loggerf(INFO, "Found blocked connector %i:%i", C->unit, C->connector);

      blockedConnector[found] = j;
      found++;
      break;
    }

    if(found > 1)
      break;
  }

  if(found <= 1){
    _free(blockedConnector);
    return 0;
  }

  return blockedConnector;
}

void Algorithm_connect_connectors(BlockConnectors * Connectors, uint8_t * blockedConnectors){
  printf("Try and connect");
  BlockConnector * A = Connectors->operator[](blockedConnectors[0]);
  BlockConnector * B = Connectors->operator[](blockedConnectors[1]);

  if(A->ports != B->ports){
    _free(blockedConnectors);
    loggerf(ERROR, "Connectors do not have the same number of ports");
    return;
  }

  bool straight = false;
  bool crossover = false;
  // Straight Connection 1<>1, 2<>2, 3<>3 ...
  for(uint8_t i = 0; i < A->ports; i++){
    if(A->B[i]->blocked && B->B[i]->blocked){
      straight = true;
      break;
    }
  }

  // Crossover connection 1<>n, 2<>n-1, ..., n<>1 
  if(!straight){
    for(uint8_t i = 0, j = B->ports - 1; i < A->ports; i++, j--){
      if(A->B[i]->blocked && B->B[j]->blocked){
        crossover = true;
        break;
      }
    }
  }

  if(!straight && !crossover){
    loggerf(ERROR, "Failed to connect");
    _free(blockedConnectors);
    return;
  }

  A->connect(B, crossover);

  delete A;
  delete B;

  if(blockedConnectors[0] < blockedConnectors[1]){
    Connectors->erase(Connectors->begin() + blockedConnectors[0]);
    Connectors->erase(Connectors->begin() + blockedConnectors[1] - 1);
  }
  else{
    Connectors->erase(Connectors->begin() + blockedConnectors[0]);
    Connectors->erase(Connectors->begin() + blockedConnectors[1]); 
  }

  _free(blockedConnectors);
}
