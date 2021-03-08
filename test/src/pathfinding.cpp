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
#include "switchboard/blockconnector.h"

#include "rollingstock/railtrain.h"

#include "train.h"
#include "pathfinding.h"

void init_test(char (* filenames)[30], int nr_files);

class TestsFixture {
public:
  TestsFixture();
  void loadSwitchboard(char (* filenames)[30], int nr_files);
  void loadStock();
  ~TestsFixture();
};

TEST_CASE_METHOD(TestsFixture, "Path Finding WayPoints", "[PF][PF-1]"){
  char filenames[1][30] = {"./testconfigs/PF-1.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;
  U->link_all();

  /*
  //          1.0> --\Sw0                      Sw1/-> 1.8>
  //          1.1> ---1.2> 1.3> 1.4> 1.5> 1.6> 1.7--> 1.9>
  //
  //          /--1.29>  1.30>  1.31>  1.32>  1.33>  1.34>  1.35>  1.36>  --\
  //         |                                                              |
  //          \- <1.10 --\Sw2                            Sw3/-> <1.18 <1.37/
  //          /> 1.11> ---1.12> 1.13> 1.14> 1.15> 1.16> 1.17--> 1.19> 1.20-\
  //         |                                                              |
  //          \--1.28  <1.27  <1.26  <1.25  <1.24  <1.23  <1.22  <1.21  <--/
  //
  //
  //                   Sw4                    MSSw0  /----> 1.46>
  //     1.40> 1.41> 1.42-----> 1.43> 1.44> -----1.45-----> 1.47>
  //                     \                      /
  //           1.48> -----1.49> 1.50> 1.51> 1.52----------> 1.53>  
  //                     Sw5                  Sw6
  //
  //                     ---->
  //     1.54> 1.55--->  1.56>
  //                \-> <1.57
  //                    <----
  //
  //
  */

  SECTION("I - Find Simple blocks"){
    auto route = PathFinding::find(U->B[3], U->B[6]);

    CHECK(route->found_forward);
    CHECK(!route->found_reverse);

    delete route;
  }

  SECTION("II - Find Simple Switch S side"){
    auto route = PathFinding::find(U->B[3], U->B[9]);

    CHECK(route->found_forward);
    CHECK(!route->found_reverse);

    CHECK(route->Sw_S[U->Sw[1]->uid]);

    delete route;
  }

  SECTION("III - Find Simple Switch s side"){
    auto route = PathFinding::find(U->B[1], U->B[6]);
    // struct paths path = pathfinding(U->B[3], U->B[6]);

    CHECK(route->found_forward);
    CHECK(!route->found_reverse);

    CHECK(route->Sw_s[U->Sw[0]->uid]);

    delete route;
  }

  SECTION("IV - Find Circle"){
    auto route = PathFinding::find(U->B[16], U->B[13]);
    // struct paths path = pathfinding(U->B[3], U->B[6]);

    CHECK(route->found_forward);
    CHECK(route->found_reverse);

    REQUIRE(route->Sw_S[U->Sw[3]->uid]);
    CHECK(route->Sw_S[U->Sw[3]->uid]->nrOptions == 2);

    REQUIRE(route->Sw_s[U->Sw[2]->uid]);
    CHECK(route->Sw_s[U->Sw[2]->uid]->nrOptions == 1);

    delete route;
  }

  SECTION("IV - Find Circle with OneWay"){
    U->B[36]->oneWay = true;
    auto route = PathFinding::find(U->B[16], U->B[13]);
    // struct paths path = pathfinding(U->B[3], U->B[6]);

    CHECK(route->found_forward);
    CHECK(route->found_reverse);

    REQUIRE(route->Sw_S[U->Sw[3]->uid]);
    CHECK(route->Sw_S[U->Sw[3]->uid]->nrOptions == 1);

    REQUIRE(route->Sw_s[U->Sw[2]->uid]);
    CHECK(route->Sw_s[U->Sw[2]->uid]->nrOptions == 1);

    delete route;
  }

  SECTION("V - MSSwitch"){
    auto route1 = PathFinding::find(U->B[40], U->B[47]);
    auto route2 = PathFinding::find(U->B[40], U->B[46]);
    auto route3 = PathFinding::find(U->B[40], U->B[53]);

    CHECK(route1->MSSw_A[U->MSSw[0]->uid]);
    CHECK(route1->MSSw_A[U->MSSw[0]->uid]->nrOptions == 1);

    CHECK(U->MSSw[0]->sideB[0].p.B == U->B[47]);
    CHECK(U->MSSw[0]->sideB[1].p.B == U->B[46]);

    CHECK(U->MSSw[0]->sideA[0].p.B == U->B[44]);
    CHECK(U->MSSw[0]->sideA[1].p.Sw == U->Sw[6]);

    CHECK(route1->found_forward);
    CHECK(route2->found_forward);
    CHECK(route3->found_forward);

    // REQUIRE(route->Sw_S[U->Sw[3]->uid]);
    // CHECK(route->Sw_S[U->Sw[3]->uid]->nrOptions == 1);

    // REQUIRE(route->Sw_s[U->Sw[2]->uid]);
    // CHECK(route->Sw_s[U->Sw[2]->uid]->nrOptions == 1);

    delete route1;
    delete route2;
    delete route3;
  }

  SECTION("VI - One Way"){
    REQUIRE(U->B[56]->oneWay);
    REQUIRE(U->B[57]->oneWay);

    auto route1 = PathFinding::find(U->B[54], U->B[56]);
    auto route2 = PathFinding::find(U->B[54], U->B[57]);

    CHECK( route1->found_forward);
    CHECK(!route1->found_reverse);
    CHECK(!route2->found_forward);
    CHECK(!route2->found_reverse);

    delete route1;
    delete route2;
  }

  // logger.setlevel_stdout(NONE);
}

TEST_CASE_METHOD(TestsFixture, "Path Finding Stations", "[PF][PF-2]"){
  char filenames[1][30] = {"./testconfigs/PF-2.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;
  U->link_all();

  /*                |--             Station 1             --|
  //         Sw0/-> 1.2>              1.3>              1.4->
  //    1.0> 1.1--> 1.5>  1.6>  1.7>--- ----1.8->  1.9> 1.10>
  //                |- Station 2(a)-~ 1\ /2 ~- Station 2(b)-|
  //                                    X
  //                |- Station 3(a)-~ 3/ \4 ~- Station 3(b)-|
  //   1.11> 1.12>  1.13> 1.14> 1.15>--- ---1.16> 1.17> 1.18>
  //
  */
  
  logger.setlevel_stdout(INFO);

  SECTION("I - Find Route Station 1"){
    auto route = PathFinding::find(U->B[0], U->St[0]); // Station 1

    CHECK(route->found_forward);
    CHECK(!route->found_reverse);

    delete route;
  }

  SECTION("II - Find Route Station 2"){
    auto route0  = PathFinding::find(U->B[0], U->St[1]);  // Station 2
    auto route0A = PathFinding::find(U->B[0], U->St[2]); // Station 2a
    auto route0B = PathFinding::find(U->B[0], U->St[3]); // Station 2b
    
    auto route11  = PathFinding::find(U->B[11], U->St[1]);  // Station 2
    auto route11A = PathFinding::find(U->B[11], U->St[2]); // Station 2a
    auto route11B = PathFinding::find(U->B[11], U->St[3]); // Station 2b

    REQUIRE(route0);
    REQUIRE(route0A);
    REQUIRE(route0B);

    REQUIRE(route11);
    REQUIRE(route11A);
    REQUIRE(route11B);

    CHECK(route0->found_forward);
    CHECK(route0A->found_forward);
    CHECK(route0B->found_forward);

    CHECK(!route11->found_forward);
    CHECK(!route11A->found_forward);
    CHECK( route11B->found_forward);

    CHECK(!route0->found_reverse);
    CHECK(!route0A->found_reverse);
    CHECK(!route0B->found_reverse);

    CHECK(!route11->found_reverse);
    CHECK(!route11A->found_reverse);
    CHECK(!route11B->found_reverse);

    delete route0;
    delete route0A;
    delete route0B;
    delete route11;
    delete route11A;
    delete route11B;
  }

  // logger.setlevel_stdout(NONE);
}
