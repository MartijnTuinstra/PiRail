#include "mem.h"
#include "logger.h"
#include "system.h"

#include "switchboard/station.h"
#include "switchboard/unit.h"

Station ** stations;
int stations_len;

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
  this->type = (enum Station_types)conf.type;

  this->name = (char *)_calloc(conf.name_len + 1, char);
  strncpy(this->name, conf.name, conf.name_len);

  Units[this->module]->insertStation(this);

  if(conf.parent != 0xFFFF && Units[this->module]->St[conf.parent]){
    this->parent = Units[this->module]->St[conf.parent];
    Units[this->module]->St[conf.parent]->childs.push_back(this);
  }
  else
    loggerf(WARNING, "Failed to link station '%s' to parent %d", this->name, conf.parent);

  this->blocks_len = conf.nr_blocks;
  this->blocks = (Block **)_calloc(this->blocks_len, Block *);

  for(int i = 0; i < conf.nr_blocks; i++){
    this->blocks[i] = U_B(module, conf.blocks[i]);
    U_B(module, conf.blocks[i])->station = this;
  }

  if(!stations){
    stations = (Station **)_calloc(1, Station *);
    stations_len = 1;
  }

  this->uid = find_free_index(stations, stations_len);

  stations[this->uid] = this;
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
