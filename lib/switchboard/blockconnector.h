#ifndef _INCLUDE_SWITCHBOARD_BLOCKCONNECTOR_H
#define _INCLUDE_SWITCHBOARD_BLOCKCONNECTOR_H

#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/signals.h"
#include "switchboard/unit.h"

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

  inline void update(uint8_t port, Block * B);
  inline void update(uint8_t port, Switch * Sw);
  inline void update(uint8_t port, MSSwitch * Sw);
  inline void update(uint8_t port, Signal * Sig);

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
BlockConnectors Algorithm_find_connectors();
uint8_t * Algorithm_find_connectable(BlockConnectors * Connectors);
void Algorithm_connect_connectors(BlockConnectors * Connectors, uint8_t * blockedConnectors);

#endif
