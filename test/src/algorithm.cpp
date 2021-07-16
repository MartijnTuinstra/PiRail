#include <time.h>
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

TEST_CASE_METHOD(TestsFixture, "Connector Algorithm", "[Alg][Alg-1]"){
  char filenames[4][30] = {"./testconfigs/Alg-1-1.bin", "./testconfigs/Alg-1-2.bin", "./testconfigs/Alg-1-3.bin", "./testconfigs/Alg-1-4.bin"};
  loadSwitchboard(filenames, 4);
  loadStock();

  REQUIRE(switchboard::Units(1));
  REQUIRE(switchboard::Units(2));
  REQUIRE(switchboard::Units(3));
  REQUIRE(switchboard::Units(4));

  switchboard::Units(1)->on_layout = true;
  switchboard::Units(2)->on_layout = true;
  switchboard::Units(3)->on_layout = true;
  switchboard::Units(4)->on_layout = true;

  /*                              --\
  //  1.0->  | --2.0-> --2.1->  |  --3.0-> --3.1-> | --4.0->
  //     C1-1 C1-1          C2-1 C1-1          C2-1 C1-2
  //     C1-2 C1-2          C2-2 C1-2          C2-2 C1-1
  // <1.1--  | <-2.2-- <-2.3--  |  <-3.2-- <-3.3-- | <-4.1--
  //                        \--
  */

  auto connectors = Algorithm::find_connectors();

  loggerf(INFO, "Have %i connectors", connectors.size());

  uint8_t x = 0;
  bool modules_linked = false;

  SECTION("I - Find and connect"){
    loggerf(CRITICAL, "SECTION I");
    while(modules_linked == false){
      if(uint8_t * findResult = Algorithm::find_connectable(&connectors)){
        Algorithm::connect_connectors(&connectors, findResult);
      }

      if(connectors.size() == 0)
        break;

      if(x == 1){
        switchboard::Units(1)->B[0]->setDetection(1);
        switchboard::Units(2)->B[0]->setDetection(1);
      }else if(x == 2){
        switchboard::Units(1)->B[0]->setDetection(0);
        switchboard::Units(2)->B[0]->setDetection(0);
        switchboard::Units(2)->B[1]->setDetection(1);
        switchboard::Units(3)->B[0]->setDetection(1);
      }else if(x == 3){
        switchboard::Units(2)->B[1]->setDetection(0);
        switchboard::Units(3)->B[0]->setDetection(0);
        switchboard::Units(3)->B[1]->setDetection(1);
        switchboard::Units(4)->B[0]->setDetection(1);
      }
      else if(x > 3){
        // _SYS_change(STATE_Modules_Coupled,1);
        modules_linked = true;
      }

      x++;
      //IF ALL JOINED
      //BREAK
    }

    switchboard::Units(3)->B[1]->setDetection(0);
    switchboard::Units(4)->B[0]->setDetection(0);

    switchboard::Units(1)->link_all();
    switchboard::Units(2)->link_all();
    switchboard::Units(3)->link_all();
    switchboard::Units(4)->link_all();

    REQUIRE(connectors.size() == 0);

    CHECK(switchboard::Units(1)->B[0]->next.p.B == switchboard::Units(2)->B[0]);
    CHECK(switchboard::Units(2)->B[0]->prev.p.B == switchboard::Units(1)->B[0]);

    CHECK(switchboard::Units(3)->B[1]->next.p.B == switchboard::Units(4)->B[0]);
    CHECK(switchboard::Units(4)->B[0]->prev.p.B == switchboard::Units(3)->B[1]);

    auto setup = Algorithm::BlockConnectorSetup("testconfigs/Alg-1-setup--.bin");
    setup.save();

  }

  SECTION("II - Connect Stored Configuration"){
    loggerf(CRITICAL, "SECTION II");
    auto setup = Algorithm::BlockConnectorSetup("testconfigs/Alg-1-setup.bin");
    int ret = setup.load(&connectors);

    REQUIRE(ret > 0);

    switchboard::Units(1)->link_all();
    switchboard::Units(2)->link_all();
    switchboard::Units(3)->link_all();
    switchboard::Units(4)->link_all();

    REQUIRE(connectors.size() == 0);

    CHECK(switchboard::Units(1)->B[0]->next.p.B == switchboard::Units(2)->B[0]);
    CHECK(switchboard::Units(2)->B[0]->prev.p.B == switchboard::Units(1)->B[0]);

    CHECK(switchboard::Units(3)->B[1]->next.p.B == switchboard::Units(4)->B[0]);
    CHECK(switchboard::Units(4)->B[0]->prev.p.B == switchboard::Units(3)->B[1]);
  }
}

TEST_CASE_METHOD(TestsFixture, "Train Following", "[Alg][Alg-2]"){
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
    loggerf(CRITICAL, "SECTION I");
    CHECK(U->B[0]->train == 0);

    U->B[0]->setDetection(1);
    Algorithm::process(U->B[0], _FORCE);

    CHECK(U->B[0]->train != 0);

    Train * T = U->B[0]->train;
    CHECK(U->B[0] == T->B);

    // Step Forward
    U->B[1]->setDetection(1);
    Algorithm::process(U->B[1], _FORCE);

    CHECK(U->B[1]->train == T);
    CHECK(T->blocks.size() == 2);

    // Release last block
    U->B[0]->setDetection(0);
    Algorithm::process(U->B[0], _FORCE);

    CHECK(U->B[0]->train == 0);
    CHECK(T->blocks.size() == 1);

    // Step backwards
    U->B[0]->setDetection(1);
    Algorithm::process(U->B[0], _FORCE);

    CHECK(U->B[0]->train == T);
    CHECK(T->blocks.size() == 2);

    // Release second block
    U->B[1]->setDetection(0);
    Algorithm::process(U->B[1], _FORCE);

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

  SECTION("VI - Reversing Full Detectable Train"){
    loggerf(CRITICAL, "SECTION VI");

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

  SECTION("VII - Reversing partial detectable"){
    loggerf(CRITICAL, "SECTION VII");
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

    T->setSpeed(0);
    T->reverse();

    train_speed_event_tick(T->speed_event_data);


    CHECK(U->B[3]->virtualBlocked);

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
    Algorithm::process(U->B[5], _FORCE);

    CHECK(U->B[5]->train == nullptr);

    U->B[3]->setDetection(1);
    Algorithm::process(U->B[3], _FORCE);

    CHECK(U->B[3]->train == T);

    CHECK(U->B[2]->expectedTrain == T);
    CHECK(U->B[3]->expectedTrain == T);
    CHECK(U->B[4]->expectedTrain == 0);
  }

  SECTION("VIII - Reversing Split detectables"){
    loggerf(CRITICAL, "SECTION VIII");
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

TEST_CASE_METHOD(TestsFixture, "Algor Queue", "[Alg][Alg-Q]"){
  char filenames[1][30] = {"./testconfigs/Alg-3.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  AlQueue.put(U->B[0]);

  CHECK(AlQueue.queue->getItems() == 1);
  CHECK(AlQueue.get() == U->B[0]);

  CHECK(AlQueue.queue->getItems() == 0);
  CHECK(AlQueue.get() == 0);

  struct timespec start, end;

  clock_gettime(CLOCK_REALTIME, &start);
  AlQueue.getWait();
  clock_gettime(CLOCK_REALTIME, &end);
  CHECK( (end.tv_sec - start.tv_sec) >= 14);

  AlQueue.put(U->B[0]);

  clock_gettime(CLOCK_REALTIME, &start);
  AlQueue.getWait();
  clock_gettime(CLOCK_REALTIME, &end);
  CHECK( (end.tv_sec - start.tv_sec) < 2);


  AlQueue.puttemp(U->B[0]);

  CHECK(AlQueue.queue->getItems() == 0);
  CHECK(AlQueue.get() == 0);

  CHECK(AlQueue.tempQueue->getItems() == 1);
  AlQueue.cpytmp();

  CHECK(AlQueue.queue->getItems() == 1);
  CHECK(AlQueue.get() == U->B[0]);
}

TEST_CASE_METHOD(TestsFixture, "Train Speed Control", "[Alg][Alg-Sp]"){
  char filenames[1][30] = {"./testconfigs/Alg-Sp.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);

  REQUIRE(U);

  switchboard::SwManager->LinkAndMap();
  
  for(uint8_t j = 0; j < U->block_len; j++){
    if(U->B[j]){
      U->B[j]->setDetection(0);
      AlQueue.put(U->B[j]);
    }
  }

  pathlist_find();

  Algorithm::BlockTick();

  Block *B = U->B[0];

  struct train_sim train = {
    .sim = 'A',
    .train_length = 0,
    .posFront = 0.0,
    .posRear = U->B[0]->length * 1.0f,
    .Front = 0,
    .B = (Block **)_calloc(10, Block *),
    .blocks = 1,
  };
  train.B[0] = B;

  change_Block(B, BLOCKED);
  AlQueue.cpytmp();
  
  Algorithm::BlockTick();

  B->train->link(0, TRAIN_ENGINE_TYPE);
  B->train->setControl(TRAIN_SEMI_AUTO);

  train.train_length = B->train->length;
  train.T = B->train;
  train.T->changeSpeed(180, 0);
 
  AlQueue.put(B);
  Algorithm::BlockTick();

  train.train_length = train.T->p.E->length / 10;

  train.engines_len = 1;
  train.engines = (struct engine_sim *)_calloc(1, struct engine_sim);
  train.engines[0].offset = 0;
  train.engines[0].length = train.T->p.E->length;

  train.posFront = train.B[0]->length - (train.engines[0].length / 10);
  train.posRear = train.B[0]->length;

  int32_t maxIterations = 3000;

  scheduler->disableEvent(RSManager->continue_event);
  scheduler->start();

  auto T = train.T;
  auto ED = T->speed_event_data;

  SECTION("I - CAUTION"){
    for(uint8_t i = 8; i < 12; i++)
      U->B[i]->state = CAUTION;
    

    while(!U->B[5]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[5]->blocked);
    CHECK(T->speed == 180);
    CHECK(T->SpeedState == TRAIN_SPEED_CHANGING);
    CHECK(ED->target_speed == CAUTION_SPEED);
    CHECK(ED->target_distance == 300);
    CHECK(ED->reason == TRAIN_SPEED_R_SIGNAL);

    while(!U->B[6]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[6]->blocked);
    CHECK(T->SpeedState == TRAIN_SPEED_UPDATE);

    while(T->speed_event_data->stepCounter > 1)
      train_testSim_tick(&train, &maxIterations);

    CHECK(T->speed < 180);
    CHECK(T->speed >= T->speed_event_data->startSpeed);
    CHECK(T->SpeedState == TRAIN_SPEED_CHANGING);
    CHECK(ED->target_speed == CAUTION_SPEED);
    CHECK(ED->target_distance == 200);
    CHECK(ED->reason == TRAIN_SPEED_R_SIGNAL);

    while(!U->B[7]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[7]->blocked);

    while(T->speed_event_data->stepCounter > 1)
      train_testSim_tick(&train, &maxIterations);

    CHECK(T->SpeedState == TRAIN_SPEED_CHANGING);
    CHECK(T->speed < 180);
    CHECK(T->speed >= T->speed_event_data->startSpeed);
    CHECK(ED->target_speed == CAUTION_SPEED);
    CHECK(ED->target_distance == 100);
    CHECK(ED->reason == TRAIN_SPEED_R_SIGNAL);

    while(!U->B[8]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[8]->blocked);
    CHECK(T->speed == ED->target_speed);
  }

  SECTION("II - DANGER"){
    T->changeSpeed(90, 0);
    for(uint8_t i = 0; i < 8; i++)
      U->B[i]->state = CAUTION;
    for(uint8_t i = 8; i < 12; i++)
      U->B[i]->state = DANGER;

    maxIterations = 2000;

    while(!U->B[6]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[6]->blocked);
    CHECK(T->SpeedState == TRAIN_SPEED_DRIVING);
    CHECK(ED->reason == TRAIN_SPEED_R_NONE);

    while(!U->B[7]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[7]->blocked);
    CHECK(T->SpeedState == TRAIN_SPEED_CHANGING);
    CHECK(ED->target_speed == 0);
    CHECK(ED->target_distance == 100);
    CHECK(ED->reason == TRAIN_SPEED_R_SIGNAL);

    while(!U->B[8]->blocked && T->speed > 0 && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(!U->B[8]->blocked);
    CHECK(U->B[7]->train->speed == 0);
    CHECK(U->B[7]->train->SpeedState == TRAIN_SPEED_STOPPING_WAIT);

    while(!U->B[8]->blocked && !T->speed_event->disabled && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(U->B[7]->train->SpeedState == TRAIN_SPEED_WAITING);
  }
  
  SECTION("III - CAUTION changing to PROCEED"){
    U->B[8]->state = CAUTION;
    U->B[9]->state = CAUTION;
    U->B[10]->state = CAUTION;
    
    maxIterations = 2000;

    while(!U->B[5]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[5]->blocked);
    CHECK(T->SpeedState == TRAIN_SPEED_CHANGING);
    CHECK(ED->target_speed == CAUTION_SPEED);
    CHECK(ED->target_distance == 300);
    CHECK(ED->reason == TRAIN_SPEED_R_SIGNAL);
    CHECK(T->speed == 180);

    while(T->speed_event_data->stepCounter != 4 && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->speed < 180);
    U->B[8]->state = PROCEED;
    
    while(T->speed_event_data->stepCounter != 5 && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->SpeedState == TRAIN_SPEED_DRIVING);
    CHECK(ED->reason == TRAIN_SPEED_R_NONE);

    while(!U->B[7]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[7]->blocked);
    CHECK(T->SpeedState == TRAIN_SPEED_CHANGING);
    CHECK(ED->target_speed == CAUTION_SPEED);
    CHECK(ED->target_distance == 200);
    CHECK(ED->reason == TRAIN_SPEED_R_SIGNAL);

    while(!U->B[8]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[8]->blocked);
    CHECK(T->SpeedState == TRAIN_SPEED_UPDATE);
    CHECK(ED->target_speed == CAUTION_SPEED);
    CHECK(ED->target_distance == 100);
    CHECK(ED->reason == TRAIN_SPEED_R_SIGNAL);

    while(!U->B[8]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[8]->blocked);
    CHECK(T->SpeedState == TRAIN_SPEED_UPDATE);
    CHECK(ED->target_speed == CAUTION_SPEED);
    CHECK(ED->target_distance == 100);
    CHECK(ED->reason == TRAIN_SPEED_R_SIGNAL);
  }

  SECTION("IV - Speed"){
    maxIterations = 2000;

    // Skip first blocks
    while(!U->B[5]->blocked){
      train_test_tick(&train, &maxIterations);
    }

    while(!U->B[7]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[7]->blocked);
    CHECK(T->SpeedState == TRAIN_SPEED_CHANGING);
    CHECK(ED->target_speed == 100);
    CHECK(ED->target_distance == 300);
    CHECK(ED->reason == TRAIN_SPEED_R_MAXSPEED);

    while(!U->B[8]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[8]->blocked);
    CHECK(T->SpeedState == TRAIN_SPEED_UPDATE);
    CHECK(ED->target_speed == 100);
    CHECK(ED->target_distance == 200);
    CHECK(ED->reason == TRAIN_SPEED_R_MAXSPEED);

    while(!U->B[9]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[9]->blocked);
    CHECK(T->SpeedState == TRAIN_SPEED_UPDATE);
    CHECK(ED->target_speed == 100);
    CHECK(ED->target_distance == 100);
    CHECK(ED->reason == TRAIN_SPEED_R_MAXSPEED);

    while(!U->B[12]->blocked && maxIterations > 0){
      Block * trainBlock = T->B;

      while(T->B == trainBlock && maxIterations > 0){
        train_testSim_tick(&train, &maxIterations);
      }

      CHECK(T->SpeedState != TRAIN_SPEED_CHANGING);
      CHECK(T->speed == 100);
    }

    REQUIRE(U->B[12]->blocked);
    CHECK(T->SpeedState != TRAIN_SPEED_CHANGING);
    CHECK(T->speed == 100);

    while(U->B[12]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(!U->B[12]->blocked);
    CHECK(T->SpeedState == TRAIN_SPEED_CHANGING);
    CHECK(ED->target_speed > 100);
    CHECK(ED->target_distance == 100);
    CHECK(ED->reason == TRAIN_SPEED_R_MAXSPEED);

    while(!U->B[14]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[14]->blocked);
    CHECK(T->SpeedState == TRAIN_SPEED_CHANGING); // FIXME, train could allready be accelerating because it left B[13]
    CHECK(ED->target_speed > 100);
    CHECK(ED->target_speed <= 180);
    CHECK(ED->target_distance == 100);
    CHECK(ED->reason == TRAIN_SPEED_R_MAXSPEED);
  }

  SECTION("V - End of route"){
    REQUIRE(U->St[0]);

    T->setRoute(U->St[0]);

    REQUIRE(T->route);
    REQUIRE(T->routeStatus == TRAIN_ROUTE_RUNNING);
    REQUIRE(T->route->destination == U->St[0]->uid);

    while(!U->B[7]->blocked && maxIterations > 0){
      train_test_tick(&train, &maxIterations);
    }

    while(!U->B[10]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->routeStatus == TRAIN_ROUTE_ENTERED_DESTINATION);

    while(!U->B[13]->blocked && T->speed > 90 && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->routeStatus == TRAIN_ROUTE_AT_DESTINATION);
    CHECK(ED->reason == TRAIN_SPEED_R_ROUTE);

    while(!U->B[13]->blocked && T->speed != 0 && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->speed == 0);
    CHECK(!U->B[13]->blocked);
    CHECK(U->B[12]->blocked);
    CHECK(!U->B[10]->blocked);
    CHECK(ED->reason == TRAIN_SPEED_R_NONE);
    
    CHECK(T->SpeedState == TRAIN_SPEED_STOPPING_WAIT);

    while(!U->B[13]->blocked && !T->speed_event->disabled && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }
    
    CHECK(T->SpeedState == TRAIN_SPEED_WAITING_DESTINATION);

  }

  SECTION("VI - At of waypoint"){
    T->setRoute(U->B[10]);

    REQUIRE(T->route);
    REQUIRE(T->routeStatus == TRAIN_ROUTE_RUNNING);
    REQUIRE(T->route->destination == 0x010A); // Unit 1 Block 10

    while(!U->B[10]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->routeStatus == TRAIN_ROUTE_AT_DESTINATION);

    while(!U->B[13]->blocked && T->speed != 0 && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->speed != 0);
    CHECK(U->B[13]->blocked);

  }

  SECTION("VII - User Stop change speed"){
    while(!U->B[5]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->speed == 180);
    T->changeSpeed(0, 300);

    CHECK(T->SpeedState == TRAIN_SPEED_CHANGING);
    CHECK(ED->target_speed == 0);
    CHECK(ED->target_distance == 300);
    CHECK(ED->reason == TRAIN_SPEED_R_MAXSPEED);

    while(!U->B[8]->blocked && T->speed && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->speed == 0);
    CHECK(T->SpeedState == TRAIN_SPEED_STOPPING);

    while(!T->speed_event->disabled && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->SpeedState == TRAIN_SPEED_IDLE);
  }

  SECTION("VIII - User Stop set speed"){
    while(!U->B[2]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->speed == 180);
    T->setSpeed(0);
    CHECK(T->speed == 0);
    CHECK(T->SpeedState == TRAIN_SPEED_STOPPING);

    maxIterations = 50;

    while(!T->speed_event->disabled && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->SpeedState == TRAIN_SPEED_IDLE);
  }

  scheduler->stop();
}
/*
TEST_CASE_METHOD(TestsFixture, "Train Route Following", "[Alg][Alg-R]"){
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

  Block *B = U[1]->B[3];

  struct train_sim train = {
    .sim = 'A',
    .train_length = 0,
    .posFront = 0.0,
    .posRear = 124.0,
    .Front = 0,
    .B = (Block **)_calloc(10, Block *),
    .blocks = 1,
  };
  train.B[0] = B;

  change_Block(B, BLOCKED);
  AlQueue.cpytmp();
  
  Algorithm::BlockTick();

  B->train->link(0, TRAIN_ENGINE_TYPE);
  B->train->setControl(TRAIN_MANUAL);

  train.train_length = B->train->length;
  train.T = B->train;
  train.T->changeSpeed(100, 0);
 
  AlQueue.put(B);
  Algorithm::BlockTick();

  train.train_length = train.T->p.E->length / 10;

  train.engines_len = 1;
  train.engines = (struct engine_sim *)_calloc(1, struct engine_sim);
  train.engines[0].offset = 0;
  train.engines[0].length = train.T->p.E->length;

  train.posFront = train.B[0]->length - (train.engines[0].length / 10);
  train.posRear = train.B[0]->length;

  int32_t maxIterations = 5000;

  SECTION("I - Just a circle"){
    while(!U[1]->B[5]->blocked && maxIterations > 0){
      train_test_tick(&train, &maxIterations);
    }

    CHECK(!U[1]->B[3]->blocked);
    CHECK(!U[1]->B[4]->blocked);
    CHECK(U[1]->B[5]->blocked);

    maxIterations = 5000;

    while(!U[1]->B[3]->blocked && maxIterations > 0){
      train_test_tick(&train, &maxIterations);
    }

    CHECK(U[1]->B[3]->blocked);
  }

  SECTION("II - A route"){
    train.T->setRoute(U[1]->B[8]);

    while(!U[3]->B[2]->blocked && maxIterations > 0){
      train_test_tick(&train, &maxIterations);
    }

    CHECK(U[3]->B[2]->blocked);
    CHECK(maxIterations > 0);
    
    while(!U[1]->B[10]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(maxIterations > 0);
    maxIterations = 1000;

    // Train should stop not stop on a waypoint
    while(!U[1]->B[8]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U[1]->B[8]->blocked);
    CHECK(maxIterations > 0);
    CHECK(U[1]->B[8]->train->speed_event_data->target_speed > 0);
  }

  SECTION("III - A blocked route"){
    // Run the scheduler to change speed of train dynamically
    //   or include it in the iterration loop since sim time does not equal real time.

    U[1]->B[8]->setDetection(1);
    Algorithm::process(U[1]->B[8], _FORCE);

    train.T->setRoute(U[1]->B[8]); // Set train destination

    while(!U[3]->B[2]->blocked && maxIterations > 0){
      train_test_tick(&train, &maxIterations);
    }

    CHECK(U[3]->B[2]->blocked);
    CHECK(maxIterations > 0);

    maxIterations = 2000;

    while(maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(maxIterations == 0);
    CHECK(!U[1]->B[9]->blocked);
    CHECK(train.T->B == U[4]->B[0]);
  }
}
*/