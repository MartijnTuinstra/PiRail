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
  //  1.0> --\Sw0                      Sw1/-> 1.8>
  //  1.1> ---1.2> 1.3> 1.4> 1.5> 1.6> 1.7--> 1.9>
  */

  SECTION("I - Find Simple blocks"){
    struct PathFinding::step s = PathFinding::find(U->B[3], U->B[6]);
    // struct paths path = pathfinding(U->B[3], U->B[6]);

    CHECK(s.found);
    CHECK(!s.next);
  }

  SECTION("II - Find Simple Switch S side"){
    struct PathFinding::step s = PathFinding::find(U->B[3], U->B[9]);
    // struct paths path = pathfinding(U->B[3], U->B[6]);

    CHECK(s.found);
    CHECK(s.next);

    loggerf(INFO, "s.next: %x", (unsigned int)s.next);

    CHECK(s.next->nrOptions == 1);
    CHECK(s.next->options[0] == 0);
    CHECK(s.next->length[0] == U->B[9]->length);
    CHECK(!s.next[0]->next);
  }

  logger.setlevel_stdout(NONE);
}
