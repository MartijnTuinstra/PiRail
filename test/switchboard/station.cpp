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

#include "algorithm.h"
#include "train.h"
#include "modules.h"

#include "rollingstock/railtrain.h"

TEST_CASE( "Station Stating", "[SB-5.1]" ) {
  init_main();

  unload_module_Configs();
  unload_rolling_Configs();
  clearAlgorithmQueue();

  Units = (Unit **)_calloc(30, Unit *);
  unit_len = 30;

  char filename[30] = "./testconfigs/SB-1.3.bin";
  auto config = ModuleConfig(filename);
  config.read();

  REQUIRE(config.parsed);
  pathlist.clear();

  new Unit(&config);
  Unit * U = Units[1];

  U->link_all();

  for(uint8_t i = 0; i < U->block_len; i++){
    U->B[i]->AlgorSearch(0);
  }

  train_link = (RailTrain **)_calloc(20, sizeof(RailTrain *));
  train_link_len = 20;

  /*
  // SECTION I
  //  --1.0->  --1.1->  --1.2->  --1.3->  --1.4->  --1.5->
  //
  // SECTION II
  //  --1.6->  --1.7->  --1.8->  1.9>  --1.10->  --1.11->
  //
  // SECTION III
  //  <-1.12- <-1.13- -1.14-> -1.15-> 
  //
  // SECTION IV
  //  <-1.43- <-1.44- <-1.45-> -1.46-> -1.47-> 
  //
  // SECTION V
  //                    /Sw1:0
  //                   /
  //  1.16->  --1.17-> ---- --1.18-> --1.19-> --1.48->
  //                     \- |end
  //
  // SECTION VI
  //              /MSSw1:0
  //  1.20->  -\  /
  //  1.21->  --1.22-> --1.23->
  //                \- --1.24->
  //
  // SECTION VII
  //  1.25-> -1.26-> -1.27-> -1.28-> -1.29->
  //         [       Station1      ]
  //
  // SECTION VIII
  //  1.49-> -1.50-> -1.51-> -1.52-> -1.53->
  //         [        Yard4        ]
  //
  // SECTION IX
  //  1.30-> -1.31-> -1.32-> -1.33-> -1.34-> -1.35->
  //         [   Station2A ] [   Station2B ]
  //         [           Station2          ]
  //
  // SECTION X
  //                           Sw1:1\    /--End
  //                                 \  /
  //  1.36-> -1.37-> -1.38-> -1.39-> ---- -1.40-> -1.41-> -1.42->
  //         [   Station3A ]              [   Station3B ]
  //         [                 Station3                 ]
  */

  SECTION("VII - Blocks and station"){
    loggerf(CRITICAL, "Station has parent %x", (unsigned int)U->St[0]->parent);
    U->B[28]->setDetection(1);

    Algor_process(U->B[28], _FORCE);
    Algor_print_block_debug(U->B[28]);

    U->B[28]->train->setSpeed(0);

    CHECK(U->St[0]->occupied);
    CHECK(U->St[0]->stoppedTrain);
    CHECK(U->St[0]->train == U->B[28]->train);

    U->B[28]->train->setSpeed(10);

    CHECK(!U->St[0]->stoppedTrain);

    // Move train out of station
    U->B[29]->setDetection(1);
    Algor_process(U->B[29], _FORCE);

    U->B[28]->setDetection(0);
    Algor_process(U->B[28], _FORCE);

    CHECK(!U->B[28]->blocked);
    CHECK(!U->St[0]->occupied);
    CHECK(!U->St[0]->train);
  }

  SECTION("VIII - Blocks and yard"){
    U->B[52]->setDetection(1);

    Algor_process(U->B[52], _FORCE);
    Algor_print_block_debug(U->B[52]);

    CHECK(U->St[7]->occupied);
  }

  SECTION("IX - Blocks and multistation"){
    U->B[32]->setDetection(1);

    Algor_process(U->B[32], _FORCE);
    Algor_print_block_debug(U->B[32]);

    U->B[32]->train->setSpeed(0);

    CHECK(U->St[2]->occupied);
    CHECK(U->St[2]->stoppedTrain);
    CHECK(!U->St[3]->occupied);
    CHECK(!U->St[3]->stoppedTrain);

    CHECK(!U->St[1]->occupied);
    CHECK(U->St[1]->stoppedTrain);
    CHECK(U->St[1]->occupiedChild);

    U->B[32]->train->setSpeed(10);

    CHECK(!U->St[1]->stoppedTrain);
    CHECK(!U->St[2]->stoppedTrain);

    // Move train to other station
    U->B[33]->setDetection(1);
    Algor_process(U->B[33], _FORCE);

    CHECK(U->St[2]->occupied);
    CHECK(U->St[3]->occupied);

    U->B[32]->setDetection(0);
    Algor_process(U->B[32], _FORCE);

    CHECK(!U->St[2]->occupied);
    CHECK(U->St[3]->occupied);
    CHECK(!U->St[1]->occupied);
    CHECK(U->St[1]->occupiedChild);

    U->B[34]->setDetection(1);
    Algor_process(U->B[34], _FORCE);
    U->B[33]->setDetection(0);
    Algor_process(U->B[33], _FORCE);

    // Move train out of station
    U->B[35]->setDetection(1);
    Algor_process(U->B[35], _FORCE);

    U->B[34]->setDetection(0);
    Algor_process(U->B[34], _FORCE);

    CHECK(!U->B[34]->blocked);
    CHECK(!U->St[3]->occupied);
    CHECK(!U->St[1]->occupiedChild);
    CHECK(!U->St[3]->train);
  }
}