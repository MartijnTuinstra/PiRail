#ifndef _INCLUDE_ROLLINGSTOCK_CAR_H
#define _INCLUDE_ROLLINGSTOCK_CAR_H

#include <stdint.h>
#include "rollingstock/declares.h"
#include "rollingstock/functions.h"

class Car {
  public:
    uint16_t nr;
    uint8_t type;
    uint8_t control;
    uint8_t dir;
    uint8_t halt;
    uint16_t max_speed;

    struct train_funcs function[29];

    uint16_t length;   //in mm
    char * name;
    char * icon_path;

  Car(char * name,int nr, char * icon, char type, uint16_t length, uint16_t speed);
  ~Car();
};

extern Car ** cars;
extern int cars_len;

#define create_car_from_conf(c) new Car(c.name, c.nr, c.icon_path, c.type, c.length, c.max_speed)

#endif