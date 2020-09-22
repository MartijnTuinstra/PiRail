#include "catch.hpp"

#include "mem.h"
#include "logger.h"
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

#include "modules.h"
#include "train.h"
#include "algorithm.h"
#include "pathfinding.h"

TEST_CASE( "Path Finding", "[PF][PF-1]"){
  logger.setlevel_stdout(DEBUG);
  init_main();

  switchboard::SwManager->clear();
  unload_rolling_Configs();
  clearAlgorithmQueue();


  char filename[30] = "./testconfigs/PF-1.bin";
  switchboard::SwManager->addFile(filename);
  switchboard::SwManager->loadFiles();
  
  load_rolling_Configs("./testconfigs/stock.bin");

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

  logger.setlevel_stdout(NONE);
}
