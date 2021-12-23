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
#include "switchboard/signals.h"
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

TEST_CASE_METHOD(TestsFixture, "Connector Algorithm", "[Alg][Alg-1]"){
  char filenames[5][30] = {"./testconfigs/Alg-1-1.bin",
                           "./testconfigs/Alg-1-2.bin",
                           "./testconfigs/Alg-1-3.bin",
                           "./testconfigs/Alg-1-4.bin",
                           "./testconfigs/Alg-1-5.bin"};
  loadSwitchboard(filenames, 5);
  loadStock();

  Unit * U[6] = {0, switchboard::Units(1), switchboard::Units(2), switchboard::Units(3), switchboard::Units(4), switchboard::Units(5)};

  REQUIRE(U[1]);
  REQUIRE(U[2]);
  REQUIRE(U[3]);
  REQUIRE(U[4]);
  REQUIRE(U[5]);

  U[1]->on_layout = true;
  U[2]->on_layout = true;
  U[3]->on_layout = true;
  U[4]->on_layout = true;
  U[5]->on_layout = true;

  /*                                            o                   o
  //                              --\           |                   |   --\
  //  1.0->  | --2.0-> --2.1->  |  --3.0-> --3.1-> | --4.0-> --4.1--> | ---5.0->
  //     C1-1 C1-1          C2-1 C1-1          C2-1 C1-2          C2-1
  //     C1-2 C1-2          C2-2 C1-2          C2-2 C1-1          C2-2
  // <1.1--  | <-2.2-- <-2.3--  |  <-3.2-- <-3.3-- | <-4.2-- <-4.3--- | <--5.x--
  //                        \--                       |           \--    |
  //                                                  o                  o
  */

  auto connectors = Algorithm::find_connectors();

  loggerf(INFO, "Have %i connectors", connectors.size());

  uint8_t x = 0;
  bool modules_linked = false;

  SECTION("I - Find and connect"){
    while(modules_linked == false){
      if(uint8_t * findResult = Algorithm::find_connectable(&connectors)){
        Algorithm::connect_connectors(&connectors, findResult);
      }

      if(connectors.size() == 0)
        break;

      if(x == 1){
        U[1]->B[0]->setDetection(1);
        U[2]->B[0]->setDetection(1);
      }else if(x == 2){
        U[1]->B[0]->setDetection(0);
        U[2]->B[0]->setDetection(0);
        U[2]->B[1]->setDetection(1);
        U[3]->B[0]->setDetection(1);
      }else if(x == 3){
        U[2]->B[1]->setDetection(0);
        U[3]->B[0]->setDetection(0);
        U[3]->B[1]->setDetection(1);
        U[4]->B[0]->setDetection(1);
      }else if(x == 4){
        U[3]->B[1]->setDetection(0);
        U[4]->B[0]->setDetection(0);
        U[4]->B[1]->setDetection(1);
        U[5]->B[0]->setDetection(1);
      }
      else if(x > 4){
        // _SYS_change(STATE_Modules_Coupled,1);
        modules_linked = true;
      }

      x++;
      //IF ALL JOINED
      //BREAK
    }

    U[4]->B[1]->setDetection(0);
    U[5]->B[0]->setDetection(0);

    switchboard::SwManager->LinkAndMap();

    REQUIRE(connectors.size() == 0);

    // U1 <-> U2
    CHECK(U[1]->B[0]->next.p.B == U[2]->B[0]);
    CHECK(U[2]->B[0]->prev.p.B == U[1]->B[0]);

    CHECK(U[1]->B[1]->prev.p.B == U[2]->B[2]);
    CHECK(U[2]->B[2]->next.p.B == U[1]->B[1]);

    // U2 <-> U3
    CHECK(U[2]->B[1]->next.p.Sw == U[3]->Sw[0]);
    CHECK(U[3]->B[2]->next.p.Sw == U[2]->Sw[0]);

    CHECK(U[3]->Sw[0]->str.p.B == U[2]->B[1]);
    CHECK(U[2]->Sw[0]->str.p.B == U[3]->B[2]);

    // U3 <-> U4
    CHECK(U[3]->B[1]->next.p.B == U[4]->B[0]);
    CHECK(U[4]->B[0]->prev.p.B == U[3]->B[1]);

    CHECK(U[3]->B[3]->prev.p.B == U[4]->B[2]);
    CHECK(U[4]->B[2]->next.p.B == U[3]->B[3]);

    CHECK(U[3]->Sig[0]->block_link.p.B == U[4]->B[0]);
    CHECK(U[4]->Sig[1]->block_link.p.B == U[3]->B[3]);

    // U4 <-> U5
    CHECK(U[4]->B[1]->next.p.Sw == U[5]->Sw[0]);
    CHECK(U[5]->Sw[0]->str.p.B  == U[4]->B[1]);

    CHECK(U[4]->Sw[0]->str.p.B  == U[5]->B[1]);
    CHECK(U[5]->B[1]->next.p.Sw == U[4]->Sw[0]);

    CHECK(U[3]->Sig[0]->block_link.p.B == U[4]->B[0]);
    CHECK(U[4]->Sig[1]->block_link.p.B == U[3]->B[3]);

    auto setup = Algorithm::BlockConnectorSetup("testconfigs/Alg-1-setup--.bin");
    setup.save();

  }

  SECTION("II - Connect Stored Configuration"){
    auto setup = Algorithm::BlockConnectorSetup("testconfigs/Alg-1-setup.bin");
    int ret = setup.load(&connectors);

    REQUIRE(ret > 0);
    
    switchboard::SwManager->LinkAndMap();

    REQUIRE(connectors.size() == 0);

    CHECK(switchboard::Units(1)->B[0]->next.p.B == switchboard::Units(2)->B[0]);
    CHECK(switchboard::Units(2)->B[0]->prev.p.B == switchboard::Units(1)->B[0]);

    CHECK(switchboard::Units(3)->B[1]->next.p.B == switchboard::Units(4)->B[0]);
    CHECK(switchboard::Units(4)->B[0]->prev.p.B == switchboard::Units(3)->B[1]);
  }
}

TEST_CASE_METHOD(TestsFixture, "Algor Queue", "[Alg][Alg-Q]"){
  char filenames[1][30] = {"./testconfigs/Alg-3.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  AlQueue.put(U->B[0]);

  CHECK(AlQueue.queue->getItems() == 1);
  CHECK(AlQueue.get() == U->B[0]);

  CHECK(AlQueue.queue->getItems() == 0);
  CHECK(AlQueue.get() == 0);

  struct timespec start, end;

  clock_gettime(CLOCK_REALTIME, &start);
  AlQueue.getWait();
  clock_gettime(CLOCK_REALTIME, &end);
  CHECK( (end.tv_sec - start.tv_sec) >= 14);

  AlQueue.put(U->B[0]);

  clock_gettime(CLOCK_REALTIME, &start);
  AlQueue.getWait();
  clock_gettime(CLOCK_REALTIME, &end);
  CHECK( (end.tv_sec - start.tv_sec) < 2);


  AlQueue.puttemp(U->B[0]);

  CHECK(AlQueue.queue->getItems() == 0);
  CHECK(AlQueue.get() == 0);

  CHECK(AlQueue.tempQueue->getItems() == 1);
  AlQueue.cpytmp();

  CHECK(AlQueue.queue->getItems() == 1);
  CHECK(AlQueue.get() == U->B[0]);
}

TEST_CASE_METHOD(TestsFixture, "Train Speed Control", "[Alg][Alg-Sp]"){
  char filenames[1][30] = {"./testconfigs/Alg-Sp.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);

  REQUIRE(U);

  switchboard::SwManager->LinkAndMap();
  
  for(uint8_t j = 0; j < U->block_len; j++){
    if(U->B[j]){
      U->B[j]->setDetection(0);
      AlQueue.put(U->B[j]);
    }
  }

  pathlist_find();

  Algorithm::BlockTick();

  Block *B = U->B[0];

  struct train_sim train = {
    .sim = 'A',
    .train_length = 0,
    .posFront = 0.0,
    .posRear = U->B[0]->length * 1.0f,
    .Front = 0,
    .B = (Block **)_calloc(10, Block *),
    .blocks = 1,
  };
  train.B[0] = B;

  change_Block(B, BLOCKED);
  AlQueue.cpytmp();
  
  Algorithm::BlockTick();

  B->train->link(0, TRAIN_ENGINE_TYPE);
  B->train->setControl(TRAIN_SEMI_AUTO);

  train.train_length = B->train->length;
  train.T = B->train;
  train.T->changeSpeed(180, 0);
 
  AlQueue.put(B);
  Algorithm::BlockTick();

  train.train_length = train.T->p.E->length / 10;

  train.engines_len = 1;
  train.engines = (struct engine_sim *)_calloc(1, struct engine_sim);
  train.engines[0].offset = 0;
  train.engines[0].length = train.T->p.E->length;

  train.posFront = train.B[0]->length - (train.engines[0].length / 10);
  train.posRear = train.B[0]->length;

  int32_t maxIterations = 3000;

  scheduler->disableEvent(RSManager->continue_event);
  scheduler->start();

  auto T = train.T;
  auto ED = T->speed_event_data;

  SECTION("I - CAUTION"){
    for(uint8_t i = 8; i < 12; i++)
      U->B[i]->state = CAUTION;
    

    while(!U->B[5]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[5]->blocked);
    CHECK(T->speed == 180);
    CHECK(T->SpeedState == TRAIN_SPEED_CHANGING);
    CHECK(ED->target_speed == CAUTION_SPEED);
    CHECK(ED->target_distance == 300);
    CHECK(ED->reason == TRAIN_SPEED_R_SIGNAL);

    while(!U->B[6]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[6]->blocked);
    CHECK(T->SpeedState == TRAIN_SPEED_UPDATE);

    while(T->speed_event_data->stepCounter > 1)
      train_testSim_tick(&train, &maxIterations);

    CHECK(T->speed < 180);
    CHECK(T->speed >= T->speed_event_data->startSpeed);
    CHECK(T->SpeedState == TRAIN_SPEED_CHANGING);
    CHECK(ED->target_speed == CAUTION_SPEED);
    CHECK(ED->target_distance == 200);
    CHECK(ED->reason == TRAIN_SPEED_R_SIGNAL);

    while(!U->B[7]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[7]->blocked);

    while(T->speed_event_data->stepCounter > 1)
      train_testSim_tick(&train, &maxIterations);

    CHECK(T->SpeedState == TRAIN_SPEED_CHANGING);
    CHECK(T->speed < 180);
    CHECK(T->speed >= T->speed_event_data->startSpeed);
    CHECK(ED->target_speed == CAUTION_SPEED);
    CHECK(ED->target_distance == 100);
    CHECK(ED->reason == TRAIN_SPEED_R_SIGNAL);

    while(!U->B[8]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[8]->blocked);
    CHECK(T->speed == ED->target_speed);
  }

  SECTION("II - DANGER"){
    T->changeSpeed(90, 0);
    for(uint8_t i = 0; i < 8; i++)
      U->B[i]->state = CAUTION;
    for(uint8_t i = 8; i < 12; i++)
      U->B[i]->state = DANGER;

    maxIterations = 2000;

    while(!U->B[6]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[6]->blocked);
    CHECK(T->SpeedState == TRAIN_SPEED_DRIVING);
    CHECK(ED->reason == TRAIN_SPEED_R_NONE);

    while(!U->B[7]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[7]->blocked);
    CHECK(T->SpeedState == TRAIN_SPEED_CHANGING);
    CHECK(ED->target_speed == 0);
    CHECK(ED->target_distance == 100);
    CHECK(ED->reason == TRAIN_SPEED_R_SIGNAL);

    while(!U->B[8]->blocked && T->speed > 0 && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(!U->B[8]->blocked);
    CHECK(U->B[7]->train->speed == 0);
    CHECK(U->B[7]->train->SpeedState == TRAIN_SPEED_STOPPING_WAIT);

    while(!U->B[8]->blocked && !T->speed_event->disabled && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(U->B[7]->train->SpeedState == TRAIN_SPEED_WAITING);
  }
  
  SECTION("III - CAUTION changing to PROCEED"){
    U->B[8]->state = CAUTION;
    U->B[9]->state = CAUTION;
    U->B[10]->state = CAUTION;
    
    maxIterations = 2000;

    while(!U->B[5]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[5]->blocked);
    CHECK(T->SpeedState == TRAIN_SPEED_CHANGING);
    CHECK(ED->target_speed == CAUTION_SPEED);
    CHECK(ED->target_distance == 300);
    CHECK(ED->reason == TRAIN_SPEED_R_SIGNAL);
    CHECK(T->speed == 180);

    while(ED->stepCounter != 4 && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->speed < 180);
    U->B[8]->state = PROCEED;
    
    while(ED->stepCounter != 5 && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->SpeedState == TRAIN_SPEED_DRIVING);
    CHECK(ED->reason == TRAIN_SPEED_R_NONE);

    while(!U->B[7]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[7]->blocked);
    CHECK(T->SpeedState == TRAIN_SPEED_CHANGING);
    CHECK(ED->target_speed == CAUTION_SPEED);
    CHECK(ED->target_distance == 200);
    CHECK(ED->reason == TRAIN_SPEED_R_SIGNAL);

    while(!U->B[8]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[8]->blocked);
    CHECK(T->SpeedState == TRAIN_SPEED_UPDATE);
    CHECK(ED->target_speed == CAUTION_SPEED);
    CHECK(ED->target_distance == 100);
    CHECK(ED->reason == TRAIN_SPEED_R_SIGNAL);

    while(!U->B[8]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[8]->blocked);
    CHECK(T->SpeedState == TRAIN_SPEED_UPDATE);
    CHECK(ED->target_speed == CAUTION_SPEED);
    CHECK(ED->target_distance == 100);
    CHECK(ED->reason == TRAIN_SPEED_R_SIGNAL);
  }

  SECTION("IV - Speed"){
    maxIterations = 2000;

    // Skip first blocks
    while(!U->B[5]->blocked){
      train_test_tick(&train, &maxIterations);
    }

    while(!U->B[7]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[7]->blocked);
    CHECK(T->SpeedState == TRAIN_SPEED_CHANGING);
    CHECK(ED->target_speed == 100);
    CHECK(ED->target_distance == 300);
    CHECK(ED->reason == TRAIN_SPEED_R_MAXSPEED);

    while(!U->B[8]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[8]->blocked);
    CHECK(T->SpeedState == TRAIN_SPEED_UPDATE);
    CHECK(ED->target_speed == 100);
    CHECK(ED->target_distance == 200);
    CHECK(ED->reason == TRAIN_SPEED_R_MAXSPEED);

    while(!U->B[9]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[9]->blocked);
    CHECK(T->SpeedState == TRAIN_SPEED_UPDATE);
    CHECK(ED->target_speed == 100);
    CHECK(ED->target_distance == 100);
    CHECK(ED->reason == TRAIN_SPEED_R_MAXSPEED);

    while(!U->B[12]->blocked && maxIterations > 0){
      Block * trainBlock = T->B;

      while(T->B == trainBlock && maxIterations > 0){
        train_testSim_tick(&train, &maxIterations);
      }

      CHECK(T->SpeedState != TRAIN_SPEED_CHANGING);
      CHECK(T->speed == 100);
    }

    REQUIRE(U->B[12]->blocked);
    CHECK(T->SpeedState != TRAIN_SPEED_CHANGING);
    CHECK(T->speed == 100);

    while(U->B[12]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(!U->B[12]->blocked);
    CHECK(T->SpeedState == TRAIN_SPEED_CHANGING);
    CHECK(ED->target_speed > 100);
    CHECK(ED->target_distance == 100);
    CHECK(ED->reason == TRAIN_SPEED_R_MAXSPEED);

    while(!U->B[14]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U->B[14]->blocked);
    CHECK(T->SpeedState == TRAIN_SPEED_CHANGING); // FIXME, train could allready be accelerating because it left B[13]
    CHECK(ED->target_speed > 100);
    CHECK(ED->target_speed <= 180);
    CHECK(ED->target_distance == 100);
    CHECK(ED->reason == TRAIN_SPEED_R_MAXSPEED);
  }

  SECTION("V - End of route"){
    REQUIRE(U->St[0]);

    T->setRoute(U->St[0]);

    REQUIRE(T->route);
    REQUIRE(T->routeStatus == TRAIN_ROUTE_RUNNING);
    REQUIRE(T->route->destination == U->St[0]->uid);

    while(!U->B[7]->blocked && maxIterations > 0){
      train_test_tick(&train, &maxIterations);
    }

    while(!U->B[10]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->routeStatus == TRAIN_ROUTE_ENTERED_DESTINATION);

    while(!U->B[13]->blocked && T->speed > 90 && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->routeStatus == TRAIN_ROUTE_AT_DESTINATION);
    CHECK(ED->reason == TRAIN_SPEED_R_ROUTE);

    while(!U->B[13]->blocked && T->speed != 0 && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->speed == 0);
    CHECK(!U->B[13]->blocked);
    CHECK(U->B[12]->blocked);
    CHECK(!U->B[10]->blocked);
    CHECK(ED->reason == TRAIN_SPEED_R_NONE);
    
    CHECK(T->SpeedState == TRAIN_SPEED_STOPPING_WAIT);

    while(!U->B[13]->blocked && !T->speed_event->disabled && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }
    
    CHECK(T->SpeedState == TRAIN_SPEED_WAITING_DESTINATION);

  }

  SECTION("VI - At of waypoint"){
    T->setRoute(U->B[10]);

    REQUIRE(T->route);
    REQUIRE(T->routeStatus == TRAIN_ROUTE_RUNNING);
    REQUIRE(T->route->destination == 0x010A); // Unit 1 Block 10

    while(!U->B[10]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->routeStatus == TRAIN_ROUTE_AT_DESTINATION);

    while(!U->B[13]->blocked && T->speed != 0 && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->speed != 0);
    CHECK(U->B[13]->blocked);

  }

  SECTION("VII - User Stop change speed"){
    while(!U->B[5]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->speed == 180);
    T->changeSpeed(0, 300);

    CHECK(T->SpeedState == TRAIN_SPEED_CHANGING);
    CHECK(ED->target_speed == 0);
    CHECK(ED->target_distance == 300);
    CHECK(ED->reason == TRAIN_SPEED_R_MAXSPEED);

    while(!U->B[8]->blocked && T->speed && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->speed == 0);
    CHECK(T->SpeedState == TRAIN_SPEED_STOPPING);

    while(!T->speed_event->disabled && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->SpeedState == TRAIN_SPEED_IDLE);
  }

  SECTION("VIII - User Stop set speed"){
    while(!U->B[2]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->speed == 180);
    T->setSpeed(0);
    CHECK(T->speed == 0);
    CHECK(T->SpeedState == TRAIN_SPEED_STOPPING);

    maxIterations = 50;

    while(!T->speed_event->disabled && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(T->SpeedState == TRAIN_SPEED_IDLE);
  }
}
/*
TEST_CASE_METHOD(TestsFixture, "Train Route Following", "[Alg][Alg-R]"){
  char filenames[4][30] = {"./testconfigs/Alg-R-1.bin",
                           "./testconfigs/Alg-R-2.bin",
                           "./testconfigs/Alg-R-3.bin",
                           "./testconfigs/Alg-R-4.bin"};
  loadSwitchboard(filenames, 4);
  loadStock();

  Unit * U[5] = {0, switchboard::Units(1),switchboard::Units(2),switchboard::Units(3),switchboard::Units(4)};

  for(uint8_t i = 1; i <= 4; i++){
    REQUIRE(U[i]);
    U[i]->on_layout = true;
  }

  auto connectors = Algorithm::find_connectors();
  auto setup = Algorithm::BlockConnectorSetup("./testconfigs/Alg-R-Setup.bin");
  
  int ret = setup.load(&connectors);
  REQUIRE(ret > 0);
  REQUIRE(connectors.size() == 0);

  switchboard::SwManager->LinkAndMap();

  for(uint8_t i = 1; i <= 4; i++){
    for(uint8_t j = 0; j < U[i]->block_len; j++){
      if(U[i]->B[j]){
        U[i]->B[j]->setDetection(0);
        AlQueue.put(U[i]->B[j]);
      }
    }
  }

  pathlist_find();

  for(uint8_t i = 1; i < 5; i++){
    for(uint8_t j = 0; j < U[i]->block_len; j++){
      if(!U[i]->B[j])
        continue;

      AlQueue.put(U[i]->B[j]);
    }
  }

  Algorithm::BlockTick();

  Block *B = U[1]->B[3];

  struct train_sim train = {
    .sim = 'A',
    .train_length = 0,
    .posFront = 0.0,
    .posRear = 124.0,
    .Front = 0,
    .B = (Block **)_calloc(10, Block *),
    .blocks = 1,
  };
  train.B[0] = B;

  change_Block(B, BLOCKED);
  AlQueue.cpytmp();
  
  Algorithm::BlockTick();

  B->train->link(0, TRAIN_ENGINE_TYPE);
  B->train->setControl(TRAIN_MANUAL);

  train.train_length = B->train->length;
  train.T = B->train;
  train.T->changeSpeed(100, 0);
 
  AlQueue.put(B);
  Algorithm::BlockTick();

  train.train_length = train.T->p.E->length / 10;

  train.engines_len = 1;
  train.engines = (struct engine_sim *)_calloc(1, struct engine_sim);
  train.engines[0].offset = 0;
  train.engines[0].length = train.T->p.E->length;

  train.posFront = train.B[0]->length - (train.engines[0].length / 10);
  train.posRear = train.B[0]->length;

  int32_t maxIterations = 5000;

  SECTION("I - Just a circle"){
    while(!U[1]->B[5]->blocked && maxIterations > 0){
      train_test_tick(&train, &maxIterations);
    }

    CHECK(!U[1]->B[3]->blocked);
    CHECK(!U[1]->B[4]->blocked);
    CHECK(U[1]->B[5]->blocked);

    maxIterations = 5000;

    while(!U[1]->B[3]->blocked && maxIterations > 0){
      train_test_tick(&train, &maxIterations);
    }

    CHECK(U[1]->B[3]->blocked);
  }

  SECTION("II - A route"){
    train.T->setRoute(U[1]->B[8]);

    while(!U[3]->B[2]->blocked && maxIterations > 0){
      train_test_tick(&train, &maxIterations);
    }

    CHECK(U[3]->B[2]->blocked);
    CHECK(maxIterations > 0);
    
    while(!U[1]->B[10]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(maxIterations > 0);
    maxIterations = 1000;

    // Train should stop not stop on a waypoint
    while(!U[1]->B[8]->blocked && maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    REQUIRE(U[1]->B[8]->blocked);
    CHECK(maxIterations > 0);
    CHECK(U[1]->B[8]->train->speed_event_data->target_speed > 0);
  }

  SECTION("III - A blocked route"){
    // Run the scheduler to change speed of train dynamically
    //   or include it in the iterration loop since sim time does not equal real time.

    U[1]->B[8]->setDetection(1);
    Algorithm::process(U[1]->B[8], _FORCE);

    train.T->setRoute(U[1]->B[8]); // Set train destination

    while(!U[3]->B[2]->blocked && maxIterations > 0){
      train_test_tick(&train, &maxIterations);
    }

    CHECK(U[3]->B[2]->blocked);
    CHECK(maxIterations > 0);

    maxIterations = 2000;

    while(maxIterations > 0){
      train_testSim_tick(&train, &maxIterations);
    }

    CHECK(maxIterations == 0);
    CHECK(!U[1]->B[9]->blocked);
    CHECK(train.T->B == U[4]->B[0]);
  }
}
*/