#include <math.h>

#include "rollingstock/railtrain.h"
#include "train.h"

#include "mem.h"
#include "logger.h"
#include "scheduler.h"

#include "websocket_stc.h"
#include "Z21_msg.h"

RailTrain ** train_link;
int train_link_len;

RailTrain::RailTrain(){
  memset(this, 0, sizeof(RailTrain));

  uint16_t id = find_free_index(train_link, train_link_len);
  this->link_id = id;

  char name[64];
  sprintf(name, "Railtrain_%i_SpeedEvent", id);
  this->speed_event = scheduler->addEvent(name, {0, 0});
  this->speed_event_data = (struct TrainSpeedEventData *)_calloc(1, struct TrainSpeedEventData);
  this->speed_event->function = (void (*)(void *))train_speed_event_tick;
  this->speed_event->function_args = (void *)this->speed_event_data;

  train_link[id] = this;
}

RailTrain::~RailTrain(){
  _free(this->speed_event_data);
}

void RailTrain::setSpeedZ21(uint16_t speed){
  if(this->type == RAILTRAIN_ENGINE_TYPE){
    this->p.E->setSpeed(this->speed);
    Z21_Set_Loco_Drive_Engine(this->p.E);
  }
  else{
    this->p.T->setSpeed(this->speed);
    Z21_Set_Loco_Drive_Train(this->p.T);
  }
}

void RailTrain::changeSpeed(uint16_t target_speed, uint8_t type){
  if(!this->p.p){
    loggerf(ERROR, "No Train");
    return;
  }

  loggerf(INFO, "train_change_speed %i -> %i", this->link_id, target_speed);
  //this->target_speed = target_speed;

  if(type == IMMEDIATE_SPEED){
    this->changing_speed = RAILTRAIN_SPEED_T_DONE;
    this->setSpeed(target_speed);
    WS_stc_UpdateTrain(this);
  }
  else if(type == GRADUAL_SLOW_SPEED){
    train_speed_event_create(this, target_speed, this->B->length*2);
  }
  else if(type == GRADUAL_FAST_SPEED){
    train_speed_event_create(this, target_speed, this->B->length);
  }
}


void RailTrain::setRoute(Block * dest){
  // struct pathfindingstep path = pathfinding(T->B, dest);

  // if(path.found){
  //   T->route = 1;
  //   T->instructions = path.instructions;
  // }
}


int RailTrain::link(int tid, char type){
  // Link train or engine to RailTrain class.
  //
  //  tID = id of train or engine
  //  type = bool train or engine

  // If it is only a engine -> make it a train
  if(type){
    if(engines[tid]->use){
      loggerf(ERROR, "Engine allready used");
      return 3;
    }

    // Create train from engine
    this->type = RAILTRAIN_ENGINE_TYPE;
    this->p.E = engines[tid];
    this->max_speed = engines[tid]->max_speed;

    //Lock engines
    engines[tid]->use = 1;
    engines[tid]->RT = this;
  }
  else{
    for(int  i = 0; i < trains[tid]->nr_engines; i++){
      if(trains[tid]->engines[i]->use){
        loggerf(ERROR, "Engine of Train allready used");
        return 3;
      }
    }

    // Crate Rail Train
    this->type = RAILTRAIN_TRAIN_TYPE;
    this->p.T = trains[tid];
    this->max_speed = trains[tid]->max_speed;

    //Lock all engines    
    for(int i = 0; i < trains[tid]->nr_engines; i++){
      trains[tid]->engines[i]->use = 1;
      trains[tid]->engines[i]->RT = this;
    }
  }

  return 1;
}

void RailTrain::unlink(){
  //TODO implement RailTrain type
  //Unlock all engines
  if(this->type == RAILTRAIN_ENGINE_TYPE){
    this->p.E->use = 0;
    this->p.E->RT = 0;
  }
  else{
    Train * T = this->p.T;
    for(int i = 0; i < T->nr_engines; i++){
      T->engines[i]->use = 0;
      T->engines[i]->RT = 0;
    }
  }
}


void train_speed_event_create(RailTrain * T, uint16_t targetSpeed, uint16_t distance){
  loggerf(INFO, "train_speed_event_create %i", T->changing_speed);

  if (T->speed == targetSpeed){
    return;
  }

  T->target_speed = targetSpeed;
  T->target_distance = distance;

  if (T->changing_speed == RAILTRAIN_SPEED_T_INIT ||
      T->changing_speed == RAILTRAIN_SPEED_T_DONE){

    T->changing_speed = RAILTRAIN_SPEED_T_CHANGING;
  }
  else if(T->changing_speed == RAILTRAIN_SPEED_T_CHANGING){
    T->changing_speed = RAILTRAIN_SPEED_T_UPDATE;
  }
  else{
    return;
  }

  train_speed_event_init(T);
}

void train_speed_event_calc(struct TrainSpeedEventData * data){
  // v = v0 + at;
  // x = v0*t + 0.5at^2;
  // a = 0.5/x * (v^2 - v0^2);
  // t = sqrt((x - v0) / (0.5a))
  float start_speed = data->T->speed * 1.0;

  loggerf(INFO, "train_speed_timer_calc %i %i %f", data->T->target_distance, data->T->target_speed, start_speed);

  float real_distance = 160.0 * 0.00001 * (data->T->target_distance - 5); // km
  data->acceleration = (1 / (2 * real_distance));
  data->acceleration *= (data->T->target_speed - start_speed) * (data->T->target_speed + start_speed);
  // data->acceleration == km/h/h

  loggerf(DEBUG, "Train_speed_timer_run (data->acceleration at %f km/h^2)", data->acceleration);
  if (data->acceleration > 64800.0){ // 5 m/s^2
    loggerf(INFO, "data->acceleration to large, reduced to 5.0m/s^2)");
    data->acceleration = 64800.0;
  }
  else if (data->acceleration < -129600.0){
    loggerf(INFO, "Deccell to large, reduced to 10.0m/s^2)");
    data->acceleration = -129600.0;
  }

  if (data->acceleration == 0 || data->acceleration == 0.0){
    loggerf(INFO, "No speed difference");
    data->T->changing_speed = RAILTRAIN_SPEED_T_DONE;
    return;
  }

  loggerf(INFO, "train_speed  sqrt((2 * (%f - %f)) / (%f))", real_distance, start_speed, data->acceleration);

  data->time = sqrt(2 * (data->acceleration) * real_distance + data->T->speed * data->T->speed) - data->T->speed;
  data->time /= data->acceleration;
  data->time *= 3600; // convert to seconds

  if(data->time < 0.0){
    data->time *= -1.0;
  }
  
  data->steps = (uint16_t)(data->time / 0.5);
  data->stepTime = ((data->time / data->steps) * 1000000L); // convert to usec

  data->startSpeed = data->T->speed;

  loggerf(INFO, "train_speed time %f", data->time);
}

void train_speed_event_init(RailTrain * T){
  loggerf(INFO, "train_speed_event_init (%i -> %ikm/h, %icm)\n", T->speed, T->target_speed, T->target_distance);

  T->speed_event_data->T = T;

  train_speed_event_calc(T->speed_event_data);

  T->speed_event->interval.tv_sec = T->speed_event_data->stepTime / 1000000L;
  T->speed_event->interval.tv_nsec = (T->speed_event_data->stepTime % 1000000UL) * 1000;
  scheduler->enableEvent(T->speed_event);
  T->speed_event_data->stepCounter = 0;

  if(T->changing_speed == RAILTRAIN_SPEED_T_DONE)
    return;
}

void train_speed_event_tick(struct TrainSpeedEventData * data){
  data->stepCounter++;

  loggerf(INFO, "train_speed_event_tick %i a:%f, s:(%i/%i), t:%i", data->T->speed, data->acceleration, data->stepCounter, data->steps, data->stepTime);

  RailTrain * T = data->T;

  if(T->changing_speed == RAILTRAIN_SPEED_T_DONE){
    scheduler->disableEvent(T->speed_event);
    return;
  }

  T->speed = (data->startSpeed + data->acceleration * ((data->stepTime * data->stepCounter) / 1000000.0 / 3600.0)) + 0.01; // hours

  loggerf(INFO, "train_speed_timer_run %i %f", T->speed, data->acceleration);

  T->setSpeedZ21(T->speed);
  WS_stc_UpdateTrain(T);

  if (data->stepCounter >= data->steps){
    T->changing_speed = RAILTRAIN_SPEED_T_DONE;
    scheduler->disableEvent(T->speed_event);
  }

  return;
}

