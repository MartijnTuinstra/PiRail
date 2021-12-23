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

  // logger.setlevel_stdout(TRACE);

  switchboard::SwManager->LinkAndMap();

  pathlist_find();

  /*
  // I
  // --1.0-> --1.1->|<<1.2>- <<1.3>-|<-1.4-- <-1.5--
  //
  // II
  // --1.6-> --1.7->|<<1.8>- <<1.9>-|<<1.10>- <<1.11>-|<-1.12-- <-1.13--
  //
  // III
  //                      Sw0/-  <<1.17>- <<1.18>-
  // --1.14-> --1.15-> -<1.16>> -<1.19>> -<1.20>>
  //
  // IV
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
    Algorithm::processBlock(&U->B[i]->Alg, _FORCE);
  }

  U->Sw[0]->setState(0);
  if(AlQueue.queue->getItems())
    Algorithm::BlockTick();

  SECTION("I - Side 1"){
    // Polarity changes ahead (from B1 to B2)
    //  polarity of B2/B3 can be changed.

    U->B[0]->setDetection(1);
    Algorithm::processBlock(&U->B[0]->Alg, _FORCE);

    auto T = U->B[0]->train;
    REQUIRE(T != nullptr);
    REQUIRE(T == RSManager->getTrain(0));

    T->setSpeed(100);
    T->B = U->B[0];

    Algorithm::Polarity_Checker(&T->B->Alg, 0);

    CHECK(U->B[2]->path->polarity  == POLARITY_REVERSED);
    CHECK(U->B[2]->polarity_status == POLARITY_REVERSED);
    CHECK(U->B[3]->polarity_status == POLARITY_REVERSED);
  }
  
  SECTION("I - Side 2"){
    // Polarity changes ahead (from B2 to B1)
    //  polarity of B1/B0 cannot be changed therefore B2/B3 must be changed.

    U->B[3]->setDetection(1);
    Algorithm::processBlock(&U->B[3]->Alg, _FORCE);

    auto T = U->B[3]->train;
    REQUIRE(T != nullptr);
    REQUIRE(T == RSManager->getTrain(0));
    
    T->setSpeed(100);
    T->B = U->B[3];

    Algorithm::Polarity_Checker(&T->B->Alg, 0);

    CHECK(U->B[2]->path->polarity  == POLARITY_REVERSED);
    CHECK(U->B[2]->polarity_status == POLARITY_REVERSED);
    CHECK(U->B[3]->polarity_status == POLARITY_REVERSED);

  }
  
  SECTION("II - Blocked polarity"){
    // Polarity changes ahead (from B2 to B1)
    //  polarity of B1/B0 cannot be changed therefore B2/B3 must be changed,
    //  this can only happen when B4/B5 is cleared.


    // logger.setlevel_stdout(TRACE);
    U->B[4]->setDetection(1);
    Algorithm::processBlock(&U->B[4]->Alg, _FORCE);

    auto T = U->B[4]->train;
    REQUIRE(T != nullptr);
    REQUIRE(T == RSManager->getTrain(0));

    U->B[3]->setDetection(1);
    Algorithm::processBlock(&U->B[3]->Alg, _FORCE);

    T->setSpeed(100);
    T->B = U->B[3];

    Algorithm::Polarity_Checker(&T->B->Alg, 0);

    CHECK(U->B[2]->path->polarity  == POLARITY_NORMAL);
    CHECK(U->B[2]->polarity_status == POLARITY_NORMAL);
    CHECK(U->B[3]->polarity_status == POLARITY_NORMAL);

    U->B[4]->setDetection(0);
    Algorithm::processBlock(&U->B[4]->Alg, _FORCE);

    Algorithm::Polarity_Checker(&T->B->Alg, 0);

    CHECK(U->B[2]->path->polarity  == POLARITY_REVERSED);
    CHECK(U->B[2]->polarity_status == POLARITY_REVERSED);
    CHECK(U->B[3]->polarity_status == POLARITY_REVERSED);

  }

  SECTION("III - Blocked polarity"){
    // Polarity changes ahead twice
    //  first polarity of B8,B9 has to be matched to B7
    //  then the polarity of B10,B11 has to be matched with B9
    //  the train continues and leaves B9
    //  then B10,B11 have to be matched with B12


    // --1.6-> --1.7->|<<1.8>- <<1.9>-|<<1.10>- <<1.11>-|<-1.12-- <-1.13--
    U->B[7]->setDetection(1);
    Algorithm::processBlock(&U->B[7]->Alg, _FORCE);

    auto T = U->B[7]->train;
    REQUIRE(T != nullptr);
    REQUIRE(T == RSManager->getTrain(0));

    REQUIRE(U->B[8]->path != U->B[10]->path);

    T->setSpeed(100);
    T->B = U->B[7];

    Algorithm::Polarity_Checker(&T->B->Alg, 0);
    T->setSpeed(0);

    CHECK(U->B[ 8]->path->polarity  == POLARITY_REVERSED);
    CHECK(U->B[10]->path->polarity  == POLARITY_NORMAL);

    U->B[8]->reverse();
    U->B[8]->setDetection(1);
    Algorithm::processBlock(&U->B[8]->Alg, _FORCE);
    T->B = U->B[8];
    T->setSpeed(100);

    Algorithm::Polarity_Checker(&T->B->Alg, 0);

    CHECK(U->B[ 8]->path->polarity  == POLARITY_REVERSED);
    CHECK(U->B[10]->path->polarity  == POLARITY_REVERSED);

    T->setSpeed(0);

    U->B[10]->reverse();
    U->B[7]->setDetection(0);
    Algorithm::processBlock(&U->B[7]->Alg, _FORCE);
    U->B[9]->setDetection(1);
    Algorithm::processBlock(&U->B[9]->Alg, _FORCE);
    U->B[8]->setDetection(0);
    Algorithm::processBlock(&U->B[8]->Alg, _FORCE);
    U->B[10]->setDetection(1);
    Algorithm::processBlock(&U->B[10]->Alg, _FORCE);
    U->B[9]->setDetection(0);
    Algorithm::processBlock(&U->B[9]->Alg, _FORCE);
    T->B = U->B[10];
    T->setSpeed(100);

    Algorithm::Polarity_Checker(&T->B->Alg, 0);

    CHECK(U->B[ 8]->path->polarity  == POLARITY_REVERSED);
    CHECK(U->B[10]->path->polarity  == POLARITY_NORMAL);

  }

  SECTION("IV - Blocked polarity"){
    // Polarity changes ahead twice
    //  first polarity of B8,B9 has to be matched to B7
    //  then the polarity of B10,B11 has to be matched with B9
    //  the train continues but has not left B9
    //  then B8,B9,B10,B11 have to be matched with B12

    // --1.6-> --1.7->|<<1.8>- <<1.9>-|<<1.10>- <<1.11>-|<-1.12-- <-1.13--
    U->B[7]->setDetection(1);
    Algorithm::processBlock(&U->B[7]->Alg, _FORCE);

    auto T = U->B[7]->train;
    REQUIRE(T != nullptr);
    REQUIRE(T == RSManager->getTrain(0));

    REQUIRE(U->B[8]->path != U->B[10]->path);

    T->setSpeed(100);
    T->B = U->B[7];

    Algorithm::Polarity_Checker(&T->B->Alg, 0);
    T->setSpeed(0);

    CHECK(U->B[ 8]->path->polarity  == POLARITY_REVERSED);
    CHECK(U->B[10]->path->polarity  == POLARITY_NORMAL);

    REQUIRE(U->B[8]->path);
    U->B[8]->path->reverse();
    U->B[8]->setDetection(1);
    Algorithm::processBlock(&U->B[8]->Alg, _FORCE);
    T->B = U->B[8];
    T->setSpeed(100);

    Algorithm::Polarity_Checker(&T->B->Alg, 0);

    CHECK(U->B[ 8]->path->polarity  == POLARITY_REVERSED);
    CHECK(U->B[10]->path->polarity  == POLARITY_REVERSED);

    T->setSpeed(0);

    REQUIRE(U->B[10]->path);
    U->B[10]->path->reverse();
    U->B[9]->setDetection(1);
    Algorithm::processBlock(&U->B[9]->Alg, _FORCE);
    U->B[7]->setDetection(0);
    Algorithm::processBlock(&U->B[7]->Alg, _FORCE);
    U->B[10]->setDetection(1);
    Algorithm::processBlock(&U->B[10]->Alg, _FORCE);
    U->B[8]->setDetection(0);
    Algorithm::processBlock(&U->B[8]->Alg, _FORCE);
    T->B = U->B[10];
    T->setSpeed(100);

    CHECK(U->B[ 9]->blocked);
    CHECK(U->B[10]->blocked);
    CHECK(U->B[9]->train == U->B[10]->train);

    Algorithm::Polarity_Checker(&T->B->Alg, 0);

    CHECK(U->B[ 8]->path->polarity  == POLARITY_NORMAL);
    CHECK(U->B[10]->path->polarity  == POLARITY_NORMAL);

  }


  /*
  SECTION("V - Train closely behind"){
    // Polarity changes ahead twice
    //  first polarity of B8,B9 has to be matched to B7
    //  then the polarity of B10,B11 has to be matched with B9
    //  the train continues but has not left B9
    //  then B8,B9,B10,B11 have to be matched with B12

    // --1.6-> --1.7->|<<1.8>- <<1.9>-|<<1.10>- <<1.11>-|<-1.12-- <-1.13--
    U->B[ 8]->path->reverse();
    U->B[10]->path->reverse();

    CHECK(U->B[ 8]->path->polarity  == POLARITY_REVERSED);
    CHECK(U->B[10]->path->polarity  == POLARITY_REVERSED);

    logger.setlevel_stdout(TRACE);
    U->B[9]->setDetection(1);
    Algorithm::processBlock(&U->B[9]->Alg, _FORCE);

    auto T = U->B[9]->train;
    REQUIRE(T != nullptr);
    REQUIRE(T == RSManager->getTrain(0));
    
    U->B[9]->setDetection(1);
    Algorithm::processBlock(&U->B[9]->Alg, _FORCE);

    REQUIRE(U->B[8]->path != U->B[10]->path);

    T->setSpeed(100);
    T->B = U->B[9];



    T->setSpeed(0);

    REQUIRE(U->B[10]->path);
    U->B[10]->path->reverse();
    U->B[9]->setDetection(1);
    Algorithm::processBlock(&U->B[9]->Alg, _FORCE);
    U->B[7]->setDetection(0);
    Algorithm::processBlock(&U->B[7]->Alg, _FORCE);
    U->B[10]->setDetection(1);
    Algorithm::processBlock(&U->B[10]->Alg, _FORCE);
    U->B[8]->setDetection(0);
    Algorithm::processBlock(&U->B[8]->Alg, _FORCE);
    T->B = U->B[10];
    T->setSpeed(100);

    CHECK(U->B[ 9]->blocked);
    CHECK(U->B[10]->blocked);
    CHECK(U->B[9]->train == U->B[10]->train);

    Algorithm::Polarity_Checker(&T->B->Alg, 0);

    CHECK(U->B[ 8]->path->polarity  == POLARITY_NORMAL);
    CHECK(U->B[10]->path->polarity  == POLARITY_NORMAL);

  }

  */
}
