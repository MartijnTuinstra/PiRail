#include "catch.hpp"

#include "mem.h"
#include "logger.h"
#include "system.h"

#include "config/ModuleConfig.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/unit.h"

#include "algorithm.h"

#include "rollingstock/railtrain.h"

TEST_CASE( "Block Link", "[SB-1.1]" ) {
  set_level(NONE);
  set_logger_print_level(NONE);

  Units = (Unit **)_calloc(30, Unit *);
  unit_len = 30;

  char filename[30] = "./testconfigs/SB-1.1.bin";
  auto config = ModuleConfig(filename);
  config.read();

  REQUIRE(config.parsed);

  new Unit(&config);
  link_all_blocks(Units[1]);
  Unit * U = Units[1];

  /*
  //  1.0->  --1.1->  --1.2->
  */

  REQUIRE(U->B[0] != 0);
  REQUIRE(U->B[1] != 0);
  REQUIRE(U->B[2] != 0);

  SECTION( "Block link check"){
    REQUIRE(U->B[1]->next.type == RAIL_LINK_R);
    REQUIRE(U->B[1]->next.p.B == U->B[2]);

    REQUIRE(U->B[1]->prev.type == RAIL_LINK_R);
    REQUIRE(U->B[1]->prev.p.B == U->B[0]);
  }

  SECTION( "NextBlock Function" ) {
    REQUIRE(U->B[1]->_Next(NEXT, 1) == U->B[2]);
    REQUIRE(U->B[1]->_Next(PREV, 1) == U->B[0]);
  }

  SECTION( "NextBlock Function Reversed Block" ) {
    U->B[1]->dir ^= 0b100;
    REQUIRE(U->B[1]->_Next(PREV, 1) == U->B[2]);
    REQUIRE(U->B[1]->_Next(NEXT, 1) == U->B[0]);
  }

  SECTION( "NextBlock Function Reverser Block" ) {
    U->B[1]->dir ^= 0b10;
    REQUIRE(U->B[1]->_Next(NEXT, 1) == U->B[2]);
    REQUIRE(U->B[1]->_Next(PREV, 1) == U->B[0]);
  }

  SECTION( "NextBlock Function Reversed Reverser Block" ) {
    U->B[1]->dir ^= 0b110;
    REQUIRE(U->B[1]->_Next(PREV, 1) == U->B[2]);
    REQUIRE(U->B[1]->_Next(NEXT, 1) == U->B[0]);
  }

  SECTION( "NextBlock Function counter direction Block" ) {
    U->B[1]->dir ^= 0b1;
    REQUIRE(U->B[1]->_Next(PREV, 1) == U->B[2]);
    REQUIRE(U->B[1]->_Next(NEXT, 1) == U->B[0]);
  }

  SECTION( "NextBlock Function reversed counter direction Block" ) {
    U->B[1]->dir ^= 0b101;
    REQUIRE(U->B[1]->_Next(NEXT, 1) == U->B[2]);
    REQUIRE(U->B[1]->_Next(PREV, 1) == U->B[0]);
  }
}


TEST_CASE( "Block Algorithm Search", "[SB-1.2]" ) {
  set_level(NONE);
  set_logger_print_level(NONE);

  Units = (Unit **)_calloc(30, Unit *);
  unit_len = 30;

  char filename[30] = "./testconfigs/SB-1.2.bin";
  auto config = ModuleConfig(filename);
  config.read();

  REQUIRE(config.parsed);
  pathlist.clear();

  new Unit(&config);
  Unit * U = Units[1];

  U->link_all();

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
  //  1.16->  --1.17-> ---- --1.18-> --1.48->
  //                     \- --1.19->
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
    Algor_print_block_debug(U->B[4]);

    REQUIRE(U->B[4]->Alg.next == 1);
    REQUIRE(U->B[4]->Alg.prev == 4);

    CHECK(U->B[4]->Alg.N[0] == U->B[5]);

    CHECK(U->B[4]->Alg.P[0] == U->B[3]);
    CHECK(U->B[4]->Alg.P[1] == U->B[2]);
    CHECK(U->B[4]->Alg.P[2] == U->B[1]);
    CHECK(U->B[4]->Alg.P[3] == U->B[0]);

    CHECK(U->B[4]->Alg.prev1 == 1);
    CHECK(U->B[4]->Alg.prev2 == 2);
    CHECK(U->B[4]->Alg.prev3 == 3);
  }

  SECTION("II - Block smaller than 1 meter"){
    U->B[10]->AlgorSearch(0);
    Algor_print_block_debug(U->B[10]);

    REQUIRE(U->B[10]->Alg.next == 1);
    REQUIRE(U->B[10]->Alg.prev == 4);

    CHECK(U->B[10]->Alg.N[0] == U->B[11]);

    CHECK(U->B[10]->Alg.P[0] == U->B[9]);
    CHECK(U->B[10]->Alg.P[1] == U->B[8]);
    CHECK(U->B[10]->Alg.P[2] == U->B[7]);
    CHECK(U->B[10]->Alg.P[3] == U->B[6]);

    CHECK(U->B[10]->Alg.prev1 == 2);
    CHECK(U->B[10]->Alg.prev2 == 3);
    CHECK(U->B[10]->Alg.prev3 == 4);
  }

  SECTION("III - Blocks change direction"){
    U->B[15]->AlgorSearch(0);
    Algor_print_block_debug(U->B[15]);

    REQUIRE(U->B[15]->Alg.next == 0);
    REQUIRE(U->B[15]->Alg.prev == 1);

    CHECK(U->B[15]->Alg.P[0] == U->B[14]);

    CHECK(U->B[15]->Alg.prev1 == 1);

    U->B[13]->reverse();
    U->B[12]->reverse();
    U->B[15]->AlgorSearch(0);
    Algor_print_block_debug(U->B[15]);

    REQUIRE(U->B[15]->Alg.next == 0);
    REQUIRE(U->B[15]->Alg.prev == 3);

    CHECK(U->B[15]->Alg.P[0] == U->B[14]);
    CHECK(U->B[15]->Alg.P[1] == U->B[13]);
    CHECK(U->B[15]->Alg.P[2] == U->B[12]);

    CHECK(U->B[15]->Alg.prev1 == 1);
    CHECK(U->B[15]->Alg.prev2 == 2);
  }

  SECTION("III - Blocks change direction"){
    U->B[47]->AlgorSearch(0);
    Algor_print_block_debug(U->B[47]);

    REQUIRE(U->B[47]->Alg.next == 0);
    REQUIRE(U->B[47]->Alg.prev == 4);

    CHECK(U->B[47]->Alg.P[0] == U->B[46]);
    CHECK(U->B[47]->Alg.P[1] == U->B[45]);
    CHECK(U->B[47]->Alg.P[2] == U->B[44]);
    CHECK(U->B[47]->Alg.P[3] == U->B[43]);

    CHECK(U->B[47]->Alg.prev1 == 1);
    CHECK(U->B[47]->Alg.prev2 == 2);
    CHECK(U->B[47]->Alg.prev3 == 3);
  }

  SECTION("V - Blocks and switch"){
    U->B[16]->AlgorSearch(0);
    U->B[18]->AlgorSearch(0);
    U->B[19]->AlgorSearch(0);
    U->B[48]->AlgorSearch(0);
    Algor_print_block_debug(U->B[16]);
    Algor_print_block_debug(U->B[18]);
    Algor_print_block_debug(U->B[19]);
    Algor_print_block_debug(U->B[48]);

    REQUIRE(U->B[16]->Alg.next == 3);
    REQUIRE(U->B[16]->Alg.prev == 0);

    CHECK(U->B[16]->Alg.N[0] == U->B[17]);
    CHECK(U->B[16]->Alg.N[1] == U->B[18]);
    CHECK(U->B[16]->Alg.N[2] == U->B[48]);

    REQUIRE(U->B[18]->Alg.next == 1);
    REQUIRE(U->B[18]->Alg.prev == 2);

    CHECK(U->B[18]->Alg.N[0] == U->B[48]);
    CHECK(U->B[18]->Alg.P[0] == U->B[17]);
    CHECK(U->B[18]->Alg.P[1] == U->B[16]);

    REQUIRE(U->B[19]->Alg.next == 0);
    REQUIRE(U->B[19]->Alg.prev == 0);

    REQUIRE(U->B[48]->Alg.next == 0);
    REQUIRE(U->B[48]->Alg.prev == 3);

    CHECK(U->B[48]->Alg.P[0] == U->B[18]);
    CHECK(U->B[48]->Alg.P[1] == U->B[17]);
    CHECK(U->B[48]->Alg.P[2] == U->B[16]);
  }

  SECTION("VI - Blocks and msswitch"){
    U->B[20]->AlgorSearch(0);
    U->B[21]->AlgorSearch(0);
    U->B[23]->AlgorSearch(0);
    U->B[24]->AlgorSearch(0);
    Algor_print_block_debug(U->B[20]);
    Algor_print_block_debug(U->B[21]);
    Algor_print_block_debug(U->B[23]);
    Algor_print_block_debug(U->B[24]);

    REQUIRE(U->B[21]->Alg.next == 2);
    REQUIRE(U->B[21]->Alg.prev == 0);

    REQUIRE(U->B[20]->Alg.next == 0);
    REQUIRE(U->B[20]->Alg.prev == 0);

    CHECK(U->B[21]->Alg.N[0] == U->B[22]);
    CHECK(U->B[21]->Alg.N[1] == U->B[23]);

    REQUIRE(U->B[23]->Alg.next == 0);
    REQUIRE(U->B[23]->Alg.prev == 2);

    REQUIRE(U->B[24]->Alg.next == 0);
    REQUIRE(U->B[24]->Alg.prev == 0);

    CHECK(U->B[23]->Alg.P[0] == U->B[22]);
    CHECK(U->B[23]->Alg.P[1] == U->B[21]);
  }

  SECTION("VII - Blocks and station"){
    U->B[28]->AlgorSearch(0);
    Algor_print_block_debug(U->B[28]);

    REQUIRE(U->B[28]->Alg.next == 1);
    REQUIRE(U->B[28]->Alg.prev == 3);

    CHECK(U->B[28]->Alg.prev1 == 2);
    CHECK(U->B[28]->Alg.prev2 == 3);
  }

  SECTION("VIII - Blocks and multistation"){
    U->B[34]->AlgorSearch(0);
    Algor_print_block_debug(U->B[34]);

    REQUIRE(U->B[34]->Alg.next == 1);
    REQUIRE(U->B[34]->Alg.prev == 4);

    CHECK(U->B[34]->Alg.prev1 == 1);
    CHECK(U->B[34]->Alg.prev2 == 3);
  }

  SECTION("IX - Blocks and switchedstation"){
    U->B[41]->AlgorSearch(0);
    Algor_print_block_debug(U->B[41]);

    REQUIRE(U->B[41]->Alg.next == 1);
    REQUIRE(U->B[41]->Alg.prev == 5);

    CHECK(U->B[41]->Alg.prev1 == 2);
    CHECK(U->B[41]->Alg.prev2 == 4);
  }
}

TEST_CASE( "Block Algorithm Stating", "[SB-1.3]" ) {
  init_main();
  set_level(NONE);
  set_logger_print_level(NONE);

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
  */

  SECTION("I - Standard"){
    U->B[4]->blocked = 1;

    Algor_process(U->B[4], _FORCE);
    Algor_print_block_debug(U->B[4]);

    CHECK(U->B[4]->Alg.P[0]->state == DANGER);
    CHECK(U->B[4]->Alg.P[1]->state == CAUTION);
    CHECK(U->B[4]->Alg.P[2]->state == PROCEED);
  }

  SECTION("II - Block smaller than 1 meter"){
    U->B[10]->blocked = 1;

    Algor_process(U->B[10], _FORCE);
    Algor_print_block_debug(U->B[10]);

    CHECK(U->B[10]->Alg.P[0]->state == DANGER);
    CHECK(U->B[10]->Alg.P[1]->state == DANGER);
    CHECK(U->B[10]->Alg.P[2]->state == CAUTION);
    CHECK(U->B[10]->Alg.P[3]->state == PROCEED);
  }

  // SECTION("III - Blocks change direction"){
  //   U->B[15]->AlgorSearch(0);
  //   Algor_print_block_debug(U->B[15]);

  //   REQUIRE(U->B[15]->Alg.next == 0);
  //   REQUIRE(U->B[15]->Alg.prev == 1);

  //   CHECK(U->B[15]->Alg.P[0] == U->B[14]);

  //   CHECK(U->B[15]->Alg.prev1 == 1);

  //   U->B[13]->reverse();
  //   U->B[12]->reverse();
  //   U->B[15]->AlgorSearch(0);
  //   Algor_print_block_debug(U->B[15]);

  //   REQUIRE(U->B[15]->Alg.next == 0);
  //   REQUIRE(U->B[15]->Alg.prev == 3);

  //   CHECK(U->B[15]->Alg.P[0] == U->B[14]);
  //   CHECK(U->B[15]->Alg.P[1] == U->B[13]);
  //   CHECK(U->B[15]->Alg.P[2] == U->B[12]);

  //   CHECK(U->B[15]->Alg.prev1 == 1);
  //   CHECK(U->B[15]->Alg.prev2 == 2);
  // }

  // SECTION("III - Blocks change direction"){
  //   U->B[47]->AlgorSearch(0);
  //   Algor_print_block_debug(U->B[47]);

  //   REQUIRE(U->B[47]->Alg.next == 0);
  //   REQUIRE(U->B[47]->Alg.prev == 4);

  //   CHECK(U->B[47]->Alg.P[0] == U->B[46]);
  //   CHECK(U->B[47]->Alg.P[1] == U->B[45]);
  //   CHECK(U->B[47]->Alg.P[2] == U->B[44]);
  //   CHECK(U->B[47]->Alg.P[3] == U->B[43]);

  //   CHECK(U->B[47]->Alg.prev1 == 1);
  //   CHECK(U->B[47]->Alg.prev2 == 2);
  //   CHECK(U->B[47]->Alg.prev3 == 3);
  // }

  SECTION("V - Blocks and switch"){
    U->B[19]->blocked = 1;

    Algor_process(U->B[19], _FORCE);

    CHECK(U->B[19]->Alg.P[0]->state == DANGER);
    CHECK(U->B[19]->Alg.P[1]->state == DANGER);
    CHECK(U->B[19]->Alg.P[2]->state == CAUTION);

    U->Sw[0]->state = 1;
    Algor_process(U->B[17], _FORCE);

    REQUIRE(U->B[17]->Alg.prev == 1);

    CHECK(U->B[17]->state == DANGER);
    CHECK(U->B[17]->Alg.P[0]->state == CAUTION);
  }

  // SECTION("VI - Blocks and msswitch"){
  //   U->B[20]->AlgorSearch(0);
  //   U->B[21]->AlgorSearch(0);
  //   U->B[23]->AlgorSearch(0);
  //   U->B[24]->AlgorSearch(0);
  //   Algor_print_block_debug(U->B[20]);
  //   Algor_print_block_debug(U->B[21]);
  //   Algor_print_block_debug(U->B[23]);
  //   Algor_print_block_debug(U->B[24]);

  //   REQUIRE(U->B[21]->Alg.next == 2);
  //   REQUIRE(U->B[21]->Alg.prev == 0);

  //   REQUIRE(U->B[20]->Alg.next == 0);
  //   REQUIRE(U->B[20]->Alg.prev == 0);

  //   CHECK(U->B[21]->Alg.N[0] == U->B[22]);
  //   CHECK(U->B[21]->Alg.N[1] == U->B[23]);

  //   REQUIRE(U->B[23]->Alg.next == 0);
  //   REQUIRE(U->B[23]->Alg.prev == 2);

  //   REQUIRE(U->B[24]->Alg.next == 0);
  //   REQUIRE(U->B[24]->Alg.prev == 0);

  //   CHECK(U->B[23]->Alg.P[0] == U->B[22]);
  //   CHECK(U->B[23]->Alg.P[1] == U->B[21]);
  // }

  SECTION("VII - Blocks and station"){
    U->B[28]->blocked = 1;

    Algor_process(U->B[28], _FORCE);
    Algor_print_block_debug(U->B[28]);

    REQUIRE(U->B[28]->Alg.prev1 == 2);

    CHECK(U->B[28]->Alg.P[0]->state == DANGER);
    CHECK(U->B[28]->Alg.P[1]->state == DANGER);
    CHECK(U->B[28]->Alg.P[2]->state == CAUTION);
  }

  SECTION("VIII - Blocks and yard"){
    U->B[52]->blocked = 1;

    Algor_process(U->B[52], _FORCE);
    Algor_print_block_debug(U->B[52]);

    REQUIRE(U->B[52]->Alg.prev1 == 2);

    CHECK(U->B[52]->Alg.P[0]->state == RESTRICTED);
    CHECK(U->B[52]->Alg.P[1]->state == RESTRICTED);
    CHECK(U->B[52]->Alg.P[2]->state == CAUTION);
  }

  SECTION("IX - Blocks and multistation"){
    U->B[34]->blocked = 1;

    Algor_process(U->B[34], _FORCE);
    Algor_print_block_debug(U->B[34]);

    CHECK(U->B[34]->Alg.P[0]->state == DANGER);
    CHECK(U->B[34]->Alg.P[1]->state == DANGER);
    CHECK(U->B[34]->Alg.P[2]->state == DANGER);
    CHECK(U->B[34]->Alg.P[3]->state == CAUTION);
  }

  SECTION("X - Blocks and switchedstation"){
    U->B[41]->blocked = 1;

    Algor_process(U->B[41], _FORCE);
    Algor_print_block_debug(U->B[41]);

    CHECK(U->B[41]->Alg.P[0]->state == DANGER);
    CHECK(U->B[41]->Alg.P[1]->state == DANGER);
    CHECK(U->B[41]->Alg.P[2]->state == RESTRICTED);
    CHECK(U->B[41]->Alg.P[3]->state == RESTRICTED);
    CHECK(U->B[41]->Alg.P[4]->state == CAUTION);
  }
}