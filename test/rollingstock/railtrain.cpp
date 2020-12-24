#include "catch.hpp"

#include "utils/mem.h"
#include "utils/logger.h"
#include "system.h"

#include "rollingstock/railtrain.h"
#include "switchboard/manager.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/station.h"
#include "switchboard/unit.h"

#include "algorithm/queue.h"
#include "algorithm/core.h"
#include "algorithm/component.h"
#include "algorithm/blockconnector.h"

void init_test(char (* filenames)[30], int nr_files);

TEST_CASE("RailTrain ContinueChecker", "[RT][RT-1]"){
  char filenames[1][30] = {"./testconfigs/RT-1.bin"};
  init_test(filenames, 1);

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  switchboard::SwManager->linkAll();

  /*
  //  
  //
  //  -1.4-> -1.5-> -1.6-> -\                          /---> -1.10-> -1.11->
  //  -1.0-> -1.1-> -1.2-> ----1.3-> -1.7-> -1.8-> -1.9----> -1.12-> -1.13->
  //
  */

  /*
    - A train can only start moving iff
      * There is a block
      * Next block is safe (below DANGER)
      * If a (ms)switch can be set to a state that satifies above
  */

  SECTION("I - Normal, all free"){
    U->B[1]->setDetection(1);
    Algorithm::process(U->B[1], _FORCE);

    RailTrain * T = U->B[1]->train;
    REQUIRE(T);
    T->B = U->B[1];
    T->directionKnown = 1;

    CHECK(T->ContinueCheck());
  }

  SECTION("II - Switch Wrong, but too far away"){
    U->Sw[0]->setState(1);
    Algorithm::tick();

    CHECK(U->Sw[0]->state == 1);

    U->B[1]->setDetection(1);
    Algorithm::process(U->B[1], _FORCE);

    RailTrain * T = U->B[1]->train;
    REQUIRE(T);
    T->B = U->B[1];
    T->directionKnown = 1;

    CHECK(T->ContinueCheck());
    Algorithm::tick();

    CHECK(U->Sw[0]->state == 1);
  }

  SECTION("III - Switch Wrong"){
    U->Sw[0]->setState(1);
    Algorithm::tick();

    CHECK(U->Sw[0]->state == 1);

    U->B[2]->setDetection(1);
    Algorithm::process(U->B[2], _FORCE);

    RailTrain * T = U->B[2]->train;
    REQUIRE(T);
    T->B = U->B[2];
    T->directionKnown = 1;

    CHECK(T->ContinueCheck());
    Algorithm::tick();

    CHECK(U->Sw[0]->state == 0);
    CHECK(U->Sw[0]->Detection->switchReserved);
    CHECK(U->Sw[0]->Detection->reservedBy == T);
  }

  SECTION("IV - Reserved Switch Wrong"){
    U->Sw[0]->setState(1);
    Algorithm::tick();

    U->Sw[0]->Detection->reservedBy = (RailTrain *)1;
    U->Sw[0]->Detection->switchReserved = 1;
    U->Sw[0]->Detection->setState(RESERVED_SWITCH);

    CHECK(U->Sw[0]->state == 1);

    U->B[2]->setDetection(1);
    Algorithm::process(U->B[2], _FORCE);

    RailTrain * T = U->B[2]->train;
    REQUIRE(T);
    T->B = U->B[2];
    T->directionKnown = 1;

    CHECK(!T->ContinueCheck());
  }

  SECTION("V - Switch Wrong state S side"){
    U->B[13]->setDetection(1);
    Algorithm::process(U->B[13], _FORCE);
    U->B[11]->setDetection(1);
    Algorithm::process(U->B[11], _FORCE);

    U->B[8]->setDetection(1);

    Algorithm::process(U->B[8], _FORCE);

    RailTrain * T = U->B[8]->train;
    REQUIRE(T);
    T->setStopped(false);
    T->B = U->B[8];
    T->directionKnown = 1;
    
    Algorithm::process(U->B[8], _FORCE);

    REQUIRE(U->B[11]->station->stoppedTrain);
    REQUIRE(U->B[13]->station->stoppedTrain);
    REQUIRE(U->B[9]->switchWrongState);


    CHECK(!T->ContinueCheck());

    U->B[11]->train->setStopped(false);

    REQUIRE(!U->B[11]->station->stoppedTrain);

    CHECK(T->ContinueCheck());

    CHECK(!U->B[9]->switchWrongState);
    CHECK(U->Sw[1]->state == 1);
  }

  SECTION("VI - Switch Wrong state s side"){
    // logger.setlevel_stdout(TRACE);
    U->Sw[0]->setState(1);
    Algorithm::tick();

    U->B[3]->setDetection(1);
    Algorithm::process(U->B[3], _FORCE);

    U->B[2]->setDetection(1);
    Algorithm::process(U->B[2], _FORCE);

    RailTrain * T = U->B[2]->train;
    REQUIRE(T);

    CHECK(!T->ContinueCheck());

    // U->B[11]->train->setStopped(false);

    // REQUIRE(!U->B[11]->station->stoppedTrain);

    // CHECK(T->ContinueCheck());

    // CHECK(!U->B[9]->switchWrongState);
    // CHECK(U->Sw[1]->state == 1);
  }
}

TEST_CASE("RailTrain Reserve Paths", "[RT][RT-2]"){
  char filenames[1][30] = {"./testconfigs/RT-1.bin"};
  init_test(filenames, 1);

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  switchboard::SwManager->linkAll();

  /*
  //  
  //
  //  -1.4-> -1.5-> -1.6-> -\                          /---> -1.10-> -1.11->
  //  -1.0-> -1.1-> -1.2-> ----1.3-> -1.7-> -1.8-> -1.9----> -1.12-> -1.13->
  //
  */

  /*
    - A moving train shall reserve the forthcoming blocks / paths.
      * Only when the direction is known
    - A train shall release the reserved blocks if it stops.
  */


  SECTION("I - Normal, all free"){
    REQUIRE(U->B[0]->path);

    U->B[0]->setDetection(1);
    Algorithm::process(U->B[0], _FORCE);

    RailTrain * T = U->B[0]->train;
    CHECK(T);

    // Direction Train of train is not known and it is not moving
    //  therefore no block can be reserved
    CHECK(!U->B[1]->reserved);
    CHECK(!U->B[0]->path->reserved);

    CHECK(U->B[0]->path->trains[0] == T);
  }

  SECTION("II - Normal, all free"){
    REQUIRE(U->B[0]->path);

    U->B[0]->setDetection(1);
    Algorithm::process(U->B[0], _FORCE);

    RailTrain * T = U->B[0]->train;
    CHECK(T);
    loggerf(INFO, "Start moving");
    T->setSpeed(10);

    loggerf(INFO, "Detecting next block");
    U->B[1]->setDetection(1);
    Algorithm::process(U->B[1], _FORCE);
    U->B[1]->recalculate = 0;
    Algorithm::process(U->B[1], _FORCE);

    REQUIRE(T->directionKnown);
    CHECK(T->B == U->B[1]);

    // Direction Train of train is known 
    //  therefore first next group can be reserved
    CHECK(U->B[2]->reserved);
    CHECK(U->B[2]->path->reserved);
    CHECK(U->B[2]->path->trains[0] == T);

    CHECK(U->B[3]->reserved);
    CHECK(U->B[3]->state == RESERVED_SWITCH);

    CHECK(U->B[7]->path->reserved);
    CHECK(U->B[7]->state == RESERVED);

    // Stop the train
    T->setSpeed(0);

    CHECK(!U->B[2]->reserved);
    CHECK(!U->B[3]->reserved);
    CHECK(!U->B[7]->reserved);
  }

  SECTION("III - Normal, switch wrong state"){
    REQUIRE(U->B[0]->path);

    U->Sw[0]->setState(1);

    U->B[0]->setDetection(1);
    Algorithm::process(U->B[0], _FORCE);

    RailTrain * T = U->B[0]->train;
    CHECK(T);
    loggerf(INFO, "Start moving");
    T->setSpeed(10);

    loggerf(INFO, "Detecting next block");
    U->B[1]->setDetection(1);
    Algorithm::process(U->B[1], _FORCE);
    U->B[1]->recalculate = 0;
    Algorithm::process(U->B[1], _FORCE);

    REQUIRE(T->directionKnown);
    CHECK(T->B == U->B[1]);

    // Direction Train of train is known 
    //  therefore first next group can be reserved
    CHECK(U->B[2]->reserved);
    CHECK(U->B[2]->path->reserved);
    CHECK(U->B[2]->path->trains[0] == T);

    CHECK(U->B[3]->reserved);
    CHECK(U->B[3]->state == RESERVED_SWITCH);
    CHECK(U->Sw[0]->state == 0);

    CHECK(U->B[7]->path->reserved);
    CHECK(U->B[7]->state == RESERVED);

    // Stop the train
    T->setSpeed(0);

    CHECK(!U->B[2]->reserved);
    CHECK(!U->B[3]->reserved);
    CHECK(!U->B[7]->reserved);
  }

  SECTION("IV - Normal, switch blocked"){
    REQUIRE(U->B[0]->path);

    U->Sw[0]->setState(1);
    U->B[3]->switchReserved = true;
    U->B[3]->reserved = 1;
    U->B[3]->reservedBy = (RailTrain *)1;

    U->B[0]->setDetection(1);
    Algorithm::process(U->B[0], _FORCE);

    RailTrain * T = U->B[0]->train;
    CHECK(T);
    loggerf(INFO, "Start moving");
    T->setSpeed(10);

    loggerf(INFO, "Detecting next block");
    U->B[1]->setDetection(1);
    Algorithm::process(U->B[1], _FORCE);
    U->B[1]->recalculate = 0;
    Algorithm::process(U->B[1], _FORCE);

    REQUIRE(T->directionKnown);
    CHECK(T->B == U->B[1]);

    // Direction Train of train is known 
    //  therefore first next group can be reserved
    CHECK(U->B[2]->reserved);
    CHECK(U->B[2]->path->reserved);
    CHECK(U->B[2]->path->trains[0] == T);

    CHECK(!U->B[7]->path->reserved);
    CHECK(U->B[7]->state != RESERVED);

    // Stop the train
    T->setSpeed(0);

    CHECK(!U->B[2]->reserved);
    CHECK(U->B[3]->reserved);
  }
}

TEST_CASE("RailTrain Reverse", "[RT][RT-3]"){
  char filenames[1][30] = {"./testconfigs/RT-2.bin"};
  init_test(filenames, 1);

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;
  
  switchboard::SwManager->linkAll();

  /*
  //  1.0> 1.1> 1.2> 1.3> 1.4> 1.5> 1.6> 1.7>
  */

  /*
    - A reversing train shall try to reverse the blocks it is in
      * If failed the reversing shall be undone
  */

  SECTION("Ia - Reverse train - 1 long - not linked"){
    loggerf(WARNING, "Ia");
    // If a train turns arround the blocks must also be reversed iff it is allowed
    U->B[3]->setDetection(1);
    Algorithm::process(U->B[3], _FORCE);

    RailTrain * T = U->B[3]->train;
    REQUIRE(T);

    CHECK(!T->B);
    CHECK(!T->dir);

    T->reverse();

    CHECK(T->dir);
    CHECK(!T->B);

    CHECK(U->B[3]->Alg.N[0] == U->B[2]);
    CHECK(U->B[3]->Alg.P[0] == U->B[4]);
  }

  SECTION("Ib - Reverse train - 1 long - linked"){
    loggerf(WARNING, "Ib");
    // Reversing of a train should not depend whether it is linked or not
    U->B[3]->setDetection(1);
    Algorithm::process(U->B[3], _FORCE);

    RailTrain * T = U->B[3]->train;
    REQUIRE(T);
    T->link(1, RAILTRAIN_ENGINE_TYPE);
    T->setStopped(false);
    T->directionKnown = true;
    T->B = U->B[3];

    CHECK(!T->dir);

    T->reverse();

    CHECK(T->dir);
    CHECK(T->B == U->B[3]);

    CHECK(U->B[3]->Alg.N[0] == U->B[2]);
    CHECK(U->B[3]->Alg.P[0] == U->B[4]);
  }

  SECTION("IIa - Reverse train - 2 long - not linked"){
    loggerf(WARNING, "IIa");
    U->B[3]->setDetection(1);
    Algorithm::process(U->B[3], _FORCE);
    U->B[4]->setDetection(1);
    Algorithm::process(U->B[4], _FORCE);

    RailTrain * T = U->B[3]->train;
    REQUIRE(T);

    CHECK(!T->B);

    T->reverse();

    CHECK(T->dir);
    CHECK(!T->B);

    CHECK(U->B[3]->Alg.N[0] == U->B[2]);
    CHECK(U->B[3]->Alg.P[0] == U->B[4]);
  }

  SECTION("IIb - Reverse train - 2 long - linked"){
    loggerf(WARNING, "IIb");
    U->B[3]->setDetection(1);
    Algorithm::process(U->B[3], _FORCE);

    RailTrain * T = U->B[3]->train;
    REQUIRE(T);
    T->link(1, RAILTRAIN_ENGINE_TYPE);
    T->setSpeed(10);

    U->B[4]->setDetection(1);
    Algorithm::process(U->B[4], _FORCE);

    CHECK(T->B == U->B[4]);

    T->reverse();

    CHECK(T->dir);
    CHECK(T->speed == 0);
    CHECK(T->B == U->B[3]);

    CHECK(U->B[3]->Alg.N[0] == U->B[2]);
    CHECK(U->B[3]->Alg.P[0] == U->B[4]);
  }
}

TEST_CASE("RailTrain Front of Train", "[RT][RT-4]"){
  char filenames[1][30] = {"./testconfigs/RT-1.bin"};
  init_test(filenames, 1);

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;
  switchboard::SwManager->linkAll();

  /*
  //  
  //
  //  -1.4-> -1.5-> -1.6-> -\                          /---> -1.10-> -1.11->
  //  -1.0-> -1.1-> -1.2-> ----1.3-> -1.7-> -1.8-> -1.9----> -1.12-> -1.13->
  //
  */

  /*
    - The train knows which block contains the front part of the train
      * It remains undefined if direction is unkown
  */

  SECTION("I - Put train on track"){
    U->B[1]->setDetection(1);
    Algorithm::process(U->B[1], _FORCE);

    RailTrain * T = U->B[1]->train;
    REQUIRE(T);

    // Link engine
    T->link(1, RAILTRAIN_TRAIN_TYPE);

    CHECK(!T->B);

    U->B[2]->setDetection(1);
    Algorithm::process(U->B[2], _FORCE);

    CHECK(!T->B);

    U->B[0]->setDetection(0);
    Algorithm::process(U->B[0], _FORCE);

    CHECK(!T->B);
  }

  SECTION("IIa - Move train after link, forward"){
    U->B[1]->setDetection(1);
    Algorithm::process(U->B[1], _FORCE);
    U->B[0]->setDetection(1);
    Algorithm::process(U->B[0], _FORCE);

    REQUIRE(U->B[1]->train != 0);
    RailTrain * T = U->B[1]->train;
    REQUIRE(U->B[0]->train == T);

    CHECK(!T->B);

    // Link engine
    T->link(1, RAILTRAIN_TRAIN_TYPE);
    T->setSpeed(10);

    U->B[2]->setDetection(1);
    Algorithm::process(U->B[2], _FORCE);

    CHECK(T->B == U->B[2]);
    CHECK(T->directionKnown);
  }

  SECTION("IIb - Move train after link, reverse"){
    U->B[7]->setDetection(1);
    Algorithm::process(U->B[7], _FORCE);
    U->B[8]->setDetection(1);
    Algorithm::process(U->B[8], _FORCE);

    REQUIRE(U->B[7]->train != 0);
    RailTrain * T = U->B[7]->train;
    REQUIRE(U->B[8]->train == T);

    CHECK(!T->B);
    CHECK(!T->dir);

    // Link engine
    T->link(1, RAILTRAIN_TRAIN_TYPE);
    T->setSpeed(10);

    U->B[3]->setDetection(1);
    Algorithm::process(U->B[3], _FORCE);

    // Train is moving in the opposite direction
    //  train is reversed automatically

    CHECK(T->dir);

    CHECK(T->B == U->B[8]);
    CHECK(T->directionKnown);
  }
}
