#include <time.h>
#include "catch.hpp"

#include "utils/mem.h"
#include "utils/logger.h"
#include "system.h"

#include "config/ModuleConfig.h"
#include "switchboard/manager.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/switchsolver.h"
#include "switchboard/polaritysolver.h"
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

TEST_CASE_METHOD(TestsFixture, "Polarity Solver", "[PolSolve][PS-1]"){
  char filenames[1][30] = {"./testconfigs/PS-1.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;

  switchboard::SwManager->LinkAndMap();

  pathlist_find();

  /*
  // I
  // --1.0-> --1.1-> <<1.2>- <<1.3>- <-1.11-- <-1.12--
  //
  // II
  //                   Sw0/-  <<1.7>- <<1.08>-
  // --1.4-> --1.5-> -<1.6>> -<1.9>> -<1.10>>
  //
  // III - IV
  //                       |---St0---|
  //              Sw2/---> 1.11> 1.12>
  // 1.08> 1.09> 1.10----> 1.13> 1.14>
  //                       |---St1---|
  //
  // V - XII
  //                             |---St2---|
  //                    Sw5/---> 1.20> 1.21> ---\Sw6
  // 1.15> 1.16> ------1.17----> 1.18> 1.19> ----1.27> 1.28-> ----\
  //                  /Sw4       |---St3---|            Sw7\       \
  //              Sw3/           |---St4---|                \       \Sw8
  // 1.22> 1.23> 1.24----------> 1.25> 1.26> ----1.29> ------1.30> ---1.31> 1.33>
  //               Sw9\Sw10                                MSSw0  \
  // <1.34 <1.35 <----------1.36 <1.37 <1.38 <1.39                 \-> 1.32>
  //                             |---St5---|
  //
  // XIII
  //
  //         <-1.40- <-1.41- <---1.42- <-1.43- <-1.44-
  //                          / Sw11
  //                    Sw12 /
  // -1.45-> -1.46-> -1.47---> -1.48-> -1.49->
  //
  */

  REQUIRE(U->block_len > 0);
  REQUIRE(U->switch_len > 0);

  for(uint8_t i = 0; i < U->block_len; i++){
    Algorithm::process(U->B[i], _FORCE);
  }

  U->Sw[0]->setState(0);
  if(AlQueue.queue->getItems())
    Algorithm::BlockTick();

  SECTION("I - Side 1"){
    U->B[0]->setDetection(1);
    Algorithm::process(U->B[0], _FORCE);

    auto T = U->B[0]->train;
    REQUIRE(T != nullptr);
    REQUIRE(T == RSManager->getTrain(0));

    // Force PolaritySolver, since train is stopped
    PolaritySolver::solve(T, U->B[2]->path, U->B[1]->path);

    CHECK(U->B[2]->path->polarity  == POLARITY_REVERSED);
    CHECK(U->B[2]->polarity_status == POLARITY_REVERSED);
    CHECK(U->B[3]->polarity_status == POLARITY_REVERSED);
  }
  
  SECTION("I - Side 2"){
    U->B[3]->setDetection(1);
    Algorithm::process(U->B[3], _FORCE);

    auto T = U->B[3]->train;
    REQUIRE(T != nullptr);
    REQUIRE(T == RSManager->getTrain(0));

    // Force PolaritySolver, since train is stopped
    PolaritySolver::solve(T, U->B[2]->path, U->B[1]->path);

    CHECK(U->B[2]->path->polarity  == POLARITY_REVERSED);
    CHECK(U->B[2]->polarity_status == POLARITY_REVERSED);
    CHECK(U->B[3]->polarity_status == POLARITY_REVERSED);

  }
  
  SECTION("II - Blocked polairity"){
    logger.setlevel_stdout(TRACE);
    U->B[11]->setDetection(1);
    Algorithm::process(U->B[11], _FORCE);

    auto T = U->B[11]->train;
    REQUIRE(T != nullptr);
    REQUIRE(T == RSManager->getTrain(0));

    U->B[3]->setDetection(1);
    Algorithm::process(U->B[3], _FORCE);

    // Force PolaritySolver, since train is stopped
    PolaritySolver::solve(T, U->B[2]->path, U->B[1]->path);

    CHECK(U->B[2]->path->polarity  == POLARITY_NORMAL);
    CHECK(U->B[2]->polarity_status == POLARITY_NORMAL);
    CHECK(U->B[3]->polarity_status == POLARITY_NORMAL);

    U->B[11]->setDetection(0);
    Algorithm::process(U->B[11], _FORCE);

    // Force PolaritySolver, since train is stopped
    PolaritySolver::solve(T, U->B[2]->path, U->B[1]->path);

    CHECK(U->B[2]->path->polarity  == POLARITY_REVERSED);
    CHECK(U->B[2]->polarity_status == POLARITY_REVERSED);
    CHECK(U->B[3]->polarity_status == POLARITY_REVERSED);

  }

  SECTION("III - Blocked polairity"){
    logger.setlevel_stdout(TRACE);
    U->B[11]->setDetection(1);
    Algorithm::process(U->B[11], _FORCE);

    auto T = U->B[11]->train;
    REQUIRE(T != nullptr);
    REQUIRE(T == RSManager->getTrain(0));

    U->B[3]->setDetection(1);
    Algorithm::process(U->B[3], _FORCE);

    // Force PolaritySolver, since train is stopped
    PolaritySolver::solve(T, U->B[2]->path, U->B[1]->path);

    CHECK(U->B[2]->path->polarity  == POLARITY_NORMAL);
    CHECK(U->B[2]->polarity_status == POLARITY_NORMAL);
    CHECK(U->B[3]->polarity_status == POLARITY_NORMAL);

    U->B[11]->setDetection(0);
    Algorithm::process(U->B[11], _FORCE);

    // Force PolaritySolver, since train is stopped
    PolaritySolver::solve(T, U->B[2]->path, U->B[1]->path);

    CHECK(U->B[2]->path->polarity  == POLARITY_REVERSED);
    CHECK(U->B[2]->polarity_status == POLARITY_REVERSED);
    CHECK(U->B[3]->polarity_status == POLARITY_REVERSED);

  }
}
