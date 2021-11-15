#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "algorithm/traincontrol.h"

#include "scheduler/scheduler.h"
#include "utils/logger.h"

TrainControl::TrainControl(){
  event = scheduler->addEvent(name, {30, 0});
  event->function = (void (*)(void *))&TrainControl::tick;
  event->function_args = this;
  
  stop();
}
TrainControl::~TrainControl(){
  scheduler->removeEvent(event);
}

void TrainControl::start(uint16_t MaximumTrains){
  scheduler->enableEvent(event);

  maximumTrains = MaximumTrains;

  randomWait = ((rand()/(1.0 + RAND_MAX)) * 7 + 1);
}

void TrainControl::stop(){
  scheduler->disableEvent(event);
}

void TrainControl::tick(){
  loggerf(DEBUG, "TrainControl::tick %x %x", this, randomWait);

  if(randomWait-- == 0){
    // Spawn train if allowed
    if(RunningTrains < maximumTrains && RunningTrains < TrainsOnLayout){
      // Pick random destinations
      rand();

      loggerf(INFO, "TrainControl::createRoute");
    }

    randomWait = ((rand()/(1.0 + RAND_MAX)) * 7 + 1);
  }

}