#include <math.h>

#include "switchboard/station.h"
#include "switchboard/switchsolver.h"
#include "rollingstock/train.h"
#include "train.h"
#include "path.h"
#include "pathfinding.h"

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

  if(B->path){
    B->path->trainEnter(this);
    enterPath(B->path);
  }

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
  loggerf(DEBUG, "train %i: setBlock %2i:%2i %x", id, sB->module, sB->id, (unsigned long)sB);
  sB->train = this;
  blocks.push_back(sB);
}

void Train::setBlock(std::vector<Block *>::iterator I, Block * sB){
  loggerf(DEBUG, "train %i: setBlock %2i:%2i %x", id, sB->module, sB->id, (unsigned long)sB);
  sB->train = this;
  blocks.insert(I, sB);
}

void Train::releaseBlock(Block * rB){
  loggerf(DEBUG, "train %i: releaseBlock %2i:%2i %x", id, rB->module, rB->id, (unsigned long)rB);
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
  
  algor_blocks * blockGroup;
  if(!dir)
    blockGroup = B->Alg.P;
  else
    blockGroup = B->Alg.N;
  Block * tB = blockGroup->B[0];


  loggerf(WARNING, "initVirtualBlocks %02i:%02i", B->module, B->id);  

  if(!directionKnown)
    return;

  uint8_t offset = 1;
  int16_t len = length / 10;

  B->setVirtualDetection(1);

  while(len > 0){
    len -= tB->length;

    tB->setVirtualDetection(1);

    if(tB->train != this){
      if(tB->train){
        if(tB->train->assigned)
          loggerf(CRITICAL, "Virtual Train intersects with other train");

        loggerf(INFO, "Virtual Train intersects with empty train");

        RSManager->removeTrain(tB->train);
      }
      setBlock(blocks.begin(), tB);
    }
    AlQueue.puttemp(tB);

    loggerf(INFO, "  block %2i:%2i   %i  %i  %i", tB->module, tB->id, blockGroup->group[3], offset, len);

    if(blockGroup->group[3] > offset){
      tB = blockGroup->B[offset++];
    }
    else
      break;
  }

  if(blockGroup->group[3] > offset){
    tB->setVirtualDetection(0);
    if(tB->train && !tB->detectionBlocked)
      tB->train->releaseBlock(tB);
    AlQueue.puttemp(tB);

    loggerf(INFO, "f block %2i:%2i", tB->module, tB->id);
  }
}

uint8_t Train::setVirtualBlocks(algor_blocks * blockGroup, uint16_t length){
  // FIXME, more than 10 blocks away of the start block cannot work
  Block * tB = blockGroup->B[0];

  loggerf(INFO, "setVirtualBlocks (%02i:%02i, prev: %i, lenght: %icm)", B->module, B->id, blockGroup->group[3], length/10);

  if(!length)
    return 0;
  
  uint8_t offset = 0;
  int16_t len = length / 10; // blockGroup->B[0]->length + 
  while(len > 0){
    len -= tB->length;

    tB->setVirtualDetection(1);

    if(tB->train != this)
      setBlock(blocks.begin(), tB);

    AlQueue.puttemp(tB);

    loggerf(INFO, "  block %2i:%2i   %i  %i  %i", tB->module, tB->id, blockGroup->group[3], offset, len);

    if(blockGroup->group[3] > (offset + 1)){
      tB = blockGroup->B[++offset];
    }
    else
      break;
  }

  return offset;
}

uint8_t Train::releaseVirtualBlocks(Algor_Blocks * blockGroup, uint8_t offset){
  // FIXME, more than 10 blocks away of the start block cannot work
  Block * tB = blockGroup->B[offset];
  loggerf(INFO, "releaseVirtualBlocks (%02i:%02i, prev: %i, lenght: %icm)", tB ? tB->module : 0, tB ? tB->id : 0, blockGroup->group[3], length/10);
  
  // Blocks must be released in oposite order. Container for storage.
  Block * ReleaseBlocks[10];
  uint8_t BlocksToRelease = 0;

  // Find end of the current detectable block
  while(blockGroup->group[3] > offset){
    if(tB->train != this || !tB->detectionBlocked)
      break;

    if(blockGroup->group[3] > ++offset)
      tB = blockGroup->B[offset];
  }

  // Find blocks to be released
  while(blockGroup->group[3] > offset){
    if(tB->train != this)
      break;

    tB->setVirtualDetection(0);
    if(tB->train && !tB->detectionBlocked)
      tB->train->releaseBlock(tB);

    ReleaseBlocks[BlocksToRelease++] = tB;

    loggerf(INFO, "f block %2i:%2i", tB->module, tB->id);

    if(blockGroup->group[3] > ++offset)
      tB = blockGroup->B[offset];
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

  return offset;
}

void Train::VirtualBlocks(){
  
  auto * Det = Detectables[Detectables.size() - 1];
  Block * End = Det->B[0]; // From the end of the train

  loggerf(INFO, "VirtualBlocks (T: %i, %02i:%02i/%02i:%02i, lenght: %icm/%icm)", id, B->module, B->id, End->module, End->id, virtualLengthBefore/10, virtualLengthAfter/10);

  if(!directionKnown)
    return;

  uint8_t beforeOffset = setVirtualBlocks(B->Alg.N, virtualLengthBefore);
  uint8_t afterOffset  = setVirtualBlocks(B->Alg.P, virtualLengthAfter);
  loggerf(INFO, "done setVirtualBlocks, offsets: %i/%i", beforeOffset, afterOffset);

  loggerf(INFO, " %02i %02i %02i %02i %02i %02i", End->id, End->Alg.P->B[0] ? End->Alg.P->B[0]->id : 0, End->Alg.P->B[1] ? End->Alg.P->B[1]->id : 0, End->Alg.P->B[2] ? End->Alg.P->B[2]->id : 0, End->Alg.P->B[3] ? End->Alg.P->B[3]->id : 0, End->Alg.P->B[4] ? End->Alg.P->B[4]->id : 0);

  releaseVirtualBlocks(End->Alg.P, afterOffset);
}

void Train::initMove(Block * newBlock){
  // First time a Train moved when not stopped
  loggerf(INFO, "initMove, RT %i to block %02i:%02i", id, newBlock->module, newBlock->id);

  if(initialized){
    move(newBlock);
    loggerf(ERROR, "allready done initMove, redirecting to move");
    return;
  }

  newBlock->train = this;

  // The direction of the train is known and therefore it is now fully initialized.
  directionKnown = true;
  initialized = true;

  SpeedState = TRAIN_SPEED_DRIVING;

  uint16_t trainLength = (length/10) + newBlock->length;

  B = FindFront(this, newBlock, trainLength);

  initializeTrainDetectables(this, B, trainLength);
  
  // Register and reserve paths
  if(B->path)
    B->path->reserve(this, B);

  if(virtualLength)
    initVirtualBlocks();
    
  move(newBlock);
}

void Train::move(Block * newBlock){
  loggerf(INFO, "move RT %i from Block %2i:%2i %c", id, newBlock->module, newBlock->id, newBlock->detectionBlocked ? 'B' : ' ');

  // If the block was just released from the detection unit
  if(!newBlock->detectionBlocked){

    // If the train has no virtual detection, then immediatly release the block
    if(!virtualLength){
      releaseBlock(newBlock);
      AlQueue.put(this);
    }
    else{
      VirtualBlocks();
    }

    for(auto TD: Detectables){
      // Find the detectable that has the block
      //  then pop it out of the list and reorder

      if(TD->B.size() <= 1)
        continue;

      if(newBlock != TD->B[TD->B.size() - 1])
        continue;

      TD->B.pop_back();
      TD->BlockedLength -= newBlock->length;
      
      TD->setExpectedTrain();

      break;
    }
  }
  // newBlock->detectionBlocked
  // If the block was just set from the detection unit
  else{
    loggerf(DEBUG, "MoveForward RT %i to block %2i:%2i", id, newBlock->module, newBlock->id);
    setBlock(newBlock);
    dereserveBlock(newBlock);

    if(virtualLength)
      VirtualBlocks();

    AlQueue.put(this);
  }
}

Block * FindFront(Train * T, Block * start, int16_t length){
  Block * listBlock = start;
  Block * B = start;

  // Find front of train
  while(length > 0 && listBlock){
    length -= listBlock->length;

    if(listBlock->train == T){
      B = listBlock; // Update front
    }

    listBlock = listBlock->Next_Block(T->dir == 1 ? PREV : NEXT, 1);
  }

  loggerf(INFO, "Front of train in block %02i:%02i", B->module, B->id);

  return B;
}

void Train::moveForward(Block * newBlock){
  B = newBlock;

  // if(virtualLength)
  //   VirtualBlocks();

  if(routeStatus){
    if(newBlock == route->destinationBlocks[0] || newBlock == route->destinationBlocks[1]){
      if(route->routeType == PATHFINDING_ROUTE_BLOCK)
        routeStatus = TRAIN_ROUTE_AT_DESTINATION;
      else
        routeStatus++;
    }
  }
}

void Train::reverse(){
  // Reverse the train requires the occupied blocks to reverse as well.
  //  a block/path can only be reversed if all trains occupying the block are stationary.
  loggerf(DEBUG, "Train %i Reverse", id);

  if(!assigned || !directionKnown)
    return;

  if(speed != 0){
    setSpeed(0);
    SpeedState = TRAIN_SPEED_STOPPING_REVERSE;
    return;
  }

  if(SpeedState == TRAIN_SPEED_STOPPING){
    loggerf(WARNING, "Train stopping and then reversing");
    SpeedState = TRAIN_SPEED_STOPPING_REVERSE;
    return;
  }

  if(reverseDirection){
    reverseBlocks();
    return;
  }

  bool reversed = false;
  for(auto p: paths){
    if(reversed)
      p->reverse(this);
    else{
      p->reverse();
      reversed = true;
    }
  }

  std::reverse(blocks.begin(), blocks.end());
  std::reverse(paths.begin(),  paths.end());
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
  loggerf(DEBUG, "train reverseBlocks");
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

  std::swap(virtualLengthBefore, virtualLengthAfter);

  setSpeed(0);

  loggerf(WARNING, "Train reversed, front %2i:%2i, dir %i", B->module, B->id, dir);
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

    bool detectable = true;
    auto Tcomp = T->composition;
    uint16_t detectableBlockedLength = 0, detectableLength = 0;
    uint16_t * vLength = &virtualLengthBefore;

    for(uint8_t i = 0; i < T->nr_stock; i++){
      // FIXME, trainset with detectable cars, does not work properly

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

      if(Tcomp[i].type == TRAIN_ENGINE_TYPE){
        vLength = &virtualLengthAfter;
        detectableLength += ((Engine *)Tcomp[i].p)->length;
        *vLength         += ((Engine *)Tcomp[i].p)->length;
      }
      else{
        detectableLength += ((Car *)Tcomp[i].p)->length;
        *vLength         += ((Car *)Tcomp[i].p)->length;
      }
    }

    Detectables.push_back(new TrainDetectable(this, detectableLength/10, detectableBlockedLength/10));

    if(Detectables.size() != T->detectables){
      loggerf(ERROR, "Failed to assign all detectables  (%i out of %i)", Detectables.size(), T->detectables);
    }

    //Lock all engines
    T->setEnginesUsed(true, this);

    virtualLength = T->virtualDetection;
  }

  loggerf(INFO, "vLength: Before %i, After %i", virtualLengthBefore, virtualLengthAfter);

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
        b->path->trainExit(T[i]);
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


void Train::enterPath(Path * P){
  paths.insert(paths.begin(), P);
}

void Train::exitPath(Path * P){
  paths.erase(std::remove_if(paths.begin(),
                              paths.end(),
                             [P](const auto & o) { return (o == P); }),
                             paths.end()
                            );
}

void Train::analyzePaths(){
  loggerf(DEBUG, "train::analyzePaths");
  paths.clear();

  for(auto b: blocks){
    if(!b->path)
      continue;

    bool noneOf = std::none_of(paths.begin(), paths.end(), [b](Path * P){ return b->path == P; });

    loggerf(DEBUG, " P%08x  %c", b->path, noneOf ? 'p' : ' ');

    if(noneOf)
      paths.push_back(b->path);
  }
}

bool Train::ContinueCheck(){
  loggerf(DEBUG, "Train %i: ContinueCheck %02i:%02i", id, B->module, B->id);
  if(routeStatus == TRAIN_ROUTE_AT_DESTINATION && stopped)
    return false;

  const std::lock_guard<std::mutex> lock(Algorithm::processMutex);

  // If a block is available
  if(B->Alg.N->group[3] > 0){
    //if(this->Route && Switch_Check_Path(this->B)){
    //  return true;
    //}

    // If it is blocked by another train dont accelerate
    if(B->Alg.N->B[0]->blocked && B->Alg.N->B[0]->train != this){
      loggerf(TRACE, "Train %i: ContinueCheck - false", id);
      return false;
    }

    // If it is not DANGER/BLOCKED then accelerate
    else if(B->Alg.N->B[0]->state > DANGER){
      loggerf(TRACE, "Train %i: ContinueCheck - true", id);
      return true;
    }
  }

  loggerf(WARNING, "NEW TRAIN::CONTINUECHECK");
  struct SwitchSolver::find f = {0, 0};
  RailLink * next = B->NextLink(NEXT);
  Block * nextBlock  = B->Next_Block(NEXT, 1);
  Block * next2Block = B->Next_Block(NEXT, 2);

  if(next->type != RAIL_LINK_E && next->type != RAIL_LINK_R){
    // Next is a (ms)switch
    //  solve it
    struct SwitchSolver::SwSolve SolveControl = {
      .train = this,
      .route = route,
      .prevBlock = B,
      .prevPtr   = B,
      .link  = next,
      .flags = NEXT | FL_SWITCH_CARE | FL_CONTINUEDANGER
    };

    f = SwitchSolver::findPath(SolveControl);
    loggerf(INFO, "First %i", f.possible);
    if(!f.possible || (nextBlock && nextBlock->state <= DANGER))
      return false;
  }

  if(!B->Alg.N->group[3])
    return f.possible;
  else if(B->Alg.N->B[0]->type != NOSTOP)
    return true;

  next = B->Alg.N->B[0]->NextLink(NEXT);

  if(next->type != RAIL_LINK_E && next->type != RAIL_LINK_R){
    // Next is a (ms)switch
    //  solve it
    struct SwitchSolver::SwSolve SolveControl = {
      .train = this,
      .route = route,
      .prevBlock = B->Alg.N->B[0],
      .prevPtr   = B->Alg.N->B[0],
      .link  = next,
      .flags = NEXT | FL_SWITCH_CARE | FL_CONTINUEDANGER
    };

    f = SwitchSolver::findPath(SolveControl);
    loggerf(INFO, "Second %i", f.possible);
    return f.possible && (next2Block && next2Block->state > DANGER);
  }
}

void Train::Continue(){
  RailLink * next = B->NextLink(NEXT);

  uint8_t nextLen = B->Alg.N->group[3];
  Block * NextBlock = B->Alg.N->B[0];

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
    SwitchSolver::solve(this, B, B->Alg.N->B[0], *next, NEXT | FL_SWITCH_CARE | FL_CONTINUEDANGER);
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

