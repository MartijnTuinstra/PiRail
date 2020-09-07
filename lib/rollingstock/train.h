#ifndef _INCLUDE_ROLLINGSTOCK_TRAIN_H
#define _INCLUDE_ROLLINGSTOCK_TRAIN_H

#include <stdint.h>
#include "switchboard/declares.h"
#include "rollingstock/declares.h"


struct __attribute__((__packed__)) train_comp {
  uint8_t type; //engine or car
  uint16_t id; //number in list
  void * p;  //pointer to types struct
};

struct train_composition {
  char * name;

  char nr_stock;
  struct train_comp * composition;
};

struct train_comp_ws;


class Train {
  public:
    uint16_t id;
    char * name;

    uint8_t nr_engines;
    Engine ** engines;

    uint8_t nr_stock;
    struct train_comp * composition; //One block memory for all nr_stocks

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
    uint8_t splitdetectables:1;

    Block * B;

    char timer;
    int timer_id;

    Train(char * name, int nr_stock, struct train_comp_ws * comps, uint8_t catagory, uint8_t save);
    ~Train();
    
    void setSpeed(uint16_t speed);
    void calcSpeed();
};

extern Train ** trains;
extern int trains_len;
extern struct train_composition ** trains_comp;
extern int trains_comp_len;


#define create_train_from_conf(t) new Train(t.name, t.nr_stock, t.composition, t.catagory, 1)

#endif
