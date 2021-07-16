#include "catch.hpp"

#include "utils/mem.h"
#include "utils/logger.h"
#include "system.h"

#include "config/ModuleConfig.h"
#include "switchboard/manager.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/switchsolver.h"
#include "switchboard/msswitch.h"
#include "switchboard/unit.h"

#include "rollingstock/train.h"

#include "algorithm/core.h"
#include "algorithm/component.h"

void init_test(char (* filenames)[30], int nr_files);
class TestsFixture {
public:
  TestsFixture();
  void loadSwitchboard(char (* filenames)[30], int nr_files);
  void loadStock();
  ~TestsFixture();
};

TEST_CASE_METHOD(TestsFixture, "Switch Link", "[SB][SB-2][SB-2.1]" ) {
  char filenames[1][30] = {"./testconfigs/SB-2.1.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  switchboard::SwManager->LinkAndMap();

  /*
  //                      /- --1.3->
  //  1.0->  --1.1-> --1:0-- --1.2->
  //
  //           Sw3/- <-1.4--
  // <-1.5-- <-1.6-- <-1.7--
  //         Sw1/Sw2
  // --1.8-> --1.9-> --1.10>
  */

  REQUIRE(U->B[0] != 0);
  REQUIRE(U->B[1] != 0);
  REQUIRE(U->B[2] != 0);
  REQUIRE(U->Sw[0] != 0);

  SECTION( "I - link check"){
    REQUIRE(U->B[1]->next.type == RAIL_LINK_S);
    REQUIRE(U->B[1]->next.p.p == U->Sw[0]);

    REQUIRE(U->B[2]->prev.type == RAIL_LINK_s);
    REQUIRE(U->B[2]->prev.p.p == U->Sw[0]);

    REQUIRE(U->B[3]->prev.type == RAIL_LINK_s);
    REQUIRE(U->B[3]->prev.p.p == U->Sw[0]);
  }

  SECTION( "II - NextBlock Switch Approach" ) {
    REQUIRE(U->B[1]->Next_Block(NEXT, 1) == U->B[2]);

    U->Sw[0]->state = 1;

    REQUIRE(U->B[1]->Next_Block(NEXT, 1) == U->B[3]);
  }

  SECTION( "III - NextBlock Switch Straight" ) {
    REQUIRE(U->B[2]->Next_Block(PREV, 1) == U->B[1]);
    REQUIRE(U->B[2]->Next_Block(PREV | FL_SWITCH_CARE, 1) == U->B[1]);

    U->Sw[0]->state = 1;

    REQUIRE(U->B[2]->Next_Block(PREV, 1) == U->B[1]);
    REQUIRE(U->B[2]->Next_Block(PREV | FL_SWITCH_CARE, 1) == 0);
  }

  SECTION( "IV - NextBlock Switch Diverging" ) {
    REQUIRE(U->B[3]->Next_Block(PREV, 1) == U->B[1]);
    REQUIRE(U->B[3]->Next_Block(PREV | FL_SWITCH_CARE, 1) == 0);

    U->Sw[0]->state = 1;

    REQUIRE(U->B[3]->Next_Block(PREV, 1) == U->B[1]);
    REQUIRE(U->B[3]->Next_Block(PREV | FL_SWITCH_CARE, 1) == U->B[1]);
  }

  SECTION( "V - Search Check"){
    for(uint8_t i = 0; i < 4; i++){
      Algorithm::process(U->B[i], _FORCE);
    }

    CHECK(U->B[0]->Alg.N->group[3] == 2);
    CHECK(U->B[0]->getBlock(NEXT, 0) == U->B[1]);
    CHECK(U->B[0]->getBlock(NEXT, 1) == U->B[2]);

    U->Sw[0]->setState(1);
    CHECK(U->Sw[0]->state == 1);

    // Algor cleared
    CHECK(U->B[0]->getBlock(NEXT, 0) == nullptr);
    CHECK(U->B[2]->getBlock(PREV, 0) == nullptr);

    Algorithm::BlockTick();

    CHECK(U->B[0]->Alg.N->group[3] == 2);
    CHECK(U->B[0]->getBlock(NEXT, 0) == U->B[1]);
    CHECK(U->B[0]->getBlock(NEXT, 1) == U->B[3]);

    U->Sw[0]->setState(0);

    // Algor not cleared, no new state
    CHECK(U->B[0]->getBlock(NEXT, 0) == nullptr);
    CHECK(U->B[2]->getBlock(PREV, 0) == nullptr);
    CHECK(U->B[3]->getBlock(PREV, 0) == nullptr);

    Algorithm::BlockTick();

    CHECK(U->B[0]->getBlock(NEXT, 0));
    CHECK(U->B[2]->getBlock(PREV, 0));
    CHECK(U->B[3]->getBlock(PREV, 0) == nullptr);
  }
}


TEST_CASE_METHOD(TestsFixture, "Switch setState", "[SB][SB-2][SB-2.2]" ) {
  char filenames[1][30] = {"./testconfigs/SB-2.1.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  switchboard::SwManager->LinkAndMap();

  /*
  //           Sw0/- --1.3->
  //  1.0->  --1.1-> --1.2->
  //
  //           Sw3/- <-1.4--
  // <-1.5-- <-1.6-- <-1.7--
  //         Sw1/Sw2
  // --1.8-> --1.9-> --1.10>
  */

  REQUIRE(U->B[0] != 0);
  REQUIRE(U->B[1] != 0);
  REQUIRE(U->B[2] != 0);
  REQUIRE(U->Sw[0] != 0);
  
  Switch * Sw = U->Sw[0];

    REQUIRE(Sw->state == 0);

  SECTION( "I - No Train"){
    Sw->setState(1, false);
    CHECK(Sw->state == 1);

    Sw->setState(0, false);
    CHECK(Sw->state == 0);

    Sw->setState(1, true);
    CHECK(Sw->state == 1);

    Sw->setState(0, true);
    CHECK(Sw->state == 0);
  }

  SECTION( "II - Blocked by stationary Train"){
    U->B[1]->setDetection(1);
    Algorithm::process(U->B[1], _FORCE);

    REQUIRE(U->B[1]->train);

    // Switch is block, however train is not moving.

    Sw->setState(1, false);
    CHECK(Sw->state == 0); // Switch is blocked

    Sw->setState(1, true);
    CHECK(Sw->state == 1); // Switch is overrided

    Sw->setState(0, true);
    CHECK(Sw->state == 0);
  }

  SECTION( "IIIa - Reserved by Train"){
    U->B[0]->setDetection(1);
    Algorithm::process(U->B[0], _FORCE);

    // Force Switchsolver, train has no speed.
    SwitchSolver::solve(U->B[0]->train, U->B[0], U->B[1], U->B[1]->next, FL_SWITCH_CARE);

    REQUIRE(U->B[0]->train);
    REQUIRE(U->B[1]->reserved);
    REQUIRE(U->B[1]->isReservedBy(U->B[0]->train));

    // Switch is block, however train is not moving.

    Sw->setState(1, false);
    CHECK(Sw->state == 0); // Switch is blocked

    Sw->setState(1, true);
    CHECK(Sw->state == 1); // Switch is overrided

    Sw->setState(0, true);
    CHECK(Sw->state == 0);

    CHECK(!U->B[1]->reserved);
    CHECK(!U->B[1]->isReservedBy(U->B[0]->train));
  }

  SECTION( "IIIb - Reserved by Train"){
    U->Sw[1]->setState(1);

    logger.setlevel_stdout(TRACE);
    for(uint8_t i = 0; i < 11; i++)
      Algorithm::process(U->B[i], _DEBUG | _FORCE);

    U->B[8]->setDetection(1);
    Algorithm::process(U->B[8], _FORCE);

    REQUIRE(U->Sw[1]->state == 1);

    // Force Switchsolver, train has no speed.
    SwitchSolver::solve(U->B[8]->train, U->B[8], U->B[9], U->B[9]->next, FL_SWITCH_CARE);
    logger.setlevel_stdout(DEBUG);

    printf("Switch states: %i %i %i\n", U->Sw[1]->state, U->Sw[2]->state, U->Sw[3]->state);

    REQUIRE(U->B[8]->train);
    CHECK(U->B[9]->reserved);
    CHECK(U->B[9]->isReservedBy(U->B[8]->train));

    CHECK(U->B[6]->reserved);
    CHECK(U->B[6]->isReservedBy(U->B[8]->train));

    CHECK(U->B[7]->reserved);
    CHECK(U->B[7]->isReservedBy(U->B[8]->train));

    REQUIRE(U->Sw[1]->state == 1);
    REQUIRE(U->Sw[2]->state == 1);
    CHECK(  U->Sw[3]->state == 0);

    // Switch is block, however train is not moving.

    U->Sw[3]->setState(1, false);
    CHECK(U->Sw[3]->state == 0); // Switch is blocked

    logger.setlevel_stdout(TRACE);
    U->Sw[3]->setState(1, true);
    logger.setlevel_stdout(INFO);
    CHECK(U->Sw[3]->state == 1); // Switch is overrided

    CHECK(!U->B[7]->reserved);
    CHECK(!U->B[6]->reserved);
    CHECK(!U->B[9]->reserved);

    U->Sw[1]->setState(0, true);
    CHECK(U->Sw[1]->state == 0); // Switch is overrided

    CHECK(!U->B[9]->reserved);
    CHECK(!U->B[9]->isReservedBy(U->B[8]->train));
  }

  SECTION( "IV - Blocked by stationary Train"){
    U->B[1]->setDetection(1);
    Algorithm::process(U->B[1], _FORCE);

    REQUIRE(U->B[1]->train);

    U->B[1]->train->speed = 10;

    // Switch is block, however train is not moving.

    Sw->setState(1, false);
    CHECK(Sw->state == 0); // Switch is blocked

    Sw->setState(1, true);
    CHECK(Sw->state == 0); // Switch is overrided, but train is not stationary
  }
}