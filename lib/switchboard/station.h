#ifndef _INCLUDE_SWITCHBOARD_STATION_H 
#define _INCLUDE_SWITCHBOARD_STATION_H

#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"

enum Station_types {
  STATION_PERSON,
  STATION_CARGO,
  STATION_YARD,       //Yard for both Person and Cargo trains
  STATION_PERSON_YARD,//Yard for Person only
  STATION_CARGO_YARD  //Yard for Cargo only
};

extern const char * station_types_string[5];

class Station {
  public:
    int module;
    int id;
    int uid;
    char * name;

    uint8_t state;

    Block ** blocks;
    uint8_t blocks_len;

    uint16_t station_length;

    enum Station_types type;

    char switches_len;
    struct switch_link ** switch_link;

    Station * parent;
    std::vector<Station *> childs;

    // Station(int module, int id, char * name, char name_len, enum Station_types type, int len, uint8_t * blocks);
    Station(int module, int id, struct station_conf conf);
    ~Station();
};

extern Station ** stations;
extern int stations_len;

#endif