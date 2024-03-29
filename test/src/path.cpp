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
#include "switchboard/polarityGroup.h"

#include "rollingstock/train.h"

#include "algorithm/core.h"

#include "train.h"
#include "path.h"
#include "testhelpers.h"

TEST_CASE_METHOD(TestsFixture, "Path Construction", "[PATH][PATH-1]" ) {
  char filenames[2][30] = {"./testconfigs/PATH-1.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;

  switchboard::SwManager->LinkAndMap();
  pathlist_find();

  /*
  // I
  //  1.0->  --1.1->  --1.2-> --1.48>
  //  <1.39 <-1.40-- <-1.41-- <-1.49-
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
  //  1.42-> -1.43-> -<1.44>> -<1.45>> <-1.46- <-1.47-
  //  1.50->|-<1.51>>|-<1.52>>|<-1.53--
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
  //                         Sw1:1       Sw1:2
  //                      End--\           /--End
  //                            \         /
  //  1.30-> -1.31-> -1.32-> ---- -1.33-> ---- -1.34-> -1.35-> -1.36->
  //         [   Station   ]                   [   Station   ]
  //         [                   Station                     ]
  */
  Path * P[10] = {0};


  SECTION("I - Same direction blocks"){
    P[0] = U->B[0]->path;
    CHECK(P[0] == U->B[1]->path);
    CHECK(P[0] == U->B[2]->path);
    CHECK(P[0] == U->B[48]->path);

    CHECK(P[0]->front == U->B[48]);
    CHECK(P[0]->end == U->B[0]);
    CHECK(P[0]->Entrance == U->B[0]);
    CHECK(P[0]->Exit == U->B[48]);
    CHECK(P[0]->next == &U->B[48]->next);
    CHECK(P[0]->prev == &U->B[0]->prev);

    P[1] = U->B[39]->path;
    CHECK(P[1] == U->B[40]->path);
    CHECK(P[1] == U->B[41]->path);
    CHECK(P[1] == U->B[49]->path);

    CHECK(P[1]->front == U->B[39]);
    CHECK(P[1]->end   == U->B[49]);
    CHECK(P[1]->Entrance == U->B[49]);
    CHECK(P[1]->Exit     == U->B[39]);
    CHECK(P[1]->next == &U->B[39]->next);
    CHECK(P[1]->prev == &U->B[49]->prev);

    CHECK(!P[0]->StationPath);
    CHECK(!P[1]->StationPath);
    CHECK(!P[0]->SwitchPath);
    CHECK(!P[1]->SwitchPath);
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

    CHECK(U->B[4]->path);

    CHECK(!P[0]->SwitchPath);
    CHECK( P[1]->SwitchPath);
    CHECK(!P[2]->SwitchPath);
    CHECK(!P[3]->SwitchPath);

    CHECK(!P[0]->StationPath);
    CHECK(!P[1]->StationPath);
    CHECK(!P[2]->StationPath);
    CHECK(!P[3]->StationPath);
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

    REQUIRE(P[2]);

    CHECK(!P[0]->SwitchPath);
    CHECK(!P[1]->SwitchPath);
    CHECK( P[2]->SwitchPath);
    CHECK(!P[3]->SwitchPath);
    CHECK(!P[4]->SwitchPath);

    CHECK(!P[0]->StationPath);
    CHECK(!P[1]->StationPath);
    CHECK(!P[2]->StationPath);
    CHECK(!P[3]->StationPath);
    CHECK(!P[4]->StationPath);
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

    
    CHECK(!P[0]->StationPath);
    CHECK( P[1]->StationPath);
    CHECK( P[2]->StationPath);
    CHECK( P[3]->StationPath);
    CHECK(!P[4]->StationPath);

    CHECK(!P[0]->SwitchPath);
    CHECK(!P[1]->SwitchPath);
    CHECK(!P[2]->SwitchPath);
    CHECK(!P[3]->SwitchPath);
    CHECK(!P[4]->SwitchPath);
  }

  SECTION("V - Blocks with direction change"){
    // Paths must not end when polarity changes.

    P[0] = U->B[17]->path;
    P[1] = U->B[18]->path;
    P[2] = U->B[19]->path;
    P[3] = U->B[20]->path;
    CHECK(P[0] == P[1]);
    CHECK(P[1] == P[2]);
    CHECK(P[2] == P[3]);

    CHECK(P[0]->Entrance == U->B[17]);
    CHECK(P[0]->Exit     == U->B[20]);

    CHECK(U->B[17]->dir == 0);
    CHECK(U->B[18]->dir == 0);
    CHECK(U->B[19]->dir == 1);
    CHECK(U->B[20]->dir == 1);

    P[0] = U->B[42]->path;
    P[1] = U->B[43]->path;
    P[2] = U->B[44]->path;
    P[3] = U->B[45]->path;
    P[4] = U->B[46]->path;
    P[5] = U->B[47]->path;
    CHECK(P[0] == P[1]);
    CHECK(P[1] == P[2]);
    CHECK(P[2] == P[3]);
    CHECK(P[3] == P[4]);
    CHECK(P[4] == P[5]);

    CHECK(U->B[42]->dir == 0);
    CHECK(U->B[43]->dir == 0);
    CHECK(U->B[44]->dir == 0);
    CHECK(U->B[45]->dir == 0);
    CHECK(U->B[46]->dir == 1);
    CHECK(U->B[47]->dir == 1);

    CHECK(P[0]->Entrance == U->B[42]);
    CHECK(P[0]->Exit     == U->B[47]);

    P[0] = U->B[50]->path;
    P[1] = U->B[51]->path;
    P[2] = U->B[52]->path;
    P[3] = U->B[53]->path;
    CHECK(P[0] == P[1]);
    CHECK(P[1] == P[2]);
    CHECK(P[2] == P[3]);

    CHECK(U->B[50]->dir == 0);
    CHECK(U->B[51]->dir == 0);
    CHECK(U->B[52]->dir == 0);
    CHECK(U->B[53]->dir == 1);

    CHECK(P[0]->Entrance == U->B[50]);
    CHECK(P[0]->Exit     == U->B[53]);
  }

  SECTION("VI - Blocks with direction change"){
    // P[0] = U->B[37]->path;
    // P[1] = U->B[21]->path;
    // P[2] = U->B[22]->path;
    // P[3] = U->B[23]->path;
    // P[4] = U->B[38]->path;
    // for(int i = 0; i < 5; i++){
    //   for(int j = 0; j < 5; j++){
    //     if(i == j)
    //       continue;

    //     CHECK(P[i] == P[j]);
    //   }
    // }

    // U->B[21]->AlgorSearch(0);
    // U->B[23]->AlgorSearch(0);
    // Algorithm::print_block_debug(U->B[21]);
    // Algorithm::print_block_debug(U->B[23]);

    // CHECK(U->B[23] == U->B[21]->Next_Block(NEXT, 2));
    // CHECK(U->B[21] == U->B[23]->Next_Block(PREV, 2));
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

    // Check if paths exists
    CHECK(P[0]);
    CHECK(P[1]);
    CHECK(P[3]);
    CHECK(P[5]);
    CHECK(P[6]);

    // Check if paths are distributed correctly
    CHECK(P[0] != P[1]);
    CHECK(P[1] == P[2]);
    CHECK(P[2] != P[3]);
    CHECK(P[3] != P[4]);
    CHECK(P[4] == P[5]);
    CHECK(P[5] != P[6]);

    CHECK(P[0] != P[3]);
    CHECK(P[0] != P[6]);
    CHECK(P[1] != P[6]);
  }
}

TEST_CASE_METHOD(TestsFixture, "Path Reverse", "[PATH][PATH-2a]") {
  char filenames[1][30] = {"./testconfigs/PATH-2.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;

  switchboard::SwManager->LinkAndMap();

  pathlist_find();

  REQUIRE(U->B[3]->path == U->B[10]->path);
  Path * P = U->B[3]->path;

  SECTION("I - Only reversing the path"){
    // The direction of a path must be reversable

    CHECK(P->next == &U->B[10]->next);
    CHECK(P->prev == &U->B[3]->prev);

    CHECK(P->getBlockAtEdge(P->next)     == U->B[11]);
    CHECK(P->getBlockAtEdge(P->prev) == U->B[2]);

    Path * P2 = U->B[1]->path;
    CHECK(P2->getBlockAtEdge(P2->next)     == U->B[2]);
    CHECK(P2->getBlockAtEdge(P2->prev) == 0);

    P->reverse();
    P2->reverse();

    CHECK(P->Entrance == U->B[10]);
    CHECK(P->Exit == U->B[3]);

    CHECK(P->next == &U->B[3]->prev);
    CHECK(P->prev == &U->B[10]->next);

    CHECK(P->direction == 1);

    for(uint8_t i = 3; i < 11; i++)
      CHECK(U->B[i]->dir == 1);

    CHECK(P->getBlockAtEdge(P->next)     == U->B[2]);
    CHECK(P->getBlockAtEdge(P->prev) == U->B[11]);

    CHECK(P2->getBlockAtEdge(P2->next)     == 0);
    CHECK(P2->getBlockAtEdge(P2->prev) == U->B[2]);

    P->reverse();

    CHECK(P->Entrance == U->B[3]);
    CHECK(P->Exit == U->B[10]);

    CHECK(P->next == &U->B[10]->next);
    CHECK(P->prev == &U->B[3]->prev);

    CHECK(P->direction == 0);

    for(uint8_t i = 3; i < 11; i++)
      CHECK(U->B[i]->dir == 0);
  }

  SECTION("II - One Train on the path"){
    // The direction of a path must be reversable
    //  when there is a train standing still in the path.
    // However, it is not allowed when a train is moving

    U->B[5]->setDetection(1);
    Algorithm::processBlock(&U->B[5]->Alg, _FORCE);

    REQUIRE(U->B[5]->train);

    U->B[5]->train->link(0, TRAIN_ENGINE_TYPE);
    U->B[5]->train->setSpeed(10);

    U->B[6]->setDetection(1);
    Algorithm::processBlock(&U->B[6]->Alg, _FORCE);

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
    // The direction of a path must be reversable
    //  when there is a train standing still in the path.
    // However, it is not allowed when a train is moving

    U->B[4]->setDetection(1);
    Algorithm::processBlock(&U->B[4]->Alg, _FORCE);

    U->B[8]->setDetection(1);
    Algorithm::processBlock(&U->B[8]->Alg, _FORCE);

    REQUIRE(U->B[4]->train);
    REQUIRE(U->B[8]->train);

    U->B[4]->train->setSpeed(10);
    U->B[4]->train->link(0, TRAIN_ENGINE_TYPE);
    U->B[8]->train->setSpeed(10);
    U->B[8]->train->link(1, TRAIN_ENGINE_TYPE);

    U->B[5]->setDetection(1);
    Algorithm::processBlock(&U->B[5]->Alg, _FORCE);
    U->B[9]->setDetection(1);
    Algorithm::processBlock(&U->B[9]->Alg, _FORCE);

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

TEST_CASE_METHOD(TestsFixture, "Path Flip Polarity", "[PATH][PATH-2b]") {
  char filenames[1][30] = {"./testconfigs/PATH-2b.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;

  switchboard::SwManager->LinkAndMap();

  pathlist_find();

  PolarityGroup * P[4] = {U->B[0]->Polarity,U->B[2]->Polarity,U->B[4]->Polarity,U->B[6]->Polarity};

  REQUIRE(P[0] == nullptr);
  REQUIRE(P[1]);
  REQUIRE(P[2]);
  REQUIRE(P[3] == nullptr);

  SECTION("I - Flipping polarity of a path"){
    // The polarity of a path must be reversable
    CHECK(P[1]->status == POLARITY_NORMAL);

    P[1]->flip();

    CHECK(P[1]->status == POLARITY_REVERSED);
    CHECK(P[2]->status == POLARITY_NORMAL);

    P[2]->flip();

    CHECK(P[2]->status == POLARITY_REVERSED);
  }

  SECTION("II - One Train on the path"){
    // The polarity of a path must be reversable
    //  even if train is in path

    U->B[3]->setDetection(1);
    Algorithm::processBlock(&U->B[3]->Alg, _FORCE);

    CHECK(P[1]->status == POLARITY_NORMAL);
    CHECK(P[2]->status == POLARITY_NORMAL);

    P[1]->flip();
    P[2]->flip();

    CHECK(P[1]->status == POLARITY_REVERSED);
    CHECK(P[2]->status == POLARITY_REVERSED);

    // However is cannot change polarity if a train is across a path boundary
    //  and will be flipped together
    // It should not matter from which group the polarity is reversed.
    U->B[4]->setDetection(1);
    Algorithm::processBlock(&U->B[4]->Alg, _FORCE);

    CHECK(P[1]->status == POLARITY_REVERSED);
    CHECK(P[2]->status == POLARITY_REVERSED);

    P[1]->flip();

    CHECK(P[1]->status == POLARITY_NORMAL);
    CHECK(P[2]->status == POLARITY_NORMAL);

    P[2]->flip();

    CHECK(P[1]->status == POLARITY_REVERSED);
    CHECK(P[2]->status == POLARITY_REVERSED);

    // If the train is still in a block that is not reverseable, then no polarity is changed.
    U->B[5]->setDetection(1);
    Algorithm::processBlock(&U->B[5]->Alg, _FORCE);
    U->B[6]->setDetection(1);
    Algorithm::processBlock(&U->B[6]->Alg, _FORCE);

    CHECK(P[1]->status == POLARITY_REVERSED);
    CHECK(P[2]->status == POLARITY_REVERSED);

    P[1]->flip();

    CHECK(P[1]->status == POLARITY_REVERSED);
    CHECK(P[2]->status == POLARITY_REVERSED);
  }

  /*
  SECTION("III - More trains"){
    // The direction of a path must be reversable
    //  when there is a train standing still in the path.
    // However, it is not allowed when a train is moving

    U->B[4]->setDetection(1);
    Algorithm::processBlock(&U->B[4]->Alg, _FORCE);

    U->B[8]->setDetection(1);
    Algorithm::processBlock(&U->B[8]->Alg, _FORCE);

    REQUIRE(U->B[4]->train);
    REQUIRE(U->B[8]->train);

    U->B[4]->train->setSpeed(10);
    U->B[4]->train->link(0, TRAIN_ENGINE_TYPE);
    U->B[8]->train->setSpeed(10);
    U->B[8]->train->link(1, TRAIN_ENGINE_TYPE);

    U->B[5]->setDetection(1);
    Algorithm::processBlock(&U->B[5]->Alg, _FORCE);
    U->B[9]->setDetection(1);
    Algorithm::processBlock(&U->B[9]->Alg, _FORCE);

    // Moving trains on the path
    P[1]->reverse();
    CHECK(P[1]->direction == 0);

    U->B[4]->train->setSpeed(0);

    // Still a moving train
    P[1]->reverse();
    CHECK(P[1]->direction == 0);

    U->B[9]->train->setSpeed(0);

    // No moving train, so reverse blocks
    P[1]->reverse();
    CHECK(P[1]->direction == 1);
  }
  */
}

TEST_CASE_METHOD(TestsFixture, "Path Reserve", "[PATH][PATH-3]") {
  char filenames[1][30] = {"./testconfigs/PATH-2.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();
  
  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;

  switchboard::SwManager->LinkAndMap();
  pathlist_find();

  for(uint8_t i = 0; i < 14; i++){
    Algorithm::processBlock(&U->B[i]->Alg, _FORCE);
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

TEST_CASE_METHOD(TestsFixture, "Path & Trains", "[PATH][PATH-4]"){
  char filenames[1][30] = {"./testconfigs/PATH-4.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  /*
  //
  // I
  //
  //    --1.0-> --1.1-> --1.2-> -<1.3>> -<1.4>> -<1.5>> --1.6-> --1.7-> --1.8->
  //
  //
  //
  //
  */
  
  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;

  switchboard::SwManager->LinkAndMap();
  pathlist_find();

  // Create custom paths such that direction changes
  for(uint8_t i = 0; i < 9; i++)
    U->B[i]->path = 0;

  Path * P;
  for(uint8_t i = 0; i < 9; i += 3){
    P = new Path(U->B[i]);
    P->add(U->B[i+1], NEXT);
    P->add(U->B[i+2], NEXT);
  }

  REQUIRE(U->B[0]->path != U->B[3]->path);
  REQUIRE(U->B[0]->path != U->B[6]->path);

  REQUIRE(U->B[3]->path != U->B[6]->path);

  for(uint8_t i = 0; i < 9; i++){
    Algorithm::processBlock(&U->B[i]->Alg, _FORCE);
  }

  SECTION("I - Not linked"){
    U->B[1]->setDetection(1);
    Algorithm::processBlock(&U->B[1]->Alg, _FORCE);

    Train * T = U->B[1]->train;

    REQUIRE(U->B[1]->path->trains.size() == 1);
    CHECK(U->B[1]->path->trains[0] == U->B[1]->train);
    REQUIRE(T->paths.size() == 1);
    CHECK(T->paths[0] == U->B[1]->path);

    U->B[2]->setDetection(1);
    Algorithm::processBlock(&U->B[2]->Alg, _FORCE);
    
    U->B[1]->setDetection(0);
    Algorithm::processBlock(&U->B[1]->Alg, _FORCE);

    REQUIRE(U->B[2]->path->trains.size() == 1);
    CHECK(U->B[2]->path->trains[0] == U->B[2]->train);
    REQUIRE(T->paths.size() == 1);
    CHECK(T->paths[0] == U->B[1]->path);

    U->B[3]->setDetection(1);
    Algorithm::processBlock(&U->B[3]->Alg, _FORCE);

    REQUIRE(U->B[3]->path->trains.size() == 1);
    CHECK(U->B[3]->path->trains[0] == U->B[3]->train);
    REQUIRE(T->paths.size() == 2);
    CHECK(T->paths[1] == U->B[3]->path);
    
    U->B[2]->setDetection(0);
    Algorithm::processBlock(&U->B[2]->Alg, _FORCE);

    CHECK(U->B[2]->path->trains.size() == 0);
    REQUIRE(T->paths.size() == 1);
    CHECK(T->paths[0] == U->B[3]->path);

    U->B[4]->setDetection(1);
    Algorithm::processBlock(&U->B[4]->Alg, _FORCE);
    U->B[3]->setDetection(0);
    Algorithm::processBlock(&U->B[3]->Alg, _FORCE);

    U->B[5]->setDetection(1);
    Algorithm::processBlock(&U->B[5]->Alg, _FORCE);
    U->B[4]->setDetection(0);
    Algorithm::processBlock(&U->B[4]->Alg, _FORCE);
    
    REQUIRE(U->B[3]->path->trains.size() == 1);
    CHECK(U->B[3]->path->trains[0] == U->B[5]->train);

    // Second Train
    U->B[2]->setDetection(1);
    Algorithm::processBlock(&U->B[2]->Alg, _FORCE);

    REQUIRE(U->B[2]->train);
    REQUIRE(U->B[2]->train != U->B[3]->train);
    REQUIRE(U->B[2]->path->trains.size() == 1);
    CHECK(U->B[2]->path->trains[0] == U->B[2]->train);
    
    U->B[3]->setDetection(1);
    Algorithm::processBlock(&U->B[3]->Alg, _FORCE);

    REQUIRE(U->B[3]->path->trains.size() == 2);
    CHECK(U->B[3]->path->trains[0] == U->B[3]->train);

    U->B[2]->setDetection(0);
    Algorithm::processBlock(&U->B[2]->Alg, _FORCE);
    
    CHECK(U->B[2]->path->trains.size() == 0);
  }

  
  SECTION("II - linked and with speed"){
    U->B[1]->setDetection(1);
    Algorithm::processBlock(&U->B[1]->Alg, _FORCE);

    REQUIRE(U->B[1]->train);

    U->B[1]->train->link(0, TRAIN_ENGINE_TYPE);
    U->B[1]->train->setSpeed(10);

    REQUIRE(U->B[1]->path->trains.size() == 1);
    CHECK(U->B[1]->path->trains[0] == U->B[1]->train);

    U->B[2]->setDetection(1);
    Algorithm::processBlock(&U->B[2]->Alg, _FORCE);
    
    U->B[1]->setDetection(0);
    Algorithm::processBlock(&U->B[1]->Alg, _FORCE);

    REQUIRE(U->B[2]->path->trains.size() == 1);
    CHECK(U->B[2]->path->trains[0] == U->B[2]->train);

    U->B[3]->setDetection(1);
    Algorithm::processBlock(&U->B[3]->Alg, _FORCE);

    REQUIRE(U->B[3]->path->trains.size() == 1);
    CHECK(U->B[3]->path->trains[0] == U->B[3]->train);
    
    U->B[2]->setDetection(0);
    Algorithm::processBlock(&U->B[2]->Alg, _FORCE);

    CHECK(U->B[2]->path->trains.size() == 0);

    U->B[4]->setDetection(1);
    Algorithm::processBlock(&U->B[4]->Alg, _FORCE);
    U->B[3]->setDetection(0);
    Algorithm::processBlock(&U->B[3]->Alg, _FORCE);

    U->B[5]->setDetection(1);
    Algorithm::processBlock(&U->B[5]->Alg, _FORCE);
    U->B[4]->setDetection(0);
    Algorithm::processBlock(&U->B[4]->Alg, _FORCE);
    
    REQUIRE(U->B[3]->path->trains.size() == 1);
    CHECK(U->B[3]->path->trains[0] == U->B[5]->train);

    // Second Train
    U->B[1]->setDetection(1);
    Algorithm::processBlock(&U->B[1]->Alg, _FORCE);

    REQUIRE(U->B[1]->train);
    
    U->B[1]->train->link(1, TRAIN_ENGINE_TYPE);
    U->B[1]->train->setSpeed(10);

    U->B[2]->setDetection(1);
    Algorithm::processBlock(&U->B[2]->Alg, _FORCE);
    U->B[1]->setDetection(0);
    Algorithm::processBlock(&U->B[1]->Alg, _FORCE);
    
    REQUIRE(U->B[2]->train != U->B[3]->train);
    REQUIRE(U->B[2]->path->trains.size() == 1);
    CHECK(U->B[2]->path->trains[0] == U->B[2]->train);
    
    U->B[3]->setDetection(1);
    Algorithm::processBlock(&U->B[3]->Alg, _FORCE);

    REQUIRE(U->B[3]->path->trains.size() == 2);
    CHECK(U->B[3]->path->trains[0] == U->B[5]->train);
    CHECK(U->B[3]->path->trains[1] == U->B[3]->train);

    U->B[2]->setDetection(0);
    Algorithm::processBlock(&U->B[2]->Alg, _FORCE);
    
    CHECK(U->B[2]->path->trains.size() == 0);
  }

}
