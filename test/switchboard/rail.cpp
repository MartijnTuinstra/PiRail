#include "catch.hpp"

#include "utils/mem.h"
#include "utils/logger.h"
#include "system.h"

#include "config/ModuleConfig.h"
#include "switchboard/manager.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/unit.h"

#include "algorithm/core.h"
#include "algorithm/component.h"

#include "train.h"
#include "rollingstock/railtrain.h"

void init_test(char (* filenames)[30], int nr_files);

class TestsFixture {
public:
  TestsFixture();
  void loadSwitchboard(char (* filenames)[30], int nr_files);
  void loadStock();
  ~TestsFixture();
};

TEST_CASE_METHOD(TestsFixture, "Block Link", "[SB][SB-1][SB-1.1]" ) {
  char filenames[1][30] = {"./testconfigs/SB-1.1.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->link_all();

  /*
  //  1.0->  --1.1->  --1.2->
  */

  REQUIRE(U->B[0] != 0);
  REQUIRE(U->B[1] != 0);
  REQUIRE(U->B[2] != 0);

  SECTION( "I - Block link check"){
    REQUIRE(U->B[1]->next.type == RAIL_LINK_R);
    REQUIRE(U->B[1]->next.p.B == U->B[2]);

    REQUIRE(U->B[1]->prev.type == RAIL_LINK_R);
    REQUIRE(U->B[1]->prev.p.B == U->B[0]);
  }

  SECTION( "II - NextBlock Function" ) {
    REQUIRE(U->B[1]->Next_Block(NEXT, 1) == U->B[2]);
    REQUIRE(U->B[1]->Next_Block(PREV, 1) == U->B[0]);
  }

  SECTION( "III - NextBlock Function Reversed Block" ) {
    U->B[1]->dir ^= 0b100;
    REQUIRE(U->B[1]->Next_Block(NEXT, 1) == U->B[0]);
    REQUIRE(U->B[1]->Next_Block(PREV, 1) == U->B[2]);
  }

  SECTION( "IV - NextBlock Function Reverser Block" ) {
    // Just the same as II
    U->B[1]->dir ^= 0b10;
    REQUIRE(U->B[1]->Next_Block(NEXT, 1) == U->B[2]);
    REQUIRE(U->B[1]->Next_Block(PREV, 1) == U->B[0]);
  }

  SECTION( "V - NextBlock Function Reversed Reverser Block" ) {
    U->B[1]->dir ^= 0b110;
    REQUIRE(U->B[1]->Next_Block(NEXT, 1) == U->B[0]);
    REQUIRE(U->B[1]->Next_Block(PREV, 1) == U->B[2]);
  }

  SECTION( "VI - NextBlock Function counter direction Block" ) {
    U->B[1]->dir ^= 0b1;
    REQUIRE(U->B[1]->Next_Block(NEXT, 1) == U->B[0]);
    REQUIRE(U->B[1]->Next_Block(PREV, 1) == U->B[2]);
  }

  SECTION( "VII - NextBlock Function reversed counter direction Block" ) {
    U->B[1]->dir ^= 0b101;
    REQUIRE(U->B[1]->Next_Block(NEXT, 1) == U->B[2]);
    REQUIRE(U->B[1]->Next_Block(PREV, 1) == U->B[0]);
  }


  SECTION("VIII - Double NextBlock"){
    CHECK(U->B[0]->Next_Block(NEXT, 2) == U->B[2]);
    CHECK(U->B[2]->Next_Block(PREV, 2) == U->B[0]);

    U->B[1]->dir = 2;
    U->B[2]->dir = 1;
    U->B[2]->next.module = 1;
    U->B[2]->next.id = 1;
    U->B[2]->next.type = RAIL_LINK_R;
    U->B[2]->next.p.p = U->B[1];
    U->B[2]->prev.module = 0;
    U->B[2]->prev.id = 0;
    U->B[2]->prev.type = RAIL_LINK_E;
    U->B[2]->prev.p.p = 0;

    CHECK(U->B[0]->Next_Block(NEXT, 2) == U->B[2]);
    CHECK(U->B[2]->Next_Block(PREV, 2) == U->B[0]);
  }
}

TEST_CASE_METHOD(TestsFixture, "Block Algorithm Search", "[SB][SB-1][SB-1.2]" ) {
  char filenames[1][30] = {"./testconfigs/SB-1.2.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  // logger.setlevel_stdout(INFO);

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

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
    Algorithm::print_block_debug(U->B[4]);

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

    U->B[4]->reverse();

    REQUIRE(U->B[4]->Alg.prev == 1);
    REQUIRE(U->B[4]->Alg.next == 4);

    CHECK(U->B[4]->Alg.P[0] == U->B[5]);

    CHECK(U->B[4]->Alg.N[0] == U->B[3]);
    CHECK(U->B[4]->Alg.N[1] == U->B[2]);
    CHECK(U->B[4]->Alg.N[2] == U->B[1]);
    CHECK(U->B[4]->Alg.N[3] == U->B[0]);

    CHECK(U->B[4]->Alg.next1 == 1);
    CHECK(U->B[4]->Alg.next2 == 2);
    CHECK(U->B[4]->Alg.next3 == 3);
  }

  SECTION("II - Block smaller than 1 meter"){
    U->B[10]->AlgorSearch(0);
    Algorithm::print_block_debug(U->B[10]);

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
    Algorithm::print_block_debug(U->B[15]);

    REQUIRE(U->B[15]->Alg.next == 0);
    REQUIRE(U->B[15]->Alg.prev == 3);

    CHECK(U->B[15]->Alg.P[0] == U->B[14]);

    CHECK(U->B[15]->Alg.prev1 == 1);
    CHECK(U->B[15]->Alg.prev2 == 2);
    CHECK(U->B[15]->Alg.prev3 == 3);

    U->B[13]->reverse();
    U->B[12]->reverse();
    U->B[15]->AlgorSearch(0);
    Algorithm::print_block_debug(U->B[15]);

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
    Algorithm::print_block_debug(U->B[47]);

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
    Algorithm::print_block_debug(U->B[16]);
    Algorithm::print_block_debug(U->B[18]);
    Algorithm::print_block_debug(U->B[19]);
    Algorithm::print_block_debug(U->B[48]);

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
    Algorithm::print_block_debug(U->B[20]);
    Algorithm::print_block_debug(U->B[21]);
    Algorithm::print_block_debug(U->B[23]);
    Algorithm::print_block_debug(U->B[24]);

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
    Algorithm::print_block_debug(U->B[28]);

    REQUIRE(U->B[28]->Alg.next == 1);
    REQUIRE(U->B[28]->Alg.prev == 3);

    CHECK(U->B[28]->Alg.prev1 == 2);
    CHECK(U->B[28]->Alg.prev2 == 3);
  }

  SECTION("VIII - Blocks and multistation"){
    U->B[34]->AlgorSearch(0);
    Algorithm::print_block_debug(U->B[34]);

    REQUIRE(U->B[34]->Alg.next == 1);
    REQUIRE(U->B[34]->Alg.prev == 4);

    CHECK(U->B[34]->Alg.prev1 == 1);
    CHECK(U->B[34]->Alg.prev2 == 3);
  }

  SECTION("IX - Blocks and switchedstation"){
    U->B[41]->AlgorSearch(0);
    Algorithm::print_block_debug(U->B[41]);

    REQUIRE(U->B[41]->Alg.next == 1);
    REQUIRE(U->B[41]->Alg.prev == 5);

    CHECK(U->B[41]->Alg.prev1 == 2);
    CHECK(U->B[41]->Alg.prev2 == 4);
  }
}

TEST_CASE_METHOD(TestsFixture, "Block Algorithm Stating", "[SB][SB-1][SB-1.3]" ) {
  char filenames[1][30] = {"./testconfigs/SB-1.3.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->link_all();

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
  //  <-1.43- <-1.44- <-1.45-> -1.46-> -1.47-> 
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
  */

  SECTION("I - Standard"){
    U->B[4]->setDetection(1);

    Algorithm::process(U->B[4], _FORCE);
    Algorithm::print_block_debug(U->B[4]);

    CHECK(U->B[4]->Alg.P[0]->state == DANGER);
    CHECK(U->B[4]->Alg.P[1]->state == CAUTION);
    CHECK(U->B[4]->Alg.P[2]->state == PROCEED);
  }

  SECTION("II - Block smaller than 1 meter"){
    U->B[10]->setDetection(1);

    Algorithm::process(U->B[10], _FORCE);
    Algorithm::print_block_debug(U->B[10]);

    CHECK(U->B[10]->Alg.P[0]->state == DANGER);
    CHECK(U->B[10]->Alg.P[1]->state == DANGER);
    CHECK(U->B[10]->Alg.P[2]->state == CAUTION);
    CHECK(U->B[10]->Alg.P[3]->state == PROCEED);
  }

  SECTION("IIb - End of line"){
    Algorithm::process(U->B[11], _FORCE);
    CHECK(U->B[11]->state == CAUTION);
  }

  SECTION("III - Blocks change direction"){
    U->B[15]->setDetection(1);
    Algorithm::process(U->B[15], _FORCE);

    CHECK(U->B[15]->state == BLOCKED);
    CHECK(U->B[14]->state == DANGER);
    CHECK(U->B[13]->reverse_state == CAUTION);
    CHECK(U->B[12]->reverse_state == PROCEED);
    
    CHECK(U->B[13]->state == PROCEED);
    CHECK(U->B[12]->state == PROCEED);
  }

  SECTION("IV - Blocks change direction"){
    U->B[47]->setDetection(1);
    Algorithm::process(U->B[47], _FORCE);

    CHECK(U->B[47]->state == BLOCKED);
    CHECK(U->B[46]->state == DANGER);
    CHECK(U->B[45]->state == CAUTION);
    CHECK(U->B[44]->state == PROCEED);
  }

  SECTION("V - Blocks and switch"){
    U->B[19]->setDetection(1);

    Algorithm::process(U->B[19], _FORCE);

    CHECK(U->B[19]->Alg.P[0]->state == DANGER);
    CHECK(U->B[19]->Alg.P[1]->state == DANGER);
    CHECK(U->B[19]->Alg.P[2]->state == CAUTION);

    U->Sw[0]->setState(1);
    Algorithm::process(U->B[17], _FORCE);

    REQUIRE(U->B[17]->Alg.prev == 1);

    CHECK(U->B[17]->state == DANGER);
    CHECK(U->B[17]->Alg.P[0]->state == CAUTION);

    U->Sw[0]->state = 0; // Force update of all blocks
    U->Sw[0]->setState(1);

    Algorithm::process(U->B[16], _FORCE);
    Algorithm::process(U->B[17], _FORCE);
    Algorithm::process(U->B[18], _FORCE);

    CHECK(U->B[17]->switchWrongFeedback);
    CHECK(U->B[17]->state == DANGER);
    CHECK(U->B[16]->state == CAUTION);

    U->Sw[0]->feedback[0]->setInput(IO_event_High); // Set feedback right

    Algorithm::process(U->B[17], _FORCE);

    CHECK(!U->B[17]->switchWrongFeedback);
    CHECK(U->B[17]->state == DANGER);  // No blocks after, so still danger
    CHECK(U->B[16]->state == CAUTION);

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

    Algorithm::process(U->B[17], _FORCE);

    CHECK(U->B[17]->switchWrongState);
    CHECK(U->B[17]->state == DANGER);
    CHECK(U->B[16]->state == CAUTION);
  }

  SECTION("VI - Blocks and msswitch"){
    logger.setlevel_stdout(DEBUG);

    U->B[23]->setDetection(1);

    Algorithm::process(U->B[23], _FORCE);

    CHECK(U->B[23]->Alg.P[0]->state == DANGER);
    CHECK(U->B[23]->Alg.P[1]->state == DANGER);
    // CHECK(U->B[19]->Alg.P[2]->state == CAUTION);

    loggerf(DEBUG, "set MSSw[0]");

    U->MSSw[0]->setState(1);
    Algorithm::process(U->B[22], _FORCE);

    REQUIRE(U->B[22]->Alg.prev == 1);

    CHECK(U->B[22]->state == PROCEED);

    U->MSSw[0]->state = 0; // Force update of all blocks
    U->MSSw[0]->setState(1);

    Algorithm::process(U->B[20], _FORCE);
    Algorithm::process(U->B[21], _FORCE);
    Algorithm::process(U->B[22], _FORCE);
    Algorithm::process(U->B[23], _FORCE);
    Algorithm::process(U->B[24], _FORCE);

    CHECK(U->B[20]->state == PROCEED);
    CHECK(U->B[21]->state == CAUTION);
    CHECK(U->B[22]->state == PROCEED);
    CHECK(U->B[24]->state == CAUTION);

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

    REQUIRE(U->B[28]->Alg.prev1 == 2);

    CHECK(U->B[28]->Alg.P[0]->state == DANGER);
    CHECK(U->B[28]->Alg.P[1]->state == DANGER);
    CHECK(U->B[28]->Alg.P[2]->state == CAUTION);
  }

  SECTION("VIII - Blocks and yard"){
    U->B[52]->setDetection(1);

    Algorithm::process(U->B[52], _FORCE);
    Algorithm::print_block_debug(U->B[52]);

    REQUIRE(U->B[52]->Alg.prev1 == 2);

    CHECK(U->B[52]->Alg.P[0]->state == RESTRICTED);
    CHECK(U->B[52]->Alg.P[1]->state == RESTRICTED);
    CHECK(U->B[52]->Alg.P[2]->state == CAUTION);
  }

  SECTION("IX - Blocks and multistation"){
    U->B[34]->setDetection(1);

    Algorithm::process(U->B[34], _FORCE);
    Algorithm::print_block_debug(U->B[34]);

    CHECK(U->B[34]->Alg.P[0]->state == DANGER);
    CHECK(U->B[34]->Alg.P[1]->state == DANGER);
    CHECK(U->B[34]->Alg.P[2]->state == DANGER);
    CHECK(U->B[34]->Alg.P[3]->state == CAUTION);
  }

  SECTION("X - Blocks and switchedstation"){
    U->B[41]->setDetection(1);

    Algorithm::process(U->B[41], _FORCE);
    Algorithm::print_block_debug(U->B[41]);

    CHECK(U->B[41]->state == BLOCKED);
    CHECK(U->B[41]->Alg.P[0]->state == DANGER);
    CHECK(U->B[41]->Alg.P[1]->state == DANGER);
    CHECK(U->B[41]->Alg.P[2]->state == RESTRICTED);
    CHECK(U->B[41]->Alg.P[3]->state == RESTRICTED);
    CHECK(U->B[41]->Alg.P[4]->state == CAUTION);
  }
}

TEST_CASE_METHOD(TestsFixture, "Block Max Speed", "[SB][SB-1][SB-1.4]") {
  char filenames[1][30] = {"./testconfigs/SB-1.4.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->link_all();

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