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

char railtrain_speed_t_strings[5][10] = {"INIT", "CHANGING", "UPDATE", "DONE", "FAIL"};

RailTrain::RailTrain(Block * B): blocks(0, 0), reservedBlocks(0, 0){
  id = RSManager->addRailTrain(this);
  loggerf(INFO, "Create RailTrain %i %x", id, this);

  p.p = 0;

  Detectables = 0;
  DetectedBlocks = 0;

  this->B = B;
  setBlock(B);

  if(B->path)
    B->path->trains.push_back(this);

  assigned = false;
  setControl(TRAIN_MANUAL);

  char name[64];
  sprintf(name, "Railtrain_%i_SpeedEvent", id);
  speed_event = scheduler->addEvent(name, {0, 0});
  speed_event_data = (struct TrainSpeedEventData *)_calloc(1, struct TrainSpeedEventData);
  speed_event->function = (void (*)(void *))train_speed_event_tick;
  speed_event->function_args = (void *)speed_event_data;

  setSpeed(0);
}

RailTrain::~RailTrain(){
  loggerf(TRACE, "Destroy RailTrain %i %x %i", id, this, Detectables);

  scheduler->removeEvent(speed_event);
  // train_link[id] = 0;
  blocks.clear();
  dereserveAll();

  if(speed_event_data)
    _free(speed_event_data);

  if(DetectedBlocks)
    _free(DetectedBlocks);

  if(route){
    delete route;
    route = 0;
  }
}

void RailTrain::setBlock(Block * sB){
  loggerf(TRACE, "train %i: setBlock %2i:%2i %x", id, sB->module, sB->id, (unsigned int)sB);
  sB->train = this;
  blocks.push_back(sB);
}

void RailTrain::releaseBlock(Block * rB){
  loggerf(TRACE, "train %i: releaseBlock %2i:%2i %x", id, rB->module, rB->id, (unsigned int)rB);
  rB->train = 0;
  blocks.erase(std::remove_if(blocks.begin(),
                              blocks.end(),
                              [rB](const auto & o) { return (o == rB); }),
               blocks.end());

  if(rB == B)
    B = blocks[0];
}

void RailTrain::reserveBlock(Block * rB){
  loggerf(TRACE, "train %i: reserveBlock %2i:%2i", id, rB->module, rB->id);

  rB->reserve(this);
  reservedBlocks.push_back(rB);
}

void RailTrain::dereserveBlock(Block * rB){
  loggerf(TRACE, "train %i: dereserveBlock %2i:%2i", id, rB->module, rB->id);
  rB->dereserve(this);

  reservedBlocks.erase(std::remove_if(reservedBlocks.begin(),
                                      reservedBlocks.end(),
                                      [rB](const auto & o) { return (o == rB); }),
                       reservedBlocks.end());
}

void RailTrain::dereserveAll(){

  for(auto b: reservedBlocks){
    if(!b)
      continue;
    b->dereserve(this);
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

        RSManager->removeRailTrain(tB->train);
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
    if(tB->train && !tB->detectionBlocked)
      tB->train->releaseBlock(tB);
    AlQueue.puttemp(tB);

    // loggerf(INFO, "f block %2i:%2i", tB->module, tB->id);
  }
}

void RailTrain::setVirtualBlocks(){
  loggerf(INFO, "setVirtualBlocks (T: %i, prev: %i, lenght: %icm)", id, B->Alg.prev, length/10);

  if(!directionKnown)
    return;

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

    loggerf(INFO, "  block %2i:%2i   %i  %i  %i", tB->module, tB->id, B->Alg.prev, offset, len);

    if(B->Alg.prev > offset){
      tB = B->Alg.P[offset++];
    }
    else
      break;
  }

  // Blocks must be released in oposite order. Container for storage.
  Block * ReleaseBlocks[10];
  uint8_t BlocksToRelease = 0;

  // Find blocks to be released
  while(B->Alg.prev > offset){
    if(tB->train != this)
      break;

    tB->setVirtualDetection(0);
    if(tB->train && !tB->detectionBlocked)
      tB->train->releaseBlock(tB);

    ReleaseBlocks[BlocksToRelease++] = tB;

    if(B->Alg.prev > offset)
      tB = B->Alg.P[offset++];
    // loggerf(INFO, "f block %2i:%2i", tB->module, tB->id);
  }

  // Release blocks
  for(int i = BlocksToRelease - 1; i >= 0; i--){
    loggerf(INFO, "--block %2i:%2i", ReleaseBlocks[i]->module, ReleaseBlocks[i]->id);
    AlQueue.puttemp(ReleaseBlocks[i]);

    if(ReleaseBlocks[i]->module == 25 &&ReleaseBlocks[i]->id == 0){
      loggerf(INFO, "SOME DING");
    }
  }

  AlQueue.cpytmp();
}

void RailTrain::initMoveForward(Block * tB){
  // First time a RailTrain moved when not stopped
  loggerf(INFO, "initMoveForward RT %i to block %02i:%02i (%i)", id, tB->module, tB->id, length);
  directionKnown = 1;

  setBlock(tB);

  int32_t blockLength = length + tB->length;
  Block * listBlock = tB;

  // Find front of train
  while(blockLength > 0 && listBlock){
    blockLength -= listBlock->length;

    if(listBlock->train == this){
      B = listBlock; // Update front
    }

    listBlock = listBlock->Next_Block(dir == 1 ? PREV : NEXT, 1);
  }

  // Fill buffer
  listBlock = B;
  blockLength = length + B->length;
  uint8_t i = 0;
  uint8_t DetectableCounter = 0;
  while(blockLength > 0 || listBlock->train == this){
    blockLength -= listBlock->length;

    if(listBlock->train == this && listBlock->detectionBlocked){
      struct RailTrainDetectables * DB = &DetectedBlocks[DetectableCounter];
      listBlock->expectedTrain = this;
      DB->B[i++] = listBlock;
      DB->BlockedLength += listBlock->length;;
      DB->BlockedBlocks++;

      for(int8_t j = i - 1; j > 0; j--)
        std::swap(DB->B[j], DB->B[j-1]);

    }

    Block * oldListBlock = listBlock;
    listBlock = listBlock->Next_Block(dir ? NEXT : PREV, 1);

    if(!listBlock)
      break;

    if(!listBlock->detectionBlocked && oldListBlock->detectionBlocked)
      DetectableCounter++;
  }

  for(uint8_t i = 0; i < Detectables; i++){
    auto DB = &DetectedBlocks[i];
    
    // char debug[1000] = "RailTrain initMoveForward Detectable ";
    // char * ptr = &debug[strlen(debug)];
    // ptr += sprintf(ptr, "(%i - %i - %3i > %3icm)\n", i, DB->BlockedBlocks, DB->BlockedLength - DB->B[DB->BlockedBlocks - 1]->length, DB->DetectableLength);
    // for(uint8_t j = 0; j < DB->BlockedBlocks; j++){
    //   ptr += sprintf(ptr, " %2i:%2i \n", DB->B[j] ? DB->B[j]->module : -1, DB->B[j] ? DB->B[j]->id : -1);
    // }
    // loggerf(INFO, "%s", debug);

    if(DB->BlockedBlocks){
      if(DB->BlockedLength - DB->B[DB->BlockedBlocks - 1]->length > DB->DetectableLength)
        DB->B[0]->expectedTrain = 0; // Clear expectedTrain
      
      Block * tmpBlock = DB->B[DB->BlockedBlocks - 1]->Next_Block(dir ? PREV : NEXT, 1);
      if(tmpBlock) // Set new expectedTrain
        tmpBlock->expectedTrain = this;
    }
    
  }

  if(virtualLength)
    setVirtualBlocks();
}

void RailTrain::moveForwardFree(Block * tB){
  loggerf(INFO, "MoveForwardFree RT %i from block %2i:%2i", id, tB->module, tB->id);

  if(!virtualLength){
    releaseBlock(tB);
    AlQueue.put(this); // Put in traincontrol queue for speed control
  }

  for(uint8_t i = 0; i < Detectables; i++){
    struct RailTrainDetectables * DB = &DetectedBlocks[i];

    // char debug[1000] = "RailTrain MoveForward Detectable ";
    // char * ptr = &debug[strlen(debug)];
    // ptr += sprintf(ptr, "(%i - %i)\n", i, DB->BlockedBlocks);
    // for(uint8_t j = 0; j < DB->BlockedBlocks; j++){
    //   ptr += sprintf(ptr, " %2i:%2i \n", DB->B[j] ? DB->B[j]->module : -1, DB->B[j] ? DB->B[j]->id : -1);
    // }
    // loggerf(INFO, "%s", debug);

    if(tB != DB->B[0])
      continue;

    DB->B[0] = 0x0;
    for(uint8_t j = 1; j < DB->BlockedBlocks; j++){
      std::swap(DB->B[j-1], DB->B[j]);
    }
    DB->BlockedBlocks--;
    DB->BlockedLength -= tB->length;
    
    if(DB->BlockedLength - DB->B[DB->BlockedBlocks - 1]->length > DB->DetectableLength)
      DB->B[0]->expectedTrain = 0;

    break;
  }
}

void RailTrain::moveForward(Block * tB){
  loggerf(DEBUG, "MoveForward RT %i to block %2i:%2i", id, tB->module, tB->id);
  setBlock(tB);
  dereserveBlock(tB);

  AlQueue.put(this); // Put in traincontrol queue

  for(uint8_t i = 0; i < Detectables; i++){
    struct RailTrainDetectables * DB = &DetectedBlocks[i];
    if(!DB->BlockedBlocks)
      continue;

    Block * _B = DB->B[DB->BlockedBlocks - 1];

    // char debug[1000] = "RailTrain MoveForward Detectable ";
    // char * ptr = &debug[strlen(debug)];
    // ptr += sprintf(ptr, "(%i - %i)\n", i, DB->BlockedBlocks);
    // for(uint8_t j = 0; j < DB->BlockedBlocks; j++){
    //   ptr += sprintf(ptr, " %2i:%2i \n", DB->B[j] ? DB->B[j]->module : -1, DB->B[j] ? DB->B[j]->id : -1);
    // }
    // loggerf(INFO, "%s", debug);

    if(_B->Alg.next > 0 && _B->Alg.N[0] == tB){
      Block * nextBlock = _B->Next_Block(NEXT, 2);

      if(nextBlock){
        nextBlock->expectedTrain = this;
      }

      if(i == 0){
        moveFrontForward(tB);
      }

      DB->BlockedLength += tB->length;
      DB->B[DB->BlockedBlocks] = tB;
      DB->BlockedBlocks++;

      if(DB->BlockedBlocks == RAILTRAIN_FIFO_SIZE){
        loggerf(ERROR, "RailTrain has more than %i blocks", RAILTRAIN_FIFO_SIZE);
        changeSpeed(0, 0);
        setControl(TRAIN_MANUAL);
      }

      if(DB->BlockedLength - DB->B[DB->BlockedBlocks - 1]->length > DB->DetectableLength)
        DB->B[0]->expectedTrain = 0;

      break;
    }
  }
}

void RailTrain::moveFrontForward(Block * _B){
  B = _B;

  if(virtualLength)
    setVirtualBlocks();

  if(routeStatus){
    if(_B == route->destinationBlocks[0] || _B == route->destinationBlocks[1]){
      if(route->routeType == PATHFINDING_ROUTE_BLOCK)
        routeStatus = RAILTRAIN_ROUTE_AT_DESTINATION;
      else
        routeStatus++;
    }
  }
}

void inline RailTrain::setSpeed(uint16_t _speed){
  // Was stopped but starting to move
  if(stopped && _speed && directionKnown){

    // Clear route otherwise ContinueCheck will fail
    if(routeStatus == RAILTRAIN_ROUTE_AT_DESTINATION)
      clearRoute();

    
    loggerf(WARNING, "Railtrain setSpeed ContinueCheck");
    if(!ContinueCheck()){
      loggerf(WARNING, "RT%i unsafe to start moving", id);
      _speed = 0;
    }
  }

  speed = _speed;

  if(speed) setStopped(false);
  else setStopped(true);

  if(!p.p) return;
  else if(type == RAILTRAIN_ENGINE_TYPE) p.E->setSpeed(speed);
  else p.T->setSpeed(speed);
}

void RailTrain::setStopped(bool stop){
  stopped = stop;

  if(stop)
    dereserveAll();

  for(auto b: blocks){
    if(b && b->station){
      b->station->setStoppedTrain(stop);
    }
  }
}

void RailTrain::changeSpeed(uint16_t targetSpeed, uint16_t distance){
  struct TrainSpeedEventRequest Request = {
      .targetSpeed = targetSpeed,
      .distance = distance,
      .reason = RAILTRAIN_SPEED_R_MAXSPEED,
      .ptr = 0
  };

  changeSpeed(Request);
}

void RailTrain::changeSpeed(struct TrainSpeedEventRequest Request){
  if(!assigned){
    loggerf(ERROR, "No Linked Train/Engine");
    return;
  }

  loggerf(INFO, "RT %i changeSpeed (%3i %3i %i) \t %s", id, Request.targetSpeed, Request.distance, Request.reason, railtrain_speed_t_strings[changing_speed]);

  if(Request.targetSpeed > MaxSpeed)
    Request.targetSpeed = MaxSpeed;

  if(Request.targetSpeed == speed){
    if(changing_speed == RAILTRAIN_SPEED_T_DONE)
      Request.reason = RAILTRAIN_SPEED_R_NONE;
    return;
  }

  if(Request.targetSpeed == speed_event_data->target_speed && 
     Request.distance == speed_event_data->target_distance &&
     Request.reason == speed_event_data->reason){
    return; // Same command
  }

  loggerf(WARNING, "RailTrain %i changeSpeed -> %i km/h", id, Request.targetSpeed);
  //target_speed = target_speed;

  if(Request.distance == 0){
    changing_speed = RAILTRAIN_SPEED_T_DONE;
    setSpeed(Request.targetSpeed);

    if(assigned)
      WS_stc_UpdateTrain(this);
  }
  else
    train_speed_event_create(this, Request);
}

void RailTrain::reverse(){
  if(!assigned || !directionKnown)
    return;

  if(speed != 0){
    setSpeed(0);
    return;
  }

  if(reverseDirection){
    reverseBlocks();
    return;
  }

  std::vector<Path *> paths;
  for(auto b: blocks){
    if(b->type == NOSTOP || !b->path)
      continue;

    bool pathFound = false;
    for(auto p: paths){
      if(p == b->path)
        pathFound = true;
    }

    if(!pathFound)
      paths.push_back(b->path);
  }

  bool reversed = false;
  for(auto p: paths){
    if(reversed){
      p->reverse(this);
    }
    else{
      p->reverse();
      reversed = true;
    }
  }
}

void RailTrain::reverseFromPath(Path * P){
  bool otherPath = false;

  for(auto b: blocks){
    if(b->type == NOSTOP)
      b->reverse();

    else if(b->path && b->path != P)
      otherPath = true;
  }

  P->dereserve(this);

  if(otherPath){
    reverseDirection = true;
  }

  reverseBlocks();
}

void RailTrain::reverseBlocks(){
  if(!assigned || !directionKnown)
    return;

  // Remove expectedTrain from the front
  // Reset  expectedTrain at the rear as it is still blocked and becomes the front
  for(uint8_t i = 0; i < Detectables; i++){
    auto DB = &DetectedBlocks[i];
    if(!DB->BlockedBlocks)
      continue;

    if(DB->B[0]->expectedTrain == 0)
      DB->B[0]->expectedTrain = this;

    Block * nextBlock = DB->B[DB->BlockedBlocks - 1]->Next_Block(PREV, 1);

    if(nextBlock->expectedTrain == this)
      nextBlock->expectedTrain = 0;

  }

  // Reverse DetectedBlocks
  uint8_t leftIndex  = 0;
  uint8_t rightIndex = Detectables - 1;
  while(leftIndex < rightIndex)
  {
    /* Copy value from original array to reverse array */
    struct RailTrainDetectables temp;
    memcpy(&temp, &DetectedBlocks[leftIndex], sizeof(struct RailTrainDetectables));
    memcpy(&DetectedBlocks[leftIndex], &DetectedBlocks[rightIndex], sizeof(struct RailTrainDetectables));
    memcpy(&DetectedBlocks[rightIndex], &temp, sizeof(struct RailTrainDetectables));
    
    leftIndex++;
    rightIndex--;
  }

  // Reverse each block in the DetectedBlocks
  for(uint8_t i = 0; i < Detectables; i++){
    auto DB = &DetectedBlocks[i];

    leftIndex  = 0;
    rightIndex = DB->BlockedBlocks - 1;
    while(leftIndex < rightIndex)
    {
      /* Copy value from original array to reverse array */
      Block * tmp_Block = DB->B[leftIndex];
      DB->B[leftIndex]  = DB->B[rightIndex];
      DB->B[rightIndex] = tmp_Block;
      
      leftIndex++;
      rightIndex--;
    }
  }

  // Reverse all blocks that the train is occupying
  //  Paths are allready reversed.
  auto switchBlocks = std::vector<Block *>();

  for(auto b: blocks){
    if(b->type == NOSTOP){
      switchBlocks.push_back(b);
    }
  }

  for(auto b: switchBlocks)
    b->reverse();


  // Add expectedTrain to the front
  // remove expectedTrain from the rear
  if(!reverseDirection){
    for(uint8_t i = 0; i < Detectables; i++){
      auto DB = &DetectedBlocks[i];
      if(!DB->BlockedBlocks)
        continue;

      Block * nextBlock = DB->B[DB->BlockedBlocks - 1]->Next_Block(NEXT, 1);
      
      if(DB->BlockedLength - DB->B[DB->BlockedBlocks - 1]->length > DB->DetectableLength)
        DB->B[0]->expectedTrain = 0;

      if(nextBlock->expectedTrain == 0)
        nextBlock->expectedTrain = this;

    }
  }

  B = DetectedBlocks[0].B[DetectedBlocks[0].BlockedBlocks - 1];
  dir ^= 1;

  setSpeed(0);
}

void RailTrain::Z21_reverse(){
  
}


void RailTrain::setRoute(Block * dest){
  route = PathFinding::find(B, dest);

  if(route && (route->found_forward || route->found_reverse))
    routeStatus = RAILTRAIN_ROUTE_RUNNING;
  else
    routeStatus = RAILTRAIN_ROUTE_DISABLED;
}


void RailTrain::setRoute(Station * dest){
  route = PathFinding::find(B, dest);

  if(route && (route->found_forward || route->found_reverse))
    routeStatus = RAILTRAIN_ROUTE_RUNNING;
  else
    routeStatus = RAILTRAIN_ROUTE_DISABLED;
}

void RailTrain::clearRoute(){
  route = 0;
  routeStatus = RAILTRAIN_ROUTE_DISABLED;
}


int RailTrain::link(int tid, char _type){  
  // Link train or engine to RailTrain class.
  //
  //  tID = id of train or engine
  //  type = bool train or engine

  char * name;

  if(assigned)
    return 4;

  // If it is only a engine -> make it a train
  if(_type == RAILTRAIN_ENGINE_TYPE){
    Engine * E = RSManager->getEngine(tid);

    if(E->use){
      loggerf(ERROR, "Engine allready used");
      return 3;
    }

    if(RSManager->getEngineDCC(E->DCC_ID) != NULL){
      loggerf(ERROR, "Engine with same DCC address allready on layout");
      return 4;
    }

    // Create train from engine
    type = _type;
    p.E = E;
    MaxSpeed = E->max_speed;
    length = E->length;
    virtualLength = false;

    name = E->name;

    Detectables = 1;
    DetectedBlocks = (RailTrainDetectables *)_calloc(Detectables, RailTrainDetectables);
    DetectedBlocks[0].DetectableLength = length / 10;

    //Lock engines
    RSManager->subDCCEngine(tid);
    E->use = true;
    E->RT = this;
  }
  else{
    Train * T = RSManager->getTrain(tid);

    if(T->enginesUsed()){
      loggerf(ERROR, "Engine of Train allready used");
      return 3;
    }

    // Crate Rail Train
    type = _type;
    p.T = T;
    MaxSpeed = T->max_speed;
    length = T->length;

    name = T->name;

    Detectables = T->detectables;
    DetectedBlocks = (RailTrainDetectables *)_calloc(Detectables, RailTrainDetectables);
    uint8_t DB_counter = 0;
    bool detectable = true;
    auto Tcomp = T->composition;
    for(uint8_t i = 0; i < T->nr_stock; i++){
      if(!detectable && Tcomp[i].type == RAILTRAIN_ENGINE_TYPE){
        DB_counter++;
        detectable = true;
      }
      else if(detectable && Tcomp[i].type != RAILTRAIN_ENGINE_TYPE){
        detectable = false;
      }

      if(detectable)
        DetectedBlocks[DB_counter].DetectableLength += ((Engine *)Tcomp[i].p)->length / 10;
    }

    //Lock all engines
    T->setEnginesUsed(true, this);

    virtualLength = T->virtualDetection;
  }

  assigned = true;

  loggerf(INFO, "RailTrain linked train/engine, %s, length: %i, maxSpeed %d", name, length, MaxSpeed);

  return 1;
}

int RailTrain::link(int tid, char type, uint8_t nrT, RailTrain ** T){  
  // Link train or engine to RailTrain class.
  //
  //  tID = id of train or engine
  //  type = bool train or engine
  //  nrT  = Number of seperate railtrain detectables
  //    T  =  seperate railtrain detectables

  for(uint8_t i = 0; i < nrT; i++){
    if(T[i]->assigned)
      return 3;
  }

  int ret = link(tid, type);

  if(ret != 1){
    return ret;
  }

  for(uint8_t i = 0; i < nrT; i++){
    for(auto b: T[i]->blocks){
      setBlock(b);
      if(b->path){
        b->path->dereserve(T[i]);
        b->path->unreg(T[i]);
      }
    }

    RSManager->removeRailTrain(T[i]);
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
    T->setEnginesUsed(false, 0);
  }

  this->assigned = false;

  _free(DetectedBlocks);
  Detectables = 0;
}

bool RailTrain::ContinueCheck(){
  loggerf(DEBUG, "RailTrain %i: ContinueCheck %02i:%02i", id, B->module, B->id);
  if(routeStatus == RAILTRAIN_ROUTE_AT_DESTINATION && stopped)
    return false;

  if(B->Alg.next > 0){
    //if(this->Route && Switch_Check_Path(this->B)){
    //  return true;
    //}
    if(B->Alg.N[0]->blocked && B->Alg.N[0]->train != this){
      loggerf(TRACE, "RailTrain %i: ContinueCheck - false", id);
      return false;
    }
    else if(B->Alg.N[0]->state > DANGER){
      loggerf(TRACE, "RailTrain %i: ContinueCheck - true", id);
      return true;
    }
  }
  struct rail_link * next = B->NextLink(NEXT);

  loggerf(DEBUG, "next type %i", next->type);

  if(next->type != RAIL_LINK_E){
    if(SwitchSolver::solve(this, B, B, *next, NEXT | SWITCH_CARE)){
      AlQueue.put(B);

      loggerf(TRACE, "RailTrain %i: ContinueCheck - true", id);
      return true;
    }
  }

  loggerf(TRACE, "RailTrain %i: ContinueCheck - false", id);
  return false;
}

uint16_t RailTrain::checkMaxSpeed(){
  uint16_t ms = MaxSpeed;

  for(auto b: blocks){
    if(b->BlockMaxSpeed < ms)
      ms = b->BlockMaxSpeed;
  }

  return ms;
}

void RailTrain_ContinueCheck(void * args){
  // Check if trains can accelerate when they are stationary.

  loggerf(TRACE, "RailTrain ContinueCheck");
  for(uint8_t i = 0; i < RSManager->RailTrains.size; i++){
    RailTrain * T = RSManager->getRailTrain(i);
    if(!T)
      continue;

    if(T->manual)
      continue;

    loggerf(DEBUG, "RailTrain ContinueCheck %i", i);
    if(T->p.p && T->speed == 0 && T->ContinueCheck()){
      loggerf(ERROR, "RailTrain ContinueCheck accelerating train %i", i);
      T->changeSpeed(40, 50);
      WS_stc_UpdateTrain(T);
    }
  }
}


void train_speed_event_create(RailTrain * T, struct TrainSpeedEventRequest Request){
  loggerf(TRACE, "train_speed_event_create %i", T->changing_speed);

  auto ED = T->speed_event_data;

  if (T->speed == Request.targetSpeed){
    return;
  }

  ED->target_speed = Request.targetSpeed;

  if(ED->target_distance != Request.distance){
    ED->startDisplacement = 0.0;
    ED->displacement = 0.0;
  }
  ED->target_distance = Request.distance;
  ED->reason = Request.reason;

  if(ED->reason == RAILTRAIN_SPEED_R_SIGNAL)
    ED->signalBlock = (Block *)Request.ptr;

  if (T->changing_speed == RAILTRAIN_SPEED_T_INIT ||
      T->changing_speed == RAILTRAIN_SPEED_T_DONE){

    T->changing_speed = RAILTRAIN_SPEED_T_CHANGING;

    train_speed_event_init(T);
  }
  else if(T->changing_speed == RAILTRAIN_SPEED_T_CHANGING){
    T->changing_speed = RAILTRAIN_SPEED_T_UPDATE;
    clock_gettime(CLOCK_REALTIME, &ED->updateTime);
  }
  else{
    return;
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

  // loggerf(TRACE, "Train_speed_timer_run (data->acceleration at %f km/h/s)", data->acceleration);
  if (data->acceleration > 64800.0){ // 5 m/s^2
    loggerf(WARNING, "data->acceleration to large, reduced to 5.0m/s^2)");
    data->acceleration = 64800.0;
  }
  else if (data->acceleration < -129600.0){
    loggerf(WARNING, "Deccell to large, reduced to 10.0m/s^2)");
    data->acceleration = -129600.0;
  }

  if (data->acceleration == 0 || data->acceleration == 0.0){
    loggerf(WARNING, "No speed difference");
    data->T->changing_speed = RAILTRAIN_SPEED_T_DONE;
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

void train_speed_event_init(RailTrain * T){
  loggerf(INFO, "train_speed_event_init (%i -> %ikm/h, %icm)\n", T->speed, T->speed_event_data->target_speed, T->speed_event_data->target_distance);

  auto ED = T->speed_event_data;

  ED->T = T;
  ED->displacement = 0.0;

  train_speed_event_calc(ED);

  T->speed_event->interval.tv_sec = ED->stepTime / 2000000L;
  T->speed_event->interval.tv_nsec = (ED->stepTime % 2000000UL) * 500;
  scheduler->enableEvent(T->speed_event);

  loggerf(INFO, "speedtimer event interval %ud %ud", T->speed_event->interval.tv_sec, T->speed_event->interval.tv_nsec);

  T->speed_event->interval.tv_sec *= 2;
  T->speed_event->interval.tv_nsec *= 2;

  loggerf(INFO, "speedtimer event interval %ud %ud", T->speed_event->interval.tv_sec, T->speed_event->interval.tv_nsec);

  if(T->changing_speed == RAILTRAIN_SPEED_T_DONE)
    return;
}

void train_speed_event_tick(struct TrainSpeedEventData * data){
  data->stepCounter++;

  loggerf(INFO, "train_speed_event_tick %i a:%f, s:(%i/%i), t:%i", data->T->speed, data->acceleration, data->stepCounter, data->steps, data->stepTime);

  RailTrain * T = data->T;

  float t = (data->stepTime * data->stepCounter) / 1000000.0; // seconds

  if(T->changing_speed == RAILTRAIN_SPEED_T_DONE){
    scheduler->disableEvent(T->speed_event);
    return;
  }

  T->speed = (data->startSpeed + data->acceleration * t) + 0.01; 

  if (data->stepCounter >= data->steps || (data->reason == RAILTRAIN_SPEED_R_SIGNAL && data->signalBlock->getSpeed() != data->target_speed)){
    T->changing_speed = RAILTRAIN_SPEED_T_DONE;
    scheduler->disableEvent(T->speed_event);
    
    if( !(data->reason == RAILTRAIN_SPEED_R_SIGNAL && data->signalBlock->getSpeed() != data->target_speed) ){
      loggerf(INFO, "train_speed_event_tick done idk why");
      T->speed = data->target_speed;
    }
    else
      loggerf(INFO, "train_speed_event_tick done free block");

    data->signalBlock = 0;
    data->reason = RAILTRAIN_SPEED_R_NONE;
  }

  T->setSpeed(T->speed);
  WS_stc_UpdateTrain(T);

  if(T->changing_speed == RAILTRAIN_SPEED_T_UPDATE){
    // auto diff = T->speed_event->next_interval - data->updateTime;
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    auto diff = now - data->updateTime;
    t = diff.tv_sec + diff.tv_nsec/1.0E9;
    data->displacement = T->speed * (t / 3600); // (km/h) * h
    train_speed_event_calc(data);
    T->changing_speed = RAILTRAIN_SPEED_T_CHANGING;
    return;
  }
  
  data->displacement = data->startDisplacement + data->startSpeed * (t / 3600) + 0.5 * data->acceleration * (t / 3600) * t; // (km/h) * h + (km/h/s) * h * s

  return;
}

