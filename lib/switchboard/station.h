#ifndef _INCLUDE_SWITCHBOARD_STATION_H 
#define _INCLUDE_SWITCHBOARD_STATION_H

#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"

enum Station_types {
  STATION_PERSON,
  STATION_CARGO,
  STATION_PERSON_YARD,//Yard for Person only
  STATION_CARGO_YARD, //Yard for Cargo only
  STATION_YARD        //Yard for both Person and Cargo trains
};

class Station {
  public:
    int module;
    int id;
    int uid;
    char * name;

    uint8_t state;

    Block ** blocks;
    int blocks_len;

    enum Station_types type;

    char switches_len;
    struct switch_link ** switch_link;

    Station(int module, int id, char * name, char name_len, enum Station_types type, int len, uint8_t * blocks);
    ~Station();
};

extern Station ** stations;
extern int stations_len;

#endif