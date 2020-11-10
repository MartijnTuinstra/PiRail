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

#include "modules.h"
#include "train.h"

#include "algorithm/queue.h"
#include "algorithm/core.h"
#include "algorithm/component.h"
#include "algorithm/blockconnector.h"

#include "pathfinding.h"

void init_test(char (* filenames)[30], int nr_files);

TEST_CASE( "Connector Algorithm", "[Alg][Alg-1]"){
  char filenames[4][30] = {"./testconfigs/Alg-1-1.bin", "./testconfigs/Alg-1-2.bin", "./testconfigs/Alg-1-3.bin", "./testconfigs/Alg-1-4.bin"};
  init_test(filenames, 4);

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
  }

  SECTION("II - Connect Stored Configuration"){
    char filename[40] = "testconfigs/Alg-1-setup.bin";
    int ret = Algorithm::load_setup(filename, &connectors);

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

TEST_CASE( "Train Following", "[Alg][Alg-2]"){
  char filenames[1][30] = {"./testconfigs/Alg-2.bin"};
  init_test(filenames, 1);

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;
  U->link_all();

  /*
  //  1.0> 1.1> 1.2> 1.3> 1.4> 1.5> 1.6> 1.7>
  */

  SECTION("I - No linked train"){
    CHECK(U->B[0]->train == 0);

    U->B[0]->setDetection(1);
    Algorithm::process(U->B[0], _FORCE);

    CHECK(U->B[0]->train != 0);

    RailTrain * T = U->B[0]->train;

    // Step Forward
    U->B[1]->setDetection(1);
    Algorithm::process(U->B[1], _FORCE);

    CHECK(U->B[1]->train == T);
    CHECK(U->B[1] == T->B);

    CHECK(T->blocks.size() == 2);

    // Release last block
    U->B[0]->setDetection(0);
    Algorithm::process(U->B[0], _FORCE);

    CHECK(U->B[0]->train == 0);
    CHECK(T->blocks.size() == 1);
  }

  SECTION("II - Simple Engine Forward"){
    CHECK(U->B[0]->train == 0);

    U->B[0]->setDetection(1);
    Algorithm::process(U->B[0], _FORCE);

    REQUIRE(U->B[0]->train != 0);
    RailTrain * T = U->B[0]->train;

    // Step Forward
    U->B[1]->setDetection(1);
    Algorithm::process(U->B[1], _FORCE);

    CHECK(U->B[1]->train == T);
    CHECK(U->B[1] == T->B);

    // Link engine
    T->link(0, RAILTRAIN_ENGINE_TYPE);

    for(auto b: T->blocks){
      CHECK((b == U->B[0] || b == U->B[1]));
    }

    U->B[0]->setDetection(0);
    Algorithm::process(U->B[0], _FORCE);

    CHECK(U->B[0]->train == 0);
  }

  SECTION("III - Full Detectable Train"){
    CHECK(U->B[0]->train == 0);

    U->B[0]->setDetection(1);
    Algorithm::process(U->B[0], _FORCE);

    REQUIRE(U->B[0]->train != 0);
    RailTrain * T = U->B[0]->train;

    // Step Forward
    U->B[1]->setDetection(1);
    Algorithm::process(U->B[1], _FORCE);

    CHECK(U->B[1]->train == T);
    CHECK(U->B[1] == T->B);

    // Step Forward
    U->B[2]->setDetection(1);
    Algorithm::process(U->B[2], _FORCE);

    CHECK(U->B[2]->train == T);

    // Step Forward
    U->B[3]->setDetection(1);
    Algorithm::process(U->B[3], _FORCE);

    CHECK(U->B[3]->train == T);

    // Link engine
    T->link(0, RAILTRAIN_TRAIN_TYPE);

    CHECK(T->blocks.size() == 4);
    for(auto b: T->blocks){
      CHECK((b == U->B[0] || b == U->B[1] || b == U->B[2] || b == U->B[3]));
    }

    // Step Forward
    U->B[0]->setDetection(0);
    U->B[4]->setDetection(1);
    Algorithm::process(U->B[0], _FORCE);
    Algorithm::process(U->B[4], _FORCE);

    CHECK(U->B[0]->train == 0);
    CHECK(U->B[4]->train == T);
    CHECK(U->B[4] == T->B);
    CHECK(T->blocks.size() == 4);
    for(auto b: T->blocks){
      CHECK((b == U->B[1] || b == U->B[2] || b == U->B[3] || b == U->B[4]));
    }
  }

  SECTION("IV - Partial Detectable Train"){
    CHECK(U->B[3]->train == 0);

    U->B[3]->setDetection(1);
    Algorithm::process(U->B[3], _FORCE);

    REQUIRE(U->B[3]->train != 0);
    RailTrain * T = U->B[3]->train;

    // Link engine
    T->link(1, RAILTRAIN_TRAIN_TYPE);

    // Train is 58cm long so two blocks should be blocked.
    CHECK(T->blocks.size() == 2);
    for(auto b: T->blocks){
      loggerf(ERROR, "Have block %2i:%2i", b->module, b->id);
      CHECK(((b != U->B[4] && b != U->B[1]) && (b == U->B[2] || b == U->B[3])));
    }

    // Step Forward
    U->B[4]->setDetection(1);
    Algorithm::process(U->B[4], _FORCE);
    U->B[3]->setDetection(0);
    Algorithm::process(U->B[3], _FORCE);

    // Train is 58cm long but moving, so an extra block is blocked.
    CHECK(T->blocks.size() == 3);
    CHECK(U->B[4] == T->B);
    for(auto b: T->blocks){
      loggerf(ERROR, "Have block %2i:%2i", b->module, b->id);
      CHECK(((b != U->B[5] && b != U->B[1]) && (b == U->B[2] || b == U->B[3] || b == U->B[4])));
      CHECK(b->train == T);
    }

    // Step Forward
    U->B[5]->setDetection(1);
    Algorithm::process(U->B[5], _FORCE);
    U->B[4]->setDetection(0);
    Algorithm::process(U->B[4], _FORCE);

    // Train is 58cm long but moving, so an extra block is blocked.
    CHECK(T->blocks.size() == 3);
    CHECK(U->B[5] == T->B);
    for(auto b: T->blocks){
      loggerf(ERROR, "Have block %2i:%2i", b->module, b->id);
      CHECK(((b != U->B[6] && b != U->B[2]) && (b == U->B[3] || b == U->B[4] || b == U->B[5])));
      CHECK(b->train == T);
    }
  }

  SECTION("V - Train with split detectables"){
    CHECK(U->B[3]->train == 0);

    U->B[3]->setDetection(1);
    Algorithm::process(U->B[3], _FORCE);

    REQUIRE(U->B[3]->train != 0);
    RailTrain * T = U->B[3]->train;

    SECTION("I - Second detectable after linking"){
      loggerf(ERROR, "I-");
      // Link engine
      T->link(2, RAILTRAIN_TRAIN_TYPE);

      U->B[1]->setDetection(1);
      Algorithm::process(U->B[1], _FORCE);
    }
    SECTION("II - Second detectable before linking"){
      loggerf(ERROR, "II-");
      U->B[1]->setDetection(1);
      Algorithm::process(U->B[1], _FORCE);

      CHECK(U->B[1]->train != T);

      // Link engine
      T->link(2, RAILTRAIN_TRAIN_TYPE);

      CHECK(U->B[1]->train == T);
    }

    // Train is 143cm long so three blocks should be blocked.
    CHECK(T->blocks.size() == 3);
    for(auto b: T->blocks){
      CHECK(((b != U->B[4] && b != U->B[0]) && (b == U->B[1] || b == U->B[2] || b == U->B[3])));
      CHECK(b->train == T);
    }

    // Step Forward
    U->B[4]->setDetection(1);
    Algorithm::process(U->B[4], _FORCE);
    U->B[3]->setDetection(0);
    Algorithm::process(U->B[3], _FORCE);
    U->B[2]->setDetection(1);
    Algorithm::process(U->B[2], _FORCE);
    U->B[1]->setDetection(0);
    Algorithm::process(U->B[1], _FORCE);

    // Train is 143cm long but moving, so an extra block is blocked.
    CHECK(T->blocks.size() == 4);
    CHECK(U->B[4] == T->B);
    for(auto b: T->blocks){
      CHECK(((b != U->B[5] && b != U->B[0]) && (b == U->B[1] || b == U->B[2] || b == U->B[3] || b == U->B[4])));
      CHECK(b->train == T);
    }

    // Step Forward
    U->B[5]->setDetection(1);
    Algorithm::process(U->B[5], _FORCE);
    U->B[4]->setDetection(0);
    Algorithm::process(U->B[4], _FORCE);
    U->B[3]->setDetection(1);
    Algorithm::process(U->B[3], _FORCE);
    U->B[2]->setDetection(0);
    Algorithm::process(U->B[2], _FORCE);

    // Train is 143cm long but moving, so an extra block is blocked.
    CHECK(T->blocks.size() == 4);
    CHECK(U->B[5] == T->B);
    for(auto b: T->blocks){
      CHECK(((b != U->B[6] && b != U->B[1]) && (b == U->B[2] || b == U->B[3] || b == U->B[4] || b == U->B[5])));
      CHECK(b->train == T);
    }
  }

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
}

TEST_CASE( "Algorithm Switch Setter", "[Alg][Alg-3]"){
  char filenames[1][30] = {"./testconfigs/Alg-3.bin"};
  init_test(filenames, 1);

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;
  U->link_all();

  // logger.setlevel_stdout(DEBUG);

  /*           Sw0/--->
  // 1.0> 1.1> 1.2----> 1.3>
  //
  //           ---\Sw1
  // 1.4> 1.5> ----1.6> 1.7>
  //
  //                       |---St0---|
  //              Sw2/---> 1.11> 1.12>
  // 1.08> 1.09> 1.10----> 1.13> 1.14>
  //                       |---St1---|
  //
  //                             |---St2---|
  //                    Sw5/---> 1.20> 1.21> ---\Sw6
  // 1.15> 1.16> ------1.17----> 1.18> 1.19> ----1.27> 1.28-> ----\
  //                  /Sw4       |---St3---|            Sw7\       \
  //              Sw3/           |---St4---|                \       \Sw8
  // 1.22> 1.23> 1.24----------> 1.25> 1.26> ----1.29> ------1.30> ---1.31> 1.33>
  //                                                       MSSw0  \
  //                                                               \-> 1.32>
  */

  for(uint8_t i = 0; i < 9; i++){
    Algorithm::process(U->B[i], _FORCE);
  }

  U->Sw[0]->setState(0);
  U->Sw[1]->setState(0);
  U->Sw[2]->setState(0);
  U->Sw[3]->setState(0);
  U->Sw[4]->setState(0);
  U->Sw[5]->setState(0);
  if(AlQueue.queue->getItems())
    Algorithm::tick();

  SECTION("I - Approaching S side"){
    U->B[0]->setDetection(1);
    Algorithm::process(U->B[0], _FORCE);

    CHECK(U->B[0]->train != nullptr);
    CHECK(U->B[0]->train == RSManager->getRailTrain(0));

    RSManager->getRailTrain(0)->setSpeed(10);
    Algorithm::process(U->B[0], _FORCE);

    CHECK(U->Sw[0]->state == 0);
    CHECK(U->Sw[0]->Detection->switchReserved);
    CHECK(U->Sw[0]->Detection->reservedBy == U->B[0]->train);

    U->B[0]->train->dereserveBlock(U->Sw[0]->Detection);
    U->Sw[0]->Detection->state = PROCEED;

    U->Sw[0]->setState(1); // Set Diverging
    CHECK(U->Sw[0]->state == 1);

    Algorithm::tick();


    Algorithm::process(U->B[0], _FORCE);

    CHECK(U->Sw[0]->state == 0);
    CHECK(U->Sw[0]->Detection->switchReserved);
    CHECK(U->Sw[0]->Detection->reservedBy == U->B[0]->train);
  }

  SECTION("II - Approaching s side"){
    U->B[4]->setDetection(1);
    Algorithm::process(U->B[4], _FORCE);

    CHECK(U->B[4]->train != nullptr);
    CHECK(U->B[4]->train == RSManager->getRailTrain(0));

    RSManager->getRailTrain(0)->setSpeed(10);
    Algorithm::process(U->B[4], _FORCE);

    CHECK(U->Sw[1]->state == 0);
    CHECK(U->Sw[1]->Detection->reservedBy == U->B[4]->train);

    U->B[4]->train->dereserveBlock(U->Sw[1]->Detection);

    U->Sw[1]->setState(1); // Set Diverging
    CHECK(U->Sw[1]->state == 1);

    Algorithm::tick();

    // Algorithm::process(U->B[4], _FORCE);

    CHECK(U->Sw[1]->state == 0);
    CHECK(U->Sw[1]->Detection->switchReserved);
    CHECK(U->Sw[1]->Detection->reservedBy == U->B[4]->train);
  }

  SECTION("III - Approaching S side with station"){
    U->B[8]->setDetection(1);
    U->B[14]->setDetection(1);
    Algorithm::process(U->B[8], _FORCE);
    Algorithm::process(U->B[14], _FORCE);

    CHECK(U->B[8]->train != nullptr);
    CHECK(U->B[8]->train == RSManager->getRailTrain(0));
    CHECK(U->B[14]->train == RSManager->getRailTrain(1));

    RSManager->getRailTrain(0)->setSpeed(10);
    Algorithm::process(U->B[8], _FORCE);

    CHECK(U->Sw[2]->state == 1);
    CHECK(U->Sw[2]->Detection->switchReserved);
    CHECK(U->Sw[2]->Detection->reservedBy == U->B[8]->train);
  }

  SECTION("IV - Approaching S side with station fully blocked"){
    U->B[8]->setDetection(1);
    U->B[12]->setDetection(1);
    U->B[14]->setDetection(1);
    Algorithm::process(U->B[8], _FORCE);
    Algorithm::process(U->B[12], _FORCE);
    Algorithm::process(U->B[14], _FORCE);

    CHECK(U->B[8]->train != nullptr);
    CHECK(U->B[8]->train == RSManager->getRailTrain(0));

    RSManager->getRailTrain(0)->setSpeed(10);
    Algorithm::process(U->B[8], _FORCE);

    CHECK(U->Sw[2]->state == 0);
    CHECK(U->Sw[2]->Detection->state == DANGER);

    CHECK(!U->Sw[2]->Detection->switchReserved);
    CHECK(U->Sw[2]->Detection->switchWrongState);
  }

  SECTION("V - Approaching s side with station and switchblock"){
    U->B[15]->setDetection(1);
    U->B[19]->setDetection(1);
    Algorithm::process(U->B[15], _FORCE);
    Algorithm::process(U->B[19], _FORCE);

    U->Sw[4]->setState(1);
    Algorithm::tick();

    // Algorithm::process(U->B[15], _FORCE);
    // U->St[3]->train -> SIGSEV
    CHECK(U->B[19]->station == U->St[3]);
    CHECK(U->B[19]->train == U->St[3]->train);
    U->B[19]->train->setSpeed(0);

    CHECK(U->B[15]->train != nullptr);
    CHECK(U->B[15]->train == RSManager->getRailTrain(0));

    RSManager->getRailTrain(0)->setSpeed(10);
    Algorithm::process(U->B[15], _FORCE);


    CHECK(U->Sw[4]->state == 0);
    CHECK(U->Sw[5]->state == 1);

    CHECK(U->Sw[4]->Detection->switchReserved);
    CHECK(U->Sw[4]->Detection->reservedBy == U->B[15]->train);
  }

  SECTION("VI- Approaching S side with full station and switchblock"){
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
    Algorithm::tick();

    CHECK(U->Sw[4]->state == 1);
    CHECK(U->Sw[5]->state == 0);

    CHECK(U->B[15]->train != nullptr);
    CHECK(U->B[15]->train == RSManager->getRailTrain(0));

    RSManager->getRailTrain(0)->setSpeed(10);
    Algorithm::process(U->B[15], _FORCE);

    CHECK(U->Sw[4]->state == 1);
    CHECK(U->Sw[5]->state == 0);

    CHECK(!U->Sw[3]->Detection->switchReserved);
  }

  SECTION("VII - Approaching switch with route"){
    U->B[22]->setDetection(1);
    Algorithm::process(U->B[22], _FORCE);

    auto route = PathFinding::find(U->B[22], U->B[21]);

    REQUIRE(route->found_forward);
    REQUIRE(U->B[22]->train);

    U->B[22]->train->route = route;
    U->B[22]->train->onroute = true;

    CHECK(U->B[22]->train != nullptr);
    CHECK(U->B[22]->train == RSManager->getRailTrain(0));

    RSManager->getRailTrain(0)->setSpeed(10);
    Algorithm::process(U->B[22], _FORCE);

    CHECK(U->Sw[3]->state == 1);
    CHECK(U->Sw[4]->state == 1);
    CHECK(U->Sw[5]->state == 1);
  }

  SECTION("VII - Approaching switch with route with blocked station"){
    U->B[22]->setDetection(1);
    U->B[21]->setDetection(1);
    Algorithm::process(U->B[22], _FORCE);
    Algorithm::process(U->B[21], _FORCE);

    auto route = PathFinding::find(U->B[22], U->B[21]);

    REQUIRE(route->found_forward);
    REQUIRE(U->B[22]->train);

    U->B[22]->train->route = route;
    U->B[22]->train->onroute = true;

    CHECK(U->B[22]->train == RSManager->getRailTrain(0));

    RSManager->getRailTrain(0)->setSpeed(10);
    Algorithm::process(U->B[22], _FORCE);

    CHECK(U->Sw[3]->state == 0);
    CHECK(U->Sw[4]->state == 0);
    CHECK(U->Sw[5]->state == 0);

    CHECK(U->B[24]->switchWrongState);
  }

  SECTION("VIII - Approaching switch with route applying detour"){
    loggerf(WARNING, "TEST VIII");
    U->B[22]->setDetection(1);
    U->B[26]->setDetection(1);
    Algorithm::process(U->B[22], _FORCE);
    Algorithm::process(U->B[26], _FORCE);

    REQUIRE(U->B[22]->train);
    CHECK(U->B[22]->train == RSManager->getRailTrain(0));
    REQUIRE(U->B[26]->train);
    CHECK(U->B[26]->train == RSManager->getRailTrain(1));

    auto route = PathFinding::find(U->B[22], U->B[33]);

    char debug[1000];
    route->print(debug);
    loggerf(INFO, "%s", debug);

    REQUIRE(route->found_forward);

    U->B[22]->train->route = route;
    U->B[22]->train->onroute = true;

    RSManager->getRailTrain(0)->setSpeed(10);
    Algorithm::process(U->B[22], _FORCE);

    CHECK(U->Sw[3]->state == 1);
    CHECK(U->Sw[4]->state == 1);
    CHECK(U->Sw[5]->state == 0);
    CHECK(U->Sw[5]->updatedState == 0); // Switch should not be updated since it is allready in position

    CHECK(U->B[24]->reservedBy == U->B[22]->train);
  }

  SECTION("IX - Approaching S with reserved switches"){
    logger.setlevel_stdout(INFO);
    loggerf(WARNING, "TEST IX");
    U->B[17]->reservedBy = (RailTrain *)1;
    U->B[17]->switchReserved = 1;
    U->B[17]->setState(RESERVED_SWITCH);

    U->Sw[3]->setState(1);
    Algorithm::tick();

    U->B[22]->setDetection(1);
    Algorithm::process(U->B[22], _FORCE);

    REQUIRE(U->B[22]->train);
    U->B[22]->train->setSpeed(10);
    Algorithm::process(U->B[22], _FORCE);

    CHECK(U->Sw[3]->state == 0);
    CHECK(U->Sw[4]->state == 0);
    CHECK(U->Sw[5]->state == 0);

    CHECK(U->B[24]->reservedBy == U->B[22]->train);
  }
}

TEST_CASE("Algor Queue", "[Alg][Alg-Q]"){
  char filenames[1][30] = {"./testconfigs/Alg-3.bin"};
  init_test(filenames, 1);
  logger.setlevel_stdout(CRITICAL);

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