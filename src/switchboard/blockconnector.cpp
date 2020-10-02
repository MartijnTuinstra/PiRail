#include "switchboard/manager.h"
#include "switchboard/blockconnector.h"
#include "utils/logger.h"
#include "utils/mem.h"

#include "websocket/stc.h"

using namespace switchboard;

BlockConnector::BlockConnector(uint8_t unit, uint16_t connector){
  memset(this, 0, sizeof(BlockConnector));

  this->unit = unit;
  this->connector = connector;
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

    BlockConnectorMatrix[A][B](this, BC, i, j);
  }

  Unit * tU = Units(this->unit);
  Unit * bcU = Units(BC->unit);
  tU->connection[this->connector - 1].unit = BC->unit;
  tU->connection[this->connector - 1].connector = BC->connector;
  tU->connection[this->connector - 1].crossover = crossover;
  bcU->connection[BC->connector - 1].unit = this->unit;
  bcU->connection[BC->connector - 1].connector = this->connector;
  bcU->connection[BC->connector - 1].crossover = crossover;

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
