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
#include "algorithm/component.h"

void init_test(char (* filenames)[30], int nr_files);

TEST_CASE( "Switch Link", "[SB][SB-2][SB-2.1]" ) {
  char filenames[1][30] = {"./testconfigs/SB-2.1.bin"};
  init_test(filenames, 1);

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->link_all();

  /*
  //                      /- --1.3->
  //  1.0->  --1.1-> --1:0-- --1.2->
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
    REQUIRE(U->B[2]->Next_Block(PREV | SWITCH_CARE, 1) == U->B[1]);

    U->Sw[0]->state = 1;

    REQUIRE(U->B[2]->Next_Block(PREV, 1) == U->B[1]);
    REQUIRE(U->B[2]->Next_Block(PREV | SWITCH_CARE, 1) == 0);
  }

  SECTION( "IV - NextBlock Switch Diverging" ) {
    REQUIRE(U->B[3]->Next_Block(PREV, 1) == U->B[1]);
    REQUIRE(U->B[3]->Next_Block(PREV | SWITCH_CARE, 1) == 0);

    U->Sw[0]->state = 1;

    REQUIRE(U->B[3]->Next_Block(PREV, 1) == U->B[1]);
    REQUIRE(U->B[3]->Next_Block(PREV | SWITCH_CARE, 1) == U->B[1]);
  }

  SECTION( "V - Search Check"){
    for(uint8_t i = 0; i < 4; i++){
      Algorithm::process(U->B[i], _FORCE);
    }

    REQUIRE(U->B[0]->Alg.next == 2);
    REQUIRE(U->B[0]->Alg.N[0] == U->B[1]);
    REQUIRE(U->B[0]->Alg.N[1] == U->B[2]);

    U->Sw[0]->setState(1);
    REQUIRE(U->Sw[0]->state == 1);

    // Algor cleared
    REQUIRE(U->B[0]->Alg.N[0] == 0);
    REQUIRE(U->B[2]->Alg.P[0] == 0);

    Algorithm::tick();

    REQUIRE(U->B[0]->Alg.next == 2);
    REQUIRE(U->B[0]->Alg.N[0] == U->B[1]);
    REQUIRE(U->B[0]->Alg.N[1] == U->B[3]);

    U->Sw[0]->setState(1);

    // Algor not cleared, no new state
    REQUIRE(U->B[0]->Alg.N[0] != 0);
    REQUIRE(U->B[3]->Alg.P[0] != 0);
  }
}