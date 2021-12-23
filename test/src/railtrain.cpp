#include <time.h>
#include "catch.hpp"

#include "utils/mem.h"
#include "utils/logger.h"
#include "system.h"

#include "config/ModuleConfig.h"
#include "switchboard/manager.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/station.h"
#include "switchboard/unit.h"
#include "switchboard/blockconnector.h"

#include "rollingstock/train.h"

#include "train.h"
#include "path.h"

#include "algorithm/queue.h"
#include "algorithm/core.h"
#include "algorithm/component.h"
#include "algorithm/blockconnector.h"

#include "testhelpers.h"

TEST_CASE_METHOD(TestsFixture, "Train Continue Check", "[RT][RT-CC]"){
  char filenames[4][30] = {"./testconfigs/Alg-R-1.bin",
                           "./testconfigs/Alg-R-2.bin",
                           "./testconfigs/Alg-R-3.bin",
                           "./testconfigs/Alg-R-4.bin"};
  loadSwitchboard(filenames, 4);
  loadStock();

  Unit * U[5] = {0, switchboard::Units(1),switchboard::Units(2),switchboard::Units(3),switchboard::Units(4)};

  for(uint8_t i = 1; i <= 4; i++){
    REQUIRE(U[i]);
    U[i]->on_layout = true;
  }

  auto connectors = Algorithm::find_connectors();
  auto setup = Algorithm::BlockConnectorSetup("./testconfigs/Alg-R-Setup.bin");
  
  int ret = setup.load(&connectors);
  REQUIRE(ret > 0);
  REQUIRE(connectors.size() == 0);

  switchboard::SwManager->LinkAndMap();

  for(uint8_t i = 1; i <= 4; i++){
    for(uint8_t j = 0; j < U[i]->block_len; j++){
      if(U[i]->B[j]){
        U[i]->B[j]->setDetection(0);
        AlQueue.put(U[i]->B[j]);
      }
    }
  }

  pathlist_find();

  for(uint8_t i = 1; i < 5; i++){
    for(uint8_t j = 0; j < U[i]->block_len; j++){
      if(!U[i]->B[j])
        continue;

      AlQueue.put(U[i]->B[j]);
    }
  }

  Algorithm::BlockTick();

  SECTION("I - Next Block Danger"){
    // Train follows another trains closely
    //  Train stops for train ahead, ContinueCheck fails while the block is still at Danger 
    // When the front trains moves on the block get release to Caution so ContinueCheck succeeds

    U[3]->B[1]->setDetection(1);
    Algorithm::processBlock(&U[3]->B[1]->Alg, _FORCE);
    U[2]->B[1]->setDetection(1);
    Algorithm::processBlock(&U[2]->B[1]->Alg, _FORCE);

    Train * T = U[2]->B[1]->train;
    T->link(0, TRAIN_ENGINE_TYPE);
    T->setSpeed(10);
    
    U[3]->B[1]->train->link(1, TRAIN_ENGINE_TYPE);
    U[3]->B[1]->train->setSpeed(10);

    U[3]->B[0]->setDetection(1);
    Algorithm::processBlock(&U[3]->B[0]->Alg, _FORCE);
    U[3]->B[1]->setDetection(0);
    Algorithm::processBlock(&U[3]->B[1]->Alg, _FORCE);
    Algorithm::BlockTick();

    U[2]->B[0]->setDetection(1);
    Algorithm::processBlock(&U[2]->B[0]->Alg, _FORCE);
    U[2]->B[1]->setDetection(0);
    Algorithm::processBlock(&U[2]->B[1]->Alg, _FORCE);
    Algorithm::BlockTick();

    REQUIRE(U[3]->B[0]->train->directionKnown);
    REQUIRE(U[2]->B[0]->train->directionKnown);
    REQUIRE(U[3]->B[3]->state == CAUTION);
    REQUIRE(U[3]->B[2]->state == DANGER);

    U[3]->B[3]->setDetection(1);
    Algorithm::processBlock(&U[3]->B[3]->Alg, _FORCE);
    U[2]->B[0]->setDetection(0);
    Algorithm::processBlock(&U[2]->B[0]->Alg, _FORCE);
    Algorithm::BlockTick();

    REQUIRE(U[3]->B[3]->state == BLOCKED);
    REQUIRE(U[3]->B[2]->state == DANGER);

    T->setSpeed(0);

    REQUIRE(U[3]->B[3]->state == BLOCKED);
    REQUIRE(U[3]->B[2]->state == DANGER);

    CHECK(!T->ContinueCheck());

    U[4]->B[1]->setDetection(1);
    Algorithm::processBlock(&U[4]->B[1]->Alg, _FORCE);
    U[3]->B[0]->setDetection(0);
    Algorithm::processBlock(&U[3]->B[0]->Alg, _FORCE);
    Algorithm::BlockTick();

    REQUIRE(U[3]->B[3]->state == BLOCKED);
    REQUIRE(U[3]->B[2]->state == CAUTION);
    REQUIRE(U[3]->B[1]->state == DANGER);

    logger.setlevel_stdout(TRACE);
    CHECK(T->ContinueCheck());
    CHECK(T->B->Alg.N->B[0]->state != DANGER);
  }

  SECTION("II - Block after switch Danger"){
    // Train drives to another aproaching trains
    //  There is a switch inbetween, where one should wait to let the other train by

    U[3]->B[3]->path->reverse();

    REQUIRE(U[2]->B[1]->path->Exit == U[2]->B[1]);

    U[1]->B[2]->setDetection(1);
    Algorithm::processBlock(&U[1]->B[2]->Alg, _FORCE);
    U[3]->B[3]->setDetection(1);
    Algorithm::processBlock(&U[3]->B[3]->Alg, _FORCE);

    Train * T = U[1]->B[2]->train;
    T->link(0, TRAIN_ENGINE_TYPE);
    T->setSpeed(10);
    
    U[3]->B[3]->train->link(1, TRAIN_ENGINE_TYPE);
    U[3]->B[3]->train->setSpeed(10);

    U[2]->B[0]->setDetection(1);
    Algorithm::processBlock(&U[2]->B[0]->Alg, _FORCE);
    U[3]->B[3]->setDetection(0);
    Algorithm::processBlock(&U[3]->B[3]->Alg, _FORCE);
    BlockTickNtimes(5);

    REQUIRE(U[2]->B[1]->reserved);
    REQUIRE(U[2]->B[1]->isReservedBy(U[2]->B[0]->train));
    REQUIRE(U[2]->B[1]->dir == 1);

    U[1]->B[1]->setDetection(1);
    Algorithm::processBlock(&U[1]->B[1]->Alg, _FORCE);
    U[1]->B[2]->setDetection(0);
    Algorithm::processBlock(&U[1]->B[2]->Alg, _FORCE);
    BlockTickNtimes(5);

    REQUIRE(U[1]->B[1]->train->directionKnown);
    REQUIRE(U[2]->B[0]->train->directionKnown);
    REQUIRE(U[2]->B[0]->train != U[1]->B[1]->train);
    REQUIRE(U[2]->B[1]->state == RESERVED);
    REQUIRE(U[2]->B[1]->reverse_state == DANGER);
    REQUIRE(U[1]->B[0]->state == DANGER);
    REQUIRE(U[1]->B[0]->reverse_state == PROCEED);

    REQUIRE(U[1]->B[0]->switchWrongState);

    T->setSpeed(0);
    train_speed_event_tick(T->speed_event_data);

    // Train is waiting at s side of the switch
    //  no path is available since it is blocked by the other train
    CHECK(!T->ContinueCheck());

    U[2]->B[1]->setDetection(1);
    Algorithm::processBlock(&U[2]->B[1]->Alg, _FORCE);
    U[2]->B[0]->setDetection(0);
    Algorithm::processBlock(&U[2]->B[0]->Alg, _FORCE);
    Algorithm::BlockTick();

    REQUIRE(U[1]->B[0]->reserved);
    REQUIRE(U[1]->B[0]->isReservedBy(U[2]->B[1]->train));
    REQUIRE(U[1]->B[0]->dir == 1);

    U[1]->B[0]->setDetection(1);
    Algorithm::processBlock(&U[1]->B[0]->Alg, _FORCE);
    U[2]->B[1]->setDetection(0);
    Algorithm::processBlock(&U[2]->B[1]->Alg, _FORCE);
    Algorithm::BlockTick();

    REQUIRE(!U[2]->B[1]->reserved);
    REQUIRE(!U[2]->B[1]->path->reserved);
    auto P = U[2]->B[1]->path;
    REQUIRE(!U[2]->B[1]->isReservedBy(T));
    {
    auto tT = U[1]->B[6]->train;
    REQUIRE(std::none_of(P->reservedTrains.begin(), P->reservedTrains.end(), [tT](auto t){return tT == t;}));
    }

    U[1]->B[6]->setDetection(1);
    Algorithm::processBlock(&U[1]->B[6]->Alg, _FORCE);
    U[1]->B[0]->setDetection(0);
    Algorithm::processBlock(&U[1]->B[0]->Alg, _FORCE);
    BlockTickNtimes(15);

    REQUIRE(!U[1]->B[0]->reserved);
    REQUIRE(!U[1]->B[0]->isReservedBy(U[2]->B[1]->train));
    REQUIRE( U[1]->B[0]->dir == 1);
    REQUIRE( U[2]->B[1]->dir == 1);

    REQUIRE(U[1]->B[0]->state == DANGER);
    REQUIRE(U[1]->B[0]->reverse_state == PROCEED);

    CHECK(U[1]->Sw[0]->state == 1);

    // Train is waiting at s side of the switch
    //  there is a path available since the other train has passed

    CHECK(T->SpeedState == TRAIN_SPEED_IDLE);

    CHECK(T->ContinueCheck());

    T->setControl(TRAIN_SEMI_AUTO);

    Train_ContinueCheck((void *)0x0);

    BlockTickNtimes(25);
    train_speed_event_tick(T->speed_event_data);

    CHECK(T->SpeedState != TRAIN_SPEED_IDLE);
    CHECK(U[1]->Sw[0]->state == 0);
    CHECK(U[1]->Sw[0]->Detection->reserved);
    CHECK(U[1]->Sw[0]->Detection->isReservedBy(T));
    CHECK(U[2]->B[0]->reserved);
    CHECK(U[2]->B[0]->isReservedBy(T));
  }

  SECTION("III - Blocked Switch"){
    // Blocked crossover
    // train has a route but another train blocks the path
    // the train with the route must wait until the path is clear

    // Initialize Detection and trains
    U[1]->B[11]->setDetection(1);
    Algorithm::processBlock(&U[1]->B[11]->Alg, _FORCE);
    U[3]->B[1]->setDetection(1);
    Algorithm::processBlock(&U[3]->B[1]->Alg, _FORCE);

    Train * T = U[3]->B[1]->train;
    T->link(0, TRAIN_ENGINE_TYPE);
    T->setSpeed(10);
    T->setRoute(U[1]->B[12]);
    
    U[1]->B[11]->train->link(1, TRAIN_ENGINE_TYPE);
    U[1]->B[11]->train->setSpeed(10);


    // Move train to switch
    Block * blockList[10] = {U[3]->B[1], U[3]->B[0], U[4]->B[1], U[4]->B[0]};

    for(uint8_t i = 1; i <= 3; i++){
      blockList[i]->setDetection(1);
      Algorithm::processBlock(&blockList[i]->Alg, _FORCE);

      blockList[i-1]->setDetection(0);
      Algorithm::processBlock(&blockList[i-1]->Alg, _FORCE);
    }

    T->setSpeed(0);
    CHECK(U[1]->B[5]->switchWrongState);
    
    // ContinueCheck must fail since the path is blocked
    CHECK(!T->ContinueCheck());

    // Failed ContinueCheck must prevent speeding up.
    T->setSpeed(10);
    CHECK(T->speed == 0);

    // Move the other train out of the way
    U[4]->B[2]->setDetection(1);
    Algorithm::processBlock(&U[4]->B[2]->Alg, _FORCE);
    U[1]->B[11]->setDetection(0);
    Algorithm::processBlock(&U[1]->B[11]->Alg, _FORCE);

    REQUIRE( U[1]->B[11]->state != BLOCKED);
    REQUIRE(!U[1]->B[11]->reserved);
//    REQUIRE(!U[1]->B[11]->reserved);
    REQUIRE(!U[1]->B[11]->blocked);

    // Path is clear so it should pass
    //  therefore speeding up is allowed.
    CHECK(T->ContinueCheck());
    T->setSpeed(10);

    CHECK(T->speed > 0);
    CHECK(U[1]->B[5]->reserved);
    CHECK(U[1]->B[11]->reserved);
    CHECK(U[1]->B[12]->path->reserved);

    CHECK(U[1]->B[11]->dir == 1);
    CHECK(U[1]->B[12]->path->reserved);
  }

  SECTION("IV - Blocked Station without route"){
    logger.setlevel_stdout(INFO);
    // Initialize Detection and trains
    U[1]->B[2]->setDetection(1);
    Algorithm::processBlock(&U[1]->B[2]->Alg, _FORCE);
    U[3]->B[1]->setDetection(1);
    Algorithm::processBlock(&U[3]->B[1]->Alg, _FORCE);

    Train * T = U[3]->B[1]->train;
    T->link(0, TRAIN_ENGINE_TYPE);
    T->setSpeed(10);
    
    U[1]->B[2]->train->link(1, TRAIN_ENGINE_TYPE);
    U[1]->B[2]->train->setSpeed(10);
    U[1]->B[2]->train->setSpeed(0);
    // Train must be stopped and occupy the station
    REQUIRE(U[1]->B[2]->train->stopped);
    REQUIRE(U[1]->B[2]->station->stoppedTrain);

    // Move train to switch
    Block * blockList[10] = {U[3]->B[1], U[3]->B[0], U[4]->B[1], U[4]->B[0]};

    for(uint8_t i = 1; i <= 3; i++){
      blockList[i]->setDetection(1);
      Algorithm::processBlock(&blockList[i]->Alg, _FORCE);

      blockList[i-1]->setDetection(0);
      Algorithm::processBlock(&blockList[i-1]->Alg, _FORCE);

      BlockTickNtimes(15);
    }

    CHECK(U[1]->Sw[6]->state);
    CHECK(U[1]->B[5]->reserved);

    T->setSpeed(0);
    train_speed_event_tick(T->speed_event_data);

    REQUIRE(!U[1]->B[5]->reserved);

    U[1]->Sw[6]->setState(0);
    Algorithm::BlockTick();
    
    logger.setlevel_stdout(TRACE);
    // ContinueCheck should not fail because there is an alternative path
    CHECK(T->ContinueCheck());
    logger.setlevel_stdout(DEBUG);

    // Therefore the train is allowed to speed up.
    T->setSpeed(10);
    CHECK(T->speed > 0);

    // Also the switch is thrown to the alternative path
    CHECK(U[1]->Sw[6]->state);
    CHECK(U[1]->Sw[5]->state);
    CHECK(U[1]->B[ 5]->reserved);
    CHECK(U[1]->B[11]->reserved);
    CHECK(U[1]->B[11]->dir == 1);    // reversed
    CHECK(U[1]->B[10]->reserved);
    CHECK(U[1]->B[10]->path->Entrance == U[1]->B[10]);

    // // Move the other train out of the way
    // U[4]->B[2]->setDetection(1);
    // Algorithm::processBlock(&U[4]->B[2]->Alg, _FORCE);
    // U[1]->B[11]->setDetection(0);
    // Algorithm::processBlock(&U[1]->B[11]->Alg, _FORCE);

    // REQUIRE( U[1]->B[11]->state != BLOCKED);
    // REQUIRE(!U[1]->B[11]->reserved);
    // REQUIRE(!U[1]->B[11]->reserved);
    // REQUIRE(!U[1]->B[11]->blocked);

    // // Path is clear so it should pass
    // //  therefore speeding up is allowed.
    // CHECK(T->ContinueCheck());
    // T->setSpeed(10);

    // CHECK(T->speed > 0);
    // CHECK(U[1]->B[5]->reserved);
    // CHECK(U[1]->B[11]->reserved);
    // CHECK(U[1]->B[12]->path->reserved);

    // CHECK(U[1]->B[11]->dir == 0b101);
    // CHECK(U[1]->B[12]->path->reserved);
  }
}
