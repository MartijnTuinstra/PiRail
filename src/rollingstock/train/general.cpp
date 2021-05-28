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

Train::Train(Block * B): blocks(0, 0), reservedBlocks(0, 0){
  id = RSManager->addTrain(this);
  loggerf(INFO, "Create Train %i %x", id, this);

  p.p = 0;

  this->B = B;
  setBlock(B);

  if(B->path)
    B->path->trains.push_back(this);

  assigned = false;
  setControl(TRAIN_MANUAL);

  char name[64];
  sprintf(name, "Train_%i_SpeedEvent", id);
  speed_event = scheduler->addEvent(name, {0, 0});
  speed_event_data = (struct TrainSpeedEventData *)_calloc(1, struct TrainSpeedEventData);
  speed_event_data->T = this;
  speed_event->function = (void (*)(void *))train_speed_event_tick;
  speed_event->function_args = (void *)speed_event_data;

  setSpeed(0);
  setStationStopped(stopped);
}

Train::~Train(){
  loggerf(TRACE, "Destroy Train %i %x %i", id, this, Detectables);

  scheduler->removeEvent(speed_event);
  // train_link[id] = 0;
  blocks.clear();
  dereserveAll();

  if(speed_event_data)
    _free(speed_event_data);

  while(Detectables.size() > 0){
    auto D = Detectables[0];
    Detectables.erase(Detectables.begin());
    delete D;
  }

  if(route){
    delete route;
    route = 0;
  }
}

void Train::setBlock(Block * sB){
  loggerf(TRACE, "train %i: setBlock %2i:%2i %x", id, sB->module, sB->id, (unsigned int)sB);
  sB->train = this;
  blocks.push_back(sB);
}

void Train::releaseBlock(Block * rB){
  loggerf(TRACE, "train %i: releaseBlock %2i:%2i %x", id, rB->module, rB->id, (unsigned int)rB);
  rB->train = 0;
  blocks.erase(std::remove_if(blocks.begin(),
                              blocks.end(),
                              [rB](const auto & o) { return (o == rB); }),
               blocks.end());

  if(rB == B)
    B = blocks[0];
}

void Train::reserveBlock(Block * rB){
  loggerf(TRACE, "train %i: reserveBlock %2i:%2i", id, rB->module, rB->id);

  rB->reserve(this);
  reservedBlocks.push_back(rB);
}

void Train::dereserveBlock(Block * rB){
  loggerf(TRACE, "train %i: dereserveBlock %2i:%2i", id, rB->module, rB->id);
  rB->dereserve(this);

  reservedBlocks.erase(std::remove_if(reservedBlocks.begin(),
                                      reservedBlocks.end(),
                                      [rB](const auto & o) { return (o == rB); }),
                       reservedBlocks.end());
}

void Train::dereserveAll(){

  for(auto b: reservedBlocks){
    if(!b)
      continue;
    b->dereserve(this);
  }

  reservedBlocks.clear();
}


void Train::initVirtualBlocks(){
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

        RSManager->removeTrain(tB->train);
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

void Train::setVirtualBlocks(){
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

void Train::initMoveForward(Block * tB){
  // First time a Train moved when not stopped
  loggerf(INFO, "initMoveForward RT %i to block %02i:%02i (%i)", id, tB->module, tB->id, length);
  directionKnown = 1;

  SpeedState = TRAIN_SPEED_DRIVING;

  setBlock(tB);

  int16_t blockLength = (length/10) + tB->length;
  Block * listBlock = tB;

  // Find front of train
  while(blockLength > 0 && listBlock){
    blockLength -= listBlock->length;

    if(listBlock->train == this){
      B = listBlock; // Update front
    }

    listBlock = listBlock->Next_Block(dir == 1 ? PREV : NEXT, 1);
  }

  // Fill lists detectables
  tB = B;

  blockLength = (length/10) + B->length;
  uint8_t DetectableCounter = 0;
  while(blockLength > 0 || tB->train == this){
    auto TD = Detectables[DetectableCounter++];

    TD->initialize(&tB, &blockLength);

    if(!tB)
      break;

    while(blockLength > 0 && !tB->detectionBlocked){
      tB = tB->Next_Block(dir ? NEXT : PREV, 1);

      if(!tB)
        break;
    }

    if(!tB)
      break;
  }

  // Register and reserve paths
  if(B->path)
    B->path->reserve(this, B);

  if(virtualLength)
    setVirtualBlocks();
}

void Train::moveForwardFree(Block * tB){
  loggerf(INFO, "MoveForwardFree RT %i from block %2i:%2i", id, tB->module, tB->id);

  if(!virtualLength){
    releaseBlock(tB);
    AlQueue.put(this); // Put in traincontrol queue for speed control
  }

  for(auto TD: Detectables){
    // Find the detectable that has the front block
    //  then pop it out of the list and reorder

    if(TD->B.size() <= 1)
      continue;

    if(tB != TD->B[TD->B.size() - 1])
      continue;

    TD->B.pop_back();
    TD->BlockedLength -= tB->length;
    
    TD->setExpectedTrain();

    break;
  }
}

void Train::moveForward(Block * tB){
  loggerf(DEBUG, "MoveForward RT %i to block %2i:%2i", id, tB->module, tB->id);
  setBlock(tB);
  dereserveBlock(tB);

  AlQueue.put(this); // Put in traincontrol queue
}

void Train::moveFrontForward(Block * _B){
  B = _B;

  if(virtualLength)
    setVirtualBlocks();

  if(routeStatus){
    if(_B == route->destinationBlocks[0] || _B == route->destinationBlocks[1]){
      if(route->routeType == PATHFINDING_ROUTE_BLOCK)
        routeStatus = TRAIN_ROUTE_AT_DESTINATION;
      else
        routeStatus++;
    }
  }
}

void Train::reverse(){
  if(!assigned || !directionKnown)
    return;

  if(speed != 0){
    setSpeed(0);
    SpeedState = TRAIN_SPEED_STOPPING_REVERSE;
    return;
  }

  if(SpeedState == TRAIN_SPEED_STOPPING){
    loggerf(WARNING, "Train reversing but not stopped");
    SpeedState = TRAIN_SPEED_STOPPING_REVERSE;
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

void Train::reverseFromPath(Path * P){
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

void Train::reverseBlocks(){
  if(!assigned || !directionKnown)
    return;

  for(auto TD: Detectables){
    TD->reverse();
  }

  std::reverse(Detectables.begin(), Detectables.end());

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

  B = Detectables[0]->B[0];
  dir ^= 1;

  setSpeed(0);
}

void Train::Z21_reverse(){
  
}



int Train::link(int tid, char _type){  
  // Link train or engine to Train class.
  //
  //  tID = id of train or engine
  //  type = bool train or engine

  char * name;

  if(assigned)
    return 4;

  // If it is only a engine -> make it a train
  if(_type == TRAIN_ENGINE_TYPE){
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

    Detectables.push_back(new TrainDetectable(this, length/10, length/10));

    // Detectables = 1;
    // DetectedBlocks = (TrainDetectables *)_calloc(Detectables, TrainDetectables);
    // DetectedBlocks[0].DetectableLength = length / 10;

    //Lock engines
    RSManager->subDCCEngine(tid);
    E->use = true;
    E->RT = this;
  }
  else{
    TrainSet * T = RSManager->getTrainSet(tid);

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

    // Detectables = T->detectables;
    // DetectedBlocks = (TrainDetectables *)_calloc(Detectables, TrainDetectables);
    bool detectable = true;
    auto Tcomp = T->composition;
    uint16_t detectableBlockedLength = 0, detectableLength = 0;

    for(uint8_t i = 0; i < T->nr_stock; i++){
      if(!detectable && Tcomp[i].type == TRAIN_ENGINE_TYPE){
        // New detectable
        Detectables.push_back(new TrainDetectable(this, detectableLength/10, detectableBlockedLength/10));
        detectableBlockedLength = 0;
        detectableLength = 0;
        detectable = true;
      }
      else if(detectable && Tcomp[i].type != TRAIN_ENGINE_TYPE){
        detectable = false;
      }

      if(detectable)
        detectableBlockedLength += ((Engine *)Tcomp[i].p)->length;

      if(Tcomp[i].type == TRAIN_ENGINE_TYPE)
        detectableLength += ((Engine *)Tcomp[i].p)->length;
      else
        detectableLength += ((Car *)Tcomp[i].p)->length;
    }

    Detectables.push_back(new TrainDetectable(this, detectableLength/10, detectableBlockedLength/10));

    if(Detectables.size() != T->detectables){
      loggerf(ERROR, "Failed to assign all detectables");
    }

    //Lock all engines
    T->setEnginesUsed(true, this);

    virtualLength = T->virtualDetection;
  }

  assigned = true;

  loggerf(INFO, "Train linked train/engine, %s, length: %i, maxSpeed %d", name, length, MaxSpeed);

  return 1;
}

int Train::link(int tid, char type, uint8_t nrT, Train ** T){  
  // Link train or engine to Train class.
  //
  //  tID = id of train or engine
  //  type = bool train or engine
  //  nrT  = Number of seperate train detectables
  //    T  =  seperate train detectables

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

    RSManager->removeTrain(T[i]);
  }

  return 1;
}

void Train::unlink(){
  //TODO implement Train type
  //Unlock all engines
  if(this->type == TRAIN_ENGINE_TYPE){
    this->p.E->use = 0;
    this->p.E->RT = 0;
  }
  else{
    TrainSet * T = this->p.T;
    T->setEnginesUsed(false, 0);
  }

  this->assigned = false;

  while(Detectables.size() > 0){
    auto D = Detectables[0];
    Detectables.erase(Detectables.begin());
    delete D;
  }

}

bool Train::ContinueCheck(){
  loggerf(DEBUG, "Train %i: ContinueCheck %02i:%02i", id, B->module, B->id);
  if(routeStatus == TRAIN_ROUTE_AT_DESTINATION && stopped)
    return false;

  const std::lock_guard<std::mutex> lock(Algorithm::processMutex);

  // If a block is available
  if(B->Alg.next > 0){
    //if(this->Route && Switch_Check_Path(this->B)){
    //  return true;
    //}

    // If it is blocked by another train dont accelerate
    if(B->Alg.N[0]->blocked && B->Alg.N[0]->train != this){
      loggerf(TRACE, "Train %i: ContinueCheck - false", id);
      return false;
    }

    // If it is not DANGER/BLOCKED then accelerate
    else if(B->Alg.N[0]->state > DANGER){
      loggerf(TRACE, "Train %i: ContinueCheck - true", id);
      return true;
    }
  }

  loggerf(WARNING, "NEW TRAIN::CONTINUECHECK");
  struct SwitchSolver::find f = {0, 0};
  struct rail_link * next = B->NextLink(NEXT);
  Block * nextBlock  = B->Next_Block(NEXT, 1);
  Block * next2Block = B->Next_Block(NEXT, 2);

  if(next->type != RAIL_LINK_E && next->type != RAIL_LINK_R){
    // Next is a (ms)switch
    //  solve it
    f = SwitchSolver::findPath(this, route, B, *next, NEXT | FL_SWITCH_CARE | FL_CONTINUEDANGER);
    loggerf(INFO, "First %i", f.possible);
    if(!f.possible || (nextBlock && nextBlock->state <= DANGER))
      return false;
  }

  if(!B->Alg.next)
    return f.possible;
  else if(B->Alg.N[0]->type != NOSTOP)
    return true;

  next = B->Alg.N[0]->NextLink(NEXT);

  if(next->type != RAIL_LINK_E && next->type != RAIL_LINK_R){
    // Next is a (ms)switch
    //  solve it
    f = SwitchSolver::findPath(this, route, B->Alg.N[0], *next, NEXT | FL_SWITCH_CARE | FL_CONTINUEDANGER);
    loggerf(INFO, "Second %i", f.possible);
    return f.possible && (next2Block && next2Block->state > DANGER);
  }
}

void Train::Continue(){
  struct rail_link * next = B->NextLink(NEXT);

  uint8_t nextLen = B->Alg.next;
  Block * NextBlock = B->Alg.N[0];

  if(next->type != RAIL_LINK_E && next->type != RAIL_LINK_R){
    // Next is a (ms)switch
    //  solve it
    if(SwitchSolver::solve(this, B, B, *next, NEXT | FL_SWITCH_CARE | FL_CONTINUEDANGER))
      return;
  }

  if(!nextLen && NextBlock->type == NOSTOP)
    return;

  next = NextBlock->NextLink(NEXT);

  if(next->type != RAIL_LINK_E && next->type != RAIL_LINK_R){
    // Next is a (ms)switch
    //  solve it
    SwitchSolver::solve(this, B, B->Alg.N[0], *next, NEXT | FL_SWITCH_CARE | FL_CONTINUEDANGER);
  }
}

void Train_ContinueCheck(void * args){
  // Check if trains can accelerate when they are stationary.

  loggerf(TRACE, "Train ContinueCheck");
  for(uint8_t i = 0; i < RSManager->Trains.size; i++){
    Train * T = RSManager->getTrain(i);
    if(!T)
      continue;

    if(T->manual)
      continue;

    loggerf(DEBUG, "Train ContinueCheck %i", i);
    if(T->p.p && T->speed == 0 && T->ContinueCheck()){
      loggerf(ERROR, "Train ContinueCheck accelerating train %i", i);
      T->Continue();
      Block * nextBlock = T->B->Next_Block(NEXT, 2);

      if(nextBlock){
        nextBlock->expectedTrain = T;
      }

      T->changeSpeed(40, 50);
      WS_stc_UpdateTrain(T);
    }
  }
}

