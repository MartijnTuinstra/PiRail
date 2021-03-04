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

#include "rollingstock/railtrain.h"

#include "train.h"
#include "sim.h"

#include "algorithm/queue.h"
#include "algorithm/core.h"
#include "algorithm/component.h"
#include "algorithm/blockconnector.h"

#include "pathfinding.h"

void init_test(char (* filenames)[30], int nr_files);
void train_testSim_tick(struct train_sim * t, int32_t * i);
void train_test_tick(struct train_sim * t, int32_t * i);
void test_Algorithm_tick();

class TestsFixture {
public:
  TestsFixture();
  void loadSwitchboard(char (* filenames)[30], int nr_files);
  void loadStock();
  ~TestsFixture();
};

TEST_CASE_METHOD(TestsFixture, "RailTrain Z21", "[Z21]"){
  char filenames[1][30] = {"./testconfigs/RT-Z21.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  // logger.setlevel_stdout(DEBUG);

  Unit * U = switchboard::Units(1);

  REQUIRE(U);
  U->on_layout = true;
  U->link_all();

  pathlist_find();

  for(uint8_t i = 0; i < 27; i++){
    Algorithm::process(U->B[i], _FORCE);
  }

  logger.setlevel_stdout(TRACE);

  SECTION("I - Speed Engine"){
    U->B[1]->setDetection(1);
    Algorithm::process(U->B[1], _FORCE);

    RailTrain * T = U->B[1]->train;
    REQUIRE(T);

    T->link(0, RAILTRAIN_ENGINE_TYPE);
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

  SECTION("I - Speed Train Double"){
    U->B[1]->setDetection(1);
    Algorithm::process(U->B[1], _FORCE);

    RailTrain * T = U->B[1]->train;
    REQUIRE(T);

    T->link(2, RAILTRAIN_TRAIN_TYPE);
    T->setSpeed(50);

    U->B[2]->setDetection(1);
    Algorithm::process(U->B[2], _FORCE);

    REQUIRE(T->directionKnown);

    Engine * E0 = RSManager->getEngine(0);
    Engine * E1 = RSManager->getEngine(1);
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
