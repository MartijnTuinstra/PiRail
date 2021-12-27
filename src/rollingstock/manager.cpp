#include "rollingstock/manager.h"

#include "config/RollingConfig.h"

#include "utils/logger.h"
#include "utils/mem.h"

#include "system.h"

RollingStock::Manager * RSManager;

namespace RollingStock {

Manager::Manager(){
  memset(DCC, 0, 10000 * sizeof(struct DCCEngine));
}

void Manager::initScheduler(){
  // Do the train continue check every 2 seconds
  continue_event = scheduler->addEvent("Train_continue", {2, 0});
  continue_event->function = &Train_ContinueCheck;
}

void Manager::clear(){
  memset(RSManager->DCC, 0, 10000 * sizeof(struct DCCEngine));

  for(uint8_t i = 0; i < PassengerCatagories_length; i++)
    _free(PassengerCatagories[i].name);
  for(uint8_t i = 0; i < CargoCatagories_length; i++)
    _free(CargoCatagories[i].name);

  _free(PassengerCatagories);//.clear();
  PassengerCatagories_length = 0;
  _free(CargoCatagories);//.clear();
  CargoCatagories_length = 0;

  Trains.clear();
  TrainSets.clear();
  Cars.clear();
  Engines.clear();
}

Manager::~Manager(){
  scheduler->removeEvent(continue_event);

  log("RollingManager", INFO, "Clearing trains memory");

  clear();

  // if(trains_comp){
  //   for(int i = 0;i<trains_comp_len;i++){
  //     if(trains_comp[i]){
  //       trains_comp[i]->name = (char *)_free(trains_comp[i]->name);
  //       trains_comp[i]->composition = (struct train_comp *)_free(trains_comp[i]->composition);
  //       trains_comp[i] = (struct train_composition *)_free(trains_comp[i]);
  //     }
  //   }
  //   trains_comp = (train_composition **)_free(trains_comp);
  // }

  if(SYS)
    SYS->trains_loaded = 0;
}

void Manager::loadFile(char * f){
  strncpy(filename, f, 99);

  // Allocation Basic Space
  // trains_comp = (struct train_composition **)_calloc(10, struct train_composition *);
  // trains_comp_len = 10;
  
  scheduler->enableEvent(continue_event);

  auto config = RollingConfig(filename);

  config.read();


  loggerf(INFO, "Initialize Catagories");
  PassengerCatagories = (struct configStruct_Category *)_calloc(config.header.PersonCatagories, struct configStruct_Category);
  PassengerCatagories_length = config.header.PersonCatagories;
  CargoCatagories     = (struct configStruct_Category *)_calloc(config.header.CargoCatagories, struct configStruct_Category);
  CargoCatagories_length = config.header.CargoCatagories;

  memcpy(PassengerCatagories, config.P_Cat, sizeof(struct configStruct_Category) * config.header.PersonCatagories);
  memcpy(CargoCatagories,     config.C_Cat, sizeof(struct configStruct_Category) * config.header.PersonCatagories);


  loggerf(INFO, "Initialize Engines");
  for(int i = 0; i < config.header.Engines; i++){
    newEngine(new Engine(config.Engines[i]));
  }

  
  loggerf(INFO, "Initialize Cars");
  for(int i = 0; i < config.header.Cars; i++){
    newCar(new Car(config.Cars[i]));
    // create_car_from_conf(config.Cars[i]);
  }

  
  loggerf(INFO, "Initialize Trains");
  for(int i = 0; i < config.header.TrainSets; i++){
    newTrainSet(new TrainSet(config.TrainSets[i]));
  }

  if(SYS)
    SYS->trains_loaded = 1;
}

Car * Manager::newCar(Car * C){
  C->id = Cars.push_back(C);

  return C;
}

Engine * Manager::newEngine(Engine * E){
  E->id = Engines.push_back(E);
  addDCC(E);

  return E;
}

TrainSet * Manager::newTrainSet(TrainSet * T){
  T->id = TrainSets.push_back(T);

  return T;
}

int Manager::addTrain(Train * T){
  return Trains.push_back(T);
}

Engine *  Manager::getEngineDCC(uint16_t i){
  auto DCCE = &DCC[i];

  loggerf(INFO, "getEngineDCC %i %i, %i, %i", i, DCCE->engineUsed, DCCE->nr_engines, DCCE->uniqueEngine);

  if(DCCE->engineUsed)
    return DCCE->E[DCCE->uniqueEngine];
  else
    return 0;
}

bool Manager::subDCCEngine(uint16_t i){
  auto E = getEngine(i);

  auto DCCE = &DCC[E->DCC_ID];

  loggerf(INFO, "subDCCEngine (%i) %i, %i, %i", E->DCC_ID, DCCE->engineUsed, DCCE->nr_engines, DCCE->uniqueEngine);

  if(DCCE->engineUsed)
    return false;
  
  uint8_t j = 0;
  while(j < DCCE->nr_engines){
    if(DCCE->E[j] == E){
      DCCE->engineUsed = true;
      DCCE->uniqueEngine = j;
      return true;
    }
    j++;
  }
  return false;
}

void Manager::unsubDCCEngine(uint16_t i){
  auto E = getEngine(i);

  auto DCCE = &DCC[E->DCC_ID];
  
  loggerf(INFO, "unsubDCCEngine (%i), %i, %i, %i", E->DCC_ID, DCCE->engineUsed, DCCE->nr_engines, DCCE->uniqueEngine);

  if(!DCCE->engineUsed)
    return;

  if(DCCE->E[DCCE->uniqueEngine] != E)
    return;

  DCCE->engineUsed = 0;
}

void Manager::addDCC(Engine * E){
  auto DCCE = &DCC[E->DCC_ID];

  loggerf(INFO, "addDCC (%i) %i, %i, %i", E->DCC_ID, DCCE->engineUsed, DCCE->nr_engines, DCCE->uniqueEngine);

  DCCE->E[DCCE->nr_engines++] = E;
}

void Manager::removeDCC(Engine * E){
  auto DCCE = &DCC[E->DCC_ID];
  bool swap = false;
  for(uint8_t i = 0; i < DCCE->nr_engines; i++){
    if(!swap){
      if(DCCE->E[i] == E){
        swap = true;
        DCCE->E[i] = 0;
      }
    }
    else{
      DCCE->E[i-1] = DCCE->E[i];
      DCCE->E[i] = 0;
    }
  }
}

void Manager::removeCar(Car * C){
  // Remove files
  remove(C->icon_path);

  // Romove Reference from Cars list
  Cars.remove(C);

  delete C;
}

void Manager::removeEngine(Engine * E){
  // Remove files
  remove(E->img_path);
  remove(E->icon_path);

  // Remove Reference from DCC list
  removeDCC(E);

  // Romove Reference from Engines list
  Engines.remove(E);

  delete E;
}

void Manager::removeTrainSet(TrainSet * T){
  // Romove Reference from Trains list
  TrainSets.remove(T);

  delete T;
}

void Manager::removeTrain(Train * T){
  Trains.remove(T);

  delete T;
}

void Manager::moveEngine(Engine * E, uint16_t DCCid){
  removeDCC(E);      // Old pointer
  E->DCC_ID = DCCid; // Update DCC
  addDCC(E);         // New Pointer
}

void Manager::writeFile(){
  auto config = RollingConfig(filename);

  config.header.PersonCatagories = PassengerCatagories_length;
  config.P_Cat = (struct configStruct_Category *)_calloc(config.header.PersonCatagories, struct configStruct_Category);
  memcpy(config.P_Cat, PassengerCatagories, config.header.PersonCatagories * sizeof(struct configStruct_Category));

  config.header.CargoCatagories = CargoCatagories_length;
  config.C_Cat = (struct configStruct_Category *)_calloc(config.header.CargoCatagories, struct configStruct_Category);
  memcpy(config.C_Cat, CargoCatagories, config.header.CargoCatagories * sizeof(struct configStruct_Category));

  for(uint8_t i = 0; i < TrainSets.size; i++){
    if(!TrainSets[i])
      continue;

    config.addTrainSet(TrainSets[i]);
  }
  for(uint8_t i = 0; i < Engines.size; i++){
    if(!Engines[i])
      continue;

    config.addEngine(Engines[i]);
  }
  for(uint8_t i = 0; i < Cars.size; i++){
    if(!Cars[i])
      continue;

    config.addCar(Cars[i]);
  }

  config.write();
}
};