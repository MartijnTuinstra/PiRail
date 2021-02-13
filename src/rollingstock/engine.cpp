#include "rollingstock/engine.h"
#include "rollingstock/manager.h"
#include "rollingstock/functions.h"
#include "train.h"

#include "Z21_msg.h"
#include "websocket/stc.h"

#include "utils/mem.h"
#include "utils/logger.h"
#include "system.h"


Engine::Engine(struct configStruct_Engine conf){
  loggerf(TRACE, "Create Engine %s", conf.name);

  memset(this, 0, sizeof(Engine));

  setName(conf.name);
  setImagePath(conf.img_path);
  setIconPath(conf.icon_path);

  //DCC cant be used twice
  if(conf.DCC_ID >= 9999){
    loggerf(CRITICAL, "DCC address (%i) not allowed", conf.DCC_ID);
    return;
  }
  if(RSManager->DCC[conf.DCC_ID])
    loggerf(WARNING,"create_engine: found duplicate: %s, overwriting!", RSManager->DCC[conf.DCC_ID]->name);


  DCC_ID = conf.DCC_ID;

  RSManager->DCC[conf.DCC_ID] = this;

  length = conf.length;
  speed_step_type = conf.Z21_SpeedSteps;

  type = conf.type;

  setSpeedSteps(conf.config_steps, (struct EngineSpeedSteps *)conf.speed_steps);

  // Copy each function
  for(uint8_t i = 0; i < 29; i++){
    function[i].button = conf.functions[i] >> 6;
    function[i].type = conf.functions[i] & 0x3F;
    function[i].state = 0;

    // loggerf(INFO, "Engine %i - Function %i - type %i - %i", DCC, i, this->function[i].type, this->function[i].button);
  }

  // int index = find_free_index(engines, engines_len);

  // engines[index] = this;
  // this->id = index;
}
Engine::Engine(uint16_t DCC, char * name){
  setName(name);
  DCC_ID = DCC;

  if(RSManager->DCC[DCC])
    loggerf(WARNING,"create_engine: found duplicate: %s, overwriting!", RSManager->DCC[DCC]->name);

  RSManager->DCC[DCC] = this;
}

Engine::~Engine(){
  loggerf(TRACE, "Destructor Engine %s", this->name);
  _free(this->name);
  _free(this->img_path);
  _free(this->icon_path);
  _free(this->steps);
}


void Engine::setName(char * new_name){
  if(name)
    _free(name);

  name = (char *)_calloc(strlen(new_name) + 10, char);
  strncpy(name, new_name, strlen(new_name) + 1);
}
void Engine::setImagePath(char * newpath){
  if(img_path)
    _free(img_path);

  img_path = (char *)_calloc(strlen(newpath) + 10, char);
  strcpy(img_path, newpath);
}
void Engine::setIconPath(char * newpath){
  if(icon_path)
    _free(icon_path);

  icon_path = (char *)_calloc(strlen(newpath) + 10, char);
  strcpy(icon_path, newpath);}

void Engine::setSpeedSteps(uint8_t nr_steps, struct EngineSpeedSteps * speed_steps){
  if(steps)
    _free(steps);

  steps_len = nr_steps;
  steps = (struct EngineSpeedSteps *)_calloc(nr_steps, struct EngineSpeedSteps);
  memcpy(steps, speed_steps, nr_steps * sizeof(struct EngineSpeedSteps));

  // Get Max speed from speedsteps
  max_speed = 0;
  for(uint8_t i = 0; i < steps_len; i++){
    if(max_speed < steps[i].speed){
      max_speed = steps[i].speed;
    }
  }
}

void Engine::setSpeed(uint16_t speed){
  // Convert from scale speed to z21 steps
  struct EngineSpeedSteps left;
  struct EngineSpeedSteps right;

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
  struct EngineSpeedSteps * left;
  struct EngineSpeedSteps * right;

  struct EngineSpeedSteps temp;
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

void Engine::setFunctions(char * funcs, uint8_t length){
  //Functions ....
  function[0].state = (funcs[0] >> 4) & 0x1;
  function[1].state =  funcs[0]       & 0x1;
  function[2].state = (funcs[0] >> 1) & 0x1;
  function[3].state = (funcs[0] >> 2) & 0x1;
  function[4].state = (funcs[0] >> 3) & 0x1;

  for(uint8_t i = 0; i < 3; i++){
    if (i > length)
      break;

    for(uint8_t j = 0; j < 8; j++){
      function[i * 8 + j + 5].state = (funcs[1 + i] >> j) & 0x1;
    }
  }
}

void Engine::Z21set(char _speed, bool _dir){
  bool samespeed = (_speed == speed);
  bool samedir =   (_dir == dir);

  speed = speed;
  dir = dir;

  readSpeed();

  if(use){
    if(!samedir && RT->type == RAILTRAIN_TRAIN_TYPE){
      //->Z21set()
      Train * T = RT->p.T;

      for(uint8_t i = 0; i < T->engines->items; i++){
        Engine * tE = T->engines->operator[](i);
        if(this != tE){
          tE->dir = dir;
        }
      }
    }

    if(!samespeed){
      RT->speed = cur_speed;

      RT->changing_speed = RAILTRAIN_SPEED_T_DONE;

      RT->setSpeed(RT->speed);
    }

    if((!samedir || !samespeed) && RT->type == RAILTRAIN_TRAIN_TYPE){
      Train * T = RT->p.T;
      // Set all other engines coupled to this train to new parameters
      for(uint8_t i = 0; i < T->engines->items; i++){
        Engine * tE = T->engines->operator[](i);
        if(this != tE){
          Z21_Set_Loco_Drive_Engine(tE);
        }
      }
    }

    WS_stc_UpdateTrain(RT);
  }
  else{
    WS_stc_DCCEngineUpdate(this);
  }
}