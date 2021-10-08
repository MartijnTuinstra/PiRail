#include "utils/logger.h"

#include "switchboard/rail.h"
#include "rollingstock/traindetection.h"
#include "rollingstock/train.h"

void initializeTrainDetectables(Train * T, Block * start, int16_t length){
  uint8_t DetectableCounter = 0;
  Block * tB = start; 

  while(length > 0 || tB->train == T){
    auto TD = T->Detectables[DetectableCounter++];

    TD->initialize(&tB, &length);

    if(!tB)
      break;

    while(length > 0 && !tB->detectionBlocked){
      tB = tB->Next_Block(T->dir ? NEXT : PREV, 1);

      if(!tB)
        break;
    }

    if(!tB)
      break;
  }
}

TrainDetectable::TrainDetectable(Train * _train, uint16_t _length , uint16_t _blockedLength){
  T = _train;

  Length = _length;
  DetectableLength = _blockedLength;

  loggerf(WARNING, "constructing trainDetectable %i / %i", _length, _blockedLength);
}
TrainDetectable::~TrainDetectable(){

}

void TrainDetectable::initialize(Block ** Front, int16_t * remainingLength){
  Block * tB = *Front;
  // uint8_t i = 0;
  int16_t remainingVirtualLength = Length;

  
  while( (*remainingLength > 0 && remainingVirtualLength > 0) || tB->train == T){
    *remainingLength       -= tB->length;
    remainingVirtualLength -= tB->length;

    if(tB->train == T && tB->detectionBlocked){
      tB->expectedTrain = T;
      BlockedLength += tB->length;;
      loggerf(WARNING, "setting expectedTrain %2i:%2i", tB->module, tB->id);
    }
    else{
      loggerf(WARNING, "                      %2i:%2i  %c  %c", tB->module, tB->id, tB->train == T ? 'T' : ' ', tB->detectionBlocked ? 'B' : ' ');
    }

    loggerf(WARNING, "initializing trainDetectable %2i:%2i / %i / %i", tB->module, tB->id, *remainingLength, remainingVirtualLength);
    B.push_back(tB);
    // B.insert(B.begin(), tB);

    tB = tB->Next_Block(T->dir ? NEXT : PREV, 1);

    if(!tB)
      break;
  }

  setExpectedTrain();

  *Front = tB;
}

void TrainDetectable::stepForward(Block * tB){
  // Find the detectable that has the front block
  //  then add it to the list

  loggerf(WARNING, "stepForward trainDetectable %2i:%2i", tB->module, tB->id);

  Block * _B = B[0];

  if(_B->Alg.N->group[3] > 0 && _B->Alg.N->B[0] == tB){
    // FIXME
    if(T->Detectables[0] == this)
      T->moveForward(tB);

    BlockedLength += tB->length;
    B.insert(B.begin(), tB);
    tB->expectedDetectable = 0;

    if(B.size() > TRAIN_DETECTABLE_SIZE_WARNING){
      loggerf(ERROR, "Train has more than %i blocks", TRAIN_FIFO_SIZE);
    }

    expectedSet = false;
    setExpectedTrain();

    T->move(tB);
  }
  else
    loggerf(ERROR, "Something went wrong here!!!!");
}

void TrainDetectable::reverse(){
  loggerf(WARNING, "Detectable reverse");
  // Reverse each block in the DetectedBlocks
  for(auto block: B)
    loggerf(WARNING, " -- %2i:%2i", block->module, block->id);
  std::reverse(B.begin(), B.end());
  for(auto block: B)
    loggerf(WARNING, " ++ %2i:%2i", block->module, block->id);
}

void TrainDetectable::setExpectedTrain(){
  if(expectedSet)
    return;

  uint8_t size = B.size();
  if(size == 0)
    return;

  if(size > 1 && BlockedLength - B[size - 1]->length > DetectableLength)
    B[size - 1]->expectedTrain = 0; // Clear expectedTrain

  // Block * tmpBlock = B[0]->Next_Block(T->dir ? PREV : NEXT, 1);
  Block * tmpBlock = B[0]->getBlock(NEXT, 0);
  if(!tmpBlock)
    tmpBlock = B[0]->Next_Block(NEXT | FL_SWITCH_CARE, 1);

  loggerf(DEBUG, "Detectable setExpectedTrain Clear:%2i:%2i / Set:%2i:%2i", size > 1 ? B[size - 1]->module : 0, size > 1 ? B[size - 1]->id : 0,
                                                                  tmpBlock ? tmpBlock->module : 0, tmpBlock ? tmpBlock->id : 0);

  if(tmpBlock){ // Set new expectedTrain
    tmpBlock->expectedTrain = T;
    tmpBlock->expectedDetectable = this;
  }

  expectedSet = true;
}

void TrainDetectable::resetExpectedTrain(){
  if(!expectedSet)
    return;

  uint8_t size = B.size();
  if(size == 0)
    return;

  if(size > 1)
    B[B.size() - 1]->expectedTrain = T; // Clear expectedTrain
  
  // Block * tmpBlock = B[0]->Next_Block(T->dir ? PREV : NEXT, 1);  
  Block * tmpBlock = B[0]->Next_Block(NEXT, 1);  
  loggerf(WARNING, "Detectable resetExpectedTrain %2i:%2i / %2i:%2i", size > 1 ? B[size - 1]->module : 0, size > 1 ? B[size - 1]->id : 0,
                                                                      tmpBlock ? tmpBlock->module : 0, tmpBlock ? tmpBlock->id : 0);
  
  if(tmpBlock && tmpBlock->expectedTrain == T){ // Set new expectedTrain
    tmpBlock->expectedTrain = 0;
    tmpBlock->expectedDetectable = 0;
  }

  expectedSet = false;
}