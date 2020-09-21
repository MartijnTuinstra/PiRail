#include "catch.hpp"

#include "mem.h"
#include "logger.h"

#include "config/ModuleConfig.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/unit.h"

#include "train.h"
#include "modules.h"
#include "algorithm.h"


TEST_CASE( "Switch Link", "[SB-2.1]" ) {
  
  unload_module_Configs();
  unload_rolling_Configs();
  clearAlgorithmQueue();

  Units = (Unit **)_calloc(30, Unit *);
  unit_len = 30;

  char filename[30] = "./testconfigs/SB-2.1.bin";
  auto config = ModuleConfig(filename);
  config.read();

  REQUIRE(config.parsed);

  new Unit(&config);
  link_all_blocks(Units[1]);
  link_all_switches(Units[1]);
  Unit * U = Units[1];

  /*
  //                      /- --1.3->
  //  1.0->  --1.1-> --1:0-- --1.2->
  */

  REQUIRE(U->B[0] != 0);
  REQUIRE(U->B[1] != 0);
  REQUIRE(U->B[2] != 0);
  REQUIRE(U->Sw[0] != 0);

  SECTION( "link check"){
    REQUIRE(U->B[1]->next.type == RAIL_LINK_S);
    REQUIRE(U->B[1]->next.p.p == U->Sw[0]);

    REQUIRE(U->B[2]->prev.type == RAIL_LINK_s);
    REQUIRE(U->B[2]->prev.p.p == U->Sw[0]);

    REQUIRE(U->B[3]->prev.type == RAIL_LINK_s);
    REQUIRE(U->B[3]->prev.p.p == U->Sw[0]);
  }

  SECTION( "NextBlock Switch Approach" ) {
    REQUIRE(U->B[1]->_Next(NEXT, 1) == U->B[2]);

    U->Sw[0]->state = 1;

    REQUIRE(U->B[1]->_Next(NEXT, 1) == U->B[3]);
  }

  SECTION( "NextBlock Switch Straight" ) {
    REQUIRE(U->B[2]->_Next(PREV, 1) == U->B[1]);
    REQUIRE(U->B[2]->_Next(PREV | SWITCH_CARE, 1) == U->B[1]);

    U->Sw[0]->state = 1;

    REQUIRE(U->B[2]->_Next(PREV, 1) == U->B[1]);
    REQUIRE(U->B[2]->_Next(PREV | SWITCH_CARE, 1) == 0);
  }

  SECTION( "NextBlock Switch Diverging" ) {
    REQUIRE(U->B[3]->_Next(PREV, 1) == U->B[1]);
    REQUIRE(U->B[3]->_Next(PREV | SWITCH_CARE, 1) == 0);

    U->Sw[0]->state = 1;

    REQUIRE(U->B[3]->_Next(PREV, 1) == U->B[1]);
    REQUIRE(U->B[3]->_Next(PREV | SWITCH_CARE, 1) == U->B[1]);
  }

}
