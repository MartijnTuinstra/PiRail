#include "catch.hpp"

#include "mem.h"
#include "logger.h"

#include "config/ModuleConfig.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/unit.h"


TEST_CASE( "MSSwitch Link", "[SB-3.1]" ) {
  set_level(NONE);
  set_logger_print_level(NONE);

  Units = (Unit **)_calloc(30, Unit *);
  unit_len = 30;

  char filename[30] = "./testconfigs/SB-3.1.bin";
  auto config = ModuleConfig(filename);
  config.read();

  REQUIRE(config.parsed);

  new Unit(&config);
  link_all_blocks(Units[1]);
  link_all_msswitches(Units[1]);
  Unit * U = Units[1];

  /*              /1.0
  //  1.0->  -\  /
  //  1.1->  --1.2-> --1.3->
  //              \- --1.4->
  */

  REQUIRE(U->B[0] != 0);
  REQUIRE(U->B[1] != 0);
  REQUIRE(U->B[2] != 0);
  REQUIRE(U->B[3] != 0);
  REQUIRE(U->B[4] != 0);
  REQUIRE(U->MSSw[0] != 0);

  SECTION( "link check"){
    REQUIRE(U->B[0]->next.type == RAIL_LINK_MA);
    REQUIRE(U->B[0]->next.p.p == U->MSSw[0]);

    REQUIRE(U->B[1]->next.type == RAIL_LINK_MA);
    REQUIRE(U->B[1]->next.p.p == U->MSSw[0]);

    REQUIRE(U->B[3]->prev.type == RAIL_LINK_MB);
    REQUIRE(U->B[3]->prev.p.p == U->MSSw[0]);

    REQUIRE(U->B[4]->prev.type == RAIL_LINK_MB);
    REQUIRE(U->B[4]->prev.p.p == U->MSSw[0]);

    REQUIRE(U->B[2]->next.type == RAIL_LINK_MB_inside);
    REQUIRE(U->B[2]->next.p.p == U->MSSw[0]);

    REQUIRE(U->B[2]->prev.type == RAIL_LINK_MA_inside);
    REQUIRE(U->B[2]->prev.p.p == U->MSSw[0]);
  }

  SECTION( "NextBlock MSSwitch A side" ) {
    REQUIRE(U->B[0]->_Next(NEXT | SWITCH_CARE, 1) == 0);
    REQUIRE(U->B[0]->_Next(NEXT | SWITCH_CARE, 2) == 0);

    REQUIRE(U->B[1]->_Next(NEXT | SWITCH_CARE, 1) == U->B[2]);
    REQUIRE(U->B[1]->_Next(NEXT | SWITCH_CARE, 2) == U->B[3]);

    U->MSSw[0]->state = 1;

    REQUIRE(U->B[0]->_Next(NEXT | SWITCH_CARE, 1) == U->B[2]);
    REQUIRE(U->B[0]->_Next(NEXT | SWITCH_CARE, 2) == U->B[4]);

    REQUIRE(U->B[1]->_Next(NEXT | SWITCH_CARE, 1) == 0);
    REQUIRE(U->B[1]->_Next(NEXT | SWITCH_CARE, 2) == 0);
  }

  SECTION( "NextBlock MSSwitch B side" ) {
    REQUIRE(U->B[4]->_Next(PREV | SWITCH_CARE, 1) == 0);
    REQUIRE(U->B[4]->_Next(PREV | SWITCH_CARE, 2) == 0);

    REQUIRE(U->B[3]->_Next(PREV | SWITCH_CARE, 1) == U->B[2]);
    REQUIRE(U->B[3]->_Next(PREV | SWITCH_CARE, 2) == U->B[1]);

    U->MSSw[0]->state = 1;

    REQUIRE(U->B[4]->_Next(PREV | SWITCH_CARE, 1) == U->B[2]);
    REQUIRE(U->B[4]->_Next(PREV | SWITCH_CARE, 2) == U->B[0]);

    REQUIRE(U->B[3]->_Next(PREV | SWITCH_CARE, 1) == 0);
    REQUIRE(U->B[3]->_Next(PREV | SWITCH_CARE, 2) == 0);
  }
}
