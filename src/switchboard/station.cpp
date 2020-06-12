#include "switchboard/station.h"
#include "mem.h"
#include "logger.h"
#include "system.h"

Station ** stations;
int stations_len;

Station::Station(int module, int id, char * name, char name_len, enum Station_types type, int len, uint8_t * blocks){
  memset(this, 0, sizeof(Station));

  this->module = module;
  this->id = id;
  this->type = type;

  this->name = (char *)_calloc(name_len+1, char);
  strncpy(this->name, name, name_len);

  // If block array is to small
  if(Units[this->module]->station_len <= this->id){
    loggerf(INFO, "Expand station len %i", Units[this->module]->station_len+8);
    Units[this->module]->St = (Station * *)_realloc(Units[this->module]->St, (Units[this->module]->station_len + 8), Station *);

    int i = Units[this->module]->station_len;
    for(; i < Units[this->module]->station_len+8; i++){
      Units[this->module]->St[i] = 0;
    }
    Units[this->module]->station_len += 8;
  }

  Units[this->module]->St[this->id] = this;

  this->blocks_len = len;
  this->blocks = (Block **)_calloc(this->blocks_len, Block *);

  for(int i = 0; i < len; i++){
    this->blocks[i] = U_B(module, blocks[i]);
    U_B(module, blocks[i])->station = this;
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
