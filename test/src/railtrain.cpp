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

#include "algorithm/queue.h"
#include "algorithm/core.h"
#include "algorithm/component.h"
#include "algorithm/blockconnector.h"

void init_test(char (* filenames)[30], int nr_files);
void train_testSim_tick(struct train_sim * t, int32_t * i);
void train_test_tick(struct train_sim * t, int32_t * i);

class TestsFixture {
public:
  TestsFixture();
  void loadSwitchboard(char (* filenames)[30], int nr_files);
  void loadStock();
  ~TestsFixture();
};

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

  for(uint8_t i = 1; i <= 4; i++){
    U[i]->link_all();
  }
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

  SECTION("I - Blocked Switch"){
    // Initialize Detection and trains
    U[1]->B[11]->setDetection(1);
    Algorithm::process(U[1]->B[11], _FORCE);
    U[3]->B[1]->setDetection(1);
    Algorithm::process(U[3]->B[1], _FORCE);

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
      Algorithm::process(blockList[i], _FORCE);

      blockList[i-1]->setDetection(0);
      Algorithm::process(blockList[i-1], _FORCE);
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
    Algorithm::process(U[4]->B[2], _FORCE);
    U[1]->B[11]->setDetection(0);
    Algorithm::process(U[1]->B[11], _FORCE);

    REQUIRE( U[1]->B[11]->state != BLOCKED);
    REQUIRE(!U[1]->B[11]->reserved);
    REQUIRE(!U[1]->B[11]->switchReserved);
    REQUIRE(!U[1]->B[11]->blocked);

    // Path is clear so it should pass
    //  therefore speeding up is allowed.
    CHECK(T->ContinueCheck());
    T->setSpeed(10);

    CHECK(T->speed > 0);
    CHECK(U[1]->B[5]->switchReserved);
    CHECK(U[1]->B[11]->switchReserved);
    CHECK(U[1]->B[12]->path->reserved);

    CHECK(U[1]->B[11]->dir == 0b101);
    CHECK(U[1]->B[12]->path->reserved);
  }

  SECTION("II - Blocked Station without route"){
    // Initialize Detection and trains
    U[1]->B[2]->setDetection(1);
    Algorithm::process(U[1]->B[2], _FORCE);
    U[3]->B[1]->setDetection(1);
    Algorithm::process(U[3]->B[1], _FORCE);

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
      Algorithm::process(blockList[i], _FORCE);

      blockList[i-1]->setDetection(0);
      Algorithm::process(blockList[i-1], _FORCE);
    }

    CHECK(U[1]->Sw[6]->state);
    CHECK(U[1]->B[5]->switchReserved);
    T->setSpeed(0);

    REQUIRE(!U[1]->B[5]->switchReserved);

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
    CHECK(U[1]->B[ 5]->switchReserved);
    CHECK(U[1]->B[11]->switchReserved);
    CHECK(U[1]->B[11]->dir & 0b100);    // reversed
    CHECK(U[1]->B[10]->reserved);
    CHECK(U[1]->B[10]->path->Entrance == U[1]->B[10]);

    // // Move the other train out of the way
    // U[4]->B[2]->setDetection(1);
    // Algorithm::process(U[4]->B[2], _FORCE);
    // U[1]->B[11]->setDetection(0);
    // Algorithm::process(U[1]->B[11], _FORCE);

    // REQUIRE( U[1]->B[11]->state != BLOCKED);
    // REQUIRE(!U[1]->B[11]->reserved);
    // REQUIRE(!U[1]->B[11]->switchReserved);
    // REQUIRE(!U[1]->B[11]->blocked);

    // // Path is clear so it should pass
    // //  therefore speeding up is allowed.
    // CHECK(T->ContinueCheck());
    // T->setSpeed(10);

    // CHECK(T->speed > 0);
    // CHECK(U[1]->B[5]->switchReserved);
    // CHECK(U[1]->B[11]->switchReserved);
    // CHECK(U[1]->B[12]->path->reserved);

    // CHECK(U[1]->B[11]->dir == 0b101);
    // CHECK(U[1]->B[12]->path->reserved);
  }
}
