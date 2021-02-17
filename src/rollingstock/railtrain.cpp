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
  blocks.erase(std::remove_if(blocks.begin(), blocks.end(), [rB](const auto & o) { return (o == rB); }), blocks.end());

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
  loggerf(INFO, "setVirtualBlocks");

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

  while(B->Alg.prev >= offset){
    tB->setVirtualDetection(0);
    if(tB->train && !tB->detectionBlocked)
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
  bool expecting = true;
  while(listBlock && (blockLength > 0 || listBlock->train == this)){
    blockLength -= listBlock->length;

    if(listBlock->train == this){

      if(expecting){
        expecting = false;
        Block * tmp_Block = listBlock->Next_Block(dir == 1 ? PREV : NEXT, 1);

        if(tmp_Block){
          tmp_Block->expectedTrain = this;
        }
      }

      if(listBlock->detectionBlocked){
        struct RailTrainBlocksFifo * DB = &DetectedBlocks[DetectableCounter];
        DB->B[i++] = listBlock;
        DB->Front++;

        for(int8_t j = i - 1; j > 0; j--)
          std::swap(DB->B[j], DB->B[j-1]);

      }
    }
    else if(!expecting){
      if(DetectableCounter + 1 < Detectables){
        DetectableCounter++;
        i = 0;
      }
      expecting = true;
    }

    listBlock = listBlock->Next_Block(dir == 1 ? NEXT : PREV, 1);
  }

  if(virtualLength)
    setVirtualBlocks();
  
}

void RailTrain::moveForward(Block * tB){
  loggerf(INFO, "MoveForward RT %i to block %2i:%2i", id, tB->module, tB->id);
  setBlock(tB);

  for(uint8_t i = 0; i < Detectables; i++){
    struct RailTrainBlocksFifo * DB = &DetectedBlocks[i];
    Block * _B = DB->B[(DB->Front + RAILTRAIN_FIFO_SIZE - 1) % RAILTRAIN_FIFO_SIZE];

    if(_B->Alg.next > 0 && _B->Alg.N[0] == tB){
      Block * nextBlock = _B->Next_Block(NEXT, 2);

      if(nextBlock){
        nextBlock->expectedTrain = this;
      }

      if(i == 0){
        B = tB;

        speedCheck = true;

        if(virtualLength)
          setVirtualBlocks();
      }

      DB->B[DB->Front] = tB;
      DB->Front++;

      break;
    }
  }
}

void inline RailTrain::setSpeed(uint16_t _speed){
  if(stopped && _speed && directionKnown){
    // Was stopped but starting to move
    loggerf(WARNING, "Railtrain setSpeed ContinueCheck");
    if(!ContinueCheck()){
      loggerf(WARNING, "RT%i unsafe to start moving", id);
      setSpeedZ21(0);
    }
  }

  speed = _speed;

  if(speed) setStopped(false);
  else setStopped(true);

  if(!p.p) return;
  else if(type == RAILTRAIN_ENGINE_TYPE) p.E->setSpeed(speed);
  else p.T->setSpeed(speed);
}

void RailTrain::setSpeedZ21(uint16_t _speed){
  loggerf(INFO, "setRailTrain %i Speed -> %i", id, _speed);
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
    if(b && b->station){
      b->station->setStoppedTrain(stop);
    }
  }
}

void RailTrain::changeSpeed(uint16_t _target_speed, uint16_t _length){
  if(!assigned){
    loggerf(ERROR, "No Linked Train/Engine");
    return;
  }

  if(_target_speed == speed){
    if(changing_speed == RAILTRAIN_SPEED_T_DONE)
      speedReason = RAILTRAIN_SPEED_R_NONE;
    return;
  }

  loggerf(DEBUG, "ailTrain::changeSpeed %i     -> %i km/h", id, _target_speed);
  //target_speed = target_speed;

  if(_length == 0){
    changing_speed = RAILTRAIN_SPEED_T_DONE;
    setSpeedZ21(_target_speed);

    if(assigned)
      WS_stc_UpdateTrain(this);
  }
  else
    train_speed_event_create(this, _target_speed, _length);
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

  // Remove expectedTrain
  for(uint8_t i = 0; i < Detectables; i++){
    auto DB = &DetectedBlocks[i];
    Block * _B = DB->B[(DB->Front + RAILTRAIN_FIFO_SIZE - 1) % RAILTRAIN_FIFO_SIZE];
    Block * nextBlock = _B->Next_Block(PREV, 1);

    loggerf(INFO, "Disregarding expectedTrain %02i -> %02i:%02i", _B->id, nextBlock->module, nextBlock->id);

    if(nextBlock->expectedTrain == this)
      nextBlock->expectedTrain = 0;

  }

  // Reverse DetectedBlocks
  uint8_t leftIndex  = 0;
  uint8_t rightIndex = Detectables - 1;
  while(leftIndex < rightIndex)
  {
    /* Copy value from original array to reverse array */
    struct RailTrainBlocksFifo temp;
    memcpy(&temp, &DetectedBlocks[leftIndex], sizeof(struct RailTrainBlocksFifo));
    memcpy(&DetectedBlocks[leftIndex], &DetectedBlocks[rightIndex], sizeof(struct RailTrainBlocksFifo));
    memcpy(&DetectedBlocks[rightIndex], &temp, sizeof(struct RailTrainBlocksFifo));
    
    leftIndex++;
    rightIndex--;
  }

  // Reverse each block in the DetectedBlocks
  for(uint8_t i = 0; i < Detectables; i++){
    auto DB = &DetectedBlocks[i];

    leftIndex  = DB->End;
    rightIndex = DB->Front - 1;
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


  // Add expectedTrain
  if(!reverseDirection){
    for(uint8_t i = 0; i < Detectables; i++){
      auto DB = &DetectedBlocks[i];
      Block * nextBlock = DB->B[(DB->Front + RAILTRAIN_FIFO_SIZE - 1) % RAILTRAIN_FIFO_SIZE]->Next_Block(NEXT, 1);

      loggerf(INFO, "Setting expectedTrain %02i:%02i", nextBlock->module, nextBlock->id);

      if(nextBlock->expectedTrain == 0)
        nextBlock->expectedTrain = this;

    }
  }

  B = DetectedBlocks[0].B[DetectedBlocks[0].Front - 1];
  dir ^= 1;

  setSpeed(0);
}

void RailTrain::reverseZ21(){
  
}


void RailTrain::setRoute(Block * dest){
  // struct pathfindingstep path = pathfinding(T->B, dest);

  route = PathFinding::find(B, dest);

  if(route)
    onroute = 1;
  else
    onroute = 0;

  // if(path.found){
  //   T->route = 1;
  //   T->instructions = path.instructions;
  // }
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
    if(RSManager->getEngine(tid)->use){
      loggerf(ERROR, "Engine allready used");
      return 3;
    }

    // Create train from engine
    Engine * E = RSManager->getEngine(tid);
    type = _type;
    p.E = E;
    MaxSpeed = E->max_speed;
    length = E->length;
    virtualLength = false;

    name = E->name;

    Detectables = 1;

    //Lock engines
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

    //Lock all engines
    T->setEnginesUsed(true, this);

    virtualLength = T->virtualDetection;
  }

  DetectedBlocks = (RailTrainBlocksFifo *)_calloc(Detectables, RailTrainBlocksFifo);

  assigned = true;

  loggerf(INFO, "RailTrain linked train/engine, %s, length: %i", name, length);

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

void RailTrain_ContinueCheck(void * args){
  // Check if trains can accelerate when they are stationary.

  loggerf(TRACE, "RailTrain ContinueCheck");
  for(uint8_t i = 0; i < RSManager->RailTrains.size; i++){
    RailTrain * T = RSManager->getRailTrain(i);
    if(!T)
      continue;

    if(T->manual)
      continue;

    loggerf(WARNING, "RailTrain ContinueCheck %i", i);
    if(T->p.p && T->speed == 0 && T->ContinueCheck()){
      loggerf(ERROR, "RailTrain ContinueCheck accelerating train %i", i);
      T->changeSpeed(40, 50);
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

  if(T->target_distance != distance){
    T->speed_event_data->startDisplacement = 0.0;
    T->speed_event_data->displacement = 0.0;
  }
  T->target_distance = distance;

  if (T->changing_speed == RAILTRAIN_SPEED_T_INIT ||
      T->changing_speed == RAILTRAIN_SPEED_T_DONE){

    T->changing_speed = RAILTRAIN_SPEED_T_CHANGING;

    train_speed_event_init(T);
  }
  else if(T->changing_speed == RAILTRAIN_SPEED_T_CHANGING){
    T->changing_speed = RAILTRAIN_SPEED_T_UPDATE;
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

  float real_distance = 160.0 * 0.00001 * (data->T->target_distance - 5) - data->displacement; // km
  data->startDisplacement = data->displacement;

  loggerf(INFO, "train_speed_timer_calc %icm %ikm/h %fkm/h %fkm", data->T->target_distance, data->T->target_speed, start_speed, real_distance);

  data->time = 3600 * 2 * real_distance / (2 * start_speed + (data->T->target_speed - start_speed)); // seconds

  if(data->time < 0.0){
    data->time *= -1.0;
  }
  data->acceleration = (data->T->target_speed - start_speed) / data->time; // km/h/s

  loggerf(INFO, "                         %fs %fkm/h/s", data->time, data->acceleration);

  loggerf(TRACE, "Train_speed_timer_run (data->acceleration at %f km/h/s)", data->acceleration);
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
  data->steps = (uint16_t)(data->time / 0.5);
  data->stepTime = ((data->time / data->steps) * 1000000L); // convert to usec

  data->startSpeed = data->T->speed;
}

void train_speed_event_init(RailTrain * T){
  loggerf(INFO, "train_speed_event_init (%i -> %ikm/h, %icm)\n", T->speed, T->target_speed, T->target_distance);

  T->speed_event_data->T = T;
  T->speed_event_data->displacement = 0.0;

  train_speed_event_calc(T->speed_event_data);

  loggerf(INFO, "train_speed_event      (a: %fkm/h/s, t: %fs, steps: %i)\n", T->speed_event_data->acceleration, T->speed_event_data->time, T->speed_event_data->steps);

  T->speed_event->interval.tv_sec = T->speed_event_data->stepTime / 1000000L;
  T->speed_event->interval.tv_nsec = (T->speed_event_data->stepTime % 1000000UL) * 1000;
  scheduler->enableEvent(T->speed_event);

  if(T->changing_speed == RAILTRAIN_SPEED_T_DONE)
    return;
}

void train_speed_event_tick(struct TrainSpeedEventData * data){
  data->stepCounter++;

  loggerf(INFO, "train_speed_event_tick %i a:%f, s:(%i/%i), t:%i", data->T->speed, data->acceleration, data->stepCounter, data->steps, data->stepTime);

  RailTrain * T = data->T;

  float t = (data->stepTime * data->stepCounter) / 1000000.0; // seconds

  switch(T->changing_speed){
    case RAILTRAIN_SPEED_T_DONE:
      scheduler->disableEvent(T->speed_event);
      return;
    case RAILTRAIN_SPEED_T_UPDATE:
      // data->displacement = data->startDisplacement + data->startSpeed * (t / 3600) + 0.5 * data->acceleration * (t / 3600) * t; // (km/h) * s + (km/h/s) * s * s
      train_speed_event_calc(data);
      T->changing_speed = RAILTRAIN_SPEED_T_CHANGING;
      break;
  }

  T->speed = (data->startSpeed + data->acceleration * t) + 0.01; 
  data->displacement = data->startDisplacement + data->startSpeed * (t / 3600) + 0.5 * data->acceleration * (t / 3600) * t; // (km/h) * h + (km/h/s) * h * s

  if (data->stepCounter >= data->steps || (T->speedReason == RAILTRAIN_SPEED_R_SIGNAL && T->speedBlock->getSpeed() != T->target_speed)){
    T->changing_speed = RAILTRAIN_SPEED_T_DONE;
    scheduler->disableEvent(T->speed_event);

    T->speed = T->target_speed;
    T->speedReason = RAILTRAIN_SPEED_R_NONE;
  }

  T->setSpeedZ21(T->speed);
  WS_stc_UpdateTrain(T);

  return;
}

