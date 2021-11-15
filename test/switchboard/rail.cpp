#include "catch.hpp"

#include "utils/mem.h"
#include "utils/logger.h"
#include "system.h"

#include "config/ModuleConfig.h"
#include "switchboard/manager.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/polarityGroup.h"
#include "switchboard/unit.h"

#include "algorithm/core.h"
#include "algorithm/component.h"

#include "train.h"
#include "path.h"
#include "rollingstock/train.h"

#include "testhelpers.h"

TEST_CASE_METHOD(TestsFixture, "Block Link", "[SB][SB-1][SB-1.1]" ) {
  char filenames[1][30] = {"./testconfigs/SB-1.1.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  switchboard::SwManager->LinkAndMap();

  /*
  // Blocks only
  //  <-1.0>>  <-1.1>>  <-1.2>>  <-1.3>>  <-1.4>>
  //
  //  <-1.13>> <-1.14>> <<1.15-> <<1.16->
  //
  // Switche
  // --1.5----> --1.6->
  //    Sw0 \-> --1.7->
  //
  // MSSwitche
  // --1.8--> \/ --1.11->
  // --1.9--> /\ --1.12->
  //    MSSw0A--B 1.10
  */

  REQUIRE(U->B[0] != 0);
  REQUIRE(U->B[1] != 0);
  REQUIRE(U->B[2] != 0);

  SECTION( "I - Block link check"){
    CHECK(U->B[1]->next.type == RAIL_LINK_R);
    CHECK(U->B[1]->next.p.B == U->B[2]);

    CHECK(U->B[1]->prev.type == RAIL_LINK_R);
    CHECK(U->B[1]->prev.p.B == U->B[0]);
    
    CHECK(U->B[5]->next.type == RAIL_LINK_S);
    CHECK(U->B[5]->next.p.Sw == U->Sw[0]);
    
    CHECK(U->B[6]->prev.type == RAIL_LINK_s);
    CHECK(U->B[6]->prev.p.Sw == U->Sw[0]);

    CHECK(U->B[7]->prev.type == RAIL_LINK_s);
    CHECK(U->B[7]->prev.p.Sw == U->Sw[0]);

    CHECK(U->Sw[0]->app.type == RAIL_LINK_R);
    CHECK(U->Sw[0]->str.type == RAIL_LINK_R);
    CHECK(U->Sw[0]->div.type == RAIL_LINK_R);
    CHECK(U->Sw[0]->app.p.B == U->B[5]);
    CHECK(U->Sw[0]->str.p.B == U->B[6]);
    CHECK(U->Sw[0]->div.p.B == U->B[7]);
    
    CHECK(U->B[8]->next.type == RAIL_LINK_MA);
    CHECK(U->B[8]->next.p.MSSw == U->MSSw[0]);
    
    CHECK(U->B[9]->next.type == RAIL_LINK_MA);
    CHECK(U->B[9]->next.p.MSSw == U->MSSw[0]);

    CHECK(U->B[11]->prev.type == RAIL_LINK_MB);
    CHECK(U->B[11]->prev.p.MSSw == U->MSSw[0]);

    CHECK(U->B[12]->prev.type == RAIL_LINK_MB);
    CHECK(U->B[12]->prev.p.MSSw == U->MSSw[0]);

    CHECK(U->MSSw[0]->sideA[0].type == RAIL_LINK_R);
    CHECK(U->MSSw[0]->sideA[1].type == RAIL_LINK_R);
    CHECK(U->MSSw[0]->sideB[0].type == RAIL_LINK_R);
    CHECK(U->MSSw[0]->sideB[1].type == RAIL_LINK_R);
    CHECK(U->MSSw[0]->sideA[0].p.B == U->B[8]);
    CHECK(U->MSSw[0]->sideA[1].p.B == U->B[9]);
    CHECK(U->MSSw[0]->sideB[1].p.B == U->B[11]);
    CHECK(U->MSSw[0]->sideB[0].p.B == U->B[12]);
  }

  SECTION( "II - NextBlock Function" ) {
    CHECK(U->B[2]->Next_Block(NEXT, 2) == U->B[4]);
    CHECK(U->B[2]->Next_Block(NEXT, 1) == U->B[3]);
    CHECK(U->B[2]->Next_Block(PREV, 1) == U->B[1]);
    CHECK(U->B[2]->Next_Block(PREV, 2) == U->B[0]);
    
    CHECK(U->B[5]->Next_Block(NEXT, 1) == U->B[6]);
    CHECK(U->B[6]->Next_Block(PREV, 1) == U->B[5]);
    CHECK(U->B[7]->Next_Block(PREV, 1) == U->B[5]);

    CHECK(U->B[8]->Next_Block(NEXT, 1) == U->B[10]);
    CHECK(U->B[9]->Next_Block(NEXT, 1) == U->B[10]);

    CHECK(U->B[11]->Next_Block(PREV, 1) == U->B[10]);
    CHECK(U->B[12]->Next_Block(PREV, 1) == U->B[10]);

    CHECK(U->B[8]->Next_Block(NEXT, 2) == U->B[12]);

    CHECK(U->B[12]->Next_Block(PREV, 2) == U->B[8]);

    CHECK(U->B[13]->Next_Block(NEXT, 2) == U->B[15]);
    CHECK(U->B[13]->Next_Block(NEXT, 3) == U->B[16]);

    CHECK(U->B[16]->Next_Block(NEXT, 2) == U->B[14]);
    CHECK(U->B[16]->Next_Block(NEXT, 3) == U->B[13]);
  }

  SECTION( "III - NextBlock Function Reversed Block" ) {
    U->B[2]->reverse();
    U->B[3]->reverse();
    U->B[4]->reverse();

    CHECK(U->B[2]->Next_Block(NEXT, 2) == U->B[0]);
    CHECK(U->B[2]->Next_Block(NEXT, 1) == U->B[1]);
    CHECK(U->B[2]->Next_Block(PREV, 1) == U->B[3]);
    CHECK(U->B[2]->Next_Block(PREV, 2) == U->B[4]);

    Block * B[10] = {0};

    U->B[0]->_NextList(U->B[0], B, 0, NEXT | FL_BLOCKS_COUNT, 5);

    for(uint8_t i = 0; i < 4; i++){
      loggerf(INFO, "%i == %i", i, i+1);
      CHECK(B[i] == U->B[i+1]);
    }

    U->B[4]->_NextList(U->B[4], B, 0, NEXT | FL_BLOCKS_COUNT, 5);

    for(uint8_t i = 0; i < 4; i++){
      loggerf(INFO, "%i == %i", 4-i, 3-i);
      CHECK(B[i] == U->B[3-i]);
    }

    //-------------
    U->B[15]->reverse();
    U->B[16]->reverse();

    U->B[13]->_NextList(U->B[13], B, 0, NEXT | FL_BLOCKS_COUNT, 5);

    for(uint8_t i = 0; i < 3; i++){
      loggerf(INFO, "%i (%2i:%2i) == %i (%2i:%2i)", i, B[i]->module, B[i]->id, i+14, U->B[i+14]->module, U->B[i+14]->id);
      CHECK(B[i] == U->B[i+14]);
    }

    U->B[16]->_NextList(U->B[16], B, 0, PREV | FL_BLOCKS_COUNT, 5);

    for(uint8_t i = 0; i < 3; i++){
      loggerf(INFO, "%i (%2i:%2i) == %i (%2i:%2i)", i, B[i]->module, B[i]->id, 16-i, U->B[16-i]->module, U->B[16-i]->id);
      CHECK(B[i] == U->B[15-i]);
    }
    
  }

  SECTION( "IV - NextBlock Function Reversed Block" ) {
    U->B[2]->flipPolarity(0);
    U->B[3]->flipPolarity(0);
    U->B[4]->flipPolarity(0);

    CHECK(U->B[2]->Next_Block(NEXT, 2) == U->B[4]);
    CHECK(U->B[2]->Next_Block(NEXT, 1) == U->B[3]);
    CHECK(U->B[2]->Next_Block(PREV, 1) == U->B[1]);
    CHECK(U->B[2]->Next_Block(PREV, 2) == U->B[0]);

    Block * B[10] = {0};

    U->B[0]->_NextList(U->B[0], B, 0, NEXT | FL_BLOCKS_COUNT, 5);

    for(uint8_t i = 0; i < 4; i++){
      loggerf(INFO, "%i == %i", i, i+1);
      CHECK(B[i] == U->B[i+1]);
    }

    U->B[4]->_NextList(U->B[4], B, 0, PREV | FL_BLOCKS_COUNT, 5);

    for(uint8_t i = 0; i < 4; i++){
      loggerf(INFO, "%i == %i", 4-i, 3-i);
      CHECK(B[i] == U->B[3-i]);
    }

    //-------------
    U->B[15]->flipPolarity(0);
    U->B[16]->flipPolarity(0);

    U->B[13]->_NextList(U->B[13], B, 0, NEXT | FL_BLOCKS_COUNT, 5);

    for(uint8_t i = 0; i < 3; i++){
      loggerf(INFO, "%i (%2i:%2i) == %i (%2i:%2i)", i, B[i]->module, B[i]->id, i+14, U->B[i+14]->module, U->B[i+14]->id);
      CHECK(B[i] == U->B[i+14]);
    }

    U->B[16]->_NextList(U->B[16], B, 0, NEXT | FL_BLOCKS_COUNT, 5);

    for(uint8_t i = 0; i < 3; i++){
      loggerf(INFO, "%i (%2i:%2i) == %i (%2i:%2i)", i, B[i]->module, B[i]->id, 16-i, U->B[16-i]->module, U->B[16-i]->id);
      CHECK(B[i] == U->B[15-i]);
    }
    
  }

  SECTION( "V - NextBlock Function Reversed Block" ) {
    U->B[2]->flipPolarity(1);
    U->B[3]->flipPolarity(1);
    U->B[4]->flipPolarity(1);

    CHECK(U->B[2]->Next_Block(NEXT, 2) == U->B[0]);
    CHECK(U->B[2]->Next_Block(NEXT, 1) == U->B[1]);
    CHECK(U->B[2]->Next_Block(PREV, 1) == U->B[3]);
    CHECK(U->B[2]->Next_Block(PREV, 2) == U->B[4]);

    Block * B[10] = {0};

    U->B[0]->_NextList(U->B[0], B, 0, NEXT | FL_BLOCKS_COUNT, 5);

    for(uint8_t i = 0; i < 4; i++){
      loggerf(INFO, "%i == %i", i, i+1);
      CHECK(B[i] == U->B[i+1]);
    }

    U->B[4]->_NextList(U->B[4], B, 0, NEXT | FL_BLOCKS_COUNT, 5);

    for(uint8_t i = 0; i < 4; i++){
      loggerf(INFO, "%i == %i", 4-i, 3-i);
      CHECK(B[i] == U->B[3-i]);
    }

    //-------------
    U->B[15]->flipPolarity(1);
    U->B[16]->flipPolarity(1);

    U->B[13]->_NextList(U->B[13], B, 0, NEXT | FL_BLOCKS_COUNT, 5);

    for(uint8_t i = 0; i < 3; i++){
      loggerf(INFO, "%i (%2i:%2i) == %i (%2i:%2i)", i, B[i]->module, B[i]->id, i+14, U->B[i+14]->module, U->B[i+14]->id);
      CHECK(B[i] == U->B[i+14]);
    }

    U->B[16]->_NextList(U->B[16], B, 0, PREV | FL_BLOCKS_COUNT, 5);

    for(uint8_t i = 0; i < 3; i++){
      loggerf(INFO, "%i (%2i:%2i) == %i (%2i:%2i)", i, B[i]->module, B[i]->id, 16-i, U->B[16-i]->module, U->B[16-i]->id);
      CHECK(B[i] == U->B[15-i]);
    }
  }

  // SECTION( "IV - NextBlock Function Reverser Block" ) {
  //   // Just the same as II
  //   U->B[2]->dir ^= 0b10;
  //   CHECK(U->B[2]->Next_Block(NEXT, 2) == U->B[4]);
  //   CHECK(U->B[2]->Next_Block(NEXT, 1) == U->B[3]);
  //   CHECK(U->B[2]->Next_Block(PREV, 1) == U->B[1]);
  //   CHECK(U->B[2]->Next_Block(PREV, 2) == U->B[0]);
  // }

  // SECTION( "V - NextBlock Function Reversed Reverser Block" ) {
  //   U->B[2]->dir ^= 0b110;
  //   CHECK(U->B[2]->Next_Block(NEXT, 2) == U->B[0]);
  //   CHECK(U->B[2]->Next_Block(NEXT, 1) == U->B[1]);
  //   CHECK(U->B[2]->Next_Block(PREV, 1) == U->B[3]);
  //   CHECK(U->B[2]->Next_Block(PREV, 2) == U->B[4]);
  // }

  // SECTION( "VI - NextBlock Function counter direction Block" ) {
  //   U->B[2]->dir ^= 0b1;
  //   CHECK(U->B[2]->Next_Block(NEXT, 2) == U->B[0]);
  //   CHECK(U->B[2]->Next_Block(NEXT, 1) == U->B[1]);
  //   CHECK(U->B[2]->Next_Block(PREV, 1) == U->B[3]);
  //   CHECK(U->B[2]->Next_Block(PREV, 2) == U->B[4]);
  // }

  // SECTION( "VII - NextBlock Function reversed counter direction Block" ) {
  //   U->B[2]->dir ^= 0b101;
  //   CHECK(U->B[2]->Next_Block(NEXT, 2) == U->B[4]);
  //   CHECK(U->B[2]->Next_Block(NEXT, 1) == U->B[3]);
  //   CHECK(U->B[2]->Next_Block(PREV, 1) == U->B[1]);
  //   CHECK(U->B[2]->Next_Block(PREV, 2) == U->B[0]);
  // }
}

TEST_CASE_METHOD(TestsFixture, "Block Algorithm Search", "[SB][SB-1][SB-1.2]" ) {
  char filenames[1][30] = {"./testconfigs/SB-1.2.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  switchboard::SwManager->LinkAndMap();

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
  //  <-1.43- <-1.44- <-1.45>> -1.46-> -1.47-> 
  //??-1.43-> -1.44-> <-1.45>> <-1.46- <-1.47-
  //
  // SECTION V
  //                    /Sw1:0
  //                   /
  //  1.16->  --1.17-> ---- --1.18-> --1.19->
  //                     \- --1.48->
  //
  // SECTION VI
  //              /MSSw1:0
  //  1.20->  -\  /
  //  1.21->  --1.22-> --1.23->
  //                \- --1.24->
  //
  // SECTION VII
  //  1.25-> -1.26-> -1.27-> -1.28-> -1.29->
  //         [       Station       ]
  //
  // SECTION VIII
  //  1.30-> -1.31-> -1.32-> -1.33-> -1.34-> -1.35->
  //         [   Station   ] [   Station   ]
  //         [           Station           ]
  //
  // SECTION IX
  //                           Sw1:1\    /--End
  //                                 \  /
  //  1.36-> -1.37-> -1.38-> -1.39-> ---- -1.40-> -1.41-> -1.42->
  //         [   Station   ]              [   Station   ]
  //         [                 Station                  ]
  */

  SECTION("I - Standard"){
    U->B[4]->AlgorSearch(0);
    Algorithm::print_block_debug(U->B[4]);

    REQUIRE(U->B[4]->Alg.N->group[3] == 1);
    REQUIRE(U->B[4]->Alg.P->group[3] == 4);

    CHECK(U->B[4]->getBlock(NEXT, 0) == U->B[5]);

    CHECK(U->B[4]->getBlock(PREV, 0) == U->B[3]);
    CHECK(U->B[4]->getBlock(PREV, 1) == U->B[2]);
    CHECK(U->B[4]->getBlock(PREV, 2) == U->B[1]);
    CHECK(U->B[4]->getBlock(PREV, 3) == U->B[0]);

    CHECK(U->B[4]->Alg.P->group[0] == 1);
    CHECK(U->B[4]->Alg.P->group[1] == 2);
    CHECK(U->B[4]->Alg.P->group[2] == 3);

    U->B[4]->reverse();

    REQUIRE(U->B[4]->Alg.P->group[3] == 1);
    REQUIRE(U->B[4]->Alg.N->group[3] == 4);

    CHECK(U->B[4]->getBlock(PREV, 0) == U->B[5]);

    CHECK(U->B[4]->getBlock(NEXT, 0) == U->B[3]);
    CHECK(U->B[4]->getBlock(NEXT, 1) == U->B[2]);
    CHECK(U->B[4]->getBlock(NEXT, 2) == U->B[1]);
    CHECK(U->B[4]->getBlock(NEXT, 3) == U->B[0]);

    CHECK(U->B[4]->Alg.N->group[0] == 1);
    CHECK(U->B[4]->Alg.N->group[1] == 2);
    CHECK(U->B[4]->Alg.N->group[2] == 3);
  }

  SECTION("II - Block smaller than 1 meter"){
    U->B[10]->AlgorSearch(0);
    Algorithm::print_block_debug(U->B[10]);

    REQUIRE(U->B[10]->Alg.N->group[3] == 1);
    REQUIRE(U->B[10]->Alg.P->group[3] == 4);

    CHECK(U->B[10]->getBlock(NEXT, 0) == U->B[11]);

    CHECK(U->B[10]->getBlock(PREV, 0) == U->B[9]);
    CHECK(U->B[10]->getBlock(PREV, 1) == U->B[8]);
    CHECK(U->B[10]->getBlock(PREV, 2) == U->B[7]);
    CHECK(U->B[10]->getBlock(PREV, 3) == U->B[6]);

    CHECK(U->B[10]->Alg.P->group[0] == 2);
    CHECK(U->B[10]->Alg.P->group[1] == 3);
    CHECK(U->B[10]->Alg.P->group[2] == 4);
  }

  SECTION("III - Blocks change direction"){
    // Blocks are grouped together with the same general direction

    U->B[12]->AlgorSearch(0);
    U->B[15]->AlgorSearch(0);
    Algorithm::print_block_debug(U->B[12]);
    Algorithm::print_block_debug(U->B[15]);

    REQUIRE(U->B[15]->Alg.N->group[3] == 0);
    REQUIRE(U->B[15]->Alg.P->group[3] == 3);

    CHECK(U->B[15]->getBlock(PREV, 0) == U->B[14]);
    CHECK(U->B[15]->getBlock(PREV, 1) == U->B[13]);
    CHECK(U->B[15]->getBlock(PREV, 2) == U->B[12]);

    CHECK(U->B[15]->Alg.P->group[0] == 1);
    CHECK(U->B[15]->Alg.P->group[1] == 2);
    CHECK(U->B[15]->Alg.P->group[2] == 3);

    REQUIRE(U->B[12]->Alg.N->group[3] == 3);
    REQUIRE(U->B[12]->Alg.P->group[3] == 0);

    CHECK(U->B[12]->getBlock(NEXT, 0) == U->B[13]);
    CHECK(U->B[12]->getBlock(NEXT, 1) == U->B[14]);
    CHECK(U->B[12]->getBlock(NEXT, 2) == U->B[15]);

    CHECK(U->B[12]->Alg.N->group[0] == 1);
    CHECK(U->B[12]->Alg.N->group[1] == 2);
    CHECK(U->B[12]->Alg.N->group[2] == 3);

    U->B[13]->reverse();
    U->B[12]->reverse();
    U->B[12]->AlgorSearch(0);
    U->B[15]->AlgorSearch(0);
    Algorithm::print_block_debug(U->B[12]);
    Algorithm::print_block_debug(U->B[15]);

    REQUIRE(U->B[15]->Alg.N->group[3] == 0);
    REQUIRE(U->B[15]->Alg.P->group[3] == 3);

    CHECK(U->B[15]->getBlock(PREV, 0) == U->B[14]);
    CHECK(U->B[15]->getBlock(PREV, 1) == U->B[13]);
    CHECK(U->B[15]->getBlock(PREV, 2) == U->B[12]);

    CHECK(U->B[15]->Alg.P->group[0] == 1);
    CHECK(U->B[15]->Alg.P->group[1] == 2);
    
    REQUIRE(U->B[12]->Alg.N->group[3] == 0);
    REQUIRE(U->B[12]->Alg.P->group[3] == 3);

    CHECK(U->B[12]->getBlock(PREV, 0) == U->B[13]);
    CHECK(U->B[12]->getBlock(PREV, 1) == U->B[14]);
    CHECK(U->B[12]->getBlock(PREV, 2) == U->B[15]);

    CHECK(U->B[12]->Alg.P->group[0] == 1);
    CHECK(U->B[12]->Alg.P->group[1] == 2);
    CHECK(U->B[12]->Alg.P->group[2] == 3);

    U->B[14]->reverse();
    U->B[15]->reverse();
    U->B[15]->AlgorSearch(0);
    Algorithm::print_block_debug(U->B[15]);

    REQUIRE(U->B[15]->Alg.P->group[3] == 0);
    REQUIRE(U->B[15]->Alg.N->group[3] == 3);

    CHECK(U->B[15]->getBlock(NEXT, 0) == U->B[14]);
    CHECK(U->B[15]->getBlock(NEXT, 1) == U->B[13]);
    CHECK(U->B[15]->getBlock(NEXT, 2) == U->B[12]);

    CHECK(U->B[15]->Alg.N->group[0] == 1);
    CHECK(U->B[15]->Alg.N->group[1] == 2);
    CHECK(U->B[15]->Alg.N->group[2] == 3);
    
    REQUIRE(U->B[12]->Alg.N->group[3] == 0);
    REQUIRE(U->B[12]->Alg.P->group[3] == 3);

    CHECK(U->B[12]->getBlock(PREV, 0) == U->B[13]);
    CHECK(U->B[12]->getBlock(PREV, 1) == U->B[14]);
    CHECK(U->B[12]->getBlock(PREV, 2) == U->B[15]);

    CHECK(U->B[12]->Alg.P->group[0] == 1);
    CHECK(U->B[12]->Alg.P->group[1] == 2);
    CHECK(U->B[12]->Alg.P->group[2] == 3);
  }

  SECTION("IV - Blocks change direction"){
    U->B[47]->AlgorSearch(0);
    Algorithm::print_block_debug(U->B[47]);

    CHECK(U->B[47]->Alg.N->group[3] == 0);
    CHECK(U->B[47]->Alg.P->group[3] == 4);

    CHECK(U->B[47]->getBlock(PREV, 0) == U->B[46]);
    CHECK(U->B[47]->getBlock(PREV, 1) == U->B[45]);
    CHECK(U->B[47]->getBlock(PREV, 2) == U->B[44]);
    CHECK(U->B[47]->getBlock(PREV, 3) == U->B[43]);

    CHECK(U->B[47]->Alg.P->group[0] == 1);
    CHECK(U->B[47]->Alg.P->group[1] == 2);
    CHECK(U->B[47]->Alg.P->group[2] == 3);

    U->B[43]->reverse();
    U->B[44]->reverse();
    U->B[47]->AlgorSearch(0);
    Algorithm::print_block_debug(U->B[47]);

    CHECK(U->B[47]->Alg.N->group[3] == 0);
    CHECK(U->B[47]->Alg.P->group[3] == 4);

    CHECK(U->B[47]->getBlock(PREV, 0) == U->B[46]);
    CHECK(U->B[47]->getBlock(PREV, 1) == U->B[45]);
    CHECK(U->B[47]->getBlock(PREV, 2) == U->B[44]);
    CHECK(U->B[47]->getBlock(PREV, 3) == U->B[43]);

    CHECK(U->B[47]->Alg.P->group[0] == 1);
    CHECK(U->B[47]->Alg.P->group[1] == 2);
    CHECK(U->B[47]->Alg.P->group[2] == 3);

    U->B[45]->reverse();
    U->B[47]->AlgorSearch(0);
    Algorithm::print_block_debug(U->B[47]);

    CHECK(U->B[47]->Alg.N->group[3] == 0);
    CHECK(U->B[47]->Alg.P->group[3] == 4);

    CHECK(U->B[47]->getBlock(PREV, 0) == U->B[46]);
    CHECK(U->B[47]->getBlock(PREV, 1) == U->B[45]);
    CHECK(U->B[47]->getBlock(PREV, 2) == U->B[44]);
    CHECK(U->B[47]->getBlock(PREV, 3) == U->B[43]);

    CHECK(U->B[47]->Alg.P->group[0] == 1);
    CHECK(U->B[47]->Alg.P->group[1] == 2);
    CHECK(U->B[47]->Alg.P->group[2] == 3);

    U->B[46]->reverse();
    U->B[47]->reverse();
    U->B[47]->AlgorSearch(0);
    Algorithm::print_block_debug(U->B[47]);

    CHECK(U->B[47]->Alg.P->group[3] == 0);
    CHECK(U->B[47]->Alg.N->group[3] == 4);

    CHECK(U->B[47]->getBlock(NEXT, 0) == U->B[46]);
    CHECK(U->B[47]->getBlock(NEXT, 1) == U->B[45]);
    CHECK(U->B[47]->getBlock(NEXT, 2) == U->B[44]);
    CHECK(U->B[47]->getBlock(NEXT, 3) == U->B[43]);

    CHECK(U->B[47]->Alg.N->group[0] == 1);
    CHECK(U->B[47]->Alg.N->group[1] == 2);
    CHECK(U->B[47]->Alg.N->group[2] == 3);
  }

  SECTION("V - Blocks and switch"){
    U->B[16]->AlgorSearch(0);
    U->B[18]->AlgorSearch(0);
    U->B[19]->AlgorSearch(0);
    U->B[48]->AlgorSearch(0);
    Algorithm::print_block_debug(U->B[16]);
    Algorithm::print_block_debug(U->B[18]);
    Algorithm::print_block_debug(U->B[19]);
    Algorithm::print_block_debug(U->B[48]);

    REQUIRE(U->B[16]->Alg.N->group[3] == 3);
    REQUIRE(U->B[16]->Alg.P->group[3] == 0);

    CHECK(U->B[16]->getBlock(NEXT, 0) == U->B[17]);
    CHECK(U->B[16]->getBlock(NEXT, 1) == U->B[18]);
    CHECK(U->B[16]->getBlock(NEXT, 2) == U->B[19]);

    REQUIRE(U->B[18]->Alg.N->group[3] == 1);
    REQUIRE(U->B[18]->Alg.P->group[3] == 2);

    CHECK(U->B[18]->getBlock(NEXT, 0) == U->B[19]);
    CHECK(U->B[18]->getBlock(PREV, 0) == U->B[17]);
    CHECK(U->B[18]->getBlock(PREV, 1) == U->B[16]);

    REQUIRE(U->B[48]->Alg.N->group[3] == 0);
    REQUIRE(U->B[48]->Alg.P->group[3] == 0);

    REQUIRE(U->B[19]->Alg.N->group[3] == 0);
    REQUIRE(U->B[19]->Alg.P->group[3] == 3);

    CHECK(U->B[19]->getBlock(PREV, 0) == U->B[18]);
    CHECK(U->B[19]->getBlock(PREV, 1) == U->B[17]);
    CHECK(U->B[19]->getBlock(PREV, 2) == U->B[16]);

    U->Sw[0]->setState(1);
    U->B[16]->AlgorSearch(0);
    
    REQUIRE(U->B[16]->Alg.N->group[3] == 2);
    REQUIRE(U->B[16]->Alg.P->group[3] == 0);

    CHECK(U->B[16]->getBlock(NEXT, 0) == U->B[17]);
    CHECK(U->B[16]->getBlock(NEXT, 1) == U->B[48]);

  }

  SECTION("VI - Blocks and msswitch"){
    U->B[20]->AlgorSearch(0);
    U->B[21]->AlgorSearch(0);
    U->B[23]->AlgorSearch(0);
    U->B[24]->AlgorSearch(0);
    Algorithm::print_block_debug(U->B[20]);
    Algorithm::print_block_debug(U->B[21]);
    Algorithm::print_block_debug(U->B[23]);
    Algorithm::print_block_debug(U->B[24]);

    REQUIRE(U->B[21]->Alg.N->group[3] == 2);
    REQUIRE(U->B[21]->Alg.P->group[3] == 0);

    REQUIRE(U->B[20]->Alg.N->group[3] == 0);
    REQUIRE(U->B[20]->Alg.P->group[3] == 0);

    CHECK(U->B[21]->getBlock(NEXT, 0) == U->B[22]);
    CHECK(U->B[21]->getBlock(NEXT, 1) == U->B[23]);

    REQUIRE(U->B[23]->Alg.N->group[3] == 0);
    REQUIRE(U->B[23]->Alg.P->group[3] == 2);

    REQUIRE(U->B[24]->Alg.N->group[3] == 0);
    REQUIRE(U->B[24]->Alg.P->group[3] == 0);

    CHECK(U->B[23]->getBlock(PREV, 0) == U->B[22]);
    CHECK(U->B[23]->getBlock(PREV, 1) == U->B[21]);
  }

  SECTION("VII - Blocks and station"){
    U->B[28]->AlgorSearch(0);
    Algorithm::print_block_debug(U->B[28]);

    REQUIRE(U->B[28]->Alg.N->group[3] == 1);
    REQUIRE(U->B[28]->Alg.P->group[3] == 3);

    CHECK(U->B[28]->Alg.P->group[0] == 2);
    CHECK(U->B[28]->Alg.P->group[1] == 3);
  }

  SECTION("VIII - Blocks and multistation"){
    U->B[34]->AlgorSearch(0);
    Algorithm::print_block_debug(U->B[34]);

    REQUIRE(U->B[34]->Alg.N->group[3] == 1);
    REQUIRE(U->B[34]->Alg.P->group[3] == 4);

    CHECK(U->B[34]->Alg.P->group[0] == 1);
    CHECK(U->B[34]->Alg.P->group[1] == 3);
  }

  SECTION("IX - Blocks and switchedstation"){
    U->B[41]->AlgorSearch(0);
    Algorithm::print_block_debug(U->B[41]);

    REQUIRE(U->B[41]->Alg.N->group[3] == 1);
    REQUIRE(U->B[41]->Alg.P->group[3] == 5);

    CHECK(U->B[41]->Alg.P->group[0] == 2);
    CHECK(U->B[41]->Alg.P->group[1] == 4);
  }
}

TEST_CASE_METHOD(TestsFixture, "Block Algorithm Stating", "[SB][SB-1][SB-1.3]" ) {
  char filenames[1][30] = {"./testconfigs/SB-1.3.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;
  switchboard::SwManager->LinkAndMap();
  pathlist_find();

  for(uint8_t i = 0; i < U->block_len; i++){
    U->B[i]->AlgorSearch(0);
  }

  /*
  // SECTION I                                             SECTION II
  //  --1.0->  --1.1->  --1.2->  --1.3->  --1.4->  --1.5-> --1.6->  --1.7->  --1.8->  1.9>  --1.10->  --1.11->
  //
  // SECTION III
  //  <-1.12- <-1.13- -1.14-> -1.15-> 
  //
  // SECTION IV
  //  <-1.43- <-1.44-  -<1.45>>  -1.46-> -1.47-> 
  //  <-1.67- <<1.68>- <<1.69>- <<1.70>- -1.71-> 
  //
  // SECTION V
  //
  //  1.16->  --1.17----> --1.18-> --1.19-> --1.48->
  //              Sw0\--> |end
  //
  // SECTION VI
  //               /MSSw1:0
  //  1.20->  -\  /
  //  1.21->  --1.22-> --1.23->
  //                \- --1.24->
  //
  // SECTION VII
  //  1.25-> -1.26-> -1.27-> -1.28-> -1.29->
  //         [       Station       ]
  //
  // SECTION VIII
  //  1.49-> -1.50-> -1.51-> -1.52-> -1.53->
  //         [        Yard         ]
  //
  // SECTION IX
  //  1.30-> -1.31-> -1.32-> -1.33-> -1.34-> -1.35->
  //         [   Station   ] [   Station   ]
  //         [           Station           ]
  //
  // SECTION X
  //                           Sw1:1\    /--End
  //                                 \  /
  //  1.36-> -1.37-> -1.38-> -1.39-> ---- -1.40-> -1.41-> -1.42->
  //         [   Station   ]              [   Station   ]
  //         [                 Station                  ]
  //
  // SECTION XI
  //
  //           [   Station   ]
  //           <-1.54- <-1.55- \
  //   <-1.56- <-1.57- <-1.58- --1.59-- <-1.60- <-1.61-
  //           [   Station   ]        \
  //   --1.62> --1.63> --1.64> ----------1.65-> --1.66>
  //           [   Station   ]
  */

  SECTION("I - Standard"){
    // Each block is a meter (which is the minium state size)
    //  the blocks will therefore be set to DANGER - CAUTION - PROCEED behind the last BLOCKED block
    U->B[4]->setDetection(1);

    Algorithm::process(U->B[4], _FORCE);
    Algorithm::print_block_debug(U->B[4]);

    REQUIRE(U->B[4]->Alg.P->group[3] > 2);

    CHECK(U->B[4]->getBlock(PREV, 0)->state == DANGER);
    CHECK(U->B[4]->getBlock(PREV, 1)->state == CAUTION);
    CHECK(U->B[4]->getBlock(PREV, 2)->state == PROCEED);

    std::vector<Block *> blocks = {U->B[0], U->B[1], U->B[2], U->B[3], U->B[4], U->B[5]};
    D_printBlockStates(std::ref(blocks));
  }

  SECTION("II - Block smaller than 1 meter"){
    // One block is smaller than a meter (which is the minium state size)
    //  an extra block is needed which will expand the state size to a meter
    //  the blocks will therefore be set to DANGER - DANGER - CAUTION - PROCEED behind the last BLOCKED block
    U->B[10]->setDetection(1);

    Algorithm::process(U->B[10], _FORCE);
    Algorithm::print_block_debug(U->B[10]);

    REQUIRE(U->B[10]->Alg.P->group[3] > 3);

    CHECK(U->B[10]->getBlock(PREV, 0)->state == DANGER);
    CHECK(U->B[10]->getBlock(PREV, 1)->state == DANGER);
    CHECK(U->B[10]->getBlock(PREV, 2)->state == CAUTION);
    CHECK(U->B[10]->getBlock(PREV, 3)->state == PROCEED);
    
    std::vector<Block *> blocks = {U->B[6], U->B[7], U->B[8], U->B[9], U->B[10], U->B[11]};
    D_printBlockStates(std::ref(blocks));
  }

  SECTION("IIb - End of line"){
    // If there is nowhere to go / end of line
    //  then the block shall be set to CAUTION
    Algorithm::process(U->B[11], _FORCE);
    CHECK(U->B[11]->state == CAUTION);
  }

  SECTION("III - Blocks change direction"){
    // The state will be set on the reverse state if the direction of the block changes
    // Create custom paths such that direction changes
    U->B[12]->path = 0;
    U->B[13]->path = 0;
    U->B[14]->path = 0;
    U->B[15]->path = 0;

    Path * P = new Path(U->B[12]);
    P->add(U->B[13], NEXT);
    P = new Path(U->B[14]);
    P->add(U->B[15], NEXT);

    REQUIRE(U->B[13]->path != U->B[14]->path);
    U->B[13]->path->reverse();

    U->B[15]->setDetection(1);
    Algorithm::process(U->B[15], _FORCE);

    CHECK(U->B[15]->state == BLOCKED);
    CHECK(U->B[14]->state == DANGER);
    CHECK(U->B[13]->reverse_state == CAUTION);
    CHECK(U->B[12]->reverse_state == PROCEED);
    
    CHECK(U->B[13]->state == PROCEED);
    CHECK(U->B[12]->state == PROCEED);
    
    std::vector<Block *> blocks = {U->B[12], U->B[13], U->B[14], U->B[15]};
    D_printBlockStates(std::ref(blocks));
  }

  SECTION("IV - Blocks change polarity"){
    // When train is in polarity group set whole group to danger.
    U->B[47]->setDetection(1);
    Algorithm::process(U->B[47], _FORCE);

    CHECK(U->B[47]->state == BLOCKED);
    CHECK(U->B[46]->state == DANGER);
    CHECK(U->B[45]->state == CAUTION);
    CHECK(U->B[44]->state == PROCEED);
    
    std::vector<Block *> blocks = {U->B[43], U->B[44], U->B[45], U->B[46], U->B[47]};
    D_printBlockStates(std::ref(blocks));

    if(!U->B[67]->next.p.p)
      U->B[67]->path->reverse();

    U->B[70]->setDetection(1);
    Algorithm::process(U->B[70], _FORCE);

    CHECK(U->B[70]->state == BLOCKED);
    CHECK(U->B[69]->state == DANGER);
    CHECK(U->B[68]->state == DANGER);
    CHECK(U->B[67]->state == CAUTION);
    
    std::vector<Block *> blocks2 = {U->B[66], U->B[67], U->B[68], U->B[69], U->B[70], U->B[71]};
    D_printBlockStates(std::ref(blocks2));

    U->B[71]->setDetection(1);
    Algorithm::process(U->B[71], _FORCE);

    U->B[70]->setDetection(0);
    Algorithm::process(U->B[70], _FORCE);
    Algorithm::process(U->B[71], _FORCE);

    CHECK(U->B[71]->state == BLOCKED);
    CHECK(U->B[70]->state == DANGER);
    CHECK(U->B[69]->state == CAUTION);
    CHECK(U->B[68]->state == PROCEED);
    CHECK(U->B[67]->state == PROCEED);
    
    D_printBlockStates(std::ref(blocks2));

  }

  SECTION("Va - Blocks and switch"){
    // A NOSTOP block expands the state
    // Or if a NOSTOP block is end of line, then set DANGER

    U->B[19]->setDetection(1);

    Algorithm::process(U->B[19], _FORCE);

    REQUIRE(U->B[19]->Alg.P->group[3] > 2);

    CHECK(U->B[19]->getBlock(PREV, 0)->state == DANGER);
    CHECK(U->B[19]->getBlock(PREV, 1)->state == DANGER);
    CHECK(U->B[19]->getBlock(PREV, 2)->state == CAUTION);
    
    std::vector<Block *> blocks = {U->B[16], U->B[17], U->B[18], U->B[19], U->B[48]};
    D_printBlockStates(std::ref(blocks));

    U->Sw[0]->setState(1);
    Algorithm::process(U->B[17], _FORCE);

    REQUIRE(U->B[17]->Alg.P->group[3] == 1);
    REQUIRE(U->B[17]->Alg.N->group[3] == 0);

    CHECK(U->B[17]->state == DANGER);
    CHECK(U->B[17]->getBlock(PREV, 0)->state == CAUTION);
    
    D_printBlockStates(std::ref(blocks));

    U->Sw[0]->state = 0; // Force update of all blocks
    U->Sw[0]->setState(1);

    Algorithm::process(U->B[16], _FORCE);
    Algorithm::process(U->B[17], _FORCE);
    Algorithm::process(U->B[18], _FORCE);

    CHECK(U->B[17]->switchWrongFeedback);
    CHECK(U->B[17]->state == DANGER);
    CHECK(U->B[16]->state == CAUTION);
    
    D_printBlockStates(std::ref(blocks));

    U->Sw[0]->feedback[0]->setInput(IO_event_High); // Set feedback right

    Algorithm::process(U->B[17], _FORCE);

    CHECK(!U->B[17]->switchWrongFeedback);
    CHECK(U->B[17]->state == DANGER);  // No blocks after, so still danger
    CHECK(U->B[16]->state == CAUTION);
    
    D_printBlockStates(std::ref(blocks));

    U->B[19]->setDetection(0);

    U->Sw[0]->setState(0);
    U->Sw[0]->feedback[0]->setInput(IO_event_Low); // Set feedback right

    Algorithm::process(U->B[16], _FORCE);
    Algorithm::process(U->B[17], _FORCE);
    Algorithm::process(U->B[18], _FORCE);
    Algorithm::process(U->B[19], _FORCE);

    CHECK(!U->B[17]->switchWrongFeedback);

    CHECK(U->B[17]->state == PROCEED);

    U->Sw[0]->Detection->switchWrongState = true;
    
    D_printBlockStates(std::ref(blocks));

    Algorithm::process(U->B[17], _FORCE);

    CHECK(U->B[17]->switchWrongState);
    CHECK(U->B[17]->state == DANGER);
    CHECK(U->B[16]->state == CAUTION);
    
    D_printBlockStates(std::ref(blocks));
  }

  
  SECTION("Vb - Blocks and switch"){
    // If path is reserved in the reversed direction set DANGER - CAUTION
    REQUIRE(U->B[19]->path);

    U->B[19]->path->reverse();
    U->B[17]->reverse();

    uint8_t indices[5] = {16, 17, 18, 19, 48};

    for(uint8_t i = 0; i < 5; i++){
      Algorithm::process(U->B[indices[i]], _FORCE);
      CHECK(U->B[indices[i]]->state == PROCEED);
      CHECK(U->B[indices[i]]->reverse_state == PROCEED);
    }

    U->B[16]->reserve(new Train(U->B[16]));

    REQUIRE(U->B[16]->reverse_state == DANGER);
    REQUIRE(U->B[17]->getNextState() == DANGER);
    
    std::vector<Block *> blocks = {U->B[16], U->B[17], U->B[18], U->B[19], U->B[48]};
    D_printBlockStates(std::ref(blocks));

    Algorithm::process(U->B[17], _FORCE);

    CHECK(U->B[17]->state == DANGER);
    CHECK(U->B[18]->state == CAUTION);
    
    D_printBlockStates(std::ref(blocks));
  }

  SECTION("VI - Blocks and msswitch"){
    U->B[23]->setDetection(1);

    Algorithm::process(U->B[23], _FORCE);

    REQUIRE(U->B[23]->Alg.P->group[3] > 1);

    CHECK(U->B[23]->getBlock(PREV, 0)->state == DANGER);
    CHECK(U->B[23]->getBlock(PREV, 1)->state == DANGER);
    // CHECK(U->B[19]->getBlock(PREV, 2)->state == CAUTION);

    loggerf(DEBUG, "set MSSw[0]");
    
    std::vector<Block *> blocks  = {U->B[21], U->B[22], U->B[23]};
    std::vector<Block *> blocks2 = {U->B[20], U->B[22], U->B[24]};
    D_printBlockStates(std::ref(blocks));
    D_printBlockStates(std::ref(blocks2));

    U->MSSw[0]->setState(1);
    Algorithm::process(U->B[22], _FORCE);

    REQUIRE(U->B[22]->Alg.P->group[3] == 1);

    CHECK(U->B[22]->state == PROCEED);

    D_printBlockStates(std::ref(blocks));
    D_printBlockStates(std::ref(blocks2));

    U->MSSw[0]->state = 0; // Force update of all blocks
    U->MSSw[0]->setState(1);

    Algorithm::process(U->B[20], _FORCE);
    Algorithm::process(U->B[21], _FORCE);
    Algorithm::process(U->B[22], _FORCE);

    CHECK(U->B[20]->state == PROCEED);
    CHECK(U->B[21]->state == CAUTION);
    CHECK(U->B[22]->state == PROCEED);
    
    D_printBlockStates(std::ref(blocks));
    D_printBlockStates(std::ref(blocks2));

    // U->Sw[0]->feedback[0]->setInput(IO_event_High); // Set feedback right TODO

    // Algorithm::process(U->B[17], _FORCE);

    // CHECK(!U->B[17]->switchWrongFeedback);
    // CHECK(U->B[17]->state == DANGER);  // No blocks after, so still danger
    // CHECK(U->B[16]->state == CAUTION);

    // U->B[19]->setDetection(0);

    // U->Sw[0]->setState(0);
    // U->Sw[0]->feedback[0]->setInput(IO_event_Low); // Set feedback right

    // Algorithm::process(U->B[16], _FORCE);
    // Algorithm::process(U->B[17], _FORCE);
    // Algorithm::process(U->B[18], _FORCE);
    // Algorithm::process(U->B[19], _FORCE);

    // CHECK(!U->B[17]->switchWrongFeedback);

    // CHECK(U->B[17]->state == PROCEED);

    // U->Sw[0]->Detection->switchWrongState = true;

    // Algorithm::process(U->B[17], _FORCE);

    // CHECK(U->B[17]->switchWrongState);
    // CHECK(U->B[17]->state == DANGER);
    // CHECK(U->B[16]->state == CAUTION);
  }

  SECTION("VII - Blocks and station"){
    U->B[28]->setDetection(1);

    Algorithm::process(U->B[28], _FORCE);
    Algorithm::print_block_debug(U->B[28]);

    REQUIRE(U->B[28]->Alg.P->group[0] == 2);
    REQUIRE(U->B[28]->Alg.P->group[3] > 2);

    CHECK(U->B[28]->getBlock(PREV, 0)->state == DANGER);
    CHECK(U->B[28]->getBlock(PREV, 1)->state == DANGER);
    CHECK(U->B[28]->getBlock(PREV, 2)->state == CAUTION);
    
    std::vector<Block *> blocks = {U->B[25], U->B[26], U->B[27], U->B[28], U->B[29]};
    D_printBlockStates(std::ref(blocks));
  }

  SECTION("VIII - Blocks and yard"){
    U->B[52]->setDetection(1);

    Algorithm::process(U->B[52], _FORCE);
    Algorithm::print_block_debug(U->B[52]);

    REQUIRE(U->B[52]->Alg.P->group[0] == 2);
    REQUIRE(U->B[52]->Alg.P->group[3] > 2);

    CHECK(U->B[52]->getBlock(PREV, 0)->state == RESTRICTED);
    CHECK(U->B[52]->getBlock(PREV, 1)->state == RESTRICTED);
    CHECK(U->B[52]->getBlock(PREV, 2)->state == CAUTION);
    
    std::vector<Block *> blocks = {U->B[49], U->B[50], U->B[51], U->B[52], U->B[53]};
    D_printBlockStates(std::ref(blocks));
  }

  SECTION("IX - Blocks and multistation"){
    U->B[34]->setDetection(1);

    Algorithm::process(U->B[34], _FORCE);
    Algorithm::print_block_debug(U->B[34]);

    REQUIRE(U->B[34]->Alg.P->group[3] > 3);

    CHECK(U->B[34]->getBlock(PREV, 0)->state == DANGER);
    CHECK(U->B[34]->getBlock(PREV, 1)->state == RESTRICTED);
    CHECK(U->B[34]->getBlock(PREV, 2)->state == RESTRICTED);
    CHECK(U->B[34]->getBlock(PREV, 3)->state == CAUTION);
    
    std::vector<Block *> blocks = {U->B[30], U->B[31], U->B[32], U->B[33], U->B[34], U->B[35]};
    D_printBlockStates(std::ref(blocks));
  }

  SECTION("X - Blocks and switchedstation"){
    REQUIRE(U->B[41]);
    REQUIRE(U->B[41]->Alg.P->group[3] > 4);
    U->B[41]->setDetection(1);

    Algorithm::process(U->B[41], _FORCE);
    Algorithm::print_block_debug(U->B[41]);

    REQUIRE(U->B[41]->Alg.P->group[3] > 4);

    CHECK(U->B[41]->state == BLOCKED);
    CHECK(U->B[41]->getBlock(PREV, 0)->state == DANGER);
    CHECK(U->B[41]->getBlock(PREV, 1)->state == DANGER);
    CHECK(U->B[41]->getBlock(PREV, 2)->state == RESTRICTED);
    CHECK(U->B[41]->getBlock(PREV, 3)->state == RESTRICTED);
    CHECK(U->B[41]->getBlock(PREV, 4)->state == CAUTION);
    
    std::vector<Block *> blocks = {U->B[36], U->B[37], U->B[38], U->B[39], U->B[40], U->B[41], U->B[42]};
    D_printBlockStates(std::ref(blocks));
  }

  SECTION("XIa - Blocks direction"){
    loggerf(ERROR, "XIa - Blocks Direction");

    // for(uint8_t i = 54; i < 67; i++)
      // Algorithm::process(U->B[i], _FORCE);
    Algorithm::process(U->B[54], _FORCE);
    Algorithm::process(U->B[57], _FORCE);
    Algorithm::process(U->B[65], _FORCE);

    CHECK(U->B[54]->state == CAUTION);
    CHECK(U->B[55]->state == CAUTION);

    CHECK(U->B[57]->state == PROCEED);
    CHECK(U->B[58]->state == PROCEED);
    CHECK(U->B[59]->state == PROCEED);

    CHECK(U->B[65]->state == PROCEED);
    CHECK(U->B[64]->state == PROCEED);
    CHECK(U->B[63]->state == PROCEED);
    
    std::vector<Block *> blocks1 = {U->B[54], U->B[55]};
    std::vector<Block *> blocks2 = {U->B[56], U->B[57], U->B[58], U->B[59], U->B[60], U->B[61]};
    std::vector<Block *> blocks3 = {U->B[62], U->B[63], U->B[64], U->B[65], U->B[65], U->B[66]};
    D_printBlockStates(std::ref(blocks1));
    D_printBlockStates(std::ref(blocks2));
    D_printBlockStates(std::ref(blocks3));

    U->Sw[2]->setState(1);

    U->B[54]->AlgorSearch(0);
    U->B[57]->AlgorSearch(0);
    Algorithm::process(U->B[54], _FORCE);
    Algorithm::process(U->B[57], _FORCE);

    CHECK(U->B[54]->state == CAUTION);
    CHECK(U->B[55]->state == CAUTION);
    CHECK(U->B[59]->state == CAUTION);

    CHECK(U->B[57]->state == PROCEED);
    CHECK(U->B[58]->state == PROCEED);

    CHECK(U->B[65]->state == PROCEED);
    CHECK(U->B[65]->reverse_state == PROCEED);
    
    D_printBlockStates(std::ref(blocks1));
    D_printBlockStates(std::ref(blocks2));
    D_printBlockStates(std::ref(blocks3));

    U->Sw[3]->setState(1);
    U->Sw[4]->setState(1);

    U->B[54]->AlgorSearch(0);
    Algorithm::process(U->B[54], _FORCE);

    CHECK(U->B[54]->state == CAUTION);
    CHECK(U->B[55]->state == CAUTION);
    CHECK(U->B[59]->state == CAUTION);
    CHECK(U->B[65]->state == PROCEED); // Wrong direction
    CHECK(U->B[65]->reverse_state == CAUTION);
    
    D_printBlockStates(std::ref(blocks1));
    D_printBlockStates(std::ref(blocks2));
    D_printBlockStates(std::ref(blocks3));

  }

  SECTION("XIb - Blocks direction"){
    REQUIRE(U->B[58]->path);

    U->B[56]->reverse(); // not reversed by path
    U->B[58]->path->reverse();
    U->B[59]->reverse();
    U->B[65]->reverse();
    U->B[66]->reverse();

    U->Sw[3]->setState(1);
    U->Sw[4]->setState(1);

    Algorithm::BlockTick();
    
    std::vector<Block *> blocks1 = {U->B[54], U->B[55]};
    std::vector<Block *> blocks2 = {U->B[56], U->B[57], U->B[58], U->B[59], U->B[60], U->B[61]};
    std::vector<Block *> blocks3 = {U->B[62], U->B[63], U->B[64], U->B[65], U->B[65], U->B[66]};
    D_printBlockStates(std::ref(blocks1));
    D_printBlockStates(std::ref(blocks2));
    D_printBlockStates(std::ref(blocks3));

    auto T = new Train(U->B[1]);

    REQUIRE(U->B[58]->getBlock(NEXT, 0) == U->B[59]);

    U->B[66]->path->reserve(T);

    U->B[65]->IOchanged = true;
    U->B[65]->algorchanged = true;
    Algorithm::process(U->B[65], _FORCE);

    // Block 66 direction is fixed
    // Therefore should block 65 be danger in reverse
    // and: - 59 in danger (because of 65 sharing NOSTOP)
    //      - 58 in caution (because 59 is in DANGER)
    //      - 57 in proceed
    //      - 56 in proceed

    CHECK(U->B[65]->state == PROCEED);
    CHECK(U->B[65]->reverse_state == DANGER);
    CHECK(U->B[59]->state == DANGER);
    CHECK(U->B[59]->reverse_state == PROCEED);

    CHECK(U->B[58]->state == CAUTION);
    CHECK(U->B[57]->state == PROCEED);
    CHECK(U->B[56]->state == PROCEED);

    D_printBlockStates(std::ref(blocks1));
    D_printBlockStates(std::ref(blocks2));
    D_printBlockStates(std::ref(blocks3));
  }
}

TEST_CASE_METHOD(TestsFixture, "Block Max Speed", "[SB][SB-1][SB-1.4]") {
  char filenames[1][30] = {"./testconfigs/SB-1.4.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  switchboard::SwManager->LinkAndMap();

  for(uint8_t i = 0; i < U->block_len; i++){
    U->B[i]->AlgorSearch(0);
  }

  /*
  //                 Sw0
  //  --1.0->  --1.1-------->  --1.2->
  //                  \______  --1.3->
  //             Sw1   \_____  --1.4->
  //
  // --1.5--> ---\ Sw2
  // --1.6--> -----1.7-----> --1.8-->
  //                Sw3 \--> --1.9-->
  //
  // --1.10--> ---\      MSSw0
  // --1.11--> -----1.12-----> --1.13-->
  //                      \--> --1.14-->
  //
  */

  // Block must have the most restrictive speed restriction.
  //   - If no restriction is set (MaxSpeed = 0) ignore.

  CHECK(U->B[0]->BlockMaxSpeed == 100);
  CHECK(U->B[0]->MaxSpeed == U->B[0]->BlockMaxSpeed);

  CHECK(U->B[1]->BlockMaxSpeed == 100);
  CHECK(U->Sw[0]->MaxSpeed[0] == 0);
  CHECK(U->Sw[0]->MaxSpeed[1] == 99);
  CHECK(U->Sw[1]->MaxSpeed[0] == 0);
  CHECK(U->Sw[1]->MaxSpeed[1] == 98);
  CHECK(U->B[1]->MaxSpeed == U->B[1]->BlockMaxSpeed);

  U->Sw[0]->setState(1);
  Algorithm::BlockTick();

  CHECK(U->B[1]->MaxSpeed == U->Sw[0]->MaxSpeed[1]);

  U->Sw[1]->setState(1);
  Algorithm::BlockTick();

  CHECK(U->B[1]->MaxSpeed == U->Sw[1]->MaxSpeed[1]);

  U->Sw[0]->setState(0);
  Algorithm::BlockTick();

  CHECK(U->B[1]->MaxSpeed == U->B[1]->BlockMaxSpeed);

  CHECK(U->B[7]->BlockMaxSpeed == 100);
  CHECK(U->Sw[2]->MaxSpeed[0] == 0);
  CHECK(U->Sw[2]->MaxSpeed[1] == 97);
  CHECK(U->Sw[3]->MaxSpeed[0] == 0);
  CHECK(U->Sw[3]->MaxSpeed[1] == 96);
  CHECK(U->B[7]->MaxSpeed == U->B[7]->BlockMaxSpeed);

  U->Sw[2]->setState(1);
  Algorithm::BlockTick();

  CHECK(U->B[7]->MaxSpeed == U->Sw[2]->MaxSpeed[1]);

  U->Sw[3]->setState(1);
  Algorithm::BlockTick();

  CHECK(U->B[7]->MaxSpeed == U->Sw[3]->MaxSpeed[1]);

  U->Sw[2]->setState(0);
  Algorithm::BlockTick();

  CHECK(U->B[7]->MaxSpeed == U->Sw[3]->MaxSpeed[1]);

  CHECK(U->B[12]->BlockMaxSpeed == 100);
  CHECK(U->MSSw[0]->stateMaxSpeed[0] == 91);
  CHECK(U->MSSw[0]->stateMaxSpeed[1] == 92);
  CHECK(U->B[12]->MaxSpeed == U->MSSw[0]->stateMaxSpeed[0]);

  U->MSSw[0]->setState(1);
  Algorithm::BlockTick();

  CHECK(U->B[12]->MaxSpeed == U->MSSw[0]->stateMaxSpeed[1]);
}

TEST_CASE_METHOD(TestsFixture, "Block Polarity", "[SB][SB-1][SB-1.5]") {
  char filenames[1][30] = {"./testconfigs/SB-1.5.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  switchboard::SwManager->LinkAndMap();

  for(uint8_t i = 0; i < U->block_len; i++){
    U->B[i]->AlgorSearch(0);
    Algorithm::print_block_debug(U->B[i]);
  }

  /*
  //  1.0->  <-1.1--
  //  1.2->  --1.3->  <<1.4>-  <<1.5>-
  //
  //  1.6->  --1.7->
  //  1.8->  --1.9->  --1.10>  --1.11>
  //
  // MSSw
  // --1.12-> -\
  // --1.13-> ---<1.14>--> --1.15->
  //                   \-- --1.16->
  //
  // <-1.17-> -\
  // --1.18-> ---<1.19>--> --1.20->
  //                   \-- <-1.21--
  */


  SECTION("I - Polarity Compare"){
    // Block 0 and 1 have oposite polarity
    CHECK(!U->B[0]->cmpPolarity(U->B[1]));
    CHECK(!U->B[1]->cmpPolarity(U->B[0]));

    // Block 2 and 3 have same polarity
    CHECK(U->B[2]->cmpPolarity(U->B[3]));
    CHECK(U->B[3]->cmpPolarity(U->B[2]));

    // Block 3 and 4 have oposite polarity
    CHECK(!U->B[3]->cmpPolarity(U->B[4]));
    CHECK(!U->B[4]->cmpPolarity(U->B[3]));

    // Block 4 and 5 have same polarity
    CHECK(U->B[4]->cmpPolarity(U->B[5]));
    CHECK(U->B[5]->cmpPolarity(U->B[4]));

    // Block 6 and 7 have same polarity
    CHECK(U->B[6]->cmpPolarity(U->B[7]));
    CHECK(U->B[7]->cmpPolarity(U->B[6]));

    // Block 9 and 10 have same polarity
    CHECK(U->B[ 9]->cmpPolarity(U->B[10]));
    CHECK(U->B[10]->cmpPolarity(U->B[ 9]));

    // MSSw0 state = 0
    // Block 13, 14 and 15 have same polarity
    CHECK(U->B[13]->cmpPolarity(U->B[14]));
    CHECK(U->B[14]->cmpPolarity(U->B[13]));
    CHECK(U->B[14]->cmpPolarity(U->B[15]));
    CHECK(U->B[15]->cmpPolarity(U->B[14]));
    
    // Block 12, 14 and 16 do not have same polarity (wrong MSSw state)
    CHECK(!U->B[12]->cmpPolarity(U->B[14]));
    CHECK(!U->B[14]->cmpPolarity(U->B[12]));
    CHECK(!U->B[14]->cmpPolarity(U->B[16]));
    CHECK(!U->B[16]->cmpPolarity(U->B[14]));

    // MSSw1 state = 0
    // Block 18, 19 and 20 have same polarity
    CHECK(U->B[18]->cmpPolarity(U->B[19]));
    CHECK(U->B[19]->cmpPolarity(U->B[18]));
    CHECK(U->B[19]->cmpPolarity(U->B[20]));
    CHECK(U->B[20]->cmpPolarity(U->B[19]));
    
    // Block 17, 19 and 21 do not have same polarity (wrong MSSw state)
    CHECK(!U->B[17]->cmpPolarity(U->B[19]));
    CHECK(!U->B[19]->cmpPolarity(U->B[17]));
    CHECK(!U->B[19]->cmpPolarity(U->B[21]));
    CHECK(!U->B[21]->cmpPolarity(U->B[19]));

    // Force MSSw states
    U->MSSw[0]->state = 1;
    U->MSSw[1]->state = 1;

    // MSSw0 state = 1
    // Block 13, 14 and 15 have same polarity but wrong MSSw state
    CHECK(!U->B[13]->cmpPolarity(U->B[14]));
    CHECK(!U->B[14]->cmpPolarity(U->B[13]));
    CHECK(!U->B[14]->cmpPolarity(U->B[15]));
    CHECK(!U->B[15]->cmpPolarity(U->B[14]));
    
    // Block 12, 14 and 16 have same polarity
    CHECK(U->B[12]->cmpPolarity(U->B[14]));
    CHECK(U->B[14]->cmpPolarity(U->B[12]));
    CHECK(U->B[14]->cmpPolarity(U->B[16]));
    CHECK(U->B[16]->cmpPolarity(U->B[14]));

    // MSSw1 state = 1
    // Block 18, 19 and 20 have same polarity but wrong MSSw state
    CHECK(!U->B[18]->cmpPolarity(U->B[20]));
    CHECK(!U->B[19]->cmpPolarity(U->B[18]));
    CHECK(!U->B[19]->cmpPolarity(U->B[20]));
    CHECK(!U->B[20]->cmpPolarity(U->B[19]));
    
    // Block 17, 19 and 21 do not have same polarity
    CHECK(!U->B[17]->cmpPolarity(U->B[19]));
    CHECK(!U->B[19]->cmpPolarity(U->B[17]));
    CHECK(!U->B[19]->cmpPolarity(U->B[21]));
    CHECK(!U->B[21]->cmpPolarity(U->B[19]));
  }

  SECTION("II - Polarity flip"){
    // Block cannot flip polarity (DISABLED)
    U->B[1]->flipPolarity();
    CHECK(U->B[1]->polarity_status == POLARITY_NORMAL);

    // Block can flip polarity (NO_IO)
    //  do not reverse the direction
    CHECK(U->B[4]->Next_Block(NEXT, 1) == U->B[3]);
    U->B[4]->flipPolarity();
    CHECK(U->B[4]->polarity_status == POLARITY_REVERSED);

    CHECK(U->B[4]->next.p.B == U->B[3]);
    CHECK(U->B[4]->Next_Block(NEXT, 1) == U->B[3]);

    // Compares should still be the same
    CHECK(!U->B[4]->cmpPolarity(U->B[3]));
    CHECK(U->B[5]->cmpPolarity(U->B[4]));
    // The CheckPolarity should be different now
    CHECK(U->B[4]->checkPolarity(U->B[3]));
    CHECK(!U->B[5]->checkPolarity(U->B[4]));

    // Block can flip polarity (NO_IO)
    //  do reverse the direction
    CHECK(U->B[5]->Next_Block(NEXT, 1) == U->B[4]);
    U->B[5]->flipPolarity(true);
    CHECK(U->B[5]->polarity_status == POLARITY_REVERSED);

    CHECK(U->B[5]->next.p.B == U->B[4]);
    CHECK(U->B[5]->Next_Block(PREV, 1) == U->B[4]);

    CHECK(U->B[5]->cmpPolarity(U->B[4]));
    CHECK(U->B[5]->checkPolarity(U->B[4]));

    U->MSSw[1]->state = 1;
    
    // Block 17, 19 and 21 do not have same polarity
    CHECK(!U->B[17]->cmpPolarity(U->B[19]));
    CHECK(!U->B[19]->cmpPolarity(U->B[17]));
    CHECK(!U->B[19]->cmpPolarity(U->B[21]));
    CHECK(!U->B[21]->cmpPolarity(U->B[19]));
    CHECK(!U->B[17]->checkPolarity(U->B[19]));
    CHECK(!U->B[19]->checkPolarity(U->B[17]));
    CHECK(!U->B[19]->checkPolarity(U->B[21]));
    CHECK(!U->B[21]->checkPolarity(U->B[19]));

    U->B[19]->flipPolarity(false);
    CHECK(!U->B[17]->cmpPolarity(U->B[19]));
    CHECK(!U->B[19]->cmpPolarity(U->B[17]));
    CHECK(!U->B[19]->cmpPolarity(U->B[21]));
    CHECK(!U->B[21]->cmpPolarity(U->B[19]));
    CHECK(U->B[17]->checkPolarity(U->B[19]));
    CHECK(U->B[19]->checkPolarity(U->B[17]));
    CHECK(U->B[19]->checkPolarity(U->B[21]));
    CHECK(U->B[21]->checkPolarity(U->B[19]));
  }

  SECTION("III - Polarity group flip"){
    switchboard::SwManager->LinkAndMap();

    // Block can flip polarity (NO_IO)
    CHECK(U->B[4]->Next_Block(NEXT, 1) == U->B[3]);
    CHECK(U->B[4]->polarity_status == POLARITY_NORMAL);
    CHECK(U->B[5]->polarity_status == POLARITY_NORMAL);

    REQUIRE(U->B[4]->Polarity);
    U->B[4]->Polarity->flip();

    CHECK(U->B[4]->polarity_status == POLARITY_REVERSED);
    CHECK(U->B[5]->polarity_status == POLARITY_REVERSED);

    CHECK(U->B[4]->next.p.B == U->B[3]);
    CHECK(U->B[5]->next.p.B == U->B[4]);
    CHECK(U->B[4]->Next_Block(NEXT, 1) == U->B[3]);
    CHECK(U->B[5]->Next_Block(NEXT, 1) == U->B[4]);

    CHECK(!U->B[4]->cmpPolarity(U->B[3]));
    CHECK(U->B[5]->cmpPolarity(U->B[4]));
    CHECK(U->B[4]->checkPolarity(U->B[3]));
    CHECK(U->B[5]->checkPolarity(U->B[4]));
  }
}
