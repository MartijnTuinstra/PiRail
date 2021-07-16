#ifndef _INCLUDE_SWITCHBOARD_BLOCKCONNECTOR_H
#define _INCLUDE_SWITCHBOARD_BLOCKCONNECTOR_H

#include <vector>
#include "switchboard/declares.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"

class BlockConnector {
  public:
    uint8_t unit;
    uint8_t connector;
    uint8_t ports;
    Block * B[10];
    Switch * Sw[10];
    MSSwitch * MSSw[10];
    Signal * Sig[10];

  BlockConnector(uint8_t unit, uint16_t connector);

  inline void update(uint8_t port, Block * B){
    this->B[port - 1] = B;

    if(this->ports <= port)
      this->ports = port;
  };

  inline void update(uint8_t port, Switch * Sw){
    this->Sw[port - 1] = Sw;
    this->update(port, Sw->Detection);
  };

  inline void update(uint8_t port, MSSwitch * Sw){
    this->MSSw[port - 1] = Sw;
    this->update(port, Sw->Detection);
  };

  inline void update(uint8_t port, Signal * Sig){
    this->Sig[port - 1] = Sig;
  };

  void connect(BlockConnector * BC, bool crossover);
  void connectSignal(BlockConnector * BC, uint8_t port, bool crossover);
};

void BlockConnectBlockBlock(BlockConnector * A, BlockConnector * B, uint8_t portA, uint8_t portB);

void BlockConnectBlockSwitch(BlockConnector * A, BlockConnector * B, uint8_t portA, uint8_t portB);
void BlockConnectBlockMSSwitch(BlockConnector * A, BlockConnector * B, uint8_t portA, uint8_t portB);
void BlockConnectSwitchBlock(BlockConnector * A, BlockConnector * B, uint8_t portA, uint8_t portB);
void BlockConnectSwitchSwitch(BlockConnector * A, BlockConnector * B, uint8_t portA, uint8_t portB);
void BlockConnectSwitchMSSwitch(BlockConnector * A, BlockConnector * B, uint8_t portA, uint8_t portB);
void BlockConnectMSSwitchBlock(BlockConnector * A, BlockConnector * B, uint8_t portA, uint8_t portB);
void BlockConnectMSSwitchSwitch(BlockConnector * A, BlockConnector * B, uint8_t portA, uint8_t portB);
void BlockConnectMSSwitchMSSwitch(BlockConnector * A, BlockConnector * B, uint8_t portA, uint8_t portB);
void BlockConnectSignalBlock(BlockConnector * A, BlockConnector * B, uint8_t portA, uint8_t portB);
void BlockConnectSignalSwitch(BlockConnector * A, BlockConnector * B, uint8_t portA, uint8_t portB);
void BlockConnectSignalMSSwitch(BlockConnector * A, BlockConnector * B, uint8_t portA, uint8_t portB);

typedef void (*BlockConnectorMatrixFunc)(BlockConnector *, BlockConnector *, uint8_t, uint8_t);
extern BlockConnectorMatrixFunc BlockConnectorMatrix[4][3];

typedef std::vector<BlockConnector *> BlockConnectors;

#endif
