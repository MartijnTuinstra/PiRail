#include "rollingstock/train.h"
#include "train.h"

#include "utils/mem.h"
#include "utils/logger.h"
#include "system.h"


struct train_composition ** trains_comp;
int trains_comp_len = 0;

Train::Train(struct trains_conf conf){
  loggerf(TRACE, "Create Train %s", conf.name);
  memset(this, 0, sizeof(Train));

  engines = new dynArray<Engine *>(5);

  setName(conf.name);
  max_speed = 0xFFFF;
  type = conf.category;
  save = true;

  detectables = 0;
  splitdetectables = false;
  setComposition(conf.nr_stock, conf.composition);

  if(this->detectables < nr_stock)
    loggerf(INFO, "Train has cars that are not detectable"); // TODO
  if(this->splitdetectables)
    loggerf(INFO, "Train has undetectable cars in between two (multiple) engines"); // TODO
}
Train::Train(char * Name){
  memset(this, 0, sizeof(Train));

  engines = new dynArray<Engine *>(5);
  setName(Name);

  max_speed = 0;
  type = 0;
  save = false;

  detectables = 0;
  splitdetectables = false;
}

Train::Train(char * Name, int Stock, struct train_comp_ws * comps, uint8_t category, uint8_t Save){
  loggerf(TRACE, "Create Train %s", Name);

  memset(this, 0, sizeof(Train));

  engines = new dynArray<Engine *>(5);

  setName(Name);

  max_speed = 0xFFFF;
  length = 0;
  type = category;
  save = Save;

  detectables = 0;
  splitdetectables = false;

  setComposition(nr_stock, comps);

  if(this->detectables < nr_stock)
    loggerf(INFO, "Train has cars that are not detectable"); // TODO
  if(this->splitdetectables)
    loggerf(INFO, "Train has undetectable cars in between two (multiple) engines"); // TODO
}

Train::~Train(){
  loggerf(TRACE, "Destroy Train %s", name);
  engines->empty();

  delete engines;

  _free(this->name);
  _free(this->composition);
}


void Train::setName(char * new_name){
  if(name)
    _free(name);

  name = (char *)_calloc(strlen(new_name) + 10, char);
  strcpy(name, new_name);
}

void Train::setComposition(int stock, struct train_comp_ws * comps){
  if(composition)
    _free(composition);

  engines->empty();

  nr_stock = stock;
  composition = (struct train_comp *)_calloc(nr_stock, struct train_comp);
  bool testsplitdetectables = false;

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

      detectables += 1;
      if(testsplitdetectables)
        splitdetectables = true;

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

      if(C->detectable)
        detectables += 1;
      else
        testsplitdetectables = true;
    }
  }
}

void Train::setSpeed(uint16_t speed){
  cur_speed = speed;

  for(int i = 0; i < engines->items; i++){
    if((*engines)[i])
      (*engines)[i]->setSpeed(speed);
  }
}

void Train::calcSpeed(){
  for(int i = 0; i < engines->items; i++){
    if((*engines)[i])
      (*engines)[i]->setSpeed(cur_speed);
  }
}

bool Train::enginesUsed(){
  for(int  i = 0; i < engines->items; i++){
    if((*engines)[i]->use){
      return true;
    }
  }

  return false;
}

void Train::setEnginesUsed(bool used, RailTrain * T){
  for(int  i = 0; i < engines->items; i++){
    (*engines)[i]->use = used;
    (*engines)[i]->RT = T;
  }
}