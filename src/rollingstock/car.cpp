#include "rollingstock/car.h"
#include "train.h"

#include "mem.h"
#include "logger.h"
#include "system.h"


Car ** cars;
int cars_len = 0;

Car::Car(char * name,int nr, char * icon, char type, uint16_t length, uint16_t speed){
  Car * Z = (Car *)_calloc(1, Car);

  Z->name = name;
  Z->icon_path = icon;

  Z->nr = nr;
  Z->length = length;
  Z->max_speed = speed;

  Z->type = type;

  int index = find_free_index(cars, cars_len);

  cars[index] = Z;

  loggerf(DEBUG, "Car \"%s\" created",name);
}

Car::~Car(){
  _free(this->name);
  _free(this->icon_path);
}
