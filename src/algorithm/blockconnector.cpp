#include "algorithm/blockconnector.h"
#include "switchboard/manager.h"
#include "switchboard/rail.h"
#include "switchboard/signals.h"

#include "config/ModuleConfig.h"

#include "utils/logger.h"

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

      loggerf(DEBUG, "Found blocked connector U%i C%i", C->unit, C->connector);

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

  if(A->unit == B->unit){
    _free(blockedConnectors);
    loggerf(WARNING, "Cannot connect two connectors on the same unit");
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

BlockConnectorSetup::BlockConnectorSetup(){
  time_t t = time(NULL);

  struct tm tm = *localtime(&t);
  sprintf(filename, "%s/%04d%02d%02d-%02d%02d%02d.bin", LayoutSetupBasePath, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

BlockConnectorSetup::BlockConnectorSetup(const char * _filename){
  strncpy(filename, _filename, 60);
}

int BlockConnectorSetup::load(BlockConnectors * Connectors){
  loggerf(INFO, "Load setup %s", filename);
  FILE * fp = fopen(filename, "rb");

  if(!fp){
    loggerf(ERROR, "Failed to open file '%s'", filename);
    return -1;
  }

  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  uint8_t * buffer = (uint8_t *)_calloc(fsize + 10, uint8_t);
  uint8_t * buffer_ptr = &buffer[0];
  fread(buffer, fsize, 1, fp);

  struct configStruct_ConnectorSetup setup;

  // uint8_t ** buf_ptr = (uint8_t **)&buffer;

  loggerf(INFO, " (%i - %i) < %i", buffer_ptr, buffer, fsize);
  for(uint8_t i = 0; (buffer_ptr - buffer) < fsize; i++){

    Config_read_ConnectorSetup(0, &setup, &buffer_ptr);

    Unit * U = Units(setup.unit);
    if(!U){
      loggerf(ERROR, "Unknown Unit (%i) in setup!! Aborting!!", setup.unit);
      return -2;
    }

    for(uint8_t j = 0; j < setup.connections; j++){
      struct configStruct_Connector connector = setup.connectors[j];

      if(U->connection[j].unit){
        if(!(U->connection[j].unit == connector.unit && U->connection[j].connector == connector.connector))
          loggerf(WARNING, "connection allready occupied");
        continue;
      }

      BlockConnector * A = 0;
      BlockConnector * B = 0;
      uint16_t idA = 0, idB = 0;

      loggerf(INFO, "load BlockConnector search A-%02i:%02i\tB-%02i:%02i", setup.unit, j+1, connector.unit, connector.connector);

      for(uint8_t k = 0; k < Connectors->size(); k++){ //auto Connector: Connectors){
        BlockConnector * C = Connectors->operator[](k);

        loggerf(INFO, "  searching %02i:%02i", C->unit, C->connector);

        if(C->unit == setup.unit && C->connector == j + 1){
          idA = k;
          A = C;
        }

        if(C->unit == connector.unit && C->connector == connector.connector){
          idB = k;
          B = C;
        }
      }

      if(!A || !B){
        loggerf(ERROR, "Cloud not find Connectors %c%c", !A ? 'A' : ' ', !B ? 'B' : ' ');
        return -3;
      }

      A->connect(B, connector.crossover);

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

    _free(setup.connectors);
  }

  fclose(fp);
  _free(buffer);

  return 1;
}

int BlockConnectorSetup::save(){
  uint8_t data[1024];
  uint8_t * dataptr = &data[0];

  for(uint8_t i = 0; i < SwManager->Units.size; i++){
    Unit * U = Units(i);
    if(!U)
      continue;
    loggerf(INFO, "Save setup unit %i, %x %x, %i", i, data, dataptr, (int)dataptr - (int)data);

    struct configStruct_ConnectorSetup setup;

    setup.unit = i;
    setup.connections = U->connections_len;

    setup.connectors = (struct configStruct_Connector *)_calloc(U->connections_len, struct configStruct_Connector);

    for(uint8_t j = 0; j < U->connections_len; j++){
      loggerf(INFO, "Saving connection %i %i %i", U->connection[j].unit, U->connection[j].connector, U->connection[j].crossover);
      setup.connectors[j].unit = U->connection[j].unit;
      setup.connectors[j].connector = U->connection[j].connector;
      setup.connectors[j].crossover = U->connection[j].crossover;
    }

    Config_write_ConnectorSetup(&setup, &dataptr);
    _free(setup.connectors);
  }

  FILE * fp = fopen(filename, "wb");

  if(!fp){
    loggerf(ERROR, "Failed to open file for setup saving");
    return -1;
  }

  fwrite(data, (int)dataptr - (int)data, 1, fp);

  fclose(fp);

  return 1;
}

}; // namespace