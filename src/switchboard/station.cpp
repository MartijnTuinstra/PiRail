#include "utils/mem.h"
#include "utils/logger.h"
#include "system.h"

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

Station::Station(int module, int id, struct station_conf conf){
  memset(this, 0, sizeof(Station));

  /*
  uint8_t type;
  uint8_t nr_blocks;
  uint8_t name_len;
  uint16_t parent;
  uint8_t * blocks;
  char * name;
  */

  this->module = module;
  this->id = id;
  U = Units(module);

  uid = SwManager->addStation(this);

  this->type = (enum Station_types)conf.type;

  this->name = (char *)_calloc(conf.name_len + 1, char);
  strncpy(this->name, conf.name, conf.name_len);

  U->insertStation(this);

  if(conf.parent != 0xFFFF && U->St[conf.parent]){
    this->parent = U->St[conf.parent];
    U->St[conf.parent]->childs.push_back(this);
  }
  else if(conf.parent != 0xFFFF)
    loggerf(WARNING, "Failed to link station '%s' to parent %d", this->name, conf.parent);

  this->blocks_len = conf.nr_blocks;
  this->blocks = (Block **)_calloc(this->blocks_len, Block *);

  for(int i = 0; i < conf.nr_blocks; i++){
    this->blocks[i] = U->B[conf.blocks[i]];
    U->B[conf.blocks[i]]->station = this;
  }
}

Station::~Station(){
  if(this->switch_link){
    for(int k = 0; k <= this->switches_len; k++){
      if(this->switch_link[k])
        _free(this->switch_link[k]);
    }
    _free(this->switch_link);
  }

  _free(this->name);
  _free(this->blocks);
}

void Station::occupy(RailTrain * T){
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