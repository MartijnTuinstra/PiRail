#include <time.h>
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

#include "rollingstock/train.h"

#include "train.h"
#include "sim.h"

#include "algorithm/queue.h"
#include "algorithm/core.h"
#include "algorithm/component.h"
#include "algorithm/blockconnector.h"

#include "pathfinding.h"
#include "path.h"
#include "testhelpers.h"

TEST_CASE_METHOD(TestsFixture, "Train Z21", "[Z21]"){
  char filenames[1][30] = {"./testconfigs/RT-Z21.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);

  REQUIRE(U);
  U->on_layout = true;

  switchboard::SwManager->LinkAndMap();

  pathlist_find();

  for(uint8_t i = 0; i < 27; i++){
    Algorithm::process(U->B[i], _FORCE);
  }

  SECTION("I - Speed Engine"){
    U->B[1]->setDetection(1);
    Algorithm::process(U->B[1], _FORCE);

    Train * T = U->B[1]->train;
    REQUIRE(T);

    T->link(0, TRAIN_ENGINE_TYPE);
    T->setSpeed(50);

    U->B[2]->setDetection(1);
    Algorithm::process(U->B[2], _FORCE);

    REQUIRE(T->directionKnown);

    Engine * E = RSManager->getEngine(0);
    // Reply speed form Z21 device
    E->Z21_setSpeedDir(36, 0);

    // Change speed from Z21 device
    E->Z21_setSpeedDir(40, 0);

    CHECK(T->speed == E->cur_speed);

    CHECK(E->Z21_set_speed == 40);
    CHECK(E->Z21_get_speed == 40);
  }

  SECTION("II - Speed Train Double"){
    U->B[1]->setDetection(1);
    Algorithm::process(U->B[1], _FORCE);

    Train * T = U->B[1]->train;
    REQUIRE(T);

    T->link(2, TRAIN_TRAIN_TYPE);
    T->setSpeed(50);

    U->B[2]->setDetection(1);
    Algorithm::process(U->B[2], _FORCE);

    REQUIRE(T->directionKnown);

    Engine * E0 = RSManager->getEngine(0);
    Engine * E1 = RSManager->getEngine(1);

    CHECK(RSManager->getEngineDCC(E0->DCC_ID) == E0);
    // Reply speed form Z21 device
    E0->Z21_setSpeedDir(36, 0);

    // Change speed from Z21 device
    E0->Z21_setSpeedDir(40, 0);

    CHECK(T->speed == E0->cur_speed);
    CHECK(T->speed == E1->cur_speed);

    CHECK(E0->Z21_set_speed == 40);
    CHECK(E0->Z21_get_speed == 40);

    CHECK(E1->Z21_set_speed == 40);
  }
}
