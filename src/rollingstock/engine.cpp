#include "rollingstock/engine.h"
#include "rollingstock/functions.h"
#include "train.h"

#include "mem.h"
#include "logger.h"
#include "system.h"


Engine ** engines;
int engines_len = 0;
Engine * DCC_train[9999];

Engine::Engine(char * name,int DCC,char * img, char * icon, char type, int length, int steps_len, struct engine_speed_steps * steps, uint8_t functions[28]){
  loggerf(TRACE, "Create Engine %s", name);

  //DCC cant be used twice
  for(int i = 0;i<engines_len;i++){
    if(engines[i] && engines[i]->DCC_ID == DCC){
      loggerf(WARNING,"create_engine: found duplicate: %s",engines[i]->name);
    }
  }

  memset(this, 0, sizeof(Engine));

  this->name = (char *)_calloc(strlen(name), char);
  strcpy(this->name, name);
  this->img_path = (char *)_calloc(strlen(img), char);
  strcpy(this->img_path, img);
  this->icon_path = (char *)_calloc(strlen(icon), char);
  strcpy(this->icon_path, icon);

  this->DCC_ID = DCC;
  DCC_train[DCC] = this;

  this->length = length;
  //TODO add to arguments
  this->speed_step_type = ENGINE_128_FAHR_STUFEN;

  this->type = type;

  this->steps_len = steps_len;
  this->steps = (struct engine_speed_steps *)_calloc(steps_len, struct engine_speed_steps);
  memcpy(this->steps, steps, sizeof(struct engine_speed_steps) * steps_len);


  // Copy each speed step
  for(uint8_t i = 0; i < steps_len; i++){
    if(this->max_speed < steps[i].speed){
      this->max_speed = steps[i].speed;
    }
  }

  // Copy each function
  for(uint8_t i = 0; i < 29; i++){
    this->function[i].button = functions[i] >> 6;
    this->function[i].type = functions[i] & 0x3F;

    // loggerf(INFO, "Engine %i - Function %i - type %i - %i", DCC, i, this->function[i].type, this->function[i].button);
  }

  int index = find_free_index(engines, engines_len);

  engines[index] = this;
  this->id = index;
}

Engine::~Engine(){
  loggerf(INFO, "Destructor Engine %s", this->name);
  _free(this->name);
  _free(this->img_path);
  _free(this->icon_path);
  _free(this->steps);
}

void Engine::setSpeed(uint16_t speed){
  // Convert from scale speed to z21 steps
  struct engine_speed_steps left;
  struct engine_speed_steps right;

  this->cur_speed = speed;

  left.speed = 0; left.step = 0;
  right.speed = 0; right.step = 0;

  for(int i = 0; i < this->steps_len; i++){
    if(this->steps[i].speed < this->cur_speed){
      left.speed = this->steps[i].speed;
      left.step = this->steps[i].step;
    }

    if(this->steps[i].speed >= this->cur_speed && right.step == 0){
      right.speed = this->steps[i].speed;
      right.step = this->steps[i].step;
    }
  }

  float ratio = ((float)(this->cur_speed - left.speed)) / ((float)(right.speed - left.speed));
  uint8_t step = ratio * ((float)(right.step - left.step)) + left.step;

  this->speed = step;
}

void Engine::readSpeed(){
  // Convert from z21 steps to scale speed
  struct engine_speed_steps * left;
  struct engine_speed_steps * right;

  struct engine_speed_steps temp;
  temp.step = 0;
  temp.speed = 0;

  left = &temp;
  right = 0;

  for(int i = 0; i < this->steps_len; i++){
    if(this->steps[i].step < this->speed){
      left = &this->steps[i];
    }

    if(this->steps[i].step >= this->speed && right == 0){
      right = &this->steps[i];
    }
  }

  float ratio = ((float)(this->speed - left->step)) / ((float)(right->step - left->step));
  uint8_t speed = ratio * ((float)(right->speed - left->speed)) + left->speed;

  this->cur_speed = speed;
}
