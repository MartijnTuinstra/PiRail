#include "rollingstock/manager.h"

#include "config/RollingConfig.h"

#include "utils/logger.h"
#include "utils/mem.h"

#include "system.h"

RollingStock::Manager * RSManager = new RollingStock::Manager();

namespace RollingStock {

Manager::Manager(){
  continue_event = scheduler->addEvent("RailTrain_continue", {2, 0});
  continue_event->function = &RailTrain_ContinueCheck;

  memset(DCC, 0, 10000 * sizeof(Engine *));
}

void Manager::clear(){
  memset(RSManager->DCC, 0, 10000 * sizeof(Engine *));

  Cars.clear();
  Engines.clear();
  Trains.clear();
  RailTrains.clear();
}

Manager::~Manager(){
  _free(filename);
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

  // if(train_link){
  //   for(int i = 0; i < train_link_len; i++){
  //     if(!train_link[i])
  //       continue;

  //     scheduler->removeEvent(train_link[i]->speed_event);

  //     _free(train_link[i]);
  //     train_link[i] = 0;
  //   }

  //   train_link = (RailTrain **)_free(train_link);
  // }

  // if(train_P_cat){
  //   for(int i = 0; i < train_P_cat_len; i++){
  //     _free(train_P_cat[i].name);
  //   }
  //   train_P_cat = (cat_conf *)_free(train_P_cat);
  // }


  // if(train_C_cat){
  //   for(int i = 0; i < train_C_cat_len; i++){
  //     _free(train_C_cat[i].name);
  //   }
  //   train_C_cat = (cat_conf *)_free(train_C_cat);
  // }


  if(SYS)
    SYS->trains_loaded = 0;
}

void Manager::loadFile(char * f){
  loadFile((const char *)f);
}

void Manager::loadFile(const char * f){
  if(filename)
    _free(filename);

  filename = (char *)_calloc(strlen(f), char);
  strcpy(filename, f);

  // Allocation Basic Space
  // trains = (Train **)_calloc(10, Train *);
  // trains_len = 10;
  // engines = (Engine **)_calloc(10, Engine *);
  // engines_len = 10;
  // cars = (Car **)_calloc(10, Car *);
  // cars_len = 10;
  // trains_comp = (struct train_composition **)_calloc(10, struct train_composition *);
  // trains_comp_len = 10;
  // train_link = (RailTrain **)_calloc(10,RailTrain *);
  // train_link_len = 10;
  
  scheduler->enableEvent(continue_event);

  // read_rolling_Configs();

  auto config = RollingConfig(filename);

  config.read();

  loggerf(INFO, "Reading Catagories");
  for(int i = 0; i < config.header.P_Catagories; i++)
    PassengerCatagories.push_back(config.P_Cat[i]);

  for(int i = 0; i < config.header.C_Catagories; i++)
    CargoCatagories.push_back(config.C_Cat[i]);


  loggerf(INFO, "Reading Engines");
  for(int i = 0; i < config.header.Engines; i++){
    newEngine(new Engine(config.Engines[i]));
  }

  
  loggerf(INFO, "Reading Cars");
  for(int i = 0; i < config.header.Cars; i++){
    newCar(new Car(config.Cars[i]));
    // create_car_from_conf(config.Cars[i]);
  }

  
  loggerf(INFO, "Reading Trains");
  for(int i = 0; i < config.header.Trains; i++){
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

  config.header.P_Catagories = PassengerCatagories.items;
  config.P_Cat = (struct cat_conf *)_calloc(config.header.P_Catagories, struct cat_conf);
  memcpy(config.P_Cat, PassengerCatagories.array, config.header.P_Catagories * sizeof(struct cat_conf));

  config.header.C_Catagories = CargoCatagories.items;
  config.C_Cat = (struct cat_conf *)_calloc(config.header.C_Catagories, struct cat_conf);
  memcpy(config.C_Cat, CargoCatagories.array, config.header.C_Catagories * sizeof(struct cat_conf));

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