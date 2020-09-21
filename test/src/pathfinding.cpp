#include "catch.hpp"

#include "mem.h"
#include "logger.h"
#include "system.h"

#include "config/ModuleConfig.h"

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
  
  unload_module_Configs();
  unload_rolling_Configs();
  clearAlgorithmQueue();

  Units = (Unit **)_calloc(30, Unit *);
  unit_len = 30;

  init_main();

  char filename[30] = "./testconfigs/PF-1.bin";
  ModuleConfig config = ModuleConfig(filename);

  config.read();

  REQUIRE(config.parsed);

  new Unit(&config);

  Units[1]->on_layout = true;

  Unit * U = Units[1];

  U->link_all();

  /*
  //  1.0> --\Sw0                      Sw1/-> 1.8>
  //  1.1> ---1.2> 1.3> 1.4> 1.5> 1.6> 1.7--> 1.9>
  */

  SECTION("I - Find"){
    struct paths path = pathfinding(U->B[1], U->B[9]);

    CHECK(path.forward);
    CHECK(!path.reverse);
  }
}
