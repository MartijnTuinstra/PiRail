#include "utils/mem.h"
#include "utils/logger.h"
#include "system.h"

#include "config/LayoutStructure.h"

#include "switchboard/manager.h"
#include "switchboard/station.h"
#include "switchboard/unit.h"

// Station ** stations;
// int stations_len;

using namespace switchboard;

const char * station_types_string[5] = {
  "STATION_PERSON",
  "STATION_CARGO",
  "STATION_YARD",
  "STATION_PERSON_YARD",
  "STATION_CARGO_YARD" 
};

Station::Station(int _module, int _id, struct configStruct_Station * conf){
  memset(this, 0, sizeof(Station));

  module = _module;
  id = _id;
  U = Units(module);

  uid = SwManager->addStation(this);

  type = (enum Station_types)conf->type;

  name = (char *)_calloc(conf->name_len + 1, char);
  strncpy(name, conf->name, conf->name_len);

  U->insertStation(this);

  if(conf->parent != 0xFF && U->St[conf->parent]){
    parent = U->St[conf->parent];
    U->St[conf->parent]->childs.push_back(this);
  }
  else if(conf->parent != 0xFF)
    loggerf(WARNING, "Failed to link station '%s' to parent %d", name, conf->parent);

  blocks_len = conf->nr_blocks;
  blocks = (Block **)_calloc(blocks_len, Block *);

  for(int i = 0; i < conf->nr_blocks; i++){
    blocks[i] = U->B[conf->blocks[i]];
    U->B[conf->blocks[i]]->station = this;
  }
}

Station::~Station(){
  if(switch_link){
    for(int k = 0; k <= switches_len; k++){
      if(switch_link[k])
        _free(switch_link[k]);
    }
    _free(switch_link);
  }

  _free(name);
  _free(blocks);
}

void Station::exportConfig(struct configStruct_Station * cfg){
  cfg->type = type;
  cfg->nr_blocks = blocks_len;
  cfg->name_len = strlen(name) + 1;
  // cfg->reserved;
  if (parent)
    cfg->parent = parent->id;
  else
    cfg->parent = 0xFF;

  cfg->blocks = (uint8_t *)_calloc(blocks_len, uint8_t);
  for(uint8_t i = 0; i < blocks_len; i++){
    cfg->blocks[i] = (uint8_t)blocks[i]->id;
  }
  cfg->name = (char *)_calloc(strlen(name) + 1, char);
  strcpy(cfg->name, name);
}

void Station::occupy(Train * T){
  if(parent)
    parent->occupiedChild = true;

  occupied = true;

  train = T;
}

void Station::release(){
  for(uint8_t i = 0; i < blocks_len; i++){
    if(!blocks[i])
      break;

    if(blocks[i]->blocked)
      return;
  }

  occupied = false;
  train = 0;

  if(parent)
    parent->releaseParent();
}

void Station::releaseParent(){
  for(auto st: childs){
    if(st->occupied){
      return;
    }
  }

  occupiedChild = false;
}

void Station::setStoppedTrain(bool stop){
  loggerf(TRACE, "Station %s setStoppedTrain %i", name, stop);
  stoppedTrain = stop;

  if(!parent)
    return;

  if(!stop){
    for(auto st: parent->childs){
      if(st->stoppedTrain)
        return;
    }
  }

  parent->stoppedTrain = stop;
}