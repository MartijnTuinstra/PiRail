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

#include "rollingstock/train.h"

#include "algorithm/core.h"

#include "train.h"
#include "path.h"

void init_test(char (* filenames)[30], int nr_files);

class TestsFixture {
public:
  TestsFixture();
  void loadSwitchboard(char (* filenames)[30], int nr_files);
  void loadStock();
  ~TestsFixture();
};

TEST_CASE_METHOD(TestsFixture, "Path Construction", "[PATH][PATH-1]" ) {
  char filenames[2][30] = {"./testconfigs/PATH-1.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;
  U->link_all();

  pathlist_find();

  /*
  // I
  //  1.0->  --1.1->  --1.2->
  //  <1.39 <-1.40-- <-1.41--
  //
  // II
  //                    /Sw1:0
  //                   /
  //  1.3->  --1.4-> ---- --1.5->
  //                   \- --1.6->
  //
  // III
  //              /MSSw1:0
  //  1.7->  -\  /
  //  1.8->  --1.9-> --1.10->
  //              \- --1.11->
  //
  // IV
  //  1.12-> -1.13-> -1.14-> -1.15-> -1.16->
  //         [       Station       ]
  //
  // V
  //  1.17-> -1.18-> <-1.19- <-1.20-
  //  1.42-> -1.43-> <-1.44-> <-1.45- <-1.46-
  //
  // VI
  //  1.37-> 1.21-> <-\
  //                   |1.22 <==> 1.37> 1.21> <1.22> <1.23 <1.38
  //  1.38-> 1.23-> <-/
  //
  // VII
  //  1.24-> -1.25-> -1.26-> -1.27-> -1.28-> -1.29->
  //         [   Station   ] [   Station   ]
  //         [           Station           ]
  //
  // VIII
  //                           Sw1:1\    /--End
  //                                 \  /
  //  1.30-> -1.31-> -1.32-> -1.33-> ---- -1.34-> -1.35-> -1.36->
  //         [   Station   ]              [   Station   ]
  //         [                 Station                  ]
  */
  Path * P[10] = {0};


  SECTION("I - Same direction blocks"){
    P[0] = U->B[0]->path;
    CHECK(P[0] == U->B[1]->path);
    CHECK(P[0] == U->B[2]->path);

    CHECK(P[0]->front == U->B[2]);
    CHECK(P[0]->end == U->B[0]);
    CHECK(P[0]->Entrance == U->B[0]);
    CHECK(P[0]->Exit == U->B[2]);
    CHECK(P[0]->next == &U->B[2]->next);
    CHECK(P[0]->prev == &U->B[0]->prev);

    P[1] = U->B[39]->path;
    CHECK(P[1] == U->B[40]->path);
    CHECK(P[1] == U->B[41]->path);

    CHECK(P[1]->front == U->B[39]);
    CHECK(P[1]->end   == U->B[41]);
    CHECK(P[1]->Entrance == U->B[41]);
    CHECK(P[1]->Exit     == U->B[39]);
    CHECK(P[1]->next == &U->B[39]->prev);
    CHECK(P[1]->prev == &U->B[41]->next);

    CHECK(U->B[39]->dir == 1);
    CHECK(U->B[40]->dir == 1);
    CHECK(U->B[41]->dir == 1);
  }

  SECTION("II - Blocks arround switch"){
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

  SECTION("III - Blocks arround msswitch crossing"){
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


  SECTION("IV - Paths and stations"){
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


  SECTION("V - Blocks with direction change"){
    // Sudden change
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

    CHECK(P[0]->Entrance == U->B[17]);
    CHECK(P[0]->Exit == U->B[20]);

    CHECK(U->B[17]->dir == 0);
    CHECK(U->B[18]->dir == 0);
    CHECK(U->B[19]->dir == 5);
    CHECK(U->B[20]->dir == 5);

    // Change over to opposite direction
    P[0] = U->B[42]->path;
    P[1] = U->B[43]->path;
    P[2] = U->B[44]->path;
    P[3] = U->B[45]->path;
    P[4] = U->B[46]->path;
    for(int i = 0; i < 5; i++){
      for(int j = 0; j < 5; j++){
        if(i == j)
          continue;

        CHECK(P[i] == P[j]);
      }
    }

    CHECK(P[0]->Entrance == U->B[42]);
    CHECK(P[0]->Exit == U->B[46]);

    CHECK(U->B[42]->dir == 0);
    CHECK(U->B[43]->dir == 0);
    CHECK(U->B[44]->dir == 2);
    CHECK(U->B[45]->dir == 5);
    CHECK(U->B[46]->dir == 5);
  }

  SECTION("VI - Blocks with direction change"){
    P[0] = U->B[37]->path;
    P[1] = U->B[21]->path;
    P[2] = U->B[22]->path;
    P[3] = U->B[23]->path;
    P[4] = U->B[38]->path;
    for(int i = 0; i < 5; i++){
      for(int j = 0; j < 5; j++){
        if(i == j)
          continue;

        CHECK(P[i] == P[j]);
      }
    }

    logger.setlevel_stdout(INFO);
    U->B[21]->AlgorSearch(0);
    U->B[23]->AlgorSearch(0);
    Algorithm::print_block_debug(U->B[21]);
    Algorithm::print_block_debug(U->B[23]);

    CHECK(U->B[23] == U->B[21]->Next_Block(NEXT, 2));
    CHECK(U->B[21] == U->B[23]->Next_Block(PREV, 2));
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

TEST_CASE_METHOD(TestsFixture, "Path Reverse", "[PATH][PATH-2]") {
  loggerf(CRITICAL, "PATH-2 TEST");
  char filenames[1][30] = {"./testconfigs/PATH-2.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;
  U->link_all();

  pathlist_find();

  REQUIRE(U->B[3]->path == U->B[10]->path);
  Path * P = U->B[3]->path;

  SECTION("I - Only reversing the path"){

    CHECK(P->next == &U->B[10]->next);
    CHECK(P->prev == &U->B[3]->prev);

    P->reverse();

    CHECK(P->Entrance == U->B[10]);
    CHECK(P->Exit == U->B[3]);

    CHECK(P->next == &U->B[3]->prev);
    CHECK(P->prev == &U->B[10]->next);

    CHECK(P->direction == 1);

    for(uint8_t i = 3; i < 11; i++)
      CHECK(U->B[i]->dir == 0b100);

    P->reverse();

    CHECK(P->Entrance == U->B[3]);
    CHECK(P->Exit == U->B[10]);

    CHECK(P->next == &U->B[10]->next);
    CHECK(P->prev == &U->B[3]->prev);

    CHECK(P->direction == 0);

    for(uint8_t i = 3; i < 11; i++)
      CHECK(U->B[i]->dir == 0b000);
  }

  SECTION("II - One Train on the path"){
    U->B[5]->setDetection(1);
    Algorithm::process(U->B[5], _FORCE);

    REQUIRE(U->B[5]->train);

    U->B[5]->train->link(0, TRAIN_ENGINE_TYPE);
    U->B[5]->train->setSpeed(10);

    U->B[6]->setDetection(1);
    Algorithm::process(U->B[6], _FORCE);

    REQUIRE(U->B[6]->train->assigned);
    REQUIRE(U->B[6]->train->directionKnown);

    // Train is moving so no reversing
    P->reverse();

    CHECK(P->direction == 0);

    U->B[6]->train->setSpeed(0);

    // Train is stopped so should be reversed
    P->reverse();

    CHECK(P->direction == 1);

    U->B[6]->train->setSpeed(0);

    CHECK(U->B[6]->train->dir == 1);

  }

  SECTION("III - More trains"){
    U->B[4]->setDetection(1);
    Algorithm::process(U->B[4], _FORCE);

    U->B[8]->setDetection(1);
    Algorithm::process(U->B[8], _FORCE);

    REQUIRE(U->B[4]->train);
    REQUIRE(U->B[8]->train);

    U->B[4]->train->setSpeed(10);
    U->B[4]->train->link(0, TRAIN_ENGINE_TYPE);
    U->B[8]->train->setSpeed(10);
    U->B[8]->train->link(1, TRAIN_ENGINE_TYPE);

    U->B[5]->setDetection(1);
    Algorithm::process(U->B[5], _FORCE);
    U->B[9]->setDetection(1);
    Algorithm::process(U->B[9], _FORCE);

    // Moving trains on the path
    P->reverse();
    CHECK(P->direction == 0);

    U->B[4]->train->setSpeed(0);

    // Still a moving train
    P->reverse();
    CHECK(P->direction == 0);

    U->B[9]->train->setSpeed(0);

    // No moving train, so reverse blocks
    P->reverse();
    CHECK(P->direction == 1);
  }
}

TEST_CASE_METHOD(TestsFixture, "Path Reserve", "[PATH][PATH-3]") {
  loggerf(CRITICAL, "PATH-3 TEST");
  char filenames[1][30] = {"./testconfigs/PATH-2.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();
  
  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;
  U->link_all();
  pathlist_find();

  for(uint8_t i = 0; i < 14; i++){
    Algorithm::process(U->B[i], _FORCE);
  }

  REQUIRE(U->B[3]->path == U->B[10]->path);
  Path * P = U->B[3]->path;

  SECTION("I - Normal block at the end"){    
    Train * RT = new Train(U->B[3]);

    P->reserve(RT);

    CHECK(P->reserved);

    for(uint8_t i = 3; i < 11; i++)
      CHECK(U->B[i]->reserved);

    CHECK(U->B[11]->state == CAUTION);
    CHECK(U->B[11]->reverse_state == DANGER);

    CHECK(U->B[12]->state == CAUTION);
    CHECK(U->B[12]->reverse_state == CAUTION);
  }

  SECTION("II - Reversed block at the end"){
    U->B[11]->reverse();
    U->B[12]->reverse();

    CHECK(U->B[11]->state == PROCEED);
    CHECK(U->B[12]->state == PROCEED);

    Train * RT = new Train(U->B[3]);

    P->reserve(RT);

    CHECK(P->reserved);

    for(uint8_t i = 3; i < 11; i++)
      CHECK(U->B[i]->reserved);

    CHECK(U->B[11]->state == DANGER);
    CHECK(U->B[11]->reverse_state == PROCEED);

    CHECK(U->B[12]->state == CAUTION);
    CHECK(U->B[12]->reverse_state == CAUTION);
  }

  SECTION("III - Partial Reserve"){   
    Train * RT = new Train(U->B[5]);

    P->reserve(RT, U->B[5]);

    CHECK(P->reserved);

    for(uint8_t i = 3; i < 6; i++)
      CHECK(!U->B[i]->reserved);
    for(uint8_t i = 6; i < 11; i++)
      CHECK(U->B[i]->reserved);
  }
}
