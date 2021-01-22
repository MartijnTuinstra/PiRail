#ifndef _INCLUDE_ROLLINGSTOCK_MANAGER_H
#define _INCLUDE_ROLLINGSTOCK_MANAGER_H

#include <stdint.h>
#include "rollingstock/car.h"
#include "rollingstock/engine.h"
#include "rollingstock/train.h"
#include "rollingstock/railtrain.h"

#include "utils/mem.h"

#include "config/RollingStructure.h"
#include "scheduler/scheduler.h"
#include "utils/dynArray.h"

namespace RollingStock {

class Manager {
public:
  ::dynArray<struct configStruct_Category> PassengerCatagories;
  ::dynArray<struct configStruct_Category> CargoCatagories;

  ::dynArray<Car *>       Cars;
  ::dynArray<Engine *>    Engines;
  ::dynArray<Train *>     Trains;
  ::dynArray<RailTrain *> RailTrains;

  Engine * DCC[10000];

  char * filename;
  struct SchedulerEvent * continue_event;

  Manager();
  ~Manager();

  void initScheduler();

  Car *       newCar(Car *);
  Engine *    newEngine(Engine *);
  Train *     newTrain(Train *);

  int addRailTrain(RailTrain *);

  inline Car *       getCar(uint16_t i){
    return Cars[i];
  }
  inline Engine *    getEngine(uint16_t i){
    return Engines[i];
  }
  inline Train *     getTrain(uint16_t i){
    return Trains[i];
  }
  inline RailTrain * getRailTrain(uint16_t i){
    return RailTrains[i];
  }

  inline Engine *  getEngineDCC(uint16_t i){
    return DCC[i];
  }
  RailTrain * getRailTrainDCC(uint16_t);

  void removeCar(Car *);
  void removeEngine(Engine *);
  void removeTrain(Train *);
  void removeRailTrain(RailTrain *);

  void moveEngine(Engine *, uint16_t);

  void print();

  void loadFile(char *);
  void writeFile();

  void clear();
};

int read();
void write();
int load(const char * filename);
void unload();

};

extern RollingStock::Manager * RSManager;

#endif