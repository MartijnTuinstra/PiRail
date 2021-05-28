#include <math.h>

#include "switchboard/station.h"
#include "switchboard/switchsolver.h"
#include "rollingstock/train.h"
#include "train.h"

#include "utils/mem.h"
#include "utils/logger.h"
#include "scheduler/scheduler.h"
#include "system.h"
#include "flags.h"
#include "algorithm/core.h"
#include "algorithm/queue.h"

#include "websocket/stc.h"
#include "Z21_msg.h"

char TrainSpeedStatesStrings[40][20] = {
  "IDLE", "RESUMING", "STOPPING", "STOPPING_REVERSE", "STOPPING_WAIT", "WAITING", "WAITING_DESTINATION",
  "DRIVING", "CHANGING", "UPDATE", "INITIALIZING"
};

uint16_t Train::checkMaxSpeed(){
  uint16_t maxspeed = MaxSpeed;

  for(auto b: blocks){
    uint16_t speed = b->getSpeed();
    if(speed < maxspeed)
      maxspeed = speed;
  }

  return maxspeed;
}

void inline Train::setSpeed(uint16_t _speed){
  // Set speed from either Websocket or Z21

  // Was stopped but starting to move
  if(stopped && _speed && directionKnown){

    // Clear route otherwise ContinueCheck will fail
    if(routeStatus == TRAIN_ROUTE_AT_DESTINATION)
      clearRoute();

    
    loggerf(WARNING, "Train setSpeed ContinueCheck");
    if(!ContinueCheck()){
      loggerf(WARNING, "T%i unsafe to start moving", id);
      return;
    }
    else{
      loggerf(WARNING, "T%i start moving", id);
      Continue();
    }
  }

  _setSpeed(_speed);
}

void inline Train::_setSpeed(uint16_t _speed){
  // set speed from change speed
  setStopped(_speed == 0);

  applySpeed(_speed);
}

void inline Train::applySpeed(uint16_t _speed){
  speed = _speed;

  if(!p.p) return;
  else if(type == TRAIN_ENGINE_TYPE) p.E->setSpeed(speed);
  else p.T->setSpeed(speed);
}

void Train::setStopped(bool stop){
  if(SpeedState == TRAIN_SPEED_IDLE){
    if(stop){
      loggerf(ERROR, "remove expected train");
      for(auto TD: Detectables)
        TD->resetExpectedTrain();

      dereserveAll();
    }
    else{
      loggerf(ERROR, "add expected train");
      SpeedState = TRAIN_SPEED_RESUMING;

      for(auto TD: Detectables)
        TD->setExpectedTrain();
    }

    setStationStopped(stop);
  }
  else if(SpeedState == TRAIN_SPEED_DRIVING && stop){
    SpeedState = TRAIN_SPEED_STOPPING;

    // Set up speed event for IDLE
    speed_event->interval.tv_sec  = 1;
    speed_event->interval.tv_nsec = 5e8;

    scheduler->enableEvent(speed_event);
  }
  else if(SpeedState == TRAIN_SPEED_INITIALIZING){
    setStationStopped(stop);
  }

  loggerf(WARNING, "Train setStopped new state == %s", TrainSpeedStatesStrings[SpeedState]);

  stopped = stop;
}

void Train::setStationStopped(bool stop){
  for(auto b: blocks){
    if(b && b->station){
      b->station->setStoppedTrain(stop);
    }
  }
}

void Train::changeSpeed(uint16_t targetSpeed, uint16_t distance){
  // Change speed over a given distance

  // Create request
  struct TrainSpeedEventRequest Request = {
      .targetSpeed = targetSpeed,
      .distance = distance,
      .reason = TRAIN_SPEED_R_MAXSPEED,
      .ptr = 0
  };

  changeSpeed(Request);
}

void Train::changeSpeed(struct TrainSpeedEventRequest Request){
  // Change speed over a given distance

  if(!assigned){
    loggerf(ERROR, "No Linked TrainSet/Engine");
    return;
  }

  loggerf(INFO, "T %i changeSpeed (=>%3i %3icm %i) \t %s", id, Request.targetSpeed, Request.distance, Request.reason, TrainSpeedStatesStrings[SpeedState]);

  if(Request.targetSpeed > MaxSpeed)
    Request.targetSpeed = MaxSpeed;

  // Train is allready at target speed, therefore cancel speed change.
  if(Request.targetSpeed == speed){
  //   if(SpeedState == TRAIN_SPEED_DONE)
  //     Request.reason = TRAIN_SPEED_R_NONE;
    return;
  }

  // Check if duplicate request
  if(Request.targetSpeed == speed_event_data->target_speed && 
     Request.distance    == speed_event_data->target_distance &&
     Request.reason      == speed_event_data->reason){
    return;
  }

  loggerf(WARNING, "Train %i changeSpeed -> %i km/h", id, Request.targetSpeed);

  // Must take immediate effect
  if(Request.distance == 0){
    SpeedState = TRAIN_SPEED_DRIVING;
    setSpeed(Request.targetSpeed);

    if(assigned)
      WS_stc_UpdateTrain(this);

    return;
  }
  
  // Initialize speed event
  auto ED = speed_event_data;

  // If new distance reset displacement counters
  if(ED->target_distance != Request.distance){
    ED->startDisplacement = 0.0;
    ED->displacement = 0.0;
  }

  // Update values
  ED->target_speed    = Request.targetSpeed;
  ED->target_distance = Request.distance;
  ED->reason          = Request.reason;

  // Add block
  ED->signalBlock = (ED->reason == TRAIN_SPEED_R_SIGNAL) ? (Block *)Request.ptr : 0;

  switch(SpeedState){
    case TRAIN_SPEED_DRIVING:
      SpeedState = TRAIN_SPEED_CHANGING;

      train_speed_event_init(this);
      break;

    case TRAIN_SPEED_CHANGING:
      SpeedState = TRAIN_SPEED_UPDATE;
      clock_gettime(CLOCK_REALTIME, &ED->updateTime);
      break;
    default:
      break;
  }
}

void train_speed_event_calc(struct TrainSpeedEventData * data){
  // v = v0 + at;
  // x = x0 + v0*t + 0.5at^2;
  // a = (v - v0)/t
  // t = 2x/(2v0+(v-v0))
  // a = x/(v0*t + 0.5t^2);
  // t = sqrt(2*(x - v0) / (a))
  float start_speed = data->T->speed * 1.0;

  float real_distance = 160.0 * 0.00001 * (data->target_distance - 5) - data->displacement; // km
  data->startDisplacement = data->displacement;

  data->time = 3600 * 2 * real_distance / (2 * start_speed + (data->target_speed - start_speed)); // seconds

  if(data->time < 0.0){
    data->time *= -1.0;
  }
  data->acceleration = (data->target_speed - start_speed) / data->time; // km/h/s

  if (data->acceleration > 64800.0){ // 5 m/s^2
    loggerf(WARNING, "data->acceleration to large, reduced to 5.0m/s^2)");
    data->acceleration = 64800.0;
  }
  else if (data->acceleration < -129600.0){
    loggerf(WARNING, "Deccell to large, reduced to 10.0m/s^2)");
    data->acceleration = -129600.0;
  }
  else if (data->acceleration == 0.0){
    loggerf(WARNING, "No speed difference");
    data->T->SpeedState = TRAIN_SPEED_DRIVING;
    return;
  }
  
  data->stepCounter = 0;
  uint16_t timeSteps = (uint16_t)(data->time / 0.5);
  uint16_t speedSteps = abs(data->target_speed - start_speed);
  data->steps = ((timeSteps < speedSteps) ? timeSteps : speedSteps);

  if(data->steps == 0){
    loggerf(ERROR, "train_speed_timer_calc has zero steps!!");
    data->steps = 1;
  }

  data->stepTime = ((data->time / data->steps) * 1000000L); // convert to usec

  loggerf(INFO, "train_speed_timer_calc %fkm/h %fkm, %2i steps %6ius", start_speed, real_distance, data->steps, data->stepTime);

  data->startSpeed = data->T->speed;
}

void train_speed_event_init(Train * T){
  loggerf(INFO, "train_speed_event_init (%i -> %ikm/h, %icm)\n", T->speed, T->speed_event_data->target_speed, T->speed_event_data->target_distance);

  auto ED = T->speed_event_data;

  ED->T = T;
  ED->displacement = 0.0;

  train_speed_event_calc(ED);

  // Step time is halved for the start offset
  T->speed_event->interval.tv_sec = ED->stepTime / 2000000L;
  T->speed_event->interval.tv_nsec = (ED->stepTime % 2000000UL) * 500;
  scheduler->enableEvent(T->speed_event);

  // Step time is reset for normal steps after first iterration.
  T->speed_event->interval.tv_sec *= 2;
  T->speed_event->interval.tv_nsec *= 2;
}

void train_speed_event_tick(struct TrainSpeedEventData * data){
  data->stepCounter++;

  Train * T = data->T;

  loggerf(DEBUG, "train_speed_event_tick %i a:%f, s:(%i/%i), t:%i\t%s", data->T->speed, data->acceleration, data->stepCounter, data->steps, data->stepTime, TrainSpeedStatesStrings[T->SpeedState]);

  float t = (data->stepTime * data->stepCounter) / 1000000.0; // seconds

  // Stop the event if done
  switch(T->SpeedState){
    case TRAIN_SPEED_STOPPING_WAIT:
      T->SpeedState = (T->routeStatus == TRAIN_ROUTE_AT_DESTINATION) ? TRAIN_SPEED_WAITING_DESTINATION : TRAIN_SPEED_WAITING;
      T->_setSpeed(0);
      scheduler->disableEvent(T->speed_event);
      return;

    case TRAIN_SPEED_STOPPING:
      T->SpeedState = TRAIN_SPEED_IDLE;
      T->_setSpeed(0);
      scheduler->disableEvent(T->speed_event);
      return;

    case TRAIN_SPEED_STOPPING_REVERSE:
      T->SpeedState = TRAIN_SPEED_IDLE;
      T->_setSpeed(0);
      T->reverse();
      scheduler->disableEvent(T->speed_event);
      return;

    default:
      break;
  }

  if(T->SpeedState != TRAIN_SPEED_CHANGING && T->SpeedState != TRAIN_SPEED_UPDATE){
    scheduler->disableEvent(T->speed_event);
    return;
  }

  // Calculate new speed
  uint16_t speed = (data->startSpeed + data->acceleration * t) + 0.01; 

  // Check if the event should be disabled
  //   either when all steps are done or when a signal released restrictions
  if (data->stepCounter >= data->steps || (data->reason == TRAIN_SPEED_R_SIGNAL && data->signalBlock->getSpeed() != data->target_speed)){
    if( !(data->reason == TRAIN_SPEED_R_SIGNAL && data->signalBlock->getSpeed() != data->target_speed) )
      speed = data->target_speed;

    if(speed){
      T->SpeedState = TRAIN_SPEED_DRIVING;
      scheduler->disableEvent(T->speed_event);
    }
    else{
      loggerf(WARNING, "Setting Stopping(wait)");
      if(data->reason == TRAIN_SPEED_R_SIGNAL || data->reason == TRAIN_SPEED_R_ROUTE)
        T->SpeedState = TRAIN_SPEED_STOPPING_WAIT;
      else
        T->SpeedState = TRAIN_SPEED_STOPPING;

      // Set stopped after 1.5 sec
      T->speed_event->interval.tv_sec = 1;
      T->speed_event->interval.tv_nsec = 5e8;
      // updateEvent is called after Event function
    }

    data->reason = TRAIN_SPEED_R_NONE;
  }

  T->_setSpeed(speed);
  WS_stc_UpdateTrain(T);

  // Update event since some parameters are changed.
  if(T->SpeedState == TRAIN_SPEED_UPDATE){
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);

    // Calculate displacement
    auto diff = now - data->updateTime;
    t = diff.tv_sec + diff.tv_nsec/1.0E9;
    data->displacement = speed * (t / 3600); // (km/h) * h

    // Calculate new curve
    train_speed_event_calc(data);
    T->SpeedState = TRAIN_SPEED_CHANGING;
    return;
  }
  
  data->displacement = data->startDisplacement + data->startSpeed * (t / 3600) + 0.5 * data->acceleration * (t / 3600) * t; // (km/h) * h + (km/h/s) * h * s

  return;
}

