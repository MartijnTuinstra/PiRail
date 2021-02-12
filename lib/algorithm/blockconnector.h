#ifndef _INCLUDE_ALGORITHM_BLOCKCONNECTOR_H
#define _INCLUDE_ALGORITHM_BLOCKCONNECTOR_H

#include <stdint.h>
#include "switchboard/blockconnector.h"

namespace Algorithm {
BlockConnectors find_connectors();
uint8_t * find_connectable(BlockConnectors * Connectors);
void connect_connectors(BlockConnectors * Connectors, uint8_t * blockedConnectors);

class BlockConnectorSetup {
public:
  char filename[61];

  BlockConnectorSetup();
  BlockConnectorSetup(const char * filename);

  int save();
  int load(BlockConnectors * Connectors);
};


}; // namespace

#endif