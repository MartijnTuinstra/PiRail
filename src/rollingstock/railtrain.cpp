#include <math.h>

#include "switchboard/station.h"
#include "rollingstock/railtrain.h"
#include "train.h"

#include "utils/mem.h"
#include "utils/logger.h"
#include "scheduler/scheduler.h"
#include "system.h"
#include "algorithm/queue.h"

#include "websocket/stc.h"
#include "Z21_msg.h"

RailTrain ** train_link;
int train_link_len;

RailTrain::RailTrain(Block * B){
  memset(this, 0, sizeof(RailTrain));

  uint16_t id = find_free_index(train_link, train_link_len);
  link_id = id;
  this->B = B;
  setBlock(B);

  if(B->path)
    B->path->trains.push_back(this);

  assigned = false;

  char name[64];
  sprintf(name, "Railtrain_%i_SpeedEvent", id);
  speed_event = scheduler->addEvent(name, {0, 0});
  speed_event_data = (struct TrainSpeedEventData *)_calloc(1, struct TrainSpeedEventData);
  speed_event->function = (void (*)(void *))train_speed_event_tick;
  speed_event->function_args = (void *)speed_event_data;

  train_link[id] = this;
}

RailTrain::~RailTrain(){
  train_link[link_id] = 0;
  _free(this->speed_event_data);
}

void RailTrain::setBlock(Block * sB){
  loggerf(INFO, "train %i: setBlock %2i:%2i %x", link_id, sB->module, sB->id, (unsigned int)sB);
  blocks.push_back(sB);
}

void RailTrain::releaseBlock(Block * rB){
  loggerf(INFO, "train %i: releaseBlock %2i:%2i %x", link_id, rB->module, rB->id, (unsigned int)rB);
  rB->train = 0;
  blocks.erase(std::remove_if(blocks.begin(), blocks.end(), [rB](const auto & o) { return (o == rB); }), blocks.end());
}

void RailTrain::reserveBlock(Block * rB){
  loggerf(INFO, "train %i: reserveBlock %2i:%2i", link_id, rB->module, rB->id);

  rB->reservedBy = this;
  rB->switchReserved = true;
  reservedBlocks.push_back(rB);
}

void RailTrain::dereserveBlock(Block * rB){
  loggerf(INFO, "train %i: dereserveBlock %2i:%2i", link_id, rB->module, rB->id);
  rB->reservedBy = 0;
  rB->switchReserved = false;

  reservedBlocks.erase(std::remove_if(reservedBlocks.begin(),
                                      reservedBlocks.end(),
                                      [rB](const auto & o) { return (o == rB); }),
                       reservedBlocks.end());
}

void RailTrain::dereserveAll(){

  for(auto b: reservedBlocks){
    b->reservedBy = 0;
    b->switchReserved = false;
  }

  reservedBlocks.clear();
}

void RailTrain::initVirtualBlocks(){
  loggerf(TRACE, "initVirtualBlocks");
  Block * tB = B;
  uint8_t offset = 0;
  int16_t len = length / 10;
  while(len > 0){
    len -= tB->length;

    tB->setVirtualDetection(1);

    if(tB->train != this){
      if(tB->train){
        if(tB->train->p.p)
          loggerf(CRITICAL, "Virtual Train intersects with other train");

        loggerf(INFO, "Virtual Train intersects with empty train");

        delete tB->train;
      }
      tB->train = this;
      tB->train->setBlock(tB);
    }
    AlQueue.puttemp(tB);

    // loggerf(INFO, "  block %2i:%2i   %i  %i  %i", tB->module, tB->id, B->Alg.prev, offset, len);

    if(B->Alg.prev > offset){
      tB = B->Alg.P[offset++];
    }
    else
      break;
  }

  if(B->Alg.prev > offset){
    tB->setVirtualDetection(0);
    if(tB->train && !tB->detectionblocked)
      tB->train->releaseBlock(tB);
    AlQueue.puttemp(tB);

    // loggerf(INFO, "f block %2i:%2i", tB->module, tB->id);
  }
}

void RailTrain::setVirtualBlocks(){
  loggerf(TRACE, "setVirtualBlocks");
  Block * tB = B->Alg.P[0];
  uint8_t offset = 1;
  int16_t len = length / 10;
  while(len > 0){
    len -= tB->length;

    tB->setVirtualDetection(1);

    if(tB->train != this){
      tB->train = this;
      tB->train->setBlock(tB);
    }
    AlQueue.puttemp(tB);

    // loggerf(INFO, "  block %2i:%2i   %i  %i  %i", tB->module, tB->id, B->Alg.prev, offset, len);

    if(B->Alg.prev > offset){
      tB = B->Alg.P[offset++];
    }
    else
      break;
  }

  while(B->Alg.prev >= offset){
    tB->setVirtualDetection(0);
    if(tB->train && !tB->detectionblocked)
      tB->train->releaseBlock(tB);

    AlQueue.puttemp(tB);

    if(B->Alg.prev > offset){
      tB = B->Alg.P[offset++];
      if(tB->train == this)
        continue;
    }

    break;
    // loggerf(INFO, "f block %2i:%2i", tB->module, tB->id);
  }
}

void RailTrain::moveForward(Block * tB){
  loggerf(WARNING, "MoveForward RT %i to block %2i:%2i", link_id, tB->module, tB->id);
  setBlock(tB);

  if(B->Alg.next > 0 && B->Alg.N[0] == tB){
    loggerf(ERROR, "Updating front block %2i:%2i", tB->module, tB->id);
    B = tB;

    if(virtualLength)
      setVirtualBlocks();
  }
}

void inline RailTrain::setSpeed(uint16_t _speed){
  speed = _speed;

  if(stopped && speed){
    // Was stopped but starting to move
  }

  if(speed) setStopped(0);
  else setStopped(1);

  if(!p.p) return;
  else if(type == RAILTRAIN_ENGINE_TYPE) p.E->setSpeed(speed);
  else p.T->setSpeed(speed);
}

void RailTrain::setSpeedZ21(uint16_t _speed){
  setSpeed(_speed);

  if(!assigned)
    return;

  if(!p.p) return;
  else if(type == RAILTRAIN_ENGINE_TYPE) Z21_Set_Loco_Drive_Engine(p.E);
  else Z21_Set_Loco_Drive_Train(p.T);
}

void RailTrain::setStopped(bool stop){
  stopped = stop;

  if(stop)
    dereserveAll();

  for(auto b: blocks){
    if(b->station){
      b->station->setStoppedTrain(stop);
    }
  }
}

void RailTrain::changeSpeed(uint16_t _target_speed, uint8_t _type){
  if(!p.p){
    loggerf(ERROR, "No Train");
    return;
  }

  loggerf(DEBUG, "train_change_speed %i -> %i", link_id, _target_speed);
  //target_speed = target_speed;

  if(_type == IMMEDIATE_SPEED){
    changing_speed = RAILTRAIN_SPEED_T_DONE;
    setSpeed(_target_speed);
    WS_stc_UpdateTrain(this);
  }
  else if(_type == GRADUAL_SLOW_SPEED){
    train_speed_event_create(this, _target_speed, B->length*2);
  }
  else if(_type == GRADUAL_FAST_SPEED){
    train_speed_event_create(this, _target_speed, B->length);
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
  if(type == RAILTRAIN_ENGINE_TYPE){
    if(engines[tid]->use){
      loggerf(ERROR, "Engine allready used");
      return 3;
    }

    // Create train from engine
    Engine * E = engines[tid];
    this->type = RAILTRAIN_ENGINE_TYPE;
    this->p.E = E;
    this->max_speed = E->max_speed;
    this->length = E->length;
    virtualLength = false;

    //Lock engines
    E->use = 1;
    E->RT = this;
  }
  else{
    for(int  i = 0; i < trains[tid]->nr_engines; i++){
      if(trains[tid]->engines[i]->use){
        loggerf(ERROR, "Engine of Train allready used");
        return 3;
      }
    }

    // Crate Rail Train
    Train * T = trains[tid];
    this->type = RAILTRAIN_TRAIN_TYPE;
    this->p.T = T;
    this->max_speed = T->max_speed;
    this->length = T->length;

    //Lock all engines    
    for(int i = 0; i < T->nr_engines; i++){
      T->engines[i]->use = 1;
      T->engines[i]->RT = this;
    }

    virtualLength = false;
    if(T->detectables != T->nr_stock){
      virtualLength = true;
      initVirtualBlocks();
    }
  }

  assigned = true;

  loggerf(INFO, "RailTrain linked train/engine, length: %i", length);

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

  this->assigned = false;
}

bool RailTrain::ContinueCheck(){
  if(this->B->Alg.next > 0){
    //if(this->Route && Switch_Check_Path(this->B)){
    //  return true;
    //}
    if(this->B->Alg.N[0]->state > DANGER){
      return true;
    }
  }
  return false;
}

void RailTrain_ContinueCheck(void * args){
  // Check if trains can accelerate when they are stationary.

  loggerf(TRACE, "RailTrain ContinueCheck");
  for(uint8_t i = 0; i < train_link_len; i++){
    RailTrain * T = train_link[i];
    if(!T)
      continue;

    if(T->p.p && T->speed == 0 && T->ContinueCheck()){
      loggerf(ERROR, "RailTrain ContinueCheck accelerating train %i", i);
      T->changeSpeed(20, GRADUAL_FAST_SPEED);
      WS_stc_UpdateTrain(T);
    }
  }
}


void train_speed_event_create(RailTrain * T, uint16_t targetSpeed, uint16_t distance){
  loggerf(TRACE, "train_speed_event_create %i", T->changing_speed);

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

  loggerf(DEBUG, "train_speed_timer_calc %i %i %f", data->T->target_distance, data->T->target_speed, start_speed);

  float real_distance = 160.0 * 0.00001 * (data->T->target_distance - 5); // km
  data->acceleration = (1 / (2 * real_distance));
  data->acceleration *= (data->T->target_speed - start_speed) * (data->T->target_speed + start_speed);
  // data->acceleration == km/h/h

  loggerf(TRACE, "Train_speed_timer_run (data->acceleration at %f km/h^2)", data->acceleration);
  if (data->acceleration > 64800.0){ // 5 m/s^2
    loggerf(DEBUG, "data->acceleration to large, reduced to 5.0m/s^2)");
    data->acceleration = 64800.0;
  }
  else if (data->acceleration < -129600.0){
    loggerf(DEBUG, "Deccell to large, reduced to 10.0m/s^2)");
    data->acceleration = -129600.0;
  }

  if (data->acceleration == 0 || data->acceleration == 0.0){
    loggerf(DEBUG, "No speed difference");
    data->T->changing_speed = RAILTRAIN_SPEED_T_DONE;
    return;
  }

  data->time = sqrt(2 * (data->acceleration) * real_distance + data->T->speed * data->T->speed) - data->T->speed;
  data->time /= data->acceleration;
  data->time *= 3600; // convert to seconds

  if(data->time < 0.0){
    data->time *= -1.0;
  }
  
  data->steps = (uint16_t)(data->time / 0.5);
  data->stepTime = ((data->time / data->steps) * 1000000L); // convert to usec

  data->startSpeed = data->T->speed;
}

void train_speed_event_init(RailTrain * T){
  loggerf(TRACE, "train_speed_event_init (%i -> %ikm/h, %icm)\n", T->speed, T->target_speed, T->target_distance);

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

  loggerf(TRACE, "train_speed_event_tick %i a:%f, s:(%i/%i), t:%i", data->T->speed, data->acceleration, data->stepCounter, data->steps, data->stepTime);

  RailTrain * T = data->T;

  if(T->changing_speed == RAILTRAIN_SPEED_T_DONE){
    scheduler->disableEvent(T->speed_event);
    return;
  }

  T->speed = (data->startSpeed + data->acceleration * ((data->stepTime * data->stepCounter) / 1000000.0 / 3600.0)) + 0.01; // hours

  T->setSpeedZ21(T->speed);
  WS_stc_UpdateTrain(T);

  if (data->stepCounter >= data->steps){
    T->changing_speed = RAILTRAIN_SPEED_T_DONE;
    scheduler->disableEvent(T->speed_event);
  }

  return;
}

