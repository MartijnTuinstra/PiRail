#include <time.h>
#include <algorithm>
#include "catch.hpp"

#include "utils/mem.h"
#include "utils/logger.h"
#include "system.h"

#include "config/ModuleConfig.h"
#include "switchboard/manager.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/switchsolver.h"
#include "switchboard/msswitch.h"
#include "switchboard/station.h"
#include "switchboard/unit.h"
#include "switchboard/blockconnector.h"

#include "rollingstock/train.h"

#include "train.h"
#include "sim.h"

#include "algorithm/queue.h"
#include "algorithm/core.h"
#include "algorithm/component.h"
#include "algorithm/blockconnector.h"

#include "pathfinding.h"
#include "path.h"
#include "testhelpers.h"

TEST_CASE_METHOD(TestsFixture, "Train Following Simple", "[TrainFollowing][TF-1]"){
  char filenames[1][30] = {"./testconfigs/Alg-2.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;
  
  switchboard::SwManager->LinkAndMap();

  pathlist_find();

  /*
  //  1.0> 1.1> 1.2> 1.3> 1.4> 1.5> 1.6> 1.7>
  */

  for(uint8_t i = 0; i < 8; i++){
    Algorithm::process(U->B[i], _FORCE);
  }

  // logger.setlevel_stdout(DEBUG);

  SECTION("I - No linked train"){
    loggerf(CRITICAL, "SECTION I");
    CHECK(U->B[0]->train == 0);

    U->B[0]->setDetection(1);
    Algorithm::processBlock(&U->B[0]->Alg, _FORCE);

    CHECK(U->B[0]->train != 0);

    Train * T = U->B[0]->train;
    CHECK(U->B[0] == T->B);

    // Step Forward
    U->B[1]->setDetection(1);
    Algorithm::processBlock(&U->B[1]->Alg, _FORCE);

    CHECK(U->B[1]->train == T);
    CHECK(T->blocks.size() == 2);

    // Release last block
    U->B[0]->setDetection(0);
    Algorithm::processBlock(&U->B[0]->Alg, _FORCE);

    CHECK(U->B[0]->train == 0);
    CHECK(T->blocks.size() == 1);

    // Step backwards
    U->B[0]->setDetection(1);
    Algorithm::processBlock(&U->B[0]->Alg, _FORCE);

    CHECK(U->B[0]->train == T);
    CHECK(T->blocks.size() == 2);

    // Release second block
    U->B[1]->setDetection(0);
    Algorithm::processBlock(&U->B[1]->Alg, _FORCE);

    CHECK(U->B[1]->train == 0);
    CHECK(T->blocks.size() == 1);
  }

  SECTION("II - Simple Engine Forward"){
    loggerf(CRITICAL, "SECTION II");
    // Link an Engine after one block is detected
    //  No speed is set, so no direction is known

    CHECK(U->B[0]->train == 0);

    U->B[0]->setDetection(1);
    Algorithm::process(U->B[0], _FORCE);

    REQUIRE(U->B[0]->train != 0);
    Train * T = U->B[0]->train;
    CHECK(U->B[0] == T->B);

    // Step Forward
    U->B[1]->setDetection(1);
    Algorithm::process(U->B[1], _FORCE);

    CHECK(U->B[1]->train == T);

    // Link engine
    T->link(0, TRAIN_ENGINE_TYPE);

    for(auto b: T->blocks){
      CHECK((b == U->B[0] || b == U->B[1]));
    }

    U->B[0]->setDetection(0);
    Algorithm::process(U->B[0], _FORCE);

    CHECK(U->B[0]->train == 0);
    CHECK(T->directionKnown == 0);

    U->B[2]->setDetection(1);
    Algorithm::process(U->B[2], _FORCE);

    CHECK(U->B[2]->train == T);
    CHECK(T->directionKnown == 0);

    // CHECK(T->Detectables[0].BlockedBlocks == 2); // FIXME

    U->B[1]->setDetection(0);
    Algorithm::process(U->B[1], _FORCE);

    CHECK(U->B[1]->train == 0);
    
    // Move Train
    T->setSpeed(10);
    U->B[3]->setDetection(1);
    Algorithm::process(U->B[3], _FORCE);

    CHECK(U->B[3]->train == T);
    CHECK(T->dir == 0);
    CHECK(T->directionKnown == 1);

    CHECK(U->B[2]->expectedTrain == 0); // Expected to get unblocked
    CHECK(U->B[3]->expectedTrain == T); // Still blocked by train
    CHECK(U->B[4]->expectedTrain == T); // Next to be blocked by train

    U->B[2]->setDetection(0);
    Algorithm::process(U->B[2], _FORCE);
    U->B[4]->setDetection(1);
    Algorithm::process(U->B[4], _FORCE);

    CHECK(!U->B[2]->train);
    CHECK(!U->B[2]->blocked);
    CHECK(U->B[3]->expectedTrain == 0); // Expected to get unblocked
    CHECK(U->B[4]->expectedTrain == T); // Still blocked by train
    CHECK(U->B[5]->expectedTrain == T); // Next to be blocked by train


    U->B[3]->setDetection(0);
    Algorithm::process(U->B[3], _FORCE);
    U->B[5]->setDetection(1);
    Algorithm::process(U->B[5], _FORCE);

    CHECK(U->B[4]->expectedTrain == 0); // Expected to get unblocked
    CHECK(U->B[5]->expectedTrain == T); // Still blocked by train
    CHECK(U->B[6]->expectedTrain == T); // Next to be blocked by train

    U->B[4]->setDetection(0);
    Algorithm::process(U->B[4], _FORCE);
    U->B[6]->setDetection(1);
    Algorithm::process(U->B[6], _FORCE);

    CHECK(U->B[5]->expectedTrain == 0); // Expected to get unblocked
    CHECK(U->B[6]->expectedTrain == T); // Still blocked by train
    CHECK(U->B[7]->expectedTrain == T); // Next to be blocked by train

  }

  SECTION("IIb- Simple Engine Forward"){
    loggerf(CRITICAL, "SECTION IIb");
    // Link an Engine after the second block is detected
    //  Speed is set, so a direction is must be known
    CHECK(U->B[0]->train == 0);

    U->B[1]->setDetection(1);
    Algorithm::process(U->B[1], _FORCE);

    REQUIRE(U->B[1]->train != 0);
    Train * T = U->B[1]->train;

    // Link engine
    T->link(0, TRAIN_ENGINE_TYPE);

    // Add extra detected block at wrong side
    U->B[0]->setDetection(1);
    Algorithm::process(U->B[0], _FORCE);

    REQUIRE(U->B[0]->train == T);
    CHECK(U->B[0]->train->dir == 0);
    CHECK(U->B[0]->train->directionKnown == 0);

    // Move Train
    T->setSpeed(10);
    U->B[2]->setDetection(1);
    Algorithm::process(U->B[2], _FORCE);

    CHECK(U->B[2]->train == T);
    CHECK(U->B[2] == T->B);
    
    CHECK(T->directionKnown);
    CHECK(U->B[3]->expectedTrain == T);
  }

  SECTION("III - Full Detectable Train"){
    loggerf(CRITICAL, "SECTION III");
    CHECK(U->B[0]->train == 0);

    U->B[0]->setDetection(1);
    Algorithm::process(U->B[0], _FORCE);

    REQUIRE(U->B[0]->train != 0);
    Train * T = U->B[0]->train;

    T->link(1, TRAIN_TRAIN_TYPE);
    T->setSpeed(10);

    // Step Forward
    U->B[1]->setDetection(1);
    Algorithm::process(U->B[1], _FORCE);

    CHECK(U->B[1]->train == T);
    CHECK(U->B[1] == T->B);
    CHECK(T->blocks.size() == 2);

    // Step Forward
    U->B[2]->setDetection(1);
    Algorithm::process(U->B[2], _FORCE);

    CHECK(U->B[2]->train == T);

    // Step Forward
    U->B[0]->setDetection(0);
    Algorithm::process(U->B[0], _FORCE);
    U->B[3]->setDetection(1);
    Algorithm::process(U->B[3], _FORCE);

    CHECK(U->B[3]->train == T);
    CHECK(U->B[0]->train == 0);

    CHECK(T->blocks.size() == 3);
    for(auto b: T->blocks){
      CHECK((b == U->B[1] || b == U->B[2] || b == U->B[3]));
    }

    // Step Forward
    U->B[1]->setDetection(0);
    Algorithm::process(U->B[1], _FORCE);
    U->B[4]->setDetection(1);
    Algorithm::process(U->B[4], _FORCE);
    
    CHECK(U->B[1]->train == 0);
    CHECK(U->B[4]->train == T);
    CHECK(U->B[4] == T->B);
    CHECK(T->blocks.size() == 3);
    for(auto b: T->blocks){
      CHECK((b == U->B[2] || b == U->B[3] || b == U->B[4]));
    }

    CHECK(U->B[2]->expectedTrain == nullptr);
    CHECK(U->B[3]->expectedTrain == T);
    CHECK(U->B[4]->expectedTrain == T);

    // Step Forward
    U->B[2]->setDetection(0);
    Algorithm::process(U->B[2], _FORCE);
    U->B[5]->setDetection(1);
    Algorithm::process(U->B[5], _FORCE);

    CHECK(U->B[2]->train == 0);
    CHECK(U->B[5]->train == T);
    CHECK(U->B[5] == T->B);

    CHECK(U->B[3]->expectedTrain == nullptr);
    CHECK(U->B[4]->expectedTrain == T);
    CHECK(U->B[5]->expectedTrain == T);
  }

  SECTION("IV - Partial Detectable Train"){
    // logger.setlevel_stdout(DEBUG);
    loggerf(CRITICAL, "SECTION IV");
    CHECK(U->B[3]->train == 0);

    U->B[3]->setDetection(1);
    AlQueue.put(U->B[3]);
    Algorithm::BlockTick();

    REQUIRE(U->B[3]->train != 0);
    Train * T = U->B[3]->train;

    // Link engine
    T->link(0, TRAIN_TRAIN_TYPE);

    // Train direction is unknown
    REQUIRE(!T->directionKnown);
    CHECK(!U->B[1]->virtualBlocked);
    CHECK(!U->B[2]->virtualBlocked);

    // Move train to next block
    T->setSpeed(20);
    U->B[4]->setDetection(1);
    AlQueue.put(U->B[4]);
    Algorithm::BlockTick();

    CHECK(!U->B[1]->virtualBlocked);
    CHECK(U->B[2]->virtualBlocked);
    CHECK(U->B[3]->virtualBlocked);

    REQUIRE(T->directionKnown);

    CHECK(U->B[3]->expectedTrain == 0); // Expected to get unblocked
    CHECK(U->B[4]->expectedTrain == T); // Still blocked by train
    CHECK(U->B[5]->expectedTrain == T); // Next to be blocked by train

    // Train is 58cm long so three blocks should be blocked.
    CHECK(U->B[4] == T->B);
    CHECK(T->blocks.size() == 3);
    for(auto b: T->blocks){
      loggerf(ERROR, "Have block %2i:%2i", b->module, b->id);
      CHECK(((b != U->B[5] && b != U->B[1]) && (b == U->B[2] || b == U->B[3] || b == U->B[4])));
      CHECK(b->train == T);
    }

    // Step Forward
    U->B[3]->setDetection(0);
    AlQueue.put(U->B[3]);
    Algorithm::BlockTick();

    // Train is 58cm long but moving, so an extra block is blocked.
    CHECK(T->blocks.size() == 3);
    for(auto b: T->blocks){
      loggerf(ERROR, "Have block %2i:%2i", b->module, b->id);
      CHECK(((b != U->B[5] && b != U->B[1]) && (b == U->B[2] || b == U->B[3] || b == U->B[4])));
      CHECK(b->train == T);
    }

    // Step Forward
    U->B[5]->setDetection(1);
    AlQueue.put(U->B[5]);
    Algorithm::BlockTick();

    CHECK(U->B[4]->expectedTrain == 0); // Expected to get unblocked
    CHECK(U->B[5]->expectedTrain == T); // Still blocked by train
    CHECK(U->B[6]->expectedTrain == T); // Next to be blocked by train
    
    U->B[4]->setDetection(0);
    AlQueue.put(U->B[4]);
    Algorithm::BlockTick();

    // Train is 58cm long but moving, so an extra block is blocked.
    CHECK(U->B[5] == T->B);
    CHECK(T->blocks.size() == 3);
    for(auto b: T->blocks){
      loggerf(ERROR, "Have block %2i:%2i", b->module, b->id);
      CHECK(((b != U->B[6] && b != U->B[2]) && (b == U->B[3] || b == U->B[4] || b == U->B[5])));
      CHECK(b->train == T);
    }

    // Simulate that two virtual blocked blocks can be freed
    U->B[2]->setVirtualDetection(1);
    U->B[2]->train = T;

    U->B[6]->setDetection(1);
    AlQueue.put(U->B[6]);
    Algorithm::BlockTick();

    CHECK(!U->B[2]->train);
    CHECK(!U->B[3]->train);
    CHECK(U->B[4]->train == T);
    CHECK(U->B[5]->train == T);
    CHECK(U->B[6]->train == T);

    CHECK(!U->B[2]->virtualBlocked);
    CHECK(!U->B[3]->virtualBlocked);
  }

  SECTION("V - Train with split detectables"){
    loggerf(CRITICAL, "SECTION V");
    CHECK(U->B[3]->train == 0);

    U->B[3]->setDetection(1);
    Algorithm::process(U->B[3], _FORCE);

    REQUIRE(U->B[3]->train != 0);
    Train * T = U->B[3]->train;

    U->B[0]->setDetection(1);
    Algorithm::process(U->B[0], _FORCE);
    REQUIRE(U->B[0]->train != 0);

    Train * tmp_RT[1] = {U->B[0]->train};

    // Link engine
    T->link(2, TRAIN_TRAIN_TYPE, 1, (Train **)&tmp_RT);

    CHECK(U->B[0]->train == T);

    // Train is 143cm long so three blocks should be blocked.
    CHECK(T->blocks.size() == 2);
    for(auto b: T->blocks){
      CHECK(((b != U->B[4] && b != U->B[1] && b != U->B[2]) && (b == U->B[0] || b == U->B[3])));
      CHECK(b->train == T);
    }

    T->setSpeed(10);

    // Step Forward
    U->B[4]->setDetection(1);
    Algorithm::process(U->B[4], _FORCE);

    CHECK(U->B[4]->blocked);
    CHECK(U->B[3]->blocked);
    CHECK(U->B[2]->blocked);
    CHECK(U->B[1]->blocked);
    CHECK(U->B[0]->blocked);

    CHECK(U->B[0]->expectedTrain == T); // Still blocked by trains rear part
    CHECK(U->B[1]->expectedTrain == T); // Next to be blocked by trains rear part

    CHECK(U->B[3]->expectedTrain == 0); // Expected to get unblocked
    CHECK(U->B[4]->expectedTrain == T); // Still blocked by trains front part
    CHECK(U->B[5]->expectedTrain == T); // Next to be blocked by trains front part

    loggerf(WARNING, "U-B[1] blocked");

    U->B[1]->setDetection(1);
    Algorithm::process(U->B[1], _FORCE);

    CHECK(U->B[4]->blocked);
    CHECK(U->B[3]->blocked);
    CHECK(U->B[2]->blocked);
    CHECK(U->B[1]->blocked);
    CHECK(U->B[0]->blocked);

    CHECK(U->B[0]->expectedTrain == 0); // Expected to get unblocked
    CHECK(U->B[1]->expectedTrain == T); // Still blocked by train
    CHECK(U->B[2]->expectedTrain == T); // Next to be blocked by train

    CHECK(U->B[3]->expectedTrain == 0); // Expected to get unblocked
    CHECK(U->B[4]->expectedTrain == T); // Still blocked by train
    CHECK(U->B[5]->expectedTrain == T); // Next to be blocked by train

    loggerf(WARNING, "U-B[3] un-blocked");

    U->B[3]->setDetection(0);
    Algorithm::process(U->B[3], _FORCE);

    CHECK(U->B[4]->blocked);
    CHECK(U->B[3]->blocked);
    CHECK(U->B[2]->blocked);
    CHECK(U->B[1]->blocked);
    CHECK(U->B[0]->blocked);

    CHECK(U->B[4]->expectedTrain == T); // Still blocked by train
    CHECK(U->B[5]->expectedTrain == T); // Next to be blocked by train

    loggerf(WARNING, "U-B[0] un-blocked");
    
    U->B[0]->setDetection(0);
    Algorithm::process(U->B[1], _FORCE);

    CHECK(U->B[4]->blocked);
    CHECK(U->B[3]->blocked);
    CHECK(U->B[2]->blocked);
    CHECK(U->B[1]->blocked);
    CHECK(!U->B[0]->blocked);


    CHECK(U->B[1]->expectedTrain == T); // Still blocked by train
    CHECK(U->B[2]->expectedTrain == T); // Next to be blocked by train

    CHECK(U->B[4]->expectedTrain == T); // Still blocked by train
    CHECK(U->B[5]->expectedTrain == T); // Next to be blocked by train
  }
  
  /*
  SECTION("VI - Two trains approaching each other"){
    U->B[2]->setDetection(1);
    U->B[5]->setDetection(1);
    Algorithm::process(U->B[2], _FORCE);
    Algorithm::process(U->B[5], _FORCE);

    CHECK(U->B[2]->train != nullptr);
    CHECK(U->B[5]->train != nullptr);
    CHECK(U->B[2]->train != U->B[5]->train);

    U->B[3]->setDetection(1);
    Algorithm::process(U->B[3], _FORCE);
    U->B[2]->setDetection(1);
    Algorithm::process(U->B[2], _FORCE);

    U->B[4]->setDetection(1);
    Algorithm::process(U->B[4], _FORCE);
    U->B[3]->setDetection(1);
    Algorithm::process(U->B[3], _FORCE);

    CHECK(U->B[4]->train != U->B[5]->train);
  }
  */
}

TEST_CASE_METHOD(TestsFixture, "Train Following Reversing", "[TrainFollowing][TF-2]"){
    char filenames[1][30] = {"./testconfigs/Alg-2.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;
  
  switchboard::SwManager->LinkAndMap();

  pathlist_find();

  /*
  //  1.0> 1.1> 1.2> 1.3> 1.4> 1.5> 1.6> 1.7>
  */

  for(uint8_t i = 0; i < 8; i++){
    Algorithm::process(U->B[i], _FORCE);
  }

  SECTION("I - Reversing Full Detectable Train"){
    loggerf(CRITICAL, "SECTION I");

    U->B[3]->setDetection(1);
    Algorithm::process(U->B[3], _FORCE);
    U->B[4]->setDetection(1);
    Algorithm::process(U->B[4], _FORCE);

    REQUIRE(U->B[3]->train != 0);
    REQUIRE(U->B[4]->train != 0);
    REQUIRE(U->B[4]->train == U->B[3]->train);
    Train * T = U->B[4]->train;

    T->link(1, TRAIN_TRAIN_TYPE);
    T->setSpeed(10);

    // Step Forward
    U->B[5]->setDetection(1);
    Algorithm::process(U->B[5], _FORCE);

    CHECK(U->B[5]->train == T);
    CHECK(U->B[5] == T->B);
    CHECK(T->blocks.size() == 3);

    CHECK(U->B[2]->expectedTrain == 0);
    CHECK(U->B[3]->expectedTrain == 0);
    CHECK(U->B[4]->expectedTrain == T);
    CHECK(U->B[5]->expectedTrain == T);
    CHECK(U->B[6]->expectedTrain == T);

    CHECK(!T->dir);

    T->setSpeed(0);
    T->reverse();

    CHECK(!T->dir);

    train_speed_event_tick(T->speed_event_data);

    CHECK(T->dir);

    CHECK(T->B == U->B[3]);

    CHECK(U->B[2]->expectedTrain == 0);
    CHECK(U->B[3]->expectedTrain == T);
    CHECK(U->B[4]->expectedTrain == T);
    CHECK(U->B[5]->expectedTrain == T);
    CHECK(U->B[6]->expectedTrain == 0);

    for(uint8_t i = 0; i < 8; i++){
      CHECK(U->B[i]->dir == PREV);
    }

    T->setSpeed(10);

    CHECK(U->B[2]->expectedTrain == T);
    CHECK(U->B[3]->expectedTrain == T);
    CHECK(U->B[4]->expectedTrain == T);
    CHECK(U->B[5]->expectedTrain == 0);
    CHECK(U->B[6]->expectedTrain == 0);

    U->B[5]->setDetection(0);
    Algorithm::process(U->B[5], _FORCE);

    CHECK(U->B[5]->train == nullptr);

    U->B[2]->setDetection(1);
    Algorithm::process(U->B[2], _FORCE);

    CHECK(U->B[2]->train == T);
    CHECK(U->B[1]->expectedTrain == T);
  }

  SECTION("II - Reversing partial detectable"){
    // The virtual blocked blocks should stay on the same side.

    loggerf(CRITICAL, "SECTION II");
    CHECK(U->B[4]->train == 0);

    U->B[4]->setDetection(1);
    Algorithm::process(U->B[4], _FORCE);

    REQUIRE(U->B[4]->train != 0);
    Train * T = U->B[4]->train;

    T->link(0, TRAIN_TRAIN_TYPE);
    T->setSpeed(10);

    // Step Forward
    U->B[5]->setDetection(1);
    Algorithm::process(U->B[5], _FORCE);

    CHECK(U->B[5]->train == T);
    CHECK(U->B[5] == T->B);
    CHECK(T->blocks.size() == 3);

    CHECK(U->B[3]->virtualBlocked);

    CHECK(U->B[3]->expectedTrain == 0);
    CHECK(U->B[4]->expectedTrain == 0);
    CHECK(U->B[5]->expectedTrain == T);
    CHECK(U->B[6]->expectedTrain == T);

    CHECK(!U->B[2]->virtualBlocked);
    CHECK( U->B[3]->virtualBlocked);
    CHECK( U->B[4]->virtualBlocked);
    CHECK( U->B[5]->virtualBlocked);
    CHECK(!U->B[6]->virtualBlocked);

    T->setSpeed(0);
    T->reverse();

    train_speed_event_tick(T->speed_event_data);

    CHECK(!U->B[2]->virtualBlocked);
    CHECK( U->B[3]->virtualBlocked);
    CHECK( U->B[4]->virtualBlocked);
    CHECK( U->B[5]->virtualBlocked);
    CHECK(!U->B[6]->virtualBlocked);

    CHECK(U->B[3]->expectedTrain == 0);
    CHECK(U->B[4]->expectedTrain == T);
    CHECK(U->B[5]->expectedTrain == T);
    CHECK(U->B[6]->expectedTrain == 0);

    for(uint8_t i = 0; i < 8; i++){
      CHECK(U->B[i]->dir == PREV);
    }

    T->setSpeed(10);

    CHECK(U->B[3]->expectedTrain == T);
    CHECK(U->B[4]->expectedTrain == T);
    CHECK(U->B[5]->expectedTrain == 0);
    CHECK(U->B[6]->expectedTrain == 0);

    U->B[5]->setDetection(0);
    Algorithm::processBlock(&U->B[5]->Alg, _FORCE);

    CHECK(U->B[5]->train == nullptr);

    U->B[3]->setDetection(1);
    Algorithm::processBlock(&U->B[3]->Alg, _FORCE);
    
    // FIXME, there is a chance it blocks too far ahead
    //    and therefore setting signals to danger before
    //    the train even entered the protected block
    CHECK(!U->B[0]->virtualBlocked);
    CHECK( U->B[1]->virtualBlocked);
    CHECK( U->B[2]->virtualBlocked);
    CHECK( U->B[3]->virtualBlocked);
    CHECK( U->B[4]->virtualBlocked);
    CHECK(!U->B[5]->virtualBlocked);
    CHECK(!U->B[6]->virtualBlocked);
 
    CHECK(U->B[3]->train == T);

    CHECK(U->B[2]->expectedTrain == T);
    CHECK(U->B[3]->expectedTrain == T);
    CHECK(U->B[4]->expectedTrain == 0);
  }

  SECTION("III - Reversing Split detectables"){
    loggerf(CRITICAL, "SECTION III");
    CHECK(U->B[5]->train == 0);

    U->B[5]->setDetection(1);
    Algorithm::process(U->B[5], _FORCE);

    REQUIRE(U->B[5]->train != 0);
    Train * T = U->B[5]->train;

    U->B[2]->setDetection(1);
    Algorithm::process(U->B[2], _FORCE);
    REQUIRE(U->B[2]->train != 0);

    Train * tmp_RT[1] = {U->B[2]->train};

    // Link engine
    T->link(2, TRAIN_TRAIN_TYPE, 1, (Train **)&tmp_RT);
    CHECK(U->B[2]->train == T);

    T->setSpeed(10);

    // Step Forward
    U->B[6]->setDetection(1);
    Algorithm::process(U->B[6], _FORCE);

    CHECK(T->B == U->B[6]);
    CHECK(U->B[1]->expectedTrain == 0);
    CHECK(U->B[2]->expectedTrain == T);
    CHECK(U->B[3]->expectedTrain == T);
    CHECK(U->B[4]->expectedTrain == 0);
    CHECK(U->B[5]->expectedTrain == 0);
    CHECK(U->B[6]->expectedTrain == T);
    CHECK(U->B[7]->expectedTrain == T);

    T->setSpeed(0);
    T->reverse();

    train_speed_event_tick(T->speed_event_data);

    CHECK(T->B == U->B[2]);

    CHECK(U->B[1]->expectedTrain == 0);
    CHECK(U->B[2]->expectedTrain == T);
    CHECK(U->B[3]->expectedTrain == 0);
    CHECK(U->B[4]->expectedTrain == 0);
    CHECK(U->B[5]->expectedTrain == T);
    CHECK(U->B[6]->expectedTrain == T);
    CHECK(U->B[7]->expectedTrain == 0);

    for(uint8_t i = 0; i < 8; i++){
      CHECK(U->B[i]->dir == PREV);
    }

    T->setSpeed(10);

    CHECK(U->B[1]->expectedTrain == T);
    CHECK(U->B[2]->expectedTrain == T);
    CHECK(U->B[3]->expectedTrain == 0);
    CHECK(U->B[4]->expectedTrain == T);
    CHECK(U->B[5]->expectedTrain == T);
    CHECK(U->B[6]->expectedTrain == 0);
    CHECK(U->B[7]->expectedTrain == 0);

    U->B[6]->setDetection(0);
    Algorithm::process(U->B[6], _FORCE);

    CHECK(U->B[1]->expectedTrain == T);
    CHECK(U->B[2]->expectedTrain == T);
    CHECK(U->B[3]->expectedTrain == 0);
    CHECK(U->B[4]->expectedTrain == T);
    CHECK(U->B[5]->expectedTrain == T);
    CHECK(U->B[6]->expectedTrain == 0);
    CHECK(U->B[7]->expectedTrain == 0);

    U->B[1]->setDetection(1);
    Algorithm::process(U->B[1], _FORCE);

    CHECK(U->B[1]->train == T);

    CHECK(U->B[0]->expectedTrain == T);
    CHECK(U->B[1]->expectedTrain == T);
    CHECK(U->B[2]->expectedTrain == 0);
    CHECK(U->B[3]->expectedTrain == 0);
    CHECK(U->B[4]->expectedTrain == T);
    CHECK(U->B[5]->expectedTrain == T);
    CHECK(U->B[6]->expectedTrain == 0);
    CHECK(U->B[7]->expectedTrain == 0);
  }
  
  /*
  SECTION("VI - Two trains approaching each other"){
    U->B[2]->setDetection(1);
    U->B[5]->setDetection(1);
    Algorithm::process(U->B[2], _FORCE);
    Algorithm::process(U->B[5], _FORCE);

    CHECK(U->B[2]->train != nullptr);
    CHECK(U->B[5]->train != nullptr);
    CHECK(U->B[2]->train != U->B[5]->train);

    U->B[3]->setDetection(1);
    Algorithm::process(U->B[3], _FORCE);
    U->B[2]->setDetection(1);
    Algorithm::process(U->B[2], _FORCE);

    U->B[4]->setDetection(1);
    Algorithm::process(U->B[4], _FORCE);
    U->B[3]->setDetection(1);
    Algorithm::process(U->B[3], _FORCE);

    CHECK(U->B[4]->train != U->B[5]->train);
  }
  */
}

TEST_CASE_METHOD(TestsFixture, "Train Following Ghosting", "[TrainFollowing][TF-3]"){
  char filenames[1][30] = {"./testconfigs/Alg-2.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;
  
  switchboard::SwManager->LinkAndMap();

  pathlist_find();

  /*
  //  1.0> 1.1> 1.2> 1.3> 1.4> 1.5> 1.6> 1.7>
  */

  for(uint8_t i = 0; i < 8; i++){
    Algorithm::process(U->B[i], _FORCE);
  }

  SECTION("I - No linked train"){
    // logger.setlevel_stdout(TRACE);
    U->B[1]->setDetection(1);
    Algorithm::processBlock(&U->B[1]->Alg, _FORCE);

    U->B[2]->setDetection(1);
    Algorithm::processBlock(&U->B[2]->Alg, _FORCE);
    
    U->B[1]->setDetection(0);
    U->B[2]->setDetection(0);
    Algorithm::processBlock(&U->B[1]->Alg, _FORCE);
    Algorithm::processBlock(&U->B[2]->Alg, _FORCE);

    CHECK(U->B[1]->train);
    CHECK(U->B[2]->train);
    
    CHECK(U->B[1]->state == UNKNOWN);
    CHECK(U->B[2]->state == UNKNOWN);

    SECTION("A - Both blocks"){
      U->B[1]->setDetection(1);
      U->B[2]->setDetection(1);
      Algorithm::processBlock(&U->B[1]->Alg, _FORCE);
      Algorithm::processBlock(&U->B[2]->Alg, _FORCE);
      
      CHECK(U->B[1]->train);
      CHECK(U->B[2]->train);
      
      CHECK(U->B[1]->state == BLOCKED);
      CHECK(U->B[2]->state == BLOCKED);

    }
    SECTION("B - Front block only"){
      U->B[2]->setDetection(1);
      Algorithm::processBlock(&U->B[2]->Alg, _FORCE);
      
      CHECK(!U->B[1]->train);
      CHECK( U->B[2]->train);
      
      CHECK(U->B[1]->state > BLOCKED);
      CHECK(U->B[1]->state != UNKNOWN);
      CHECK(U->B[2]->state == BLOCKED);
    }
    SECTION("C - Rear block only"){
      U->B[1]->setDetection(1);
      Algorithm::processBlock(&U->B[1]->Alg, _FORCE);
      
      CHECK( U->B[1]->train);
      CHECK(!U->B[2]->train);
      
      CHECK(U->B[1]->state == BLOCKED);
      CHECK(U->B[2]->state > BLOCKED);
      CHECK(U->B[2]->state != UNKNOWN);
    }
  }
}

TEST_CASE_METHOD(TestsFixture, "Train Following Random releases/detections", "[TrainFollowing][TF-4]"){
  char filenames[1][30] = {"./testconfigs/Alg-2.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;
  
  switchboard::SwManager->LinkAndMap();

  pathlist_find();

  /*
  //  1.0> 1.1> 1.2> 1.3> 1.4> 1.5> 1.6> 1.7>
  */

  for(uint8_t i = 0; i < 8; i++){
    Algorithm::process(U->B[i], _FORCE);
  }

  SECTION("I - Engine"){
    loggerf(CRITICAL, "SECTION I");
    // logger.setlevel_stdout(TRACE);
    // Link an Engine after one block is detected
    //  No speed is set, so no direction is known

    CHECK(U->B[0]->train == 0);

    U->B[0]->setDetection(1);
    Algorithm::processBlock(&U->B[0]->Alg, _FORCE);

    REQUIRE(U->B[0]->train != 0);
    Train * T = U->B[0]->train;
    CHECK(U->B[0] == T->B);

    T->link(0, TRAIN_ENGINE_TYPE);
    T->setSpeed(10);

    // Step Forward
    U->B[1]->setDetection(1);
    Algorithm::processBlock(&U->B[1]->Alg, _FORCE);

    CHECK(T->initialized);

    U->B[0]->setDetection(0);
    Algorithm::processBlock(&U->B[0]->Alg, _FORCE);
    U->B[2]->setDetection(1);
    Algorithm::processBlock(&U->B[2]->Alg, _FORCE);
    U->B[1]->setDetection(0);
    Algorithm::processBlock(&U->B[1]->Alg, _FORCE);

    CHECK(!U->B[1]->train);
    CHECK(U->B[1]->expectedTrain != T);
    CHECK(U->B[2]->expectedTrain == T);
    CHECK(U->B[3]->expectedTrain == T);

    // Random detection at the rear
    U->B[1]->setDetection(1);
    Algorithm::processBlock(&U->B[1]->Alg, _FORCE);

    CHECK(U->B[1]->train == T);
    CHECK(T->Detectables[0]->B[0] == U->B[1]);
    CHECK(std::any_of(T->blocks.begin(), T->blocks.end(), [U](Block * B){ return U->B[1] == B; } ));
  }

}