#include "rollingstock/manager.h"

#include "config/RollingConfig.h"

#include "utils/logger.h"
#include "utils/mem.h"

#include "system.h"

RollingStock::Manager * RSManager;

namespace RollingStock {

Manager::Manager(){
  memset(DCC, 0, 10000 * sizeof(Engine *));
}

void Manager::initScheduler(){
  continue_event = scheduler->addEvent("RailTrain_continue", {2, 0});
  continue_event->function = &RailTrain_ContinueCheck;
}

void Manager::clear(){
  memset(RSManager->DCC, 0, 10000 * sizeof(Engine *));

  for(uint8_t i = 0; i < PassengerCatagories_length; i++)
    _free(PassengerCatagories[i].name);
  for(uint8_t i = 0; i < CargoCatagories_length; i++)
    _free(CargoCatagories[i].name);

  _free(PassengerCatagories);//.clear();
  PassengerCatagories_length = 0;
  _free(CargoCatagories);//.clear();
  CargoCatagories_length = 0;

  RailTrains.clear();
  Trains.clear();
  Cars.clear();
  Engines.clear();
}

Manager::~Manager(){
  scheduler->removeEvent(continue_event);

  log("Clearing trains memory",INFO);

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
  PassengerCatagories = (struct configStruct_Category *)_calloc(config.header->PersonCatagories, struct configStruct_Category);
  PassengerCatagories_length = config.header->PersonCatagories;
  CargoCatagories     = (struct configStruct_Category *)_calloc(config.header->CargoCatagories, struct configStruct_Category);
  CargoCatagories_length = config.header->CargoCatagories;

  memcpy(PassengerCatagories, config.P_Cat, sizeof(struct configStruct_Category) * config.header->PersonCatagories);
  memcpy(CargoCatagories,     config.C_Cat, sizeof(struct configStruct_Category) * config.header->PersonCatagories);


  loggerf(INFO, "Initialize Engines");
  for(int i = 0; i < config.header->Engines; i++){
    newEngine(new Engine(config.Engines[i]));
  }

  
  loggerf(INFO, "Initialize Cars");
  for(int i = 0; i < config.header->Cars; i++){
    newCar(new Car(config.Cars[i]));
    // create_car_from_conf(config.Cars[i]);
  }

  
  loggerf(INFO, "Initialize Trains");
  for(int i = 0; i < config.header->Trains; i++){
    newTrain(new Train(config.Trains[i]));
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
  DCC[E->DCC_ID] = E;

  return E;
}

Train * Manager::newTrain(Train * T){
  T->id = Trains.push_back(T);

  return T;
}

int Manager::addRailTrain(RailTrain * T){
  return RailTrains.push_back(T);
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
  DCC[E->DCC_ID] = nullptr;

  // Romove Reference from Engines list
  Engines.remove(E);

  delete E;
}

void Manager::removeTrain(Train * T){
  // Romove Reference from Trains list
  Trains.remove(T);

  delete T;
}

void Manager::removeRailTrain(RailTrain * T){
  RailTrains.remove(T);

  delete T;
}

void Manager::moveEngine(Engine * E, uint16_t DCCid){
  DCC[E->DCC_ID] = NULL; // Old pointer
  DCC[DCCid] = E;          // New pointer

  E->DCC_ID = DCCid; // Update DCC
}

void Manager::writeFile(){
  auto config = RollingConfig(filename);

  config.header->PersonCatagories = PassengerCatagories_length;
  config.P_Cat = (struct configStruct_Category *)_calloc(config.header->PersonCatagories, struct configStruct_Category);
  memcpy(config.P_Cat, PassengerCatagories, config.header->PersonCatagories * sizeof(struct configStruct_Category));

  config.header->CargoCatagories = CargoCatagories_length;
  config.C_Cat = (struct configStruct_Category *)_calloc(config.header->CargoCatagories, struct configStruct_Category);
  memcpy(config.C_Cat, CargoCatagories, config.header->CargoCatagories * sizeof(struct configStruct_Category));

  for(uint8_t i = 0; i < Trains.size; i++){
    if(!Trains[i])
      continue;

    config.addTrain(Trains[i]);
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