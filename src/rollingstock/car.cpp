#include "rollingstock/car.h"
#include "train.h"

#include "utils/mem.h"
#include "utils/logger.h"
#include "system.h"


Car::Car(char * Name){
  memset(this, 0, sizeof(Car));

  loggerf(TRACE, "Create Car %s", Name);

  setName(Name);
}

Car::Car(struct configStruct_Car data){
  memset(this, 0, sizeof(Car));

  loggerf(TRACE, "Create Car %s", data.name);

  setName(data.name);
  setIconPath(data.icon_path);

  nr = data.nr;
  length = data.length;
  max_speed = data.max_speed;

  type = data.type;

  readFlags(data.flags);
}

Car::~Car(){
  loggerf(TRACE, "Destroy Car %s", name);
  _free(name);
  _free(icon_path);
}

void Car::setName(char * Name){
  if(name)
    _free(name);

  name = (char *)_calloc(strlen(Name), char);
  strcpy(name, Name);
}

void Car::setIconPath(char * path){
  if(icon_path)
    _free(icon_path);

  icon_path = (char *)_calloc(strlen(path) + 10, char);
  strcpy(icon_path, path);
}

void Car::readFlags(uint8_t Flags){
  // F_CAR_DETECTABLE
  detectable = (Flags & 0x01) ? true : false;
}