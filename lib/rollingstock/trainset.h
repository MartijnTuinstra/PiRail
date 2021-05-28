#ifndef _INCLUDE_ROLLINGSTOCK_TRAINSET_H
#define _INCLUDE_ROLLINGSTOCK_TRAINSET_H

#include <stdint.h>
#include "utils/mem.h"
#include "utils/logger.h"

#include "switchboard/declares.h"
#include "rollingstock/declares.h"
#include "rollingstock/manager.h"

#include "utils/dynArray.h"

#include "config/RollingStructure.h"


struct __attribute__((__packed__)) RollingStockLink {
  uint8_t type; //engine or car
  uint16_t id; //number in list
  void * p;  //pointer to types struct
};

struct TrainSetConfig {
  char * name;

  char nr_stock;
  struct RollingStockLink * composition;
};


class TrainSet {
  public:
    uint16_t id;
    char * name;

    dynArray<Engine *> * engines;

    uint8_t nr_stock;
    struct RollingStockLink * composition; //One block memory for all nr_stocks

    uint16_t length; //in mm

    uint16_t max_speed;
    uint16_t cur_speed;

    uint8_t type;

    uint8_t in_use:1;
    uint8_t control:2;
    uint8_t dir:1;
    uint8_t halt:2;
    uint8_t save:1;

    uint8_t detectables:7;
    uint8_t virtualDetection:1;

    TrainSet(struct configStruct_TrainSet);
    TrainSet(char *);
    TrainSet(char * name, int nr_stock, struct configStruct_TrainSetLink * comps, uint8_t catagory, uint8_t save);
    ~TrainSet();
    
    void setName(char *);
    void setComposition(int, struct configStruct_TrainSetLink *);

    void setSpeed(uint16_t speed);
    void calcSpeed();

    bool enginesUsed();
    void setEnginesUsed(bool, Train *);
};

#endif
