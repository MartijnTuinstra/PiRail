#include "catch.hpp"

#include "mem.h"
#include "logger.h"

#include "config/ModuleConfig.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/station.h"
#include "switchboard/unit.h"

#include "rollingstock/railtrain.h"

#include "path.h"

TEST_CASE( "Path Construction", "[PATH][PATH-1]" ) {
  set_level(NONE);
  set_logger_print_level(NONE);

  Units = (Unit **)_calloc(30, Unit *);
  unit_len = 30;

  char filename[30] = "./testconfigs/PATH-1.bin";
  auto config = ModuleConfig(filename);
  config.read();

  REQUIRE(config.parsed);
  pathlist.clear();

  new Unit(&config);
  Unit * U = Units[1];

  U->link_all();

  pathlist_find();

  /*
  //  1.0->  --1.1->  --1.2->
  //
  //                    /Sw1:0
  //                   /
  //  1.3->  --1.4-> ---- --1.5->
  //                   \- --1.6->
  //
  //              /MSSw1:0
  //  1.7->  -\  /
  //  1.8->  --1.9-> --1.10->
  //              \- --1.11->
  //
  //
  //  1.12-> -1.13-> -1.14-> -1.15-> -1.16->
  //         [       Station       ]
  //
  //  1.17-> -1.18-> <-1.19- <-1.20-
  //
  //  1.21-> <-1.22-> <-1.23-
  //
  //
  //  1.24-> -1.25-> -1.26-> -1.27-> -1.28-> -1.29->
  //         [   Station   ] [   Station   ]
  //         [           Station           ]
  //
  //                           Sw1:1\    /--End
  //                                 \  /
  //  1.30-> -1.31-> -1.32-> -1.33-> ---- -1.34-> -1.35-> -1.36->
  //         [   Station   ]              [   Station   ]
  //         [                 Station                  ]
  */
  Path * P[10] = {0};

  SECTION("Same direction blocks"){
    P[0] = U->B[0]->path;
    CHECK(P[0] == U->B[1]->path);
    CHECK(P[0] == U->B[2]->path);
  }

  SECTION("Blocks arround switch"){
    // Blocks surrounding the switch do not have the same path
    P[0] = U->B[3]->path;
    P[1] = U->B[4]->path;
    P[2] = U->B[5]->path;
    P[3] = U->B[6]->path;
    for(int i = 0; i < 4; i++){
      for(int j = 0; j < 4; j++){
        if(i == j)
          continue;

        CHECK(P[i] != P[j]);
      }
    }
  }

  SECTION("Blocks arround msswitch crossing"){
    // Blocks surrounding the crossing do not have the same path
    P[0] = U->B[7]->path;
    P[1] = U->B[8]->path;
    P[2] = U->B[9]->path;
    P[3] = U->B[10]->path;
    P[4] = U->B[11]->path;
    for(int i = 0; i < 5; i++){
      for(int j = 0; j < 5; j++){
        if(i == j)
          continue;

        CHECK(P[i] != P[j]);
      }
    }
  }


  SECTION("Paths and stations"){
    P[0] = U->B[12]->path;
    P[1] = U->B[13]->path;
    P[2] = U->B[14]->path;
    P[3] = U->B[15]->path;
    P[4] = U->B[16]->path;

    // Blocks surrounding the station have a different path
    CHECK(P[0] != P[1]);
    CHECK(P[0] != P[4]);
    CHECK(P[3] != P[4]);

    // Blocks in the station have the same path
    CHECK(P[1] != nullptr);
    CHECK(P[1] == P[2]);
    CHECK(P[2] == P[3]);
  }


  SECTION("Blocks with sudden direction change"){
    P[0] = U->B[17]->path;
    P[1] = U->B[18]->path;
    P[2] = U->B[19]->path;
    P[3] = U->B[20]->path;
    for(int i = 0; i < 4; i++){
      for(int j = 0; j < 4; j++){
        if(i == j)
          continue;

        CHECK(P[i] == P[j]);
      }
    }
  }

  SECTION("Blocks with direction change"){
    P[0] = U->B[21]->path;
    P[1] = U->B[22]->path;
    P[2] = U->B[23]->path;
    for(int i = 0; i < 3; i++){
      for(int j = 0; j < 3; j++){
        if(i == j)
          continue;

        CHECK(P[i] == P[j]);
      }
    }
  }

  SECTION("Station with substations"){
    P[0] = U->B[24]->path;
    P[1] = U->B[25]->path;
    P[2] = U->B[26]->path;
    P[3] = U->B[27]->path;
    P[4] = U->B[28]->path;
    P[5] = U->B[29]->path;

    CHECK(P[0] != P[1]);
    CHECK(P[1] == P[2]);
    CHECK(P[2] != P[3]);
    CHECK(P[3] == P[4]);
    CHECK(P[4] != P[5]);

    CHECK(P[0] != P[3]);
    CHECK(P[0] != P[5]);
    CHECK(P[1] != P[5]);
  }

  SECTION("Station with substations and switch"){
    P[0] = U->B[30]->path;
    P[1] = U->B[31]->path;
    P[2] = U->B[32]->path;
    P[3] = U->B[33]->path;
    P[4] = U->B[34]->path;
    P[5] = U->B[35]->path;
    P[6] = U->B[36]->path;

    CHECK(P[0] != P[1]);
    CHECK(P[1] == P[2]);
    CHECK(P[2] != P[3]);
    CHECK(P[3] == nullptr);
    CHECK(P[3] != P[4]);
    CHECK(P[4] == P[5]);
    CHECK(P[5] != P[6]);

    CHECK(P[0] != P[3]);
    CHECK(P[0] != P[6]);
    CHECK(P[1] != P[6]);
  }
}

TEST_CASE( "Path Reverse", "[PATH][PATH-2]") {
  set_level(NONE);
  set_logger_print_level(DEBUG);

  Units = (Unit **)_calloc(30, Unit *);
  unit_len = 30;
  train_link = (RailTrain **)_calloc(10,RailTrain *);
  train_link_len = 10;

  char filename[30] = "./testconfigs/PATH-2.bin";
  auto config = ModuleConfig(filename);
  config.read();

  REQUIRE(config.parsed);
  pathlist.clear();

  new Unit(&config);
  Unit * U = Units[1];

  U->link_all();

  pathlist_find();

  REQUIRE(U->B[0]->path == U->B[10]->path);
  Path * P = U->B[0]->path;

  U->B[2]->blocked = 1;
  U->B[2]->train = new RailTrain(U->B[2]);

  U->B[8]->blocked = 1;
  U->B[8]->train = new RailTrain(U->B[8]);

  REQUIRE(train_link[0]->dir == 0);
  REQUIRE(train_link[1]->dir == 0);

  P->reverse();

  CHECK(P->direction == 1);

  for(uint8_t i = 0; i < 11; i++)
    CHECK(U->B[i]->dir == 0b100);

  CHECK(train_link[0]->dir == 1);
  CHECK(train_link[1]->dir == 1);

  train_link[0]->speed = 10;

  P->reverse();

  CHECK(P->direction == 1);

  for(uint8_t i = 0; i < 11; i++)
    CHECK(U->B[i]->dir == 0b100);

  CHECK(train_link[0]->dir == 1);
  CHECK(train_link[1]->dir == 1);
}


TEST_CASE( "Path Reserve", "[PATH][PATH-3]") {
  set_level(NONE);
  set_logger_print_level(DEBUG);

  Units = (Unit **)_calloc(30, Unit *);
  unit_len = 30;
  train_link = (RailTrain **)_calloc(10,RailTrain *);
  train_link_len = 10;

  char filename[30] = "./testconfigs/PATH-2.bin";
  auto config = ModuleConfig(filename);
  config.read();

  REQUIRE(config.parsed);
  pathlist.clear();

  new Unit(&config);
  Unit * U = Units[1];

  U->link_all();

  pathlist_find();

  REQUIRE(U->B[0]->path == U->B[10]->path);
  Path * P = U->B[0]->path;

  P->reserve();

  CHECK(P->reserved == 1);

  for(uint8_t i = 0; i < 11; i++)
    CHECK(U->B[i]->reserved == 1);
}