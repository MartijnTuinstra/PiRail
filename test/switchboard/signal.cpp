#include "catch.hpp"

#include "utils/mem.h"
#include "utils/logger.h"
#include "system.h"

#include "config/ModuleConfig.h"
#include "switchboard/manager.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/signals.h"
#include "switchboard/unit.h"
#include "switchboard/blockconnector.h"

#include "algorithm/blockconnector.h"
#include "sim.h"
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

TEST_CASE_METHOD(TestsFixture, "Signal 1", "[SB][SB-4][SB-4.1]" ) {
  char filenames[1][30] = {"./testconfigs/SB-4.1.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->link_all();

  /*       o>        <o
  //       |          |
  //  1.0->  --1.1->  --1.2->  --1.3->
  //                  
  //
  //       o>
  //       |
  //  1.4->  ---------1.5->  --1.6->
  //       o>     /
  //       |     /
  //  1.7->  ---/
  //           /
  //      |  -'
  */

  // The signal shall show the state of the protected block.
  //  - when no path is available it must show danger.

  REQUIRE(U->B[1]->forward_signal != 0);
  REQUIRE(U->B[1]->reverse_signal != 0);

  REQUIRE(U->Sig[0]->B == U->B[1]);
  REQUIRE(U->Sig[1]->B == U->B[1]);

  REQUIRE(U->Sig[2]->B == U->B[5]);
  REQUIRE(U->Sig[3]->B == U->B[5]);

  loggerf(ERROR, "Starting Test");

  SECTION( "I - Signal State Check"){
    U->B[1]->setState(DANGER);

    CHECK(U->Sig[0]->state == DANGER);

    U->B[1]->setState(CAUTION);
    U->B[1]->setReversedState(DANGER);

    CHECK(U->Sig[0]->state == CAUTION);
    CHECK(U->Sig[1]->state == DANGER);
  }

  SECTION( "II - Signal On Switch State Check"){
    U->B[5]->setState(CAUTION); // FIXME

    CHECK(U->Sig[2]->state == CAUTION);
    CHECK(U->Sig[3]->state == CAUTION);
    CHECK(U->Sig[3]->switchDanger == true);

    U->Sw[0]->updateState(DIVERGING_SWITCH);

    CHECK(U->Sig[2]->switchDanger == true);
    CHECK(U->Sig[3]->switchDanger == true);

    U->Sw[1]->updateState(DIVERGING_SWITCH);

    CHECK(U->Sig[3]->switchDanger == false);
  }
}

TEST_CASE_METHOD(TestsFixture, "Signal 2", "[SB][SB-4][SB-4.2]" ) {

  char filenames[2][30] = {"./testconfigs/SB-4.2-1.bin", "./testconfigs/SB-4.2-2.bin"};
  loadSwitchboard(filenames, 2);
  loadStock();

  Unit * U1 = switchboard::Units(1);
  Unit * U2 = switchboard::Units(2);

  REQUIRE(U1);
  REQUIRE(U2);
  
  /*       o>
  //       |
  //  1.0->  | --2.0->
  //     C1-1 C1-1                
  //
  //       o>
  //       |     C1-2
  //  1.1->   |  ----------- --2.1->
  //          |  -2.2-> --'
  //     C1-2
  */

  // Signals could be placed on different modules. 
  // The protected block is assigned after the modules are connected.

  U1->on_layout = 1;
  U2->on_layout = 1;

  auto connectors = Algorithm::find_connectors();

  loggerf(INFO, "Have %i connectors", connectors.size());

  U1->B[0]->setDetection(1);
  U2->B[0]->setDetection(1);

  if(uint8_t * findResult = Algorithm::find_connectable(&connectors))
    Algorithm::connect_connectors(&connectors, findResult);

  loggerf(INFO, "Have %i connectors", connectors.size());

  link_all_blocks(U1);
  link_all_blocks(U2);

  U1->B[0]->setDetection(0);
  U2->B[0]->setDetection(0);

  REQUIRE(connectors.size() == 0);

  REQUIRE(U1->B[0]->next.p.B == U2->B[0]);
  REQUIRE(U1->B[1]->next.p.Sw == U2->Sw[0]);

  REQUIRE(U2->B[0]->forward_signal->size() != 0);

  REQUIRE(U2->B[1]->forward_signal->size() != 0);
  REQUIRE(U2->Sw[0]->Signals.size() != 0);
  REQUIRE(U1->Sig[1]->Switches.size() != 0);

  loggerf(ERROR, "Starting Test");

  SECTION( "I - Signal State Check"){
    U2->B[0]->setState(DANGER);

    CHECK(U1->Sig[0]->state == DANGER);

    U2->B[0]->setState(CAUTION);

    CHECK(U1->Sig[0]->state == CAUTION);
  }

  SECTION( "II - Signal On Switch State Check"){
    U2->B[1]->setState(CAUTION);

    CHECK(U1->Sig[1]->state == CAUTION);
    CHECK(U1->Sig[1]->switchDanger == false);

    U2->Sw[0]->updateState(DIVERGING_SWITCH);

    CHECK(U1->Sig[1]->switchDanger == true);
  }
}
