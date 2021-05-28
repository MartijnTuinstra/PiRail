#ifndef _INCLUDE_SWITCHBOARD_STATION_H 
#define _INCLUDE_SWITCHBOARD_STATION_H

#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"

#include "rollingstock/train.h"

#include "config/LayoutStructure.h"

enum Station_types {
  STATION_PERSON,
  STATION_CARGO,
  STATION_YARD,       //Yard for both Person and Cargo trains
  STATION_PERSON_YARD,//Yard for Person only
  STATION_CARGO_YARD  //Yard for Cargo only
};

extern const char * station_types_string[5];

struct configStruct_Station;

class Station {
  public:
    int module;
    int id;
    int uid;
    Unit * U;
    char * name;

    uint8_t state; // REMOVE
    bool occupied;       // There is a train on the station blocks
    bool occupiedChild;  // There is a child station which is occupied
    bool stoppedTrain;   // There is a train stopped on the station blocks

    Train * train;

    Block ** blocks;
    uint8_t blocks_len;

    uint16_t station_length;

    enum Station_types type;

    char switches_len;
    struct switch_link ** switch_link;

    Station * parent;
    std::vector<Station *> childs;

    Station(int mdoule, int id, struct configStruct_Station *);
    ~Station();

    void exportConfig(struct configStruct_Station *);

    void occupy(Train * T);
    void release();
    void releaseParent();

    void setStoppedTrain(bool);
};

extern Station ** stations;
extern int stations_len;

#endif