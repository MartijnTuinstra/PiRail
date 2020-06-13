#include "rollingstock/car.h"
#include "train.h"

#include "mem.h"
#include "logger.h"
#include "system.h"


Car ** cars;
int cars_len = 0;

Car::Car(char * name,int nr, char * icon, char type, uint16_t length, uint16_t speed){
  loggerf(DEBUG, "Create Car %s", name);

  memset(this, 0, sizeof(Car));

  this->name = name;
  this->icon_path = icon;

  this->nr = nr;
  this->length = length;
  this->max_speed = speed;

  this->type = type;

  int index = find_free_index(cars, cars_len);

  cars[index] = this;

  loggerf(DEBUG, "Car \"%s\" created",name);
}

Car::~Car(){
  _free(this->name);
  _free(this->icon_path);
}
