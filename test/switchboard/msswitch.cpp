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

#include "train.h"

#include "algorithm/core.h"

void init_test(char (* filenames)[30], int nr_files);

TEST_CASE( "MSSwitch Link", "[SB][SB-3][SB-3.1]" ) {
  char filenames[1][30] = {"./testconfigs/SB-3.1.bin"};
  init_test(filenames, 1);

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->link_all();

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
    REQUIRE(U->B[0]->Next_Block(NEXT | SWITCH_CARE, 1) == 0);
    REQUIRE(U->B[0]->Next_Block(NEXT | SWITCH_CARE, 2) == 0);

    REQUIRE(U->B[1]->Next_Block(NEXT | SWITCH_CARE, 1) == U->B[2]);
    REQUIRE(U->B[1]->Next_Block(NEXT | SWITCH_CARE, 2) == U->B[3]);

    U->MSSw[0]->state = 1;

    REQUIRE(U->B[0]->Next_Block(NEXT | SWITCH_CARE, 1) == U->B[2]);
    REQUIRE(U->B[0]->Next_Block(NEXT | SWITCH_CARE, 2) == U->B[4]);

    REQUIRE(U->B[1]->Next_Block(NEXT | SWITCH_CARE, 1) == 0);
    REQUIRE(U->B[1]->Next_Block(NEXT | SWITCH_CARE, 2) == 0);
  }

  SECTION( "NextBlock MSSwitch B side" ) {
    REQUIRE(U->B[4]->Next_Block(PREV | SWITCH_CARE, 1) == 0);
    REQUIRE(U->B[4]->Next_Block(PREV | SWITCH_CARE, 2) == 0);

    REQUIRE(U->B[3]->Next_Block(PREV | SWITCH_CARE, 1) == U->B[2]);
    REQUIRE(U->B[3]->Next_Block(PREV | SWITCH_CARE, 2) == U->B[1]);

    U->MSSw[0]->state = 1;

    REQUIRE(U->B[4]->Next_Block(PREV | SWITCH_CARE, 1) == U->B[2]);
    REQUIRE(U->B[4]->Next_Block(PREV | SWITCH_CARE, 2) == U->B[0]);

    REQUIRE(U->B[3]->Next_Block(PREV | SWITCH_CARE, 1) == 0);
    REQUIRE(U->B[3]->Next_Block(PREV | SWITCH_CARE, 2) == 0);
  }
}
