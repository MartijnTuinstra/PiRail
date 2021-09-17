#ifndef _INCLUDE_ROLLINGSTOCK_TRAINDETECTION_H
#define _INCLUDE_ROLLINGSTOCK_TRAINDETECTION_H

#include <stdint.h>
#include <vector>

#include "switchboard/declares.h"
#include "rollingstock/declares.h"

#define TRAIN_FIFO_SIZE 64 // Blocks
#define TRAIN_DETECTABLE_SIZE_WARNING 32

void initializeTrainDetectables(Train *, Block *, int16_t);

class TrainDetectable {
  public:
  Train * T;

  std::vector<Block *> B;   // List of blocked blocks (detection only)
                            //  Front/0 -> begin
  
  bool expectedSet = 0;

  uint16_t Length;           // The total length of the rolling stock
  uint16_t DetectableLength; // The detectable length of the rolling stock
  uint16_t BlockedLength;    // The total length currently blocked by detectable

  TrainDetectable(Train *, uint16_t, uint16_t);
  ~TrainDetectable();

  void initialize(Block **, int16_t *);

  void stepForward(Block *);

  void reverse();

  void setExpectedTrain();
  void resetExpectedTrain();
};

#endif