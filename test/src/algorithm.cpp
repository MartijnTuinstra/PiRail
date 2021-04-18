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

#include "rollingstock/railtrain.h"

#include "train.h"
#include "sim.h"

#include "algorithm/queue.h"
#include "algorithm/core.h"
#include "algorithm/component.h"
#include "algorithm/blockconnector.h"

#include "pathfinding.h"

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
  U->link_all();

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

    RailTrain * T = U->B[0]->train;
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
    RailTrain * T = U->B[0]->train;
    CHECK(U->B[0] == T->B);

    // Step Forward
    U->B[1]->setDetection(1);
    Algorithm::process(U->B[1], _FORCE);

    CHECK(U->B[1]->train == T);

    // Link engine
    T->link(0, RAILTRAIN_ENGINE_TYPE);

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
    RailTrain * T = U->B[1]->train;

    // Link engine
    T->link(0, RAILTRAIN_ENGINE_TYPE);

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
    RailTrain * T = U->B[0]->train;

    T->link(1, RAILTRAIN_TRAIN_TYPE);
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
    RailTrain * T = U->B[3]->train;

    // Link engine
    T->link(0, RAILTRAIN_TRAIN_TYPE);

    // Train direction is unknown
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
    RailTrain * T = U->B[3]->train;

    U->B[0]->setDetection(1);
    Algorithm::process(U->B[0], _FORCE);
    REQUIRE(U->B[0]->train != 0);

    RailTrain * tmp_RT[1] = {U->B[0]->train};

    // Link engine
    T->link(2, RAILTRAIN_TRAIN_TYPE, 1, (RailTrain **)&tmp_RT);

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
    RailTrain * T = U->B[4]->train;

    T->link(1, RAILTRAIN_TRAIN_TYPE);
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

    T->setSpeed(0);
    T->reverse();

    CHECK(T->B == U->B[3]);

    CHECK(U->B[2]->expectedTrain == T);
    CHECK(U->B[3]->expectedTrain == T);
    CHECK(U->B[4]->expectedTrain == T);
    CHECK(U->B[5]->expectedTrain == 0);
    CHECK(U->B[6]->expectedTrain == 0);

    for(uint8_t i = 0; i < 8; i++){
      CHECK(U->B[i]->dir == 0b100);
    }

    T->setSpeed(10);

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
    RailTrain * T = U->B[4]->train;

    T->link(0, RAILTRAIN_TRAIN_TYPE);
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

    CHECK(U->B[3]->virtualBlocked);

    CHECK(U->B[3]->expectedTrain == T);
    CHECK(U->B[4]->expectedTrain == T);
    CHECK(U->B[5]->expectedTrain == 0);
    CHECK(U->B[6]->expectedTrain == 0);

    for(uint8_t i = 0; i < 8; i++){
      CHECK(U->B[i]->dir == 0b100);
    }

    T->setSpeed(10);

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
    RailTrain * T = U->B[5]->train;

    U->B[2]->setDetection(1);
    Algorithm::process(U->B[2], _FORCE);
    REQUIRE(U->B[2]->train != 0);

    RailTrain * tmp_RT[1] = {U->B[2]->train};

    // Link engine
    T->link(2, RAILTRAIN_TRAIN_TYPE, 1, (RailTrain **)&tmp_RT);
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

    CHECK(T->B == U->B[2]);

    CHECK(U->B[1]->expectedTrain == T);
    CHECK(U->B[2]->expectedTrain == T);
    CHECK(U->B[3]->expectedTrain == 0);
    CHECK(U->B[4]->expectedTrain == T);
    CHECK(U->B[5]->expectedTrain == T);
    CHECK(U->B[6]->expectedTrain == 0);
    CHECK(U->B[7]->expectedTrain == 0);

    for(uint8_t i = 0; i < 8; i++){
      CHECK(U->B[i]->dir == 0b100);
    }

    T->setSpeed(10);

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

TEST_CASE_METHOD(TestsFixture, "Algorithm Switch Setter", "[Alg][Alg-3]"){
  char filenames[1][30] = {"./testconfigs/Alg-3.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;
  U->link_all();

  pathlist_find();

  /*
  // I
  //           Sw0/--->
  // 1.0> 1.1> 1.2----> 1.3>
  //
  // II
  //           ---\Sw1
  // 1.4> 1.5> ----1.6> 1.7>
  //
  // III - IV
  //                       |---St0---|
  //              Sw2/---> 1.11> 1.12>
  // 1.08> 1.09> 1.10----> 1.13> 1.14>
  //                       |---St1---|
  //
  // V - XII
  //                             |---St2---|
  //                    Sw5/---> 1.20> 1.21> ---\Sw6
  // 1.15> 1.16> ------1.17----> 1.18> 1.19> ----1.27> 1.28-> ----\
  //                  /Sw4       |---St3---|            Sw7\       \
  //              Sw3/           |---St4---|                \       \Sw8
  // 1.22> 1.23> 1.24----------> 1.25> 1.26> ----1.29> ------1.30> ---1.31> 1.33>
  //               Sw9\Sw10                                MSSw0  \
  // <1.34 <1.35 <----------1.36 <1.37 <1.38 <1.39                 \-> 1.32>
  //                             |---St5---|
  //
  // XIII
  //
  //         <-1.40- <-1.41- <---1.42- <-1.43- <-1.44-
  //                          / Sw11
  //                    Sw12 /
  // -1.45-> -1.46-> -1.47---> -1.48-> -1.49->
  //
  */

  for(uint8_t i = 0; i < 39; i++){
    Algorithm::process(U->B[i], _FORCE);
  }

  U->Sw[0]->setState(0);
  U->Sw[1]->setState(0);
  U->Sw[2]->setState(0);
  U->Sw[3]->setState(0);
  U->Sw[4]->setState(0);
  U->Sw[5]->setState(0);
  if(AlQueue.queue->getItems())
    Algorithm::BlockTick();

  SECTION("I - Approaching S side"){
    // End of divergence is end of track therefore no valid solution
    //  - Always set the switch to straight

    U->B[0]->setDetection(1);
    Algorithm::process(U->B[0], _FORCE);

    auto T = U->B[0]->train;
    REQUIRE(T != nullptr);
    REQUIRE(T == RSManager->getRailTrain(0));
    
    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[0], U->B[2], U->B[2]->next, NEXT | SWITCH_CARE);

    CHECK(U->Sw[0]->state == 0);
    CHECK(U->Sw[0]->Detection->state == CAUTION);
    CHECK(U->Sw[0]->Detection->switchReserved);
    CHECK(U->Sw[0]->Detection->isReservedBy(T));

    T->dereserveBlock(U->Sw[0]->Detection);
    U->Sw[0]->Detection->state = PROCEED;

    U->Sw[0]->setState(1); // Set Diverging
    CHECK(U->Sw[0]->state == 1);

    Algorithm::BlockTick();

    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[0], U->B[2], U->B[2]->next, NEXT | SWITCH_CARE);

    CHECK(U->Sw[0]->state == 0);
    CHECK(U->Sw[0]->Detection->state == RESERVED_SWITCH);
    CHECK(U->Sw[0]->Detection->switchReserved);
    CHECK(U->Sw[0]->Detection->isReservedBy(T));

    Algorithm::BlockTick();
    
    CHECK(U->Sw[0]->Detection->state == CAUTION);
  }

  SECTION("II - Approaching s side"){
    // Set the switch to the correct state
    //  - set straight

    U->B[4]->setDetection(1);
    Algorithm::process(U->B[4], _FORCE);

    auto T = U->B[4]->train;
    CHECK(T != nullptr);
    CHECK(T == RSManager->getRailTrain(0));

    // Force SwitchSolver, since no speed
    SwitchSolver::solve(T, U->B[4], U->B[5], U->B[5]->next, NEXT | SWITCH_CARE);

    CHECK(U->Sw[1]->state == 0);
    CHECK(U->Sw[1]->Detection->state == CAUTION);
    CHECK(U->Sw[1]->Detection->switchReserved);
    CHECK(U->Sw[1]->Detection->isReservedBy(T));

    T->dereserveBlock(U->Sw[1]->Detection);

    U->Sw[1]->setState(1); // Set Diverging
    CHECK(U->Sw[1]->state == 1);

    Algorithm::BlockTick();

    // Force SwitchSolver, since no speed
    SwitchSolver::solve(T, U->B[4], U->B[5], U->B[5]->next, NEXT | SWITCH_CARE);

    CHECK(U->Sw[1]->state == 0);
    CHECK(U->Sw[1]->Detection->state == RESERVED_SWITCH);
    CHECK(U->Sw[1]->Detection->switchReserved);
    CHECK(U->Sw[1]->Detection->isReservedBy(T));

    Algorithm::BlockTick();
    
    CHECK(U->Sw[1]->Detection->state == CAUTION);
  }

  SECTION("IIIa - Approaching S side with station"){
    // Train will try to go around stopped train inside station
    //  - Set switch to diverging
    //  - Reserve Switch

    U->B[8]->setDetection(1);
    U->B[14]->setDetection(1);
    Algorithm::process(U->B[8], _FORCE);
    Algorithm::process(U->B[14], _FORCE);

    auto T = U->B[8]->train;
    REQUIRE(T != nullptr);
    REQUIRE(T == RSManager->getRailTrain(0));
    CHECK(U->B[14]->train == RSManager->getRailTrain(1));
    CHECK(U->B[14]->train->stopped);

    RSManager->getRailTrain(0)->setSpeed(10);
    
    // Force switchsolver, since speed is too low
    SwitchSolver::solve(T, U->B[8], U->B[10], U->B[10]->next, NEXT | SWITCH_CARE);

    CHECK(U->Sw[2]->state == 1);
    CHECK(U->Sw[2]->Detection->state == RESERVED_SWITCH);
    CHECK(U->Sw[2]->Detection->switchReserved);
    CHECK(U->Sw[2]->Detection->isReservedBy(T));
  }

  SECTION("IIIb - Approaching S side with station"){
    // Train will try to go around stopped train inside station
    //   even if it is allready reserved
    //  - Set switch to diverging
    //  - Reserve Switch

    U->B[8]->setDetection(1);
    Algorithm::process(U->B[8], _FORCE);
    
    auto T = U->B[8]->train;
    REQUIRE(T == RSManager->getRailTrain(0));

    RSManager->getRailTrain(0)->setSpeed(10);
    
    // Force switchsolver, since speed is too low
    SwitchSolver::solve(T, U->B[8], U->B[10], U->B[10]->next, NEXT | SWITCH_CARE);
    
    CHECK(U->Sw[2]->state == 0);
    CHECK(U->Sw[2]->Detection->state == CAUTION);
    CHECK( U->Sw[2]->Detection->switchReserved);
    CHECK(!U->Sw[2]->Detection->reserved);
    CHECK(U->Sw[2]->Detection->isReservedBy(U->B[8]->train));

    U->B[14]->setDetection(1);
    Algorithm::process(U->B[14], _FORCE);

    CHECK(U->B[14]->train == RSManager->getRailTrain(1));

    // Force switchsolver, since speed is too low
    SwitchSolver::solve(T, U->B[8], U->B[10], U->B[10]->next, NEXT | SWITCH_CARE);
    
    CHECK(U->Sw[2]->state == 1);
  }

  SECTION("IIIc - Approaching S side with station"){
    // Paths after a switch must be set to the correct direction
    //  - Reserve Switch
    //  - Reserve Path

    U->B[11]->path->reverse();
    U->B[13]->path->reverse();

    CHECK(U->B[11]->dir == 4);
    CHECK(U->B[13]->dir == 4);

    U->B[8]->setDetection(1);
    Algorithm::process(U->B[8], _FORCE);
    
    CHECK(U->B[8]->train == RSManager->getRailTrain(0));

    RSManager->getRailTrain(0)->setSpeed(10);
    
    // Force switchsolver, since speed is too low
    SwitchSolver::solve(U->B[8]->train, U->B[8], U->B[10], U->B[10]->next, NEXT | SWITCH_CARE);
    
    CHECK(U->Sw[2]->state == 0);
    CHECK(U->Sw[2]->Detection->state == CAUTION);
    CHECK( U->Sw[2]->Detection->switchReserved);
    CHECK(!U->Sw[2]->Detection->reserved);
    CHECK(U->Sw[2]->Detection->isReservedBy(U->B[8]->train));

    CHECK(U->B[11]->dir == 4);
    CHECK(U->B[13]->dir == 0);
  }
  
  SECTION("IIId - Approaching S side with station"){
    // Paths after a switch must be set to the correct direction
    //  - Set Switch to diverging
    //  - Reserve Switch
    //  - Reserve Path

    U->B[11]->path->reverse();
    U->B[13]->path->reverse();
    U->B[13]->path->reserve(new RailTrain(U->B[0]));

    CHECK(U->B[11]->dir == 4);
    CHECK(U->B[13]->dir == 4);

    U->B[8]->setDetection(1);
    Algorithm::process(U->B[8], _FORCE);
    
    auto T = U->B[8]->train;
    CHECK(T == RSManager->getRailTrain(1));

    T->setSpeed(10);
    
    // Force switchsolver, since speed is too low
    SwitchSolver::solve(U->B[8]->train, U->B[8], U->B[10], U->B[10]->next, NEXT | SWITCH_CARE);
    
    CHECK(U->Sw[2]->state == 1);
    CHECK(U->Sw[2]->Detection->state == RESERVED_SWITCH);
    CHECK( U->Sw[2]->Detection->switchReserved);
    CHECK(!U->Sw[2]->Detection->reserved);
    CHECK(U->Sw[2]->Detection->isReservedBy(U->B[8]->train));

    CHECK(U->B[11]->dir == 0);
    CHECK(U->B[13]->dir == 4);
  }

  SECTION("IV - Approaching S side with station fully blocked"){
    // If no valid solution is possible, both states lead to invalid solution.
    //  - enable switchWrongState
    //  - block state must be DANGER
    //  - block must not be reserved

    U->B[8]->setDetection(1);
    U->B[12]->setDetection(1);
    U->B[14]->setDetection(1);
    Algorithm::process(U->B[8], _FORCE);
    Algorithm::process(U->B[12], _FORCE);
    Algorithm::process(U->B[14], _FORCE);

    CHECK(U->B[8]->train != nullptr);
    CHECK(U->B[8]->train == RSManager->getRailTrain(0));

    RSManager->getRailTrain(0)->setSpeed(10);

    // Force SwitchSolver, since speed is too low
    SwitchSolver::solve(U->B[8]->train, U->B[8], U->B[10], U->B[10]->next, NEXT | SWITCH_CARE);

    CHECK(U->Sw[2]->state == 0);
    CHECK(U->Sw[2]->Detection->state == DANGER);
    CHECK(U->Sw[2]->Detection->state != RESERVED_SWITCH);
    CHECK(U->Sw[2]->Detection->switchWrongState);
    CHECK(!U->Sw[2]->Detection->switchReserved);
    CHECK(!U->Sw[2]->Detection->reserved);
  }

  SECTION("V - Approaching s side with station and switchblock"){
    // Go around B[19]
    //  - set Sw[4] straight
    //  - set Sw[5] diverging

    U->B[15]->setDetection(1);
    U->B[19]->setDetection(1);
    Algorithm::process(U->B[15], _FORCE);
    Algorithm::process(U->B[19], _FORCE);

    U->Sw[4]->setState(1);
    Algorithm::BlockTick();

    // Algorithm::process(U->B[15], _FORCE);
    // U->St[3]->train -> SIGSEV
    CHECK(U->B[19]->station == U->St[3]);
    CHECK(U->B[19]->train == U->St[3]->train);
    U->B[19]->train->setSpeed(0);

    CHECK(U->B[15]->train != nullptr);
    CHECK(U->B[15]->train == RSManager->getRailTrain(0));

    RSManager->getRailTrain(0)->setSpeed(10);
    
    // Force SwitchSolver, since speed is too low
    SwitchSolver::solve(U->B[15]->train, U->B[15], U->B[16], U->B[16]->next, NEXT | SWITCH_CARE);

    CHECK(U->Sw[4]->state == 0);
    CHECK(U->Sw[5]->state == 1);

    CHECK(U->Sw[4]->Detection->switchReserved);
    CHECK(U->Sw[4]->Detection->isReservedBy(U->B[15]->train));
  }

  SECTION("VI- Approaching S side with full station and switchblock"){
    // No valid solution therefore
    //  - do not set switcWrongState (switch state is not changed therefore it is implicit that switc is in the wrong state)
    //  - set DANGER
    //  - not reserved

    U->B[15]->setDetection(1);
    U->B[19]->setDetection(1);
    U->B[21]->setDetection(1);
    Algorithm::process(U->B[15], _FORCE);
    Algorithm::process(U->B[19], _FORCE);
    Algorithm::process(U->B[21], _FORCE);

    // U->St[2]->train->setSpeed(0);
    // U->St[3]->train->setSpeed(0);
    U->B[19]->train->setSpeed(0);
    U->B[21]->train->setSpeed(0);

    // U->B[15]->train->dereserveBlock(U->Sw[4]->Detection); // Allow switch to change

    U->Sw[4]->setState(1, 0);
    Algorithm::BlockTick();

    CHECK(U->Sw[4]->state == 1);
    CHECK(U->Sw[5]->state == 0);

    CHECK(U->B[15]->train != nullptr);
    CHECK(U->B[15]->train == RSManager->getRailTrain(0));

    RSManager->getRailTrain(0)->setSpeed(10);
        
    // Force SwitchSolver, since speed is too low
    SwitchSolver::solve(U->B[15]->train, U->B[15], U->B[16], U->B[16]->next, NEXT | SWITCH_CARE);

    CHECK(U->Sw[4]->state == 1);
    CHECK(U->Sw[5]->state == 0);

    CHECK( U->Sw[4]->Detection->state == DANGER);
    CHECK(!U->Sw[4]->Detection->switchWrongState);
    CHECK(!U->Sw[4]->Detection->switchReserved);
    CHECK(!U->Sw[4]->Detection->reserved);
  }

  SECTION("VII - Approaching switch with route"){
    U->B[22]->setDetection(1);
    Algorithm::process(U->B[22], _FORCE);

    auto T = U->B[22]->train;

    REQUIRE(T);

    T->setRoute(U->B[21]);
    
    REQUIRE(T->route);
    REQUIRE(T->route->found_forward);

    CHECK(T != nullptr);
    CHECK(T == RSManager->getRailTrain(0));

    RSManager->getRailTrain(0)->setSpeed(10);

    // Force SwitchSolver, since speed is too low
    SwitchSolver::solve(U->B[22]->train, U->B[22], U->B[24], U->B[24]->next, NEXT | SWITCH_CARE);


    CHECK(U->Sw[3]->state == 1);
    CHECK(U->Sw[4]->state == 1);
    CHECK(U->Sw[5]->state == 1);
  }

  SECTION("VIII - Approaching switch with route with blocked station"){
    U->B[22]->setDetection(1);
    U->B[21]->setDetection(1);
    Algorithm::process(U->B[22], _FORCE);
    Algorithm::process(U->B[21], _FORCE);

    auto T = U->B[22]->train;

    REQUIRE(T);

    T->setRoute(U->B[21]);

    REQUIRE(T->route);
    REQUIRE(T->route->found_forward);

    CHECK(T == RSManager->getRailTrain(0));

    RSManager->getRailTrain(0)->setSpeed(10);

    // Force SwitchSolver, since speed is too low
    SwitchSolver::solve(U->B[22]->train, U->B[22], U->B[24], U->B[24]->next, NEXT | SWITCH_CARE);

    CHECK(U->Sw[3]->state == 0);
    CHECK(U->Sw[4]->state == 0);
    CHECK(U->Sw[5]->state == 0);

    CHECK(U->B[24]->switchWrongState);
  }

  SECTION("IX - Approaching switch with route applying detour"){
    U->B[22]->setDetection(1);
    U->B[26]->setDetection(1);
    Algorithm::process(U->B[22], _FORCE);
    Algorithm::process(U->B[26], _FORCE);

    auto T = U->B[22]->train;

    REQUIRE(T);
    CHECK(T == RSManager->getRailTrain(0));
    REQUIRE(U->B[26]->train);
    CHECK(U->B[26]->train == RSManager->getRailTrain(1));

    T->setRoute(U->B[33]);

    REQUIRE(T->route);
    REQUIRE(T->route->found_forward);

    RSManager->getRailTrain(0)->setSpeed(10);

    // Force SwitchSolver, since speed is too low
    SwitchSolver::solve(U->B[22]->train, U->B[22], U->B[24], U->B[24]->next, NEXT | SWITCH_CARE);

    CHECK(U->Sw[3]->state == 1);
    CHECK(U->Sw[4]->state == 1);
    CHECK(U->Sw[5]->state == 0);
    CHECK(U->Sw[5]->updatedState == 0); // Switch should not be updated since it is allready in position

    CHECK(U->B[24]->isReservedBy(U->B[22]->train));
  }

  SECTION("X - Approaching S with reserved switches"){
    RailTrain * tmpRT = new RailTrain(U->B[17]);
    U->B[17]->reserve(tmpRT);

    U->Sw[3]->setState(1);
    Algorithm::BlockTick();

    U->B[22]->setDetection(1);
    Algorithm::process(U->B[22], _FORCE);

    REQUIRE(U->B[22]->train);
    U->B[22]->train->setSpeed(10);

    // Force SwitchSolver, since speed is too low
    SwitchSolver::solve(U->B[22]->train, U->B[22], U->B[24], U->B[24]->next, NEXT | SWITCH_CARE);

    CHECK(U->Sw[3]->state == 0);
    CHECK(U->Sw[4]->state == 0);
    CHECK(U->Sw[5]->state == 0);

    CHECK(U->B[24]->isReservedBy(U->B[22]->train));
  }

  SECTION("XI - Reversed Station path"){
    Algorithm::print_block_debug(U->B[37]);
    REQUIRE(U->B[36]->path != U->B[37]->path);
    REQUIRE(U->B[37]->path->Entrance != U->B[37]);

    CHECK(U->B[37]->path->direction == 1);

    U->B[22]->setDetection(1);
    Algorithm::process(U->B[22], _FORCE);

    REQUIRE(U->B[22]->train);
    U->B[22]->train->setRoute(U->B[39]);
    U->B[22]->train->setSpeed(10);

    // Force SwitchSolver, since speed is too low
    SwitchSolver::solve(U->B[22]->train, U->B[22], U->B[24], U->B[24]->next, NEXT | SWITCH_CARE);

    CHECK(U->Sw[3]->state == 0);
    CHECK(U->Sw[9]->state == 1);
    CHECK(U->Sw[10]->state == 1);

    CHECK(U->B[24]->isReservedBy(U->B[22]->train));
    CHECK(U->B[36]->isReservedBy(U->B[22]->train));
    CHECK(U->B[37]->isReservedBy(U->B[22]->train));

    CHECK(U->B[37]->path->direction == 0);
  }

  SECTION("XII - Reserved reversed Station path"){
    REQUIRE(U->B[36]->path != U->B[37]->path);
    REQUIRE(U->B[37]->path->Entrance != U->B[37]);

    RailTrain * tmpRT = new RailTrain(U->B[37]);
    U->B[37]->path->reserve(tmpRT);

    U->B[22]->setDetection(1);
    Algorithm::process(U->B[22], _FORCE);

    REQUIRE(U->B[22]->train);
    U->B[22]->train->setRoute(U->B[39]);
    auto route = U->B[22]->train->route;

    REQUIRE(route);
    U->B[22]->train->setSpeed(10);

    // Force SwitchSolver, since speed is too low
    SwitchSolver::solve(U->B[22]->train, U->B[22], U->B[24], U->B[24]->next, NEXT | SWITCH_CARE);

    CHECK(U->B[24]->switchWrongState);
    CHECK(!U->B[24]->reserved);
    CHECK(!U->B[26]->reserved);

    CHECK(U->B[37]->isReservedBy(U->B[22]->train) == false);

    
  }

  SECTION("XIII - Switchover"){
    U->Sw[11]->setState(1);
    U->Sw[12]->setState(1);

    Algorithm::BlockTick();

    auto T = new RailTrain(U->B[45]);
    U->B[45]->path->reserve(T);

    CHECK(U->B[42]->state == DANGER);

    U->B[45]->expectedTrain = T;
    U->B[45]->setDetection(1);
    Algorithm::process(U->B[45], _FORCE);
    
    // Force SwitchSolver, since speed is too low
    SwitchSolver::solve(T, U->B[45], U->B[47], U->B[47]->next, NEXT | SWITCH_CARE);

    CHECK(U->B[42]->dir == 0b101);
    CHECK(U->B[43]->dir == 0b101);
    CHECK(U->B[44]->dir == 0b101);
    CHECK(!U->B[43]->path->direction);

    CHECK(U->B[42]->isReservedBy(T));
    CHECK(U->B[43]->isReservedBy(T));

    CHECK(U->B[42]->state == RESERVED_SWITCH);
    CHECK(U->B[43]->state == RESERVED);
    CHECK(U->B[44]->state == RESERVED);

    CHECK(U->B[47]->state == RESERVED_SWITCH);
    CHECK(U->B[44]->state == RESERVED);
  }

  // TODO add testcases for MSSwitch
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

  
  U->link_all();
  
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

  B->train->link(0, RAILTRAIN_ENGINE_TYPE);
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
    CHECK(T->changing_speed == RAILTRAIN_SPEED_T_CHANGING);
    CHECK(ED->target_speed == CAUTION_SPEED);
    CHECK(ED->target_distance == 300);
    CHECK(ED->reason == RAILTRAIN_SPEED_R_SIGNAL);

    while(!U->B[6]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[6]->blocked);
    CHECK(T->changing_speed == RAILTRAIN_SPEED_T_UPDATE);

    while(T->speed_event_data->stepCounter > 1)
      train_testSim_tick(&train, &maxIterations);

    CHECK(T->speed < 180);
    CHECK(T->speed >= T->speed_event_data->startSpeed);
    CHECK(T->changing_speed == RAILTRAIN_SPEED_T_CHANGING);
    CHECK(ED->target_speed == CAUTION_SPEED);
    CHECK(ED->target_distance == 200);
    CHECK(ED->reason == RAILTRAIN_SPEED_R_SIGNAL);

    while(!U->B[7]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[7]->blocked);

    while(T->speed_event_data->stepCounter > 1)
      train_testSim_tick(&train, &maxIterations);

    CHECK(T->changing_speed == RAILTRAIN_SPEED_T_CHANGING);
    CHECK(T->speed < 180);
    CHECK(T->speed >= T->speed_event_data->startSpeed);
    CHECK(ED->target_speed == CAUTION_SPEED);
    CHECK(ED->target_distance == 100);
    CHECK(ED->reason == RAILTRAIN_SPEED_R_SIGNAL);

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
    CHECK(T->changing_speed == RAILTRAIN_SPEED_T_DONE);
    CHECK(ED->reason == RAILTRAIN_SPEED_R_NONE);

    while(!U->B[7]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[7]->blocked);
    CHECK(T->changing_speed == RAILTRAIN_SPEED_T_CHANGING);
    CHECK(ED->target_speed == 0);
    CHECK(ED->target_distance == 100);
    CHECK(ED->reason == RAILTRAIN_SPEED_R_SIGNAL);

    while(!U->B[8]->blocked && T->speed != 0 && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(!U->B[8]->blocked);
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
    CHECK(T->changing_speed == RAILTRAIN_SPEED_T_CHANGING);
    CHECK(ED->target_speed == CAUTION_SPEED);
    CHECK(ED->target_distance == 300);
    CHECK(ED->reason == RAILTRAIN_SPEED_R_SIGNAL);
    CHECK(T->speed == 180);

    while(T->speed_event_data->stepCounter != 4 && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->speed < 180);
    U->B[8]->state = PROCEED;
    
    while(T->speed_event_data->stepCounter != 5 && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->changing_speed == RAILTRAIN_SPEED_T_DONE);
    CHECK(ED->reason == RAILTRAIN_SPEED_R_NONE);

    while(!U->B[7]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[7]->blocked);
    CHECK(T->changing_speed == RAILTRAIN_SPEED_T_CHANGING);
    CHECK(ED->target_speed == CAUTION_SPEED);
    CHECK(ED->target_distance == 200);
    CHECK(ED->reason == RAILTRAIN_SPEED_R_SIGNAL);

    while(!U->B[8]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[8]->blocked);
    CHECK(T->changing_speed == RAILTRAIN_SPEED_T_UPDATE);
    CHECK(ED->target_speed == CAUTION_SPEED);
    CHECK(ED->target_distance == 100);
    CHECK(ED->reason == RAILTRAIN_SPEED_R_SIGNAL);

    while(!U->B[8]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[8]->blocked);
    CHECK(T->changing_speed == RAILTRAIN_SPEED_T_UPDATE);
    CHECK(ED->target_speed == CAUTION_SPEED);
    CHECK(ED->target_distance == 100);
    CHECK(ED->reason == RAILTRAIN_SPEED_R_SIGNAL);
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
    CHECK(T->changing_speed == RAILTRAIN_SPEED_T_CHANGING);
    CHECK(ED->target_speed == 100);
    CHECK(ED->target_distance == 300);
    CHECK(ED->reason == RAILTRAIN_SPEED_R_MAXSPEED);

    while(!U->B[8]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[8]->blocked);
    CHECK(T->changing_speed == RAILTRAIN_SPEED_T_UPDATE);
    CHECK(ED->target_speed == 100);
    CHECK(ED->target_distance == 200);
    CHECK(ED->reason == RAILTRAIN_SPEED_R_MAXSPEED);

    while(!U->B[9]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[9]->blocked);
    CHECK(T->changing_speed == RAILTRAIN_SPEED_T_UPDATE);
    CHECK(ED->target_speed == 100);
    CHECK(ED->target_distance == 100);
    CHECK(ED->reason == RAILTRAIN_SPEED_R_MAXSPEED);

    while(!U->B[12]->blocked && maxIterations > 0){
      Block * trainBlock = T->B;

      while(T->B == trainBlock && maxIterations > 0){
        train_testSim_tick(&train, &maxIterations);
      }

      CHECK(T->changing_speed != RAILTRAIN_SPEED_T_CHANGING);
      CHECK(T->speed == 100);
    }

    REQUIRE(U->B[12]->blocked);
    CHECK(T->changing_speed != RAILTRAIN_SPEED_T_CHANGING);
    CHECK(T->speed == 100);

    while(U->B[12]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(!U->B[12]->blocked);
    CHECK(T->changing_speed == RAILTRAIN_SPEED_T_CHANGING);
    CHECK(ED->target_speed > 100);
    CHECK(ED->target_distance == 100);
    CHECK(ED->reason == RAILTRAIN_SPEED_R_MAXSPEED);

    while(!U->B[14]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[14]->blocked);
    CHECK(T->changing_speed == RAILTRAIN_SPEED_T_CHANGING); // FIXME, train could allready be accelerating because it left B[13]
    CHECK(ED->target_speed > 100);
    CHECK(ED->target_speed <= 180);
    CHECK(ED->target_distance == 100);
    CHECK(ED->reason == RAILTRAIN_SPEED_R_MAXSPEED);
  }

  SECTION("V - End of route"){
    REQUIRE(U->St[0]);

    T->setRoute(U->St[0]);

    REQUIRE(T->route);
    REQUIRE(T->routeStatus == RAILTRAIN_ROUTE_RUNNING);
    REQUIRE(T->route->destination == U->St[0]->uid);

    while(!U->B[7]->blocked && maxIterations > 0){
      train_test_tick(&train, &maxIterations);
    }

    while(!U->B[10]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->routeStatus == RAILTRAIN_ROUTE_ENTERED_DESTINATION);

    while(!U->B[13]->blocked && T->speed > 90 && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->routeStatus == RAILTRAIN_ROUTE_AT_DESTINATION);
    CHECK(ED->reason == RAILTRAIN_SPEED_R_ROUTE);

    while(!U->B[13]->blocked && T->speed != 0 && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->speed == 0);
    CHECK(!U->B[13]->blocked);
    CHECK(U->B[12]->blocked);
    CHECK(!U->B[10]->blocked);
    CHECK(ED->reason == RAILTRAIN_SPEED_R_NONE);

  }

  SECTION("VI - At of waypoint"){
    T->setRoute(U->B[10]);

    REQUIRE(T->route);
    REQUIRE(T->routeStatus == RAILTRAIN_ROUTE_RUNNING);
    REQUIRE(T->route->destination == 0x010A); // Unit 1 Block 10

    while(!U->B[10]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->routeStatus == RAILTRAIN_ROUTE_AT_DESTINATION);

    while(!U->B[13]->blocked && T->speed != 0 && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->speed != 0);
    CHECK(U->B[13]->blocked);

  }

  scheduler->stop();
}

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

  B->train->link(0, RAILTRAIN_ENGINE_TYPE);
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
    while(!U[1]->B[5]->blocked && maxIterations){
      train_test_tick(&train, &maxIterations);
    }

    CHECK(!U[1]->B[3]->blocked);
    CHECK(!U[1]->B[4]->blocked);
    CHECK(U[1]->B[5]->blocked);

    maxIterations = 5000;

    while(!U[1]->B[3]->blocked && maxIterations){
      train_test_tick(&train, &maxIterations);
    }

    CHECK(U[1]->B[3]->blocked);
  }

  SECTION("II - A route"){
    logger.setlevel_stdout(TRACE);

    train.T->setRoute(U[1]->B[8]);

    while(!U[3]->B[2]->blocked && maxIterations > 0){
      train_test_tick(&train, &maxIterations);
    }

    CHECK(U[3]->B[2]->blocked);
    CHECK(maxIterations > 0);
    
    while(!U[1]->B[6]->blocked && maxIterations > 0){
      train_test_tick(&train, &maxIterations);
    }

    REQUIRE(maxIterations > 0);
    maxIterations = 100;

    // Train should stop not stop on a waypoint
    while(!U[1]->B[8]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(U[1]->B[8]->blocked);
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
