#include "catch.hpp"

#include "mem.h"
#include "logger.h"
#include "system.h"

#include "config/ModuleConfig.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/signals.h"
#include "switchboard/unit.h"

#include "algorithm.h"

#include "rollingstock/railtrain.h"

TEST_CASE( "Signal", "[SB-4.1]" ) {
  set_level(NONE);
  set_logger_print_level(NONE);

  Units = (Unit **)_calloc(30, Unit *);
  unit_len = 30;

  char filename[30] = "./testconfigs/SB-4.1.bin";
  auto config = ModuleConfig(filename);
  config.read();

  REQUIRE(config.parsed);

  new Unit(&config);
  link_all_blocks(Units[1]);
  Unit * U = Units[1];

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

  REQUIRE(U->B[1]->forward_signal != 0);
  REQUIRE(U->B[1]->reverse_signal != 0);

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
    U->B[5]->setState(CAUTION);

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
