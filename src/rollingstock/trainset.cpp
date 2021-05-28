#include "rollingstock/trainset.h"
#include "train.h"

#include "utils/mem.h"
#include "utils/logger.h"
#include "system.h"


struct train_composition ** trains_comp;
int trains_comp_len = 0;

TrainSet::TrainSet(struct configStruct_TrainSet conf){
  loggerf(TRACE, "Create Train %s", conf.name);
  memset(this, 0, sizeof(TrainSet));

  engines = new dynArray<Engine *>(5);

  setName(conf.name);
  max_speed = 0xFFFF;
  type = conf.category;
  save = true;

  setComposition(conf.nr_stock, conf.composition);
}
TrainSet::TrainSet(char * Name){
  memset(this, 0, sizeof(TrainSet));

  engines = new dynArray<Engine *>(5);
  setName(Name);

  max_speed = 0;
  type = 0;
  save = false;
}

TrainSet::TrainSet(char * Name, int Stock, struct configStruct_TrainSetLink * comps, uint8_t category, uint8_t Save){
  loggerf(TRACE, "Create Train %s", Name);

  memset(this, 0, sizeof(TrainSet));

  engines = new dynArray<Engine *>(5);

  setName(Name);

  max_speed = 0xFFFF;
  length = 0;
  type = category;
  save = Save;

  setComposition(nr_stock, comps);
}

TrainSet::~TrainSet(){
  loggerf(TRACE, "Destroy Train %s", name);
  engines->empty();

  delete engines;

  if(name)
    _free(name);

  if(composition)
    _free(composition);
}


void TrainSet::setName(char * new_name){
  if(name)
    _free(name);

  uint16_t len = strlen(new_name) + 1;
  name = (char *)_calloc(len + 5, char);
  memcpy(name, new_name, len);
}

void TrainSet::setComposition(int stock, struct configStruct_TrainSetLink * comps){
  if(composition)
    _free(composition);

  engines->empty();

  nr_stock = stock;
  composition = (struct RollingStockLink *)_calloc(nr_stock, struct RollingStockLink);
  bool splitdetectables = true;

  for(int i = 0;i<nr_stock;i++){
    composition[i].type = comps[i].type;
    composition[i].id = comps[i].id;

    if(comps[i].type == 0){
      loggerf(DEBUG, "Add engine %i", comps[i].id);
      Engine * E = RSManager->getEngine(comps[i].id);

      if(!E){
        loggerf(ERROR, "Engine (%i) doesn't exist", comps[i].id);
        continue;
      }

      length += E->length;
      if(max_speed > E->max_speed && E->max_speed != 0){
        max_speed = E->max_speed;
      }

      composition[i].p = E;

      engines->push_back(E);

      if(splitdetectables){
        splitdetectables = false;
        detectables++;
      }

      // loggerf(TRACE, "Train engine index: %d", index);
    }
    else{
      loggerf(DEBUG, "Add car %i", comps[i].id);

      Car * C = RSManager->getCar(comps[i].id);
      //Car
      if(!C){
        loggerf(ERROR, "Car (%i) doesn't exist", comps[i].id);
        continue;
      }
      
      length += C->length;
      if(max_speed > C->max_speed && C->max_speed != 0){
        max_speed = C->max_speed;
      }

      composition[i].p = C;

      if(!C->detectable){
        splitdetectables = true;
        virtualDetection = true;
      }
    }
  }
}

void TrainSet::setSpeed(uint16_t speed){
  cur_speed = speed;

  for(int i = 0; i < engines->items; i++){
    if((*engines)[i])
      (*engines)[i]->setSpeed(speed);
  }
}

void TrainSet::calcSpeed(){
  for(int i = 0; i < engines->items; i++){
    if((*engines)[i])
      (*engines)[i]->setSpeed(cur_speed);
  }
}

bool TrainSet::enginesUsed(){
  for(int  i = 0; i < engines->items; i++){
    if((*engines)[i]->use || RSManager->getEngineDCC((*engines)[i]->DCC_ID) != nullptr){
      return true;
    }
  }

  return false;
}

void TrainSet::setEnginesUsed(bool used, Train * T){
  for(int  i = 0; i < engines->items; i++){
    if(used)
      RSManager->subDCCEngine((*engines)[i]->id);
    else
      RSManager->unsubDCCEngine((*engines)[i]->id);

    (*engines)[i]->use = used;
    (*engines)[i]->RT = T;
  }
}