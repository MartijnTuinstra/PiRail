#include "catch.hpp"

#include "mem.h"
#include "logger.h"
#include "system.h"

#include "config/ModuleConfig.h"

#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/station.h"
#include "switchboard/unit.h"
#include "switchboard/blockconnector.h"

#include "rollingstock/railtrain.h"

#include "train.h"
#include "algorithm.h"

TEST_CASE( "Alg-1", "[Alg][Alg-1]"){
  if(Units){
    for(uint8_t u = 0; u < unit_len; u++){
      if(!Units[u])
        continue;

      delete Units[u];
      Units[u] = 0;
    }
    _free(Units);
  }

  Units = (Unit **)_calloc(30, Unit *);
  unit_len = 30;

  init_main();

  char filename[4][30] = {"./testconfigs/Alg-1-1.bin", "./testconfigs/Alg-1-2.bin", "./testconfigs/Alg-1-3.bin", "./testconfigs/Alg-1-4.bin"};
  ModuleConfig config[4] = {ModuleConfig(filename[0]), ModuleConfig(filename[1]), ModuleConfig(filename[2]), ModuleConfig(filename[3])};

  config[0].read();
  config[1].read();
  config[2].read();
  config[3].read();

  // REQUIRE(config[0].parsed);
  // REQUIRE(config[1].parsed);
  // REQUIRE(config[2].parsed);
  // REQUIRE(config[3].parsed);

  new Unit(&config[0]);
  new Unit(&config[1]);
  new Unit(&config[2]);
  new Unit(&config[3]);

  Units[1]->on_layout = true;
  Units[2]->on_layout = true;
  Units[3]->on_layout = true;
  Units[4]->on_layout = true;

  /*                              --\
  //  1.0->  | --2.0-> --2.1->  |  --3.0-> --3.1-> | --4.0->
  //     C1-1 C1-1          C2-1 C1-1          C2-1 C1-2
  //     C1-2 C1-2          C2-2 C1-2          C2-2 C1-1
  // <1.1--  | <-2.2-- <-2.3--  |  <-3.2-- <-3.3-- | <-4.1--
  //                        \--
  */

  auto connectors = Algorithm_find_connectors();

  loggerf(INFO, "Have %i connectors", connectors.size());

  uint8_t x = 0;
  bool modules_linked = false;

  SECTION("I - Find and connect"){

    while(modules_linked == false){
      if(uint8_t * findResult = Algorithm_find_connectable(&connectors)){
        Algorithm_connect_connectors(&connectors, findResult);
      }

      if(connectors.size() == 0)
        break;

      if(x == 1){
        Units[1]->B[0]->setDetection(1);
        Units[2]->B[0]->setDetection(1);
      }else if(x == 2){
        Units[1]->B[0]->setDetection(0);
        Units[2]->B[0]->setDetection(0);
        Units[2]->B[1]->setDetection(1);
        Units[3]->B[0]->setDetection(1);
      }else if(x == 3){
        Units[2]->B[1]->setDetection(0);
        Units[3]->B[0]->setDetection(0);
        Units[3]->B[1]->setDetection(1);
        Units[4]->B[0]->setDetection(1);
      }
      else if(x > 3){
        // _SYS_change(STATE_Modules_Coupled,1);
        modules_linked = true;
      }

      x++;
      //IF ALL JOINED
      //BREAK
    }
    
    Units[3]->B[1]->setDetection(0);
    Units[4]->B[0]->setDetection(0);

    link_all_blocks(Units[1]);
    link_all_blocks(Units[2]);
    link_all_blocks(Units[3]);
    link_all_blocks(Units[4]);

    REQUIRE(connectors.size() == 0);

    CHECK(Units[1]->B[0]->next.p.B == Units[2]->B[0]);
    CHECK(Units[2]->B[0]->prev.p.B == Units[1]->B[0]);

    CHECK(Units[3]->B[1]->next.p.B == Units[4]->B[0]);
    CHECK(Units[4]->B[0]->prev.p.B == Units[3]->B[1]);
  }

  SECTION("II - Connect Stored Configuration"){
    char filename[40] = "testconfigs/Alg-1-setup.bin";
    int ret = Algorithm_load_setup(filename, &connectors);

    REQUIRE(ret > 0);

    link_all_blocks(Units[1]);
    link_all_blocks(Units[2]);
    link_all_blocks(Units[3]);
    link_all_blocks(Units[4]);

    REQUIRE(connectors.size() == 0);

    CHECK(Units[1]->B[0]->next.p.B == Units[2]->B[0]);
    CHECK(Units[2]->B[0]->prev.p.B == Units[1]->B[0]);

    CHECK(Units[3]->B[1]->next.p.B == Units[4]->B[0]);
    CHECK(Units[4]->B[0]->prev.p.B == Units[3]->B[1]);
  }
}

TEST_CASE( "Alg-2", "[Alg][Alg-2]"){
  // Train Following

  if(Units){
    for(uint8_t u = 0; u < unit_len; u++){
      if(!Units[u])
        continue;

      delete Units[u];
      Units[u] = 0;
    }
    _free(Units);
  }

  Units = (Unit **)_calloc(30, Unit *);
  unit_len = 30;

  init_main();

  char filename[30] = "./testconfigs/Alg-2.bin";
  ModuleConfig config = ModuleConfig(filename);
  load_rolling_Configs("./testconfigs/stock.bin");

  config.read();

  REQUIRE(config.parsed);

  new Unit(&config);

  Units[1]->on_layout = true;
  Unit * U = Units[1];

  link_all_blocks(U);

  /*
  //  1.0> 1.1> 1.2> 1.3> 1.4> 1.5> 1.6> 1.7>
  */

  SECTION("I - No linked train"){
    CHECK(U->B[0]->train == 0);

    U->B[0]->setDetection(1);
    Algor_process(U->B[0], _FORCE);

    CHECK(U->B[0]->train != 0);

    RailTrain * T = U->B[0]->train;

    // Step Forward
    U->B[1]->setDetection(1);
    Algor_process(U->B[1], _FORCE);

    CHECK(U->B[1]->train == T);
    CHECK(U->B[1] == T->B);

    CHECK(T->blocks.size() == 2);

    // Release last block
    U->B[0]->setDetection(0);
    Algor_process(U->B[0], _FORCE);

    CHECK(U->B[0]->train == 0);
    CHECK(T->blocks.size() == 1);
  }

  SECTION("II - Simple Engine Forward"){
    CHECK(U->B[0]->train == 0);

    U->B[0]->setDetection(1);
    Algor_process(U->B[0], _FORCE);

    REQUIRE(U->B[0]->train != 0);
    RailTrain * T = U->B[0]->train;

    // Step Forward
    U->B[1]->setDetection(1);
    Algor_process(U->B[1], _FORCE);

    CHECK(U->B[1]->train == T);
    CHECK(U->B[1] == T->B);

    // Link engine
    T->link(0, RAILTRAIN_ENGINE_TYPE);

    for(auto b: T->blocks){
      CHECK((b == U->B[0] || b == U->B[1]));
    }

    U->B[0]->setDetection(0);
    Algor_process(U->B[0], _FORCE);

    CHECK(U->B[0]->train == 0);
  }

  SECTION("III - Full Detectable Train"){
    CHECK(U->B[0]->train == 0);

    U->B[0]->setDetection(1);
    Algor_process(U->B[0], _FORCE);

    REQUIRE(U->B[0]->train != 0);
    RailTrain * T = U->B[0]->train;

    // Step Forward
    U->B[1]->setDetection(1);
    Algor_process(U->B[1], _FORCE);

    CHECK(U->B[1]->train == T);
    CHECK(U->B[1] == T->B);

    // Step Forward
    U->B[2]->setDetection(1);
    Algor_process(U->B[2], _FORCE);

    CHECK(U->B[2]->train == T);

    // Step Forward
    U->B[3]->setDetection(1);
    Algor_process(U->B[3], _FORCE);

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
    Algor_process(U->B[0], _FORCE);
    Algor_process(U->B[4], _FORCE);

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
    Algor_process(U->B[3], _FORCE);

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
    Algor_process(U->B[4], _FORCE);
    U->B[3]->setDetection(0);
    Algor_process(U->B[3], _FORCE);
    processAlgorQueue();

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
    Algor_process(U->B[5], _FORCE);
    U->B[4]->setDetection(0);
    Algor_process(U->B[4], _FORCE);
    processAlgorQueue();

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
    Algor_process(U->B[3], _FORCE);

    REQUIRE(U->B[3]->train != 0);
    RailTrain * T = U->B[3]->train;

    SECTION("I - Second detectable after linking"){
      loggerf(ERROR, "I-");
      // Link engine
      T->link(2, RAILTRAIN_TRAIN_TYPE);

      U->B[1]->setDetection(1);
      Algor_process(U->B[1], _FORCE);
    }
    SECTION("II - Second detectable before linking"){
      loggerf(ERROR, "II-");
      U->B[1]->setDetection(1);
      Algor_process(U->B[1], _FORCE);

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
    Algor_process(U->B[4], _FORCE);
    U->B[3]->setDetection(0);
    Algor_process(U->B[3], _FORCE);
    U->B[2]->setDetection(1);
    Algor_process(U->B[2], _FORCE);
    U->B[1]->setDetection(0);
    Algor_process(U->B[1], _FORCE);
    processAlgorQueue();

    // Train is 143cm long but moving, so an extra block is blocked.
    CHECK(T->blocks.size() == 4);
    CHECK(U->B[4] == T->B);
    for(auto b: T->blocks){
      CHECK(((b != U->B[5] && b != U->B[0]) && (b == U->B[1] || b == U->B[2] || b == U->B[3] || b == U->B[4])));
      CHECK(b->train == T);
    }

    // Step Forward
    U->B[5]->setDetection(1);
    Algor_process(U->B[5], _FORCE);
    U->B[4]->setDetection(0);
    Algor_process(U->B[4], _FORCE);
    U->B[3]->setDetection(1);
    Algor_process(U->B[3], _FORCE);
    U->B[2]->setDetection(0);
    Algor_process(U->B[2], _FORCE);
    processAlgorQueue();

    // Train is 143cm long but moving, so an extra block is blocked.
    CHECK(T->blocks.size() == 4);
    CHECK(U->B[5] == T->B);
    for(auto b: T->blocks){
      CHECK(((b != U->B[6] && b != U->B[1]) && (b == U->B[2] || b == U->B[3] || b == U->B[4] || b == U->B[5])));
      CHECK(b->train == T);
    }

  }
}