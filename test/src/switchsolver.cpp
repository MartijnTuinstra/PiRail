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

TEST_CASE_METHOD(TestsFixture, "Switch Solver No Route", "[SwSolve][SwS-1]"){
  char filenames[1][30] = {"./testconfigs/SwS-1.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;

  switchboard::SwManager->LinkAndMap();

  pathlist_find();

  /*
  // I - II
  //                   Sw0/> --1.3-> --1.4->
  // --1.0-> --1.1-> --1.2-> --1.5-> --1.6->
  //
  // III - IV
  //                   Sw2 /> --1.10-> --1.11->
  //                   Sw1/-> --1.12-> --1.13->
  // --1.7-> --1.8-> --1.9--> --1.14-> --1.15->
  //
  // V - VI
  //
  // <-1.16-- <-1.17-- -\ Sw3                       Sw10/- <-1.18-- <-1.19--
  // <-1.20-- <-1.21-- <-1.22-- <-1.23-- <-1.24-- <-1.25-- <-1.26-- <-1.27--
  //                   Sw4 \ Sw5                  Sw8 / Sw9
  // --1.28-> --1.29-> -<1.30>> -<1.31>> -<1.32>> -<1.33>> --1.34-> --1.35->
  //                     Sw6 \> -<1.36>> -<1.37>> -/ Sw7
  //
  // VII - VIII
  //
  // <-1.38-- <-1.39-- <-1.40-- <-1.41-- <-1.42--
  //                  Sw11 \ MSSw0
  // --1.43-> --1.44-> -<1.45>> --1.46-> --1.47->
  //                   MSSw0 \> <-1.48-- <-1.49--
  //
  */
 

  SECTION("I - S side"){
    // Switch has no preference and will therefore stay in the same position
    //  also if the blocks behind the switch are blocked.
    //  except if the block behind the switch is reversed and reserved/blocked TODO
    
    U->B[0]->setDetection(1);
    Algorithm::process(U->B[0], _FORCE);

    auto T = U->B[0]->train;
    REQUIRE(T != nullptr);
    REQUIRE(T == RSManager->getTrain(0));
    
    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[0], U->B[2], U->B[2]->next, NEXT | FL_SWITCH_CARE);

    // Switch should stay and should now be reserved (locked for other trains)
    CHECK(U->Sw[0]->state == 0);
    CHECK(U->Sw[0]->Detection->state == RESERVED);
    CHECK(U->Sw[0]->Detection->reserved);
    CHECK(U->Sw[0]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->Sw[0]->Detection);
    U->Sw[0]->Detection->state = PROCEED;

    U->Sw[0]->setState(1);         // Set Diverging
    REQUIRE(U->Sw[0]->state == 1); // Verify that the switch is thrown
    Algorithm::process(U->B[1], _FORCE);   // Re-search all blocks
    Algorithm::process(U->B[2], _FORCE);
    Algorithm::process(U->B[3], _FORCE);

    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[0], U->B[2], U->B[2]->next, NEXT | FL_SWITCH_CARE);

    // Switch should stay and should now be reserved (locked for other trains)
    CHECK(U->Sw[0]->state == 1);
    CHECK(U->Sw[0]->Detection->state == RESERVED);
    CHECK(U->Sw[0]->Detection->reserved);
    CHECK(U->Sw[0]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->Sw[0]->Detection);
    U->Sw[0]->Detection->state = PROCEED;
  }

  SECTION("I - s side"){
    // Switch has no preference and will therefore stay in the same position
    //  also if the blocks behind the switch are blocked.
    //  except if the block behind the switch is reversed and reserved/blocked TODO
    
    U->B[4]->path->reverse();
    U->B[6]->path->reverse();
    U->B[4]->setDetection(1);
    U->B[6]->setDetection(1);
    Algorithm::process(U->B[4], _FORCE);
    Algorithm::process(U->B[6], _FORCE);

    REQUIRE(U->B[4]->train);
    REQUIRE(U->B[6]->train);

    auto T = U->B[4]->train;
    REQUIRE(T == RSManager->getTrain(0));
    
    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[4], U->B[3], U->B[3]->prev, NEXT | FL_SWITCH_CARE);

    // Switch should switch to match path
    CHECK(U->Sw[0]->state == 1);
    CHECK(U->Sw[0]->Detection->state == RESERVED);
    CHECK(U->Sw[0]->Detection->reserved);
    CHECK(U->Sw[0]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->Sw[0]->Detection);
    T->dereserveBlock(U->B[0]);
    T->dereserveBlock(U->B[1]);
    U->Sw[0]->Detection->state = PROCEED;

    U->Sw[0]->setState(1);         // Set Diverging
    REQUIRE(U->Sw[0]->state == 1); // Verify that the switch is thrown
    Algorithm::BlockTick();        // Re-search all blocks

    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(U->B[6]->train, U->B[6], U->B[5], U->B[5]->prev, NEXT | FL_SWITCH_CARE); 

    // Switch should switch to match path
    CHECK(U->Sw[0]->state == 0);
    CHECK(U->Sw[0]->Detection->state == RESERVED);
    CHECK(U->Sw[0]->Detection->reserved);
    CHECK(U->Sw[0]->Detection->isReservedBy(U->B[6]->train));

    // Reset
    T->dereserveBlock(U->Sw[0]->Detection);
    U->Sw[0]->Detection->state = PROCEED;
    
    // CHECK(U->Sw[0]->Detection->state == CAUTION);
  }

  SECTION("II - S side"){
    // Switch has no preference and will therefore stay in the same position
    //  also if the blocks behind the switch are blocked.
    //->except if the block behind the switch is reversed and reserved/blocked TODO

    U->B[6]->path->reverse();
    
    U->B[0]->setDetection(1);
    U->B[6]->setDetection(1);
    Algorithm::process(U->B[0], _FORCE);
    Algorithm::process(U->B[6], _FORCE);

    U->B[6]->path->reserve(U->B[6]->train, U->B[6]);

    CHECK(U->Sw[0]->state == 0);

    auto T = U->B[0]->train;
    REQUIRE(T != nullptr);
    REQUIRE(T == RSManager->getTrain(0));
    REQUIRE(U->B[6]->train == RSManager->getTrain(1));
    
    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[0], U->B[2], U->B[2]->next, NEXT | FL_SWITCH_CARE);

    // Switch should change to other state and should now be reserved (locked for other trains)
    CHECK(U->Sw[0]->state == 1);
    CHECK(U->Sw[0]->Detection->state == RESERVED);
    CHECK(U->Sw[0]->Detection->reserved);
    CHECK(U->Sw[0]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->Sw[0]->Detection);
    U->Sw[0]->Detection->state = PROCEED;

    U->Sw[0]->setState(1);         // Set Diverging
    REQUIRE(U->Sw[0]->state == 1); // Verify that the switch is thrown
    Algorithm::process(U->B[1], _FORCE);   // Re-search all blocks
    Algorithm::process(U->B[2], _FORCE);
    Algorithm::process(U->B[3], _FORCE);

    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[0], U->B[2], U->B[2]->next, NEXT | FL_SWITCH_CARE);

    // Switch should stay and should now be reserved (locked for other trains)
    CHECK(U->Sw[0]->state == 1);
    CHECK(U->Sw[0]->Detection->state == RESERVED);
    CHECK(U->Sw[0]->Detection->reserved);
    CHECK(U->Sw[0]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->Sw[0]->Detection);
    U->Sw[0]->Detection->state = PROCEED;
  }

  SECTION("III - S side"){
    // Switch has no preference and will therefore stay in the same position
    //  also if the blocks behind the switch are blocked.
    //  except if the block behind the switch is reversed and reserved/blocked TODO
    
    U->B[7]->setDetection(1);
    Algorithm::process(U->B[7], _FORCE);

    auto T = U->B[7]->train;
    REQUIRE(T != nullptr);
    REQUIRE(T == RSManager->getTrain(0));
    REQUIRE(U->Sw[1]->state == 0);
    REQUIRE(U->Sw[2]->state == 0);
    
    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[7], U->B[9], U->B[9]->next, NEXT | FL_SWITCH_CARE);

    // Switch should stay and should now be reserved (locked for other trains)
    //  solver should not touch Sw2
    CHECK(U->Sw[1]->state == 0);
    CHECK(U->Sw[2]->state == 0);
    CHECK(U->Sw[1]->Detection->state == RESERVED);
    CHECK(U->Sw[1]->Detection->reserved);
    CHECK(U->Sw[1]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->Sw[1]->Detection);
    U->Sw[1]->Detection->state = PROCEED;

    U->Sw[1]->setState(1);         // Set Diverging
    REQUIRE(U->Sw[1]->state == 1); // Verify that the switch is thrown
    Algorithm::BlockTick();        // Re-search all blocks

    U->B[ 9]->setState(PROCEED, 0); // Force railstates
    U->B[10]->setState(PROCEED, 0); // Force railstates

    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[7], U->B[9], U->B[9]->next, NEXT | FL_SWITCH_CARE);

    // Switch should stay and should now be reserved (locked for other trains)
    //  same applies for Sw2
    CHECK(U->Sw[1]->state == 1);
    CHECK(U->Sw[2]->state == 0);
    CHECK(U->Sw[1]->Detection->state == RESERVED);
    CHECK(U->Sw[1]->Detection->reserved);
    CHECK(U->Sw[1]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->Sw[1]->Detection);
    U->Sw[1]->Detection->state = PROCEED;
  }

  SECTION("III - s side"){
    // Switch has no preference and will therefore stay in the same position
    //  also if the blocks behind the switch are blocked.
    //  except if the block behind the switch is reversed and reserved/blocked TODO
    
    U->B[11]->path->reverse();
    U->B[11]->setDetection(1);
    Algorithm::process(U->B[11], _FORCE);

    REQUIRE(U->B[11]->train);

    auto T = U->B[11]->train;
    REQUIRE(T == RSManager->getTrain(0));
    
    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[11], U->B[10], U->B[10]->prev, NEXT | FL_SWITCH_CARE);

    // Switch should switch to match path
    CHECK(U->Sw[1]->state == 1);
    CHECK(U->Sw[2]->state == 0);
    CHECK(U->Sw[1]->Detection->state == RESERVED);
    CHECK(U->Sw[1]->Detection->reserved);
    CHECK(U->Sw[1]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->Sw[0]->Detection);
    U->Sw[0]->Detection->state = PROCEED;

    // U->Sw[0]->setState(1);         // Set Diverging
    // REQUIRE(U->Sw[0]->state == 1); // Verify that the switch is thrown
    // Algorithm::BlockTick();        // Re-search all blocks

    // // Force Switchsolver, since train is stopped
    // SwitchSolver::solve(U->B[6]->train, U->B[6], U->B[5], U->B[5]->prev, NEXT | FL_SWITCH_CARE);

    // // Switch should switch to match path
    // CHECK(U->Sw[0]->state == 0);
    // CHECK(U->Sw[0]->Detection->state == RESERVED);
    // CHECK(U->Sw[0]->Detection->reserved);
    // CHECK(U->Sw[0]->Detection->isReservedBy(T));

    // // Reset
    // T->dereserveBlock(U->Sw[0]->Detection);
    // U->Sw[0]->Detection->state = PROCEED;
    
    // CHECK(U->Sw[0]->Detection->state == CAUTION);
  }

  SECTION("V - No Crossover"){
    // Switch has no preference and will therefore stay in the same position
    //  also if the blocks behind the switch are blocked.
    //  except if the block behind the switch is reversed and reserved/blocked TODO

    U->B[28]->setDetection(1);
    Algorithm::process(U->B[28], _FORCE);

    auto T = U->B[28]->train;
    REQUIRE(T != nullptr);
    REQUIRE(T == RSManager->getTrain(0));
    
    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[28], U->B[29], U->B[29]->next, NEXT | FL_SWITCH_CARE);

    // Switch should stay and should now be reserved (locked for other trains)
    CHECK(U->Sw[5]->state == 0);
    CHECK(U->Sw[6]->state == 0);
    CHECK(U->Sw[5]->Detection->state == RESERVED);
    CHECK(U->Sw[5]->Detection->reserved);
    CHECK(U->Sw[5]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->Sw[5]->Detection);
    U->Sw[5]->Detection->state = PROCEED;

    U->Sw[5]->setState(1);         // Set Diverging
    U->Sw[6]->setState(1);         // Set Diverging
    REQUIRE(U->Sw[5]->state == 1); // Verify that the switch is thrown
    REQUIRE(U->Sw[6]->state == 1); // Verify that the switch is thrown
    Algorithm::BlockTick();        // Re-search all blocks

    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[28], U->B[29], U->B[29]->next, NEXT | FL_SWITCH_CARE);

    // Switch 6 should stay and should now be reserved (locked for other trains)
    //  switch 5 should change to fix path
    CHECK(U->Sw[5]->state == 0);
    CHECK(U->Sw[6]->state == 1);
    CHECK(U->Sw[5]->Detection->state == RESERVED);
    CHECK(U->Sw[5]->Detection->reserved);
    CHECK(U->Sw[5]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->Sw[5]->Detection);
    U->Sw[5]->Detection->state = PROCEED;

    CHECK(U->Sw[ 7]->state == 0);
    CHECK(U->Sw[ 8]->state == 0);
    CHECK(U->Sw[ 9]->state == 0);
    CHECK(U->Sw[10]->state == 0);

    U->B[36]->setDetection(1);
    Algorithm::process(U->B[36], _FORCE);

    REQUIRE(U->B[36]->train);
    REQUIRE(U->B[36]->train != T);
    T = U->B[36]->train;
    
    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[36], U->B[37], U->B[37]->next, NEXT | FL_SWITCH_CARE);

    // Switch should 9 & 10 should be untouched.
    //  Switch 8 should stay and should now be reserved (locked for other trains)
    //   switch 7 should change to fix path.
    CHECK(U->Sw[ 7]->state == 1);
    CHECK(U->Sw[ 8]->state == 0);
    CHECK(U->Sw[ 9]->state == 0);
    CHECK(U->Sw[10]->state == 0);
    CHECK(U->Sw[ 7]->Detection->state == RESERVED);
    CHECK(U->Sw[ 7]->Detection->reserved);
    CHECK(U->Sw[ 7]->Detection->isReservedBy(T));
    CHECK(U->Sw[10]->Detection->state != RESERVED);
    CHECK(!U->Sw[10]->Detection->reserved);
    CHECK(!U->Sw[10]->Detection->isReservedBy(T));

  }

  SECTION("V - Crossover"){
    // Switch has no preference and will therefore stay in the same position
    //  also if the blocks behind the switch are blocked.
    //  except if the block behind the switch is reversed and reserved/blocked TODO

    U->B[27]->setDetection(1);
    Algorithm::process(U->B[27], _FORCE);

    auto T = U->B[27]->train;
    REQUIRE(T != nullptr);
    REQUIRE(T == RSManager->getTrain(0));
    
    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[27], U->B[26], U->B[26]->next, NEXT | FL_SWITCH_CARE);

    // Switch should stay and should now be reserved (locked for other trains)
    CHECK(U->Sw[ 9]->state == 0);
    CHECK(U->Sw[10]->state == 0);
    CHECK(U->Sw[10]->Detection->state == RESERVED);
    CHECK(U->Sw[10]->Detection->reserved);
    CHECK(U->Sw[10]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->Sw[9]->Detection);
    U->Sw[9]->Detection->state = PROCEED;

    U->Sw[ 9]->setState(1);         // Set Diverging
    U->Sw[10]->setState(1);         // Set Diverging
    REQUIRE(U->Sw[ 7]->state == 0);
    REQUIRE(U->Sw[ 8]->state == 0);
    REQUIRE(U->Sw[ 9]->state == 1); // Verify that the switch is thrown
    REQUIRE(U->Sw[10]->state == 1); // Verify that the switch is thrown
    Algorithm::BlockTick();        // Re-search all blocks

    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[27], U->B[26], U->B[26]->next, NEXT | FL_SWITCH_CARE);

    // Switch 7 & 9 should stay and should now be reserved (locked for other trains)
    //  switch 8 & 10 should change to fix path
    CHECK(U->Sw[ 7]->state == 0);
    CHECK(U->Sw[ 8]->state == 1);
    CHECK(U->Sw[ 9]->state == 1);
    CHECK(U->Sw[10]->state == 0);
    CHECK(U->Sw[ 7]->Detection->state == RESERVED);
    CHECK(U->Sw[ 7]->Detection->reserved);
    CHECK(U->Sw[ 7]->Detection->isReservedBy(T));
    CHECK(U->Sw[10]->Detection->state == RESERVED);
    CHECK(U->Sw[10]->Detection->reserved);
    CHECK(U->Sw[10]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->Sw[ 7]->Detection);
    T->dereserveBlock(U->Sw[10]->Detection);
    U->Sw[ 7]->Detection->state = PROCEED;
    U->Sw[10]->Detection->state = PROCEED;
  }

  SECTION("VII - Straight"){
    // Switch has no preference and will therefore stay in the same position
    //  also if the blocks behind the switch are blocked.
    //  except if the block behind the switch is reversed and reserved/blocked
    
    U->B[43]->setDetection(1);
    Algorithm::process(U->B[43], _FORCE);

    REQUIRE(U->B[43]->train);

    auto T = U->B[43]->train;
    REQUIRE(T == RSManager->getTrain(0));
    
    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[43], U->B[44], U->B[44]->next, NEXT | FL_SWITCH_CARE);

    // Switch should switch to match path
    CHECK(U->MSSw[0]->state == 0);
    CHECK(U->MSSw[0]->Detection->state == RESERVED);
    CHECK(U->MSSw[0]->Detection->reserved);
    CHECK(U->MSSw[0]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->MSSw[0]->Detection);
    U->MSSw[0]->Detection->state = PROCEED;

    U->MSSw[0]->setState(1);              // Set Diverging
    REQUIRE(U->MSSw[0]->state == 1);      // Verify that the switch is thrown
    Algorithm::process(U->B[44], _FORCE); // Re-search all blocks
    Algorithm::process(U->B[45], _FORCE);
    Algorithm::process(U->B[46], _FORCE);

    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[43], U->B[44], U->B[44]->next, NEXT | FL_SWITCH_CARE);

    // Switch should switch to match path
    CHECK(U->MSSw[0]->state == 0);
    CHECK(U->MSSw[0]->Detection->state == RESERVED);
    CHECK(U->MSSw[0]->Detection->reserved);
    CHECK(U->MSSw[0]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->MSSw[0]->Detection);
    U->MSSw[0]->Detection->state = PROCEED;
    
    // CHECK(U->Sw[0]->Detection->state == CAUTION);
  }

  SECTION("VII - Crossed"){
    // Switch has no preference and will therefore stay in the same position
    //  also if the blocks behind the switch are blocked.
    //  except if the block behind the switch is reversed and reserved/blocked
    
    U->B[49]->setDetection(1);
    Algorithm::process(U->B[49], _FORCE);

    REQUIRE(U->B[49]->train);

    auto T = U->B[49]->train;
    REQUIRE(T == RSManager->getTrain(0));
    
    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[49], U->B[48], U->B[48]->next, NEXT | FL_SWITCH_CARE);

    // Switch should switch to match path
    CHECK(U->MSSw[0]->state == 1);
    CHECK(U->MSSw[0]->Detection->state == RESERVED);
    CHECK(U->MSSw[0]->Detection->reserved);
    CHECK(U->MSSw[0]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->MSSw[0]->Detection);
    U->MSSw[0]->Detection->state = PROCEED;

    U->MSSw[0]->setState(1);              // Set Diverging
    REQUIRE(U->MSSw[0]->state == 1);      // Verify that the switch is thrown
    Algorithm::process(U->B[44], _FORCE); // Re-search all blocks
    Algorithm::process(U->B[45], _FORCE);
    Algorithm::process(U->B[46], _FORCE);
    Algorithm::process(U->B[48], _FORCE);
    Algorithm::process(U->B[39], _FORCE);

    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[49], U->B[48], U->B[48]->next, NEXT | FL_SWITCH_CARE);

    // Switch should switch to match path
    CHECK(U->MSSw[0]->state == 1);
    CHECK(U->MSSw[0]->Detection->state == RESERVED);
    CHECK(U->MSSw[0]->Detection->reserved);
    CHECK(U->MSSw[0]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->MSSw[0]->Detection);
    U->MSSw[0]->Detection->state = PROCEED;
    
    // CHECK(U->Sw[0]->Detection->state == CAUTION);
  }
}

TEST_CASE_METHOD(TestsFixture, "Switch Solver Route", "[SwSolve][SwS-2]"){
  char filenames[1][30] = {"./testconfigs/SwS-1.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;

  switchboard::SwManager->LinkAndMap();

  pathlist_find();

  /*
  // I - II
  //                   Sw0/> --1.3-> --1.4->
  // --1.0-> --1.1-> --1.2-> --1.5-> --1.6->
  //
  // III - IV
  //                   Sw2 /> --1.10-> --1.11->
  //                   Sw1/-> --1.12-> --1.13->
  // --1.7-> --1.8-> --1.9--> --1.14-> --1.15->
  //
  // V - VI
  //
  // <-1.16-- <-1.17-- -\ Sw3                       Sw10/- <-1.18-- <-1.19--
  // <-1.20-- <-1.21-- <-1.22-- <-1.23-- <-1.24-- <-1.25-- <-1.26-- <-1.27--
  //                   Sw4 \ Sw5                  Sw8 / Sw9
  // --1.28-> --1.29-> -<1.30>> -<1.31>> -<1.32>> -<1.33>> --1.34-> --1.35->
  //                     Sw6 \> -<1.36>> -<1.37>> -/ Sw7
  //
  // VII - VIII
  //
  // <-1.38-- <-1.39-- <-1.40-- <-1.41-- <-1.42--
  //                  Sw11 \ MSSw0
  // --1.43-> --1.44-> -<1.45>> --1.46-> --1.47->
  //                   MSSw0 \> <-1.48-- <-1.49--
  //
  */
 

  SECTION("I - S side"){
    U->B[0]->setDetection(1);
    Algorithm::process(U->B[0], _FORCE);

    auto T = U->B[0]->train;
    REQUIRE(T != nullptr);
    REQUIRE(T == RSManager->getTrain(0));

    T->setRoute(U->B[4]);
    
    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[0], U->B[2], U->B[2]->next, NEXT | FL_SWITCH_CARE);

    // Switch should change due to the route and should now be reserved (locked for other trains)
    CHECK(U->Sw[0]->state == 1);
    CHECK(U->Sw[0]->Detection->state == RESERVED);
    CHECK(U->Sw[0]->Detection->reserved);
    CHECK(U->Sw[0]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->Sw[0]->Detection);
    U->Sw[0]->Detection->state = PROCEED;
    T->setRoute(U->B[6]);

    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[0], U->B[2], U->B[2]->next, NEXT | FL_SWITCH_CARE);

    // Switch should stay and should now be reserved (locked for other trains)
    CHECK(U->Sw[0]->state == 0);
    CHECK(U->Sw[0]->Detection->state == RESERVED);
    CHECK(U->Sw[0]->Detection->reserved);
    CHECK(U->Sw[0]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->Sw[0]->Detection);
    U->Sw[0]->Detection->state = PROCEED;
  }

  SECTION("I - s side"){    
    U->B[4]->path->reverse();
    U->B[6]->path->reverse();
    U->B[4]->setDetection(1);
    U->B[6]->setDetection(1);
    Algorithm::process(U->B[4], _FORCE);
    Algorithm::process(U->B[6], _FORCE);

    REQUIRE(U->B[4]->train);
    REQUIRE(U->B[6]->train);

    auto T = U->B[4]->train;
    REQUIRE(T == RSManager->getTrain(0));

    T->setRoute(U->B[0]);
    
    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[4], U->B[3], U->B[3]->prev, NEXT | FL_SWITCH_CARE);

    // Switch should switch to match path
    CHECK(U->Sw[0]->state == 1);
    CHECK(U->Sw[0]->Detection->state == RESERVED);
    CHECK(U->Sw[0]->Detection->reserved);
    CHECK(U->Sw[0]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->Sw[0]->Detection);
    T->dereserveBlock(U->B[0]);
    T->dereserveBlock(U->B[1]);
    U->Sw[0]->Detection->state = PROCEED;

    U->B[6]->train->setRoute(U->B[0]);
    REQUIRE(U->Sw[0]->state == 1); // Verify that the switch is thrown
    Algorithm::BlockTick();        // Re-search all blocks

    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(U->B[6]->train, U->B[6], U->B[5], U->B[5]->prev, NEXT | FL_SWITCH_CARE); 

    // Switch should switch to match path
    CHECK(U->Sw[0]->state == 0);
    CHECK(U->Sw[0]->Detection->state == RESERVED);
    CHECK(U->Sw[0]->Detection->reserved);
    CHECK(U->Sw[0]->Detection->isReservedBy(U->B[6]->train));

    // Reset
    T->dereserveBlock(U->Sw[0]->Detection);
    U->Sw[0]->Detection->state = PROCEED;
    
    // CHECK(U->Sw[0]->Detection->state == CAUTION);
  }
/*
  SECTION("II - S side"){
    // Switch has no preference and will therefore stay in the same position
    //  also if the blocks behind the switch are blocked.
    //->except if the block behind the switch is reversed and reserved/blocked TODO

    U->B[6]->path->reverse();
    
    U->B[0]->setDetection(1);
    U->B[6]->setDetection(1);
    Algorithm::process(U->B[0], _FORCE);
    Algorithm::process(U->B[6], _FORCE);

    U->B[6]->path->reserve(U->B[6]->train, U->B[6]);

    CHECK(U->Sw[0]->state == 0);

    auto T = U->B[0]->train;
    REQUIRE(T != nullptr);
    REQUIRE(T == RSManager->getTrain(0));
    REQUIRE(U->B[6]->train == RSManager->getTrain(1));
    
    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[0], U->B[2], U->B[2]->next, NEXT | FL_SWITCH_CARE);

    // Switch should change to other state and should now be reserved (locked for other trains)
    CHECK(U->Sw[0]->state == 1);
    CHECK(U->Sw[0]->Detection->state == RESERVED);
    CHECK(U->Sw[0]->Detection->reserved);
    CHECK(U->Sw[0]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->Sw[0]->Detection);
    U->Sw[0]->Detection->state = PROCEED;

    U->Sw[0]->setState(1);         // Set Diverging
    REQUIRE(U->Sw[0]->state == 1); // Verify that the switch is thrown
    Algorithm::process(U->B[1], _FORCE);   // Re-search all blocks
    Algorithm::process(U->B[2], _FORCE);
    Algorithm::process(U->B[3], _FORCE);

    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[0], U->B[2], U->B[2]->next, NEXT | FL_SWITCH_CARE);

    // Switch should stay and should now be reserved (locked for other trains)
    CHECK(U->Sw[0]->state == 1);
    CHECK(U->Sw[0]->Detection->state == RESERVED);
    CHECK(U->Sw[0]->Detection->reserved);
    CHECK(U->Sw[0]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->Sw[0]->Detection);
    U->Sw[0]->Detection->state = PROCEED;
  }
*/
  SECTION("III - S side"){
    U->B[7]->setDetection(1);
    Algorithm::process(U->B[7], _FORCE);

    auto T = U->B[7]->train;
    REQUIRE(T != nullptr);
    REQUIRE(T == RSManager->getTrain(0));
    REQUIRE(U->Sw[1]->state == 0);
    REQUIRE(U->Sw[2]->state == 0);

    T->setRoute(U->B[13]);
    
    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[7], U->B[9], U->B[9]->next, NEXT | FL_SWITCH_CARE);

    CHECK(U->Sw[1]->state == 1);
    CHECK(U->Sw[2]->state == 1);
    CHECK(U->Sw[1]->Detection->state == RESERVED);
    CHECK(U->Sw[1]->Detection->reserved);
    CHECK(U->Sw[1]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->Sw[1]->Detection);
    U->Sw[1]->Detection->state = PROCEED;

    U->Sw[1]->setState(1);         // Set Diverging
    REQUIRE(U->Sw[1]->state == 1); // Verify that the switch is thrown
    Algorithm::BlockTick();        // Re-search all blocks

    U->B[ 9]->setState(PROCEED, 0); // Force railstates
    U->B[10]->setState(PROCEED, 0); // Force railstates

    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[7], U->B[9], U->B[9]->next, NEXT | FL_SWITCH_CARE);

    // Switch should stay and should now be reserved (locked for other trains)
    //  same applies for Sw2
    CHECK(U->Sw[1]->state == 1);
    CHECK(U->Sw[2]->state == 1);
    CHECK(U->Sw[1]->Detection->state == RESERVED);
    CHECK(U->Sw[1]->Detection->reserved);
    CHECK(U->Sw[1]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->Sw[1]->Detection);
    U->Sw[1]->Detection->state = PROCEED;
  }
/*
  SECTION("III - s side"){
    // Switch has no preference and will therefore stay in the same position
    //  also if the blocks behind the switch are blocked.
    //  except if the block behind the switch is reversed and reserved/blocked TODO
    
    U->B[11]->path->reverse();
    U->B[11]->setDetection(1);
    Algorithm::process(U->B[11], _FORCE);

    REQUIRE(U->B[11]->train);

    auto T = U->B[11]->train;
    REQUIRE(T == RSManager->getTrain(0));
    
    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[11], U->B[10], U->B[10]->prev, NEXT | FL_SWITCH_CARE);

    // Switch should switch to match path
    CHECK(U->Sw[1]->state == 1);
    CHECK(U->Sw[2]->state == 0);
    CHECK(U->Sw[1]->Detection->state == RESERVED);
    CHECK(U->Sw[1]->Detection->reserved);
    CHECK(U->Sw[1]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->Sw[0]->Detection);
    U->Sw[0]->Detection->state = PROCEED;

    // U->Sw[0]->setState(1);         // Set Diverging
    // REQUIRE(U->Sw[0]->state == 1); // Verify that the switch is thrown
    // Algorithm::BlockTick();        // Re-search all blocks

    // // Force Switchsolver, since train is stopped
    // SwitchSolver::solve(U->B[6]->train, U->B[6], U->B[5], U->B[5]->prev, NEXT | FL_SWITCH_CARE);

    // // Switch should switch to match path
    // CHECK(U->Sw[0]->state == 0);
    // CHECK(U->Sw[0]->Detection->state == RESERVED);
    // CHECK(U->Sw[0]->Detection->reserved);
    // CHECK(U->Sw[0]->Detection->isReservedBy(T));

    // // Reset
    // T->dereserveBlock(U->Sw[0]->Detection);
    // U->Sw[0]->Detection->state = PROCEED;
    
    // CHECK(U->Sw[0]->Detection->state == CAUTION);
  }
*/
  SECTION("V - No Crossover"){
    // Switch has no preference and will therefore stay in the same position
    //  also if the blocks behind the switch are blocked.
    //  except if the block behind the switch is reversed and reserved/blocked TODO

    U->B[28]->setDetection(1);
    Algorithm::process(U->B[28], _FORCE);

    auto T = U->B[28]->train;
    REQUIRE(T != nullptr);
    REQUIRE(T == RSManager->getTrain(0));

    T->setRoute(U->B[35]);
    
    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[28], U->B[29], U->B[29]->next, NEXT | FL_SWITCH_CARE);

    // Switch should stay and should now be reserved (locked for other trains)
    //  it is a valid option to leave both switches untouched
    CHECK(U->Sw[5]->state == 0);
    CHECK(U->Sw[6]->state == 0);
    CHECK(U->Sw[5]->Detection->state == RESERVED);
    CHECK(U->Sw[5]->Detection->reserved);
    CHECK(U->Sw[5]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->Sw[5]->Detection);
    U->Sw[5]->Detection->state = PROCEED;

    U->Sw[5]->setState(1);         // Set Diverging
    U->Sw[6]->setState(1);         // Set Diverging
    REQUIRE(U->Sw[5]->state == 1); // Verify that the switch is thrown
    REQUIRE(U->Sw[6]->state == 1); // Verify that the switch is thrown
    Algorithm::BlockTick();        // Re-search all blocks

    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[28], U->B[29], U->B[29]->next, NEXT | FL_SWITCH_CARE);

    // Only switch 5 should move since
    CHECK(U->Sw[5]->state == 0);
    CHECK(U->Sw[6]->state == 1);
    CHECK(U->Sw[5]->Detection->state == RESERVED);
    CHECK(U->Sw[5]->Detection->reserved);
    CHECK(U->Sw[5]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->Sw[5]->Detection);
    U->Sw[5]->Detection->state = PROCEED;

    CHECK(U->Sw[ 7]->state == 0);
    CHECK(U->Sw[ 8]->state == 0);
    CHECK(U->Sw[ 9]->state == 0);
    CHECK(U->Sw[10]->state == 0);

    U->B[36]->setDetection(1);
    Algorithm::process(U->B[36], _FORCE);

    REQUIRE(U->B[36]->train);
    REQUIRE(U->B[36]->train != T);
    T = U->B[36]->train;
    
    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[36], U->B[37], U->B[37]->next, NEXT | FL_SWITCH_CARE);

    // Switch should 9 & 10 should be untouched.
    //  Switch 8 should stay and should now be reserved (locked for other trains)
    //   switch 7 should change to fix path.
    CHECK(U->Sw[ 7]->state == 1);
    CHECK(U->Sw[ 8]->state == 0);
    CHECK(U->Sw[ 9]->state == 0);
    CHECK(U->Sw[10]->state == 0);
    CHECK(U->Sw[ 7]->Detection->state == RESERVED);
    CHECK(U->Sw[ 7]->Detection->reserved);
    CHECK(U->Sw[ 7]->Detection->isReservedBy(T));
    CHECK(U->Sw[10]->Detection->state != RESERVED);
    CHECK(!U->Sw[10]->Detection->reserved);
    CHECK(!U->Sw[10]->Detection->isReservedBy(T));

  }
/*
  SECTION("V - Crossover"){
    // Switch has no preference and will therefore stay in the same position
    //  also if the blocks behind the switch are blocked.
    //  except if the block behind the switch is reversed and reserved/blocked TODO

    U->B[27]->setDetection(1);
    Algorithm::process(U->B[27], _FORCE);

    auto T = U->B[27]->train;
    REQUIRE(T != nullptr);
    REQUIRE(T == RSManager->getTrain(0));
    
    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[27], U->B[26], U->B[26]->next, NEXT | FL_SWITCH_CARE);

    // Switch should stay and should now be reserved (locked for other trains)
    CHECK(U->Sw[ 9]->state == 0);
    CHECK(U->Sw[10]->state == 0);
    CHECK(U->Sw[10]->Detection->state == RESERVED);
    CHECK(U->Sw[10]->Detection->reserved);
    CHECK(U->Sw[10]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->Sw[9]->Detection);
    U->Sw[9]->Detection->state = PROCEED;

    U->Sw[ 9]->setState(1);         // Set Diverging
    U->Sw[10]->setState(1);         // Set Diverging
    REQUIRE(U->Sw[ 7]->state == 0);
    REQUIRE(U->Sw[ 8]->state == 0);
    REQUIRE(U->Sw[ 9]->state == 1); // Verify that the switch is thrown
    REQUIRE(U->Sw[10]->state == 1); // Verify that the switch is thrown
    Algorithm::BlockTick();        // Re-search all blocks

    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[27], U->B[26], U->B[26]->next, NEXT | FL_SWITCH_CARE);

    // Switch 7 & 9 should stay and should now be reserved (locked for other trains)
    //  switch 8 & 10 should change to fix path
    CHECK(U->Sw[ 7]->state == 0);
    CHECK(U->Sw[ 8]->state == 1);
    CHECK(U->Sw[ 9]->state == 1);
    CHECK(U->Sw[10]->state == 0);
    CHECK(U->Sw[ 7]->Detection->state == RESERVED);
    CHECK(U->Sw[ 7]->Detection->reserved);
    CHECK(U->Sw[ 7]->Detection->isReservedBy(T));
    CHECK(U->Sw[10]->Detection->state == RESERVED);
    CHECK(U->Sw[10]->Detection->reserved);
    CHECK(U->Sw[10]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->Sw[ 7]->Detection);
    T->dereserveBlock(U->Sw[10]->Detection);
    U->Sw[ 7]->Detection->state = PROCEED;
    U->Sw[10]->Detection->state = PROCEED;
  }

  SECTION("VII - Straight"){
    // Switch has no preference and will therefore stay in the same position
    //  also if the blocks behind the switch are blocked.
    //  except if the block behind the switch is reversed and reserved/blocked
    
    U->B[43]->setDetection(1);
    Algorithm::process(U->B[43], _FORCE);

    REQUIRE(U->B[43]->train);

    auto T = U->B[43]->train;
    REQUIRE(T == RSManager->getTrain(0));
    
    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[43], U->B[44], U->B[44]->next, NEXT | FL_SWITCH_CARE);

    // Switch should switch to match path
    CHECK(U->MSSw[0]->state == 0);
    CHECK(U->MSSw[0]->Detection->state == RESERVED);
    CHECK(U->MSSw[0]->Detection->reserved);
    CHECK(U->MSSw[0]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->MSSw[0]->Detection);
    U->MSSw[0]->Detection->state = PROCEED;

    U->MSSw[0]->setState(1);              // Set Diverging
    REQUIRE(U->MSSw[0]->state == 1);      // Verify that the switch is thrown
    Algorithm::process(U->B[44], _FORCE); // Re-search all blocks
    Algorithm::process(U->B[45], _FORCE);
    Algorithm::process(U->B[46], _FORCE);

    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[43], U->B[44], U->B[44]->next, NEXT | FL_SWITCH_CARE);

    // Switch should switch to match path
    CHECK(U->MSSw[0]->state == 0);
    CHECK(U->MSSw[0]->Detection->state == RESERVED);
    CHECK(U->MSSw[0]->Detection->reserved);
    CHECK(U->MSSw[0]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->MSSw[0]->Detection);
    U->MSSw[0]->Detection->state = PROCEED;
    
    // CHECK(U->Sw[0]->Detection->state == CAUTION);
  }

  SECTION("VII - Crossed"){
    // Switch has no preference and will therefore stay in the same position
    //  also if the blocks behind the switch are blocked.
    //  except if the block behind the switch is reversed and reserved/blocked
    
    U->B[49]->setDetection(1);
    Algorithm::process(U->B[49], _FORCE);

    REQUIRE(U->B[49]->train);

    auto T = U->B[49]->train;
    REQUIRE(T == RSManager->getTrain(0));
    
    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[49], U->B[48], U->B[48]->next, NEXT | FL_SWITCH_CARE);

    // Switch should switch to match path
    CHECK(U->MSSw[0]->state == 1);
    CHECK(U->MSSw[0]->Detection->state == RESERVED);
    CHECK(U->MSSw[0]->Detection->reserved);
    CHECK(U->MSSw[0]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->MSSw[0]->Detection);
    U->MSSw[0]->Detection->state = PROCEED;

    U->MSSw[0]->setState(1);              // Set Diverging
    REQUIRE(U->MSSw[0]->state == 1);      // Verify that the switch is thrown
    Algorithm::process(U->B[44], _FORCE); // Re-search all blocks
    Algorithm::process(U->B[45], _FORCE);
    Algorithm::process(U->B[46], _FORCE);
    Algorithm::process(U->B[48], _FORCE);
    Algorithm::process(U->B[39], _FORCE);

    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[49], U->B[48], U->B[48]->next, NEXT | FL_SWITCH_CARE);

    // Switch should switch to match path
    CHECK(U->MSSw[0]->state == 1);
    CHECK(U->MSSw[0]->Detection->state == RESERVED);
    CHECK(U->MSSw[0]->Detection->reserved);
    CHECK(U->MSSw[0]->Detection->isReservedBy(T));

    // Reset
    T->dereserveBlock(U->MSSw[0]->Detection);
    U->MSSw[0]->Detection->state = PROCEED;
    
    // CHECK(U->Sw[0]->Detection->state == CAUTION);
  }
  */
}

TEST_CASE_METHOD(TestsFixture, "Switch Solver", "[SwSolve][SwS-3]"){
  char filenames[1][30] = {"./testconfigs/SwS-3.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;

  switchboard::SwManager->LinkAndMap();

  pathlist_find();

  /*
  // I
  //           Sw0/--->
  // 1.0> 1.1> 1.2----> 1.3>
  //
  // II
  //           ---\Sw1
  // 1.4> 1.5> ----1.6> 1.7>
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

  for(uint8_t i = 0; i < 45; i++){
    Algorithm::process(U->B[i], _FORCE);
  }

  U->Sw[0]->setState(0);
  U->Sw[1]->setState(0);
  U->Sw[2]->setState(0);
  U->Sw[3]->setState(0);
  U->Sw[4]->setState(0);
  U->Sw[5]->setState(0);
  if(AlQueue.queue->getItems())
    Algorithm::BlockTick();

  SECTION("I - Approaching S side"){
    // End of divergence is end of track therefore no valid solution
    //  - Always set the switch to straight

    U->B[0]->setDetection(1);
    Algorithm::process(U->B[0], _FORCE);

    auto T = U->B[0]->train;
    REQUIRE(T != nullptr);
    REQUIRE(T == RSManager->getTrain(0));
    
    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[0], U->B[2], U->B[2]->next, NEXT | FL_SWITCH_CARE);

    CHECK(U->Sw[0]->state == 0);
    CHECK(U->Sw[0]->Detection->state == CAUTION);
    CHECK(U->Sw[0]->Detection->reserved);
    CHECK(U->Sw[0]->Detection->isReservedBy(T));

    T->dereserveBlock(U->Sw[0]->Detection);
    U->Sw[0]->Detection->state = PROCEED;

    U->Sw[0]->setState(1); // Set Diverging
    CHECK(U->Sw[0]->state == 1);

    Algorithm::BlockTick();

    // Force Switchsolver, since train is stopped
    SwitchSolver::solve(T, U->B[0], U->B[2], U->B[2]->next, NEXT | FL_SWITCH_CARE);

    CHECK(U->Sw[0]->state == 0);
    CHECK(U->Sw[0]->Detection->state == RESERVED);
    CHECK(U->Sw[0]->Detection->reserved);
    CHECK(U->Sw[0]->Detection->isReservedBy(T));

    Algorithm::BlockTick();
    
    CHECK(U->Sw[0]->Detection->state == CAUTION);
  }

  SECTION("II - Approaching s side"){
    // Set the switch to the correct state
    //  - set straight
    U->B[4]->setDetection(1);
    Algorithm::process(U->B[4], _FORCE);

    auto T = U->B[4]->train;
    CHECK(T != nullptr);
    CHECK(T == RSManager->getTrain(0));

    // Force SwitchSolver, since no speed
    SwitchSolver::solve(T, U->B[4], U->B[5], U->B[5]->next, NEXT | FL_SWITCH_CARE);

    CHECK(U->Sw[1]->state == 0);
    // CHECK(U->Sw[1]->Detection->state == CAUTION); // Switchsolver does not set state
    CHECK(U->Sw[1]->Detection->reserved);
    CHECK(U->Sw[1]->Detection->isReservedBy(T));

    T->dereserveBlock(U->Sw[1]->Detection);

    U->Sw[1]->setState(1); // Set Diverging
    CHECK(U->Sw[1]->state == 1);

    Algorithm::BlockTick();

    // Force SwitchSolver, since no speed
    SwitchSolver::solve(T, U->B[4], U->B[5], U->B[5]->next, NEXT | FL_SWITCH_CARE);

    CHECK(U->Sw[1]->state == 0);
    CHECK(U->Sw[1]->Detection->state == RESERVED);
    CHECK(U->Sw[1]->Detection->reserved);
    CHECK(U->Sw[1]->Detection->isReservedBy(T));

    // Algorithm::BlockTick();
    
    // CHECK(U->Sw[1]->Detection->state == CAUTION); Not the job of switchsolver
  }

  SECTION("IIIa - Approaching S side with station"){
    // Train will try to go around stopped train inside station
    //  - Set switch to diverging
    //  - Reserve Switch

    U->B[8]->setDetection(1);
    U->B[14]->setDetection(1);
    Algorithm::process(U->B[8], _FORCE);
    Algorithm::process(U->B[14], _FORCE);

    auto T = U->B[8]->train;
    REQUIRE(T != nullptr);
    REQUIRE(T == RSManager->getTrain(0));
    CHECK(U->B[14]->train == RSManager->getTrain(1));
    CHECK(U->B[14]->train->stopped);
    CHECK(U->B[14]->station->stoppedTrain);

    RSManager->getTrain(0)->setSpeed(10);

    // Force switchsolver, since speed is too low
    SwitchSolver::solve(T, U->B[8], U->B[10], U->B[10]->next, NEXT | FL_SWITCH_CARE);

    CHECK(U->Sw[2]->state == 1);
    CHECK(U->Sw[2]->Detection->state == RESERVED);
    CHECK(U->Sw[2]->Detection->reserved);
    CHECK(U->Sw[2]->Detection->isReservedBy(T));
  }

  SECTION("IIIb - Approaching S side with station"){
    // Train will try to go around stopped train inside station
    //   even if it is allready reserved
    //  - Set switch to diverging
    //  - Reserve Switch

    U->B[8]->setDetection(1);
    Algorithm::process(U->B[8], _FORCE);
    
    auto T = U->B[8]->train;
    REQUIRE(T == RSManager->getTrain(0));

    RSManager->getTrain(0)->setSpeed(10);
    
    // Force switchsolver, since speed is too low
    SwitchSolver::solve(T, U->B[8], U->B[10], U->B[10]->next, NEXT | FL_SWITCH_CARE);
    
    CHECK(U->Sw[2]->state == 0);
    CHECK(U->Sw[2]->Detection->reserved);
    CHECK(U->Sw[2]->Detection->isReservedBy(U->B[8]->train));

    U->B[14]->setDetection(1);
    Algorithm::process(U->B[14], _FORCE);

    CHECK(U->B[14]->train == RSManager->getTrain(1));
    CHECK(U->B[14]->station->occupied);
    CHECK(U->B[14]->station->stoppedTrain);

    // Force switchsolver, since speed is too low
    SwitchSolver::solve(T, U->B[8], U->B[10], U->B[10]->next, NEXT | FL_SWITCH_CARE);
    
    CHECK(U->Sw[2]->state == 1);
    CHECK(U->Sw[2]->Detection->reserved);
    CHECK(U->Sw[2]->Detection->isReservedBy(U->B[8]->train));
  }

  SECTION("IIIc - Approaching S side with station"){
    // Paths after a switch must be set to the correct direction
    //  - Reserve Switch
    //  - Reserve Path

    U->B[11]->path->reverse();
    U->B[13]->path->reverse();

    CHECK(U->B[11]->dir == PREV);
    CHECK(U->B[13]->dir == PREV);

    U->B[8]->setDetection(1);
    Algorithm::process(U->B[8], _FORCE);
    
    CHECK(U->B[8]->train == RSManager->getTrain(0));

    RSManager->getTrain(0)->setSpeed(10);
    
    // Force switchsolver, since speed is too low
    SwitchSolver::solve(U->B[8]->train, U->B[8], U->B[10], U->B[10]->next, NEXT | FL_SWITCH_CARE);
    
    CHECK(U->Sw[2]->state == 0);
    CHECK(U->Sw[2]->Detection->state == CAUTION);
    CHECK(U->Sw[2]->Detection->reserved);
    CHECK(U->Sw[2]->Detection->isReservedBy(U->B[8]->train));

    CHECK(U->B[11]->dir == PREV);
    CHECK(U->B[13]->dir == NEXT);
  }
  
  SECTION("IIId - Approaching S side with station"){
    // Paths after a switch must be set to the correct direction
    //  - Set Switch to diverging
    //  - Reserve Switch
    //  - Reserve Path

    U->B[11]->path->reverse();
    U->B[13]->path->reverse();
    U->B[13]->path->reserve(new Train(U->B[0]));

    CHECK(U->B[11]->dir == PREV);
    CHECK(U->B[13]->dir == PREV);

    U->B[8]->setDetection(1);
    Algorithm::process(U->B[8], _FORCE);
    
    auto T = U->B[8]->train;
    CHECK(T == RSManager->getTrain(1));

    T->setSpeed(10);
    
    // Force switchsolver, since speed is too low
    SwitchSolver::solve(U->B[8]->train, U->B[8], U->B[10], U->B[10]->next, NEXT | FL_SWITCH_CARE);
    
    CHECK(U->Sw[2]->state == 1);
    CHECK(U->Sw[2]->Detection->state == RESERVED);
    CHECK(U->Sw[2]->Detection->reserved);
    CHECK(U->Sw[2]->Detection->isReservedBy(U->B[8]->train));

    CHECK(U->B[11]->dir == 0);
    CHECK(U->B[13]->dir == PREV);
  }

  SECTION("IV - Approaching S side with station fully blocked"){
    // If no valid solution is possible, both states lead to invalid solution.
    //  - enable switchWrongState
    //  - block state must be DANGER
    //  - block must not be reserved

    U->B[8]->setDetection(1);
    U->B[12]->setDetection(1);
    U->B[14]->setDetection(1);
    Algorithm::process(U->B[8], _FORCE);
    Algorithm::process(U->B[12], _FORCE);
    Algorithm::process(U->B[14], _FORCE);

    CHECK(U->B[8]->train != nullptr);
    CHECK(U->B[8]->train == RSManager->getTrain(0));

    RSManager->getTrain(0)->setSpeed(10);

    // Force SwitchSolver, since speed is too low
    SwitchSolver::solve(U->B[8]->train, U->B[8], U->B[10], U->B[10]->next, NEXT | FL_SWITCH_CARE);

    CHECK(U->Sw[2]->state == 0);
    CHECK(U->Sw[2]->Detection->state == DANGER);
    CHECK(U->Sw[2]->Detection->state != RESERVED);
    CHECK(U->Sw[2]->Detection->switchWrongState);
    CHECK(!U->Sw[2]->Detection->reserved);
    CHECK(!U->Sw[2]->Detection->reserved);
  }

  SECTION("V - Approaching s side with station and switchblock"){
    // Go around B[19]
    //  - set Sw[4] straight
    //  - set Sw[5] diverging

    U->B[15]->setDetection(1);
    U->B[19]->setDetection(1);
    Algorithm::process(U->B[15], _FORCE);
    Algorithm::process(U->B[19], _FORCE);

    U->Sw[4]->setState(1);
    Algorithm::BlockTick();

    // Algorithm::process(U->B[15], _FORCE);
    // U->St[3]->train -> SIGSEV
    CHECK(U->B[19]->station == U->St[3]);
    CHECK(U->B[19]->train == U->St[3]->train);
    U->B[19]->train->setSpeed(0);

    CHECK(U->B[15]->train != nullptr);
    CHECK(U->B[15]->train == RSManager->getTrain(0));

    RSManager->getTrain(0)->setSpeed(10);
    
    // Force SwitchSolver, since speed is too low
    SwitchSolver::solve(U->B[15]->train, U->B[15], U->B[16], U->B[16]->next, NEXT | FL_SWITCH_CARE);

    CHECK(U->Sw[4]->state == 0);
    CHECK(U->Sw[5]->state == 1);

    CHECK(U->Sw[4]->Detection->reserved);
    CHECK(U->Sw[4]->Detection->isReservedBy(U->B[15]->train));
  }

  SECTION("VI- Approaching S side with full station and switchblock"){
    // No valid solution therefore
    //  - do not set switcWrongState (switch state is not changed therefore it is implicit that switc is in the wrong state)
    //  - set DANGER
    //  - not reserved

    U->B[15]->setDetection(1);
    U->B[19]->setDetection(1);
    U->B[21]->setDetection(1);
    Algorithm::process(U->B[15], _FORCE);
    Algorithm::process(U->B[19], _FORCE);
    Algorithm::process(U->B[21], _FORCE);

    // U->St[2]->train->setSpeed(0);
    // U->St[3]->train->setSpeed(0);
    U->B[19]->train->setSpeed(0);
    U->B[21]->train->setSpeed(0);

    // U->B[15]->train->dereserveBlock(U->Sw[4]->Detection); // Allow switch to change

    U->Sw[4]->setState(1, false, false);
    Algorithm::BlockTick();

    CHECK(U->Sw[4]->state == 1);
    CHECK(U->Sw[5]->state == 0);

    CHECK(U->B[15]->train != nullptr);
    CHECK(U->B[15]->train == RSManager->getTrain(0));

    RSManager->getTrain(0)->setSpeed(10);
        
    // Force SwitchSolver, since speed is too low
    SwitchSolver::solve(U->B[15]->train, U->B[15], U->B[16], U->B[16]->next, NEXT | FL_SWITCH_CARE);

    CHECK(U->Sw[4]->state == 1);
    CHECK(U->Sw[5]->state == 0);

    CHECK( U->Sw[4]->Detection->state == DANGER);
    CHECK(!U->Sw[4]->Detection->switchWrongState);
    CHECK(!U->Sw[4]->Detection->reserved);
    CHECK(!U->Sw[4]->Detection->reserved);
  }

  SECTION("VII - Approaching switch with route"){
    U->B[22]->setDetection(1);
    Algorithm::process(U->B[22], _FORCE);

    auto T = U->B[22]->train;

    REQUIRE(T);

    T->setRoute(U->B[21]);
    
    REQUIRE(T->route);
    REQUIRE(T->route->found_forward);

    CHECK(T != nullptr);
    CHECK(T == RSManager->getTrain(0));

    RSManager->getTrain(0)->setSpeed(10);

    // Force SwitchSolver, since speed is too low
    SwitchSolver::solve(U->B[22]->train, U->B[22], U->B[24], U->B[24]->next, NEXT | FL_SWITCH_CARE);


    CHECK(U->Sw[3]->state == 1);
    CHECK(U->Sw[4]->state == 1);
    CHECK(U->Sw[5]->state == 1);
  }

  SECTION("VIII - Approaching switch with route with blocked station"){
    U->B[22]->setDetection(1);
    U->B[21]->setDetection(1);
    Algorithm::process(U->B[22], _FORCE);
    Algorithm::process(U->B[21], _FORCE);

    auto T = U->B[22]->train;

    REQUIRE(T);

    T->setRoute(U->B[21]);

    REQUIRE(T->route);
    REQUIRE(T->route->found_forward);

    CHECK(T == RSManager->getTrain(0));

    RSManager->getTrain(0)->setSpeed(10);

    // Force SwitchSolver, since speed is too low
    SwitchSolver::solve(U->B[22]->train, U->B[22], U->B[24], U->B[24]->next, NEXT | FL_SWITCH_CARE);

    CHECK(U->Sw[3]->state == 0);
    CHECK(U->Sw[4]->state == 0);
    CHECK(U->Sw[5]->state == 0);

    CHECK(U->B[24]->switchWrongState);
  }

  SECTION("IX - Approaching switch with route applying detour"){
    U->B[22]->setDetection(1);
    U->B[26]->setDetection(1);
    Algorithm::process(U->B[22], _FORCE);
    Algorithm::process(U->B[26], _FORCE);

    auto T = U->B[22]->train;

    REQUIRE(T);
    CHECK(T == RSManager->getTrain(0));
    REQUIRE(U->B[26]->train);
    CHECK(U->B[26]->train == RSManager->getTrain(1));

    T->setRoute(U->B[33]);

    REQUIRE(T->route);
    REQUIRE(T->route->found_forward);

    RSManager->getTrain(0)->setSpeed(10);

    // Force SwitchSolver, since speed is too low
    SwitchSolver::solve(U->B[22]->train, U->B[22], U->B[24], U->B[24]->next, NEXT | FL_SWITCH_CARE);

    CHECK(U->Sw[3]->state == 1);
    CHECK(U->Sw[4]->state == 1);
    CHECK(U->Sw[5]->state == 0);
    CHECK(U->Sw[5]->updatedState == 0); // Switch should not be updated since it is allready in position

    CHECK(U->B[24]->isReservedBy(U->B[22]->train));
  }

  SECTION("X - Approaching S with reserved switches"){
    Train * tmpRT = new Train(U->B[17]);
    U->B[17]->reserve(tmpRT);

    U->Sw[3]->setState(1);
    Algorithm::BlockTick();

    U->B[22]->setDetection(1);
    Algorithm::process(U->B[22], _FORCE);

    REQUIRE(U->B[22]->train);
    U->B[22]->train->setSpeed(10);

    // Force SwitchSolver, since speed is too low
    SwitchSolver::solve(U->B[22]->train, U->B[22], U->B[24], U->B[24]->next, NEXT | FL_SWITCH_CARE);

    CHECK(U->Sw[3]->state == 0);
    CHECK(U->Sw[4]->state == 0);
    CHECK(U->Sw[5]->state == 0);

    CHECK(U->B[24]->isReservedBy(U->B[22]->train));
  }

  SECTION("XI - Reversed Station path"){
    Algorithm::print_block_debug(U->B[37]);
    REQUIRE(U->B[36]->path != U->B[37]->path);
    REQUIRE(U->B[37]->path->Entrance == U->B[38]);
    REQUIRE(U->B[37]->path->Exit     == U->B[37]);

    CHECK(U->B[37]->path->direction == 0);
    CHECK(U->B[37]->path->polarity  == 0);

    U->B[22]->setDetection(1);
    Algorithm::process(U->B[22], _FORCE);

    REQUIRE(U->B[22]->train);
    U->B[22]->train->setRoute(U->B[39]);
    U->B[22]->train->setSpeed(10);

    REQUIRE(U->B[22]->train->routeStatus == TRAIN_ROUTE_RUNNING);

    // Force SwitchSolver, since speed is too low
    SwitchSolver::solve(U->B[22]->train, U->B[22], U->B[24], U->B[24]->next, NEXT | FL_SWITCH_CARE);

    CHECK(U->Sw[3]->state == 0);
    CHECK(U->Sw[9]->state == 1);
    CHECK(U->Sw[10]->state == 1);

    CHECK(U->B[24]->isReservedBy(U->B[22]->train));
    CHECK(U->B[36]->isReservedBy(U->B[22]->train));
    CHECK(U->B[37]->isReservedBy(U->B[22]->train));

    CHECK(U->B[36]->dir == 1); // Reversed
    CHECK(U->B[37]->path->direction == 1);

    // Switch solver should not do POLARITY, FIXME
    CHECK(U->B[37]->path->polarity  == POLARITY_REVERSED);
    CHECK(U->B[36]->polarity_status == POLARITY_REVERSED);

    // Running again should not change anything
    SwitchSolver::solve(U->B[22]->train, U->B[22], U->B[24], U->B[24]->next, NEXT | FL_SWITCH_CARE);

    CHECK(U->Sw[3]->state == 0);
    CHECK(U->Sw[9]->state == 1);
    CHECK(U->Sw[10]->state == 1);

    CHECK(U->B[24]->isReservedBy(U->B[22]->train));
    CHECK(U->B[36]->isReservedBy(U->B[22]->train));
    CHECK(U->B[37]->isReservedBy(U->B[22]->train));

    CHECK(U->B[36]->dir == 1);
    CHECK(U->B[37]->path->direction == 1);

    // Switch solver should not do POLARITY, FIXME
    CHECK(U->B[37]->path->polarity  == POLARITY_REVERSED);
    CHECK(U->B[36]->polarity_status == POLARITY_REVERSED);
  }

  SECTION("XII - Reserved reversed Station path"){
    REQUIRE(U->B[36]->path != U->B[37]->path);
    REQUIRE(U->B[37]->path->Entrance != U->B[37]);

    Train * tmpRT = new Train(U->B[37]);
    U->B[37]->path->reserve(tmpRT);

    U->B[22]->setDetection(1);
    Algorithm::process(U->B[22], _FORCE);

    REQUIRE(U->B[22]->train);
    U->B[22]->train->setRoute(U->B[39]);
    auto route = U->B[22]->train->route;

    REQUIRE(route);
    U->B[22]->train->setSpeed(10);

    // Force SwitchSolver, since speed is too low
    SwitchSolver::solve(U->B[22]->train, U->B[22], U->B[24], U->B[24]->next, NEXT | FL_SWITCH_CARE);

    CHECK(U->B[24]->switchWrongState);
    CHECK(!U->B[24]->reserved);
    CHECK(!U->B[26]->reserved);

    CHECK(U->B[37]->isReservedBy(U->B[22]->train) == false);

    
  }

  SECTION("XIII - Switchover"){
    CHECK(!U->B[43]->path->direction);

    U->Sw[11]->setState(1);
    U->Sw[12]->setState(1);

    Algorithm::BlockTick();

    auto T = new Train(U->B[45]);
    U->B[45]->path->reserve(T);

    CHECK(U->B[42]->state == DANGER);


    U->B[45]->expectedTrain = T;
    U->B[45]->setDetection(1);
    Algorithm::process(U->B[45], _FORCE);
    
    // Force SwitchSolver, since speed is too low
    SwitchSolver::solve(T, U->B[45], U->B[47], U->B[47]->next, NEXT | FL_SWITCH_CARE);

    CHECK(U->B[42]->dir == 1);
    CHECK(U->B[43]->dir == 1);
    CHECK(U->B[44]->dir == 1);
    CHECK(U->B[43]->path->direction);

    CHECK(U->B[42]->isReservedBy(T));
    CHECK(U->B[43]->isReservedBy(T));

    CHECK(U->B[42]->state == RESERVED);
    CHECK(U->B[43]->state == RESERVED);
    CHECK(U->B[44]->state == RESERVED);

    CHECK(U->B[47]->state == RESERVED);
    CHECK(U->B[44]->state == RESERVED);
  }

  // TODO add testcases for MSSwitch
}
