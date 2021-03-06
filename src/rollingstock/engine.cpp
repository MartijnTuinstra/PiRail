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

  if(conf.DCC_ID >= 9999){
    loggerf(CRITICAL, "DCC address (%i) not allowed", conf.DCC_ID);
    return;
  }

  DCC_ID = conf.DCC_ID;

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
}
Engine::Engine(uint16_t DCC, char * name){
  setName(name);
  DCC_ID = DCC;
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

  
  if(!configSteps)
    configSteps = (struct EngineSpeedSteps *)_calloc(nr_steps, struct EngineSpeedSteps);
  else
    configSteps = (struct EngineSpeedSteps *)_realloc(configSteps, nr_steps, struct EngineSpeedSteps);

  memcpy(configSteps, speed_steps, nr_steps * sizeof(struct EngineSpeedSteps));
  configSteps_len = nr_steps;

  struct EngineSpeedSteps temp = {.speed = 0, .step = 0};

  struct EngineSpeedSteps * s1 = &temp;
  struct EngineSpeedSteps * s2;

  steps[0] = {.speed = 0, .step = 0};
  uint8_t i = 0;
  uint8_t j;

  do {
    s2 = &configSteps[i];

    float x1 = ((float)(s2->speed - s1->speed))/((float)(s2->step - s1->step));
    for(j = s1->step + 1; j <= s2->step; j++){
      steps[j].step = j;
      steps[j].speed = (x1 * j + 0.5) + s1->speed;
    }

    s1 = s2;
  }
  while(++i < nr_steps - 1);

  steps_len = j;

  // Get Max speed from speedsteps
  max_speed = 0;
  for(uint8_t i = 0; i < nr_steps; i++){
    if(max_speed < configSteps[i].speed){
      max_speed = configSteps[i].speed;
    }
  }
}

void Engine::convertRealSpeedtoZ21(uint16_t speed){
  // Convert from scale speed to z21 steps
  struct EngineSpeedSteps temp = {.speed = 0, .step = 0};

  struct EngineSpeedSteps * left  = &temp;
  struct EngineSpeedSteps * right = &steps[steps_len - 1];

  cur_speed = speed;

  for(int i = 0; i < steps_len; i++){
    if(steps[i].speed < cur_speed){
      left = &steps[i];
    }

    if(steps[i].speed >= cur_speed && &steps[i] != right){
      right = &steps[i];
      break;
    }
  }

  if(cur_speed - left->speed < right->speed - cur_speed)
    Z21_set_speed = left->step;  // If closer to the left marker
  else
    Z21_set_speed = right->step; // If closer to the right marker


  // float ratio = ((float)(cur_speed - left->speed)) / ((float)(right.speed - left->speed));
  // uint8_t step = ratio * ((float)(right.step - left->step)) + left->step;

  // Z21_set_speed = step;
}

void Engine::convertZ21toRealSpeed(uint16_t Z21Speed){
  // Convert from z21 steps to scale speed
  Z21_get_speed = Z21Speed;
  cur_speed = steps[Z21_get_speed].speed;
}

void Engine::Z21_setFunctions(char * funcs, uint8_t length){
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

void Engine::setSpeed(uint16_t speed){
  setSpeed(speed, dir);
}

void Engine::setSpeed(uint16_t speed, bool _dir){
  dir = _dir;
  convertRealSpeedtoZ21(speed);
  
  loggerf(DEBUG, "Engine setSpeed %i %i -> %i", speed, _dir, Z21_set_speed);

  if(Z21_get_speed != Z21_set_speed)
    Z21_Set_Loco_Drive_Engine(this);
}

void Engine::Z21_setSpeedDir(char _speed, bool _dir){
  bool samespeed = (_speed == Z21_set_speed);
  bool samedir =   (_dir == dir);

  dir = _dir;

  convertZ21toRealSpeed(_speed);

  loggerf(DEBUG, "Engine Z21_setSpeedDir %i %i -> %i %i", _speed, _dir, Z21_get_speed, cur_speed);

  // If linked to a RailTrain
  if(use){
    if(!samedir && RT->dir != dir){
      RT->Z21_reverse();
    }

    if(!samespeed){
      RT->setSpeed(cur_speed);
      // This will update other Engines in the train
    }

    WS_stc_UpdateTrain(RT);
  }
  else{
    WS_stc_DCCEngineUpdate(this);
  }
}