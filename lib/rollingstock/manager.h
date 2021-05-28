#ifndef _INCLUDE_ROLLINGSTOCK_MANAGER_H
#define _INCLUDE_ROLLINGSTOCK_MANAGER_H

#include <stdint.h>
#include "rollingstock/car.h"
#include "rollingstock/engine.h"
#include "rollingstock/trainset.h"
#include "rollingstock/train.h"

#include "utils/mem.h"

#include "config/RollingStructure.h"
#include "scheduler/scheduler.h"
#include "utils/dynArray.h"

namespace RollingStock {

struct DCCEngine {
  Engine * E[10];
  uint8_t nr_engines;
  uint8_t engineUsed:1;
  uint8_t uniqueEngine:7;
};

class Manager {
public:
  struct configStruct_Category * PassengerCatagories;
  uint16_t PassengerCatagories_length;
  struct configStruct_Category * CargoCatagories;
  uint16_t CargoCatagories_length;

  ::dynArray<Car *>       Cars;
  ::dynArray<Engine *>    Engines;
  ::dynArray<TrainSet *>  TrainSets;
  ::dynArray<Train *>     Trains;

  DCCEngine DCC[10000];

  char filename[100] = "";
  struct SchedulerEvent * continue_event = 0;

  Manager();
  ~Manager();

  void initScheduler();

  Car *       newCar(Car *);
  Engine *    newEngine(Engine *);
  TrainSet *  newTrainSet(TrainSet *);

  int addTrain(Train *);

  inline Car *       getCar(uint16_t i){
    return Cars[i];
  }
  inline Engine *    getEngine(uint16_t i){
    return Engines[i];
  }
  inline TrainSet *  getTrainSet(uint16_t i){
    return TrainSets[i];
  }
  inline Train * getTrain(uint16_t i){
    return Trains[i];
  }
  Engine *  getEngineDCC(uint16_t i);

  bool subDCCEngine(uint16_t i);
  void unsubDCCEngine(uint16_t i);
  void addDCC(Engine * E);
  void removeDCC(Engine * E);

  Train * getTrainDCC(uint16_t);

  void removeCar(Car *);
  void removeEngine(Engine *);
  void removeTrainSet(TrainSet *);
  void removeTrain(Train *);

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