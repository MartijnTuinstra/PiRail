#ifndef _INCLUDE_ROLLINGSTOCK_CAR_H
#define _INCLUDE_ROLLINGSTOCK_CAR_H

#include <stdint.h>

#include "rollingstock/declares.h"
#include "rollingstock/functions.h"

#include "config/RollingStructure.h"

class Car {
  public:
    uint16_t nr;
    uint16_t id;

    uint8_t type;
    uint8_t control;
    uint8_t dir;
    uint8_t halt;
    uint16_t max_speed;

    bool detectable;

    struct train_funcs function[29];

    uint16_t length;   //in mm
    char * name;
    char * icon_path;

  Car(char *);
  Car(struct configStruct_Car);
  ~Car();

  void setName(char * name);
  void setIconPath(char *);
  void readFlags(uint8_t);
};

#endif
