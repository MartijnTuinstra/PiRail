#ifndef _INCLUDE_ALGORITHM_TRAINCONTROL_H
#define _INCLUDE_ALGORITHM_TRAINCONTROL_H

#include <stdint.h>

#include "time.h"
#include "scheduler/scheduler.h"
#include "utils/logger.h"

class TrainControl{
private:
  char name[50] = "TrainControl::tick";
  struct SchedulerEvent * event = 0;
  bool stopped = true;

  uint8_t randomWait = 0;

public:
  uint16_t maximumTrains = 0;   // Maximum number of trains that are allowed on the layout by the controller
  uint16_t RunningTrains = 0;   // Number of trains that is currently active on the layout (not stopped in a yard)
  uint16_t TrainsOnLayout = 0;  // Number of trains on the layout (all and everywhere)

  TrainControl();
  ~TrainControl();

  void start(uint16_t);
  void stop();

  void tick();
};

#endif