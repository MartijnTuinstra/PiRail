#include "algorithm/blockconnector.h"
#include "switchboard/manager.h"
#include "switchboard/rail.h"

#include "logger.h"

namespace Algorithm {
using namespace switchboard;

// typedef std::vector<BlockConnector *> BlockConnectors;
BlockConnectors find_connectors(){
  BlockConnectors Connectors;

  for(uint8_t i = 0;i < SwManager->Units.size;i++){
    Unit * U = Units(i);
    if(!U)
      continue;

    if(!U->on_layout)
      continue;

    for(uint16_t j = 0;j < U->block_len; j++){
      if(!U->B[j])
        continue;

      Block * B = U->B[j];

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

    for(uint16_t j = 0;j < U->switch_len; j++){
      if(!U->Sw[j])
        continue;

      Switch * Sw = U->Sw[j];

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

    for(uint16_t j = 0;j < U->signal_len; j++){
      if(!U->Sig[j])
        continue;

      Signal * Sig = U->Sig[j];

      if(Sig->block_link.type == RAIL_LINK_C){
        uint8_t connector = Sig->block_link.module;
        uint8_t port = Sig->block_link.id;

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

uint8_t * find_connectable(BlockConnectors * Connectors){
  uint8_t * blockedConnector = (uint8_t *)_calloc(2, uint8_t);
  uint8_t found = 0;

  for(uint8_t j = 0; j < Connectors->size(); j++){ //auto Connector: Connectors){
    BlockConnector * C = Connectors->operator[](j);

    for(uint8_t i = 0; i < 10; i++){
      if(!C->B[i])
        continue;

      if(!C->B[i]->blocked)
        continue;

      loggerf(DEBUG, "Found blocked connector %i:%i", C->unit, C->connector);

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

void connect_connectors(BlockConnectors * Connectors, uint8_t * blockedConnectors){
  loggerf(DEBUG, "connect_connectors");
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

int load_setup(char * filename, BlockConnectors * Connectors){
  FILE * fp = fopen(filename, "rb");

  if(!fp){
    loggerf(ERROR, "Failed to open file '%s'", filename);
    return -1;
  }

  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char * buffer = (char *)_calloc(fsize + 10, char);
  char * buffer_ptr = buffer;
  fread(buffer, fsize, 1, fp);

  // uint8_t ** buf_ptr = (uint8_t **)&buffer;

  for(uint8_t i = 0; (buffer_ptr - buffer) < fsize; i++){
    uint8_t unit = buffer_ptr[0];

    Unit * U = Units(unit);
    if(!U){
      loggerf(ERROR, "Unknown Unit (%i) in setup!! Aborting!!", unit);
      return -2;
    }

    uint8_t connections = buffer_ptr[1];
    buffer_ptr += 2;

    for(uint8_t j = 0; j < connections; j++){
      uint8_t newunit = buffer_ptr[j * 3];
      uint8_t newconnector = buffer_ptr[j * 3 + 1];
      bool crossover = buffer_ptr[j * 3 + 2];

      if(U->connection[j].unit){
        if(!(U->connection[j].unit == newunit && U->connection[j].connector == newconnector))
          loggerf(ERROR, "connection allready occupied");
        continue;
      }

      BlockConnector * A = 0;
      BlockConnector * B = 0;
      uint16_t idA = 0, idB = 0;

      for(uint8_t k = 0; k < Connectors->size(); k++){ //auto Connector: Connectors){
        BlockConnector * C = Connectors->operator[](k);

        if(C->unit == unit && C->connector == j + 1){
          idA = k;
          A = C;
        }

        if(C->unit == newunit && C->connector == newconnector){
          idB = k;
          B = C;
        }
      }

      if(!A || !B){
        loggerf(ERROR, "Cloud not find Connectors");
        return -2;
      }

      A->connect(B, crossover);

      delete A;
      delete B;

      if(idA < idB){
        Connectors->erase(Connectors->begin() + idA);
        Connectors->erase(Connectors->begin() + idB - 1);
      }
      else{
        Connectors->erase(Connectors->begin() + idA);
        Connectors->erase(Connectors->begin() + idB); 
      }
    }

    buffer_ptr += 3 * connections;
  }

  return 1;
}

}; // namespace