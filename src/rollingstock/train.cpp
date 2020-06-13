#include "rollingstock/train.h"
#include "train.h"

#include "mem.h"
#include "logger.h"
#include "system.h"


Train ** trains;
int trains_len = 0;
struct train_composition ** trains_comp;
int trains_comp_len = 0;

Train::Train(char * name, int nr_stock, struct train_comp_ws * comps, uint8_t catagory, uint8_t save){
//   Train * Z = (Train *)_calloc(1, Train);
  memset(this, 0, sizeof(Train));

  this->nr_stock = nr_stock;
  this->composition = (struct train_comp *)_calloc(nr_stock, struct train_comp);

  this->max_speed = 0xFFFF;
  this->length = 0;
  this->type = catagory;
  this->save = save;

  this->engines = (Engine **)_calloc(1, Engine *);
  this->nr_engines = 0;

  for(int i = 0;i<nr_stock;i++){
    this->composition[i].type = comps[i].type;
    this->composition[i].id = comps[i].id;

    if(comps[i].type == 0){
      loggerf(DEBUG, "Add engine %i", comps[i].id);
      Engine * E = ::engines[comps[i].id];
      //Engine
      if(comps[i].id >= engines_len || E == 0){
        loggerf(ERROR, "Engine (%i) doesn't exist", comps[i].id);
        continue;
      }

      this->length += E->length;
      if(this->max_speed > E->max_speed && E->max_speed != 0){
        this->max_speed = E->max_speed;
      }

      this->composition[i].p = E;

      int index = find_free_index(this->engines, this->nr_engines);

      this->engines[index] = E;

      loggerf(TRACE, "Train engine index: %d", index);
    }
    else{
      loggerf(DEBUG, "Add car %i", comps[i].id);
      //Car
      if(comps[i].id >= cars_len || cars[comps[i].id] == 0){
        loggerf(ERROR, "Car (%i) doesn't exist", comps[i].id);
        continue;
      }
      
      this->length += cars[comps[i].id]->length;
      if(this->max_speed > cars[comps[i].id]->max_speed && cars[comps[i].id]->max_speed != 0){
        this->max_speed = cars[comps[i].id]->max_speed;
      }

      this->composition[i].p = cars[comps[i].id];
    }
  }

  this->name = name;
  _free(comps);

  int index = find_free_index(trains, trains_len);

  trains[index] = this;
  this->id = index;

  loggerf(DEBUG, "Train created at %i",index);
}

Train::~Train(){
  _free(this->name);
  _free(this->engines);
  _free(this->composition);
}


void Train::setSpeed(uint16_t speed){
  this->cur_speed = speed;

  for(int i = 0; i < this->nr_engines; i++){
    this->engines[i]->setSpeed(speed);
  }
}

void Train::calcSpeed(){
  for(int i = 0; i < this->nr_engines; i++){
    this->engines[i]->setSpeed(this->cur_speed);
  }
}

