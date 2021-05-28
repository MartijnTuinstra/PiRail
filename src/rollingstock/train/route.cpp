#include <math.h>

#include "switchboard/station.h"
#include "switchboard/switchsolver.h"
#include "rollingstock/train.h"
#include "train.h"

#include "utils/mem.h"
#include "utils/logger.h"
// #include "scheduler/scheduler.h"
// #include "system.h"
// #include "flags.h"
// #include "algorithm/core.h"
// #include "algorithm/queue.h"

#include "websocket/stc.h"
// #include "Z21_msg.h"


void Train::setRoute(Block * dest){
  route = PathFinding::find(B, dest);

  if(route && (route->found_forward || route->found_reverse))
    routeStatus = TRAIN_ROUTE_RUNNING;
  else
    routeStatus = TRAIN_ROUTE_DISABLED;

  WS_stc_TrainRouteUpdate(this);
}


void Train::setRoute(Station * dest){
  route = PathFinding::find(B, dest);

  if(route && (route->found_forward || route->found_reverse))
    routeStatus = TRAIN_ROUTE_RUNNING;
  else
    routeStatus = TRAIN_ROUTE_DISABLED;
  
  WS_stc_TrainRouteUpdate(this);
}

void Train::clearRoute(){
  route = 0;
  routeStatus = TRAIN_ROUTE_DISABLED;
  
  WS_stc_TrainRouteUpdate(this);
}
