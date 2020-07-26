#include "catch.hpp"

#include "mem.h"
#include "logger.h"

#include "config/ModuleConfig.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/station.h"
#include "switchboard/unit.h"
#include "switchboard/blockconnector.h"
// #include "path.h"

// #include "algorithm.h"

TEST_CASE( "Alg-1", "[Alg][Alg-1]"){
// int main(){
  set_level(NONE);
  set_logger_print_level(DEBUG);

  if(Units){
    for(uint8_t u = 0; u < unit_len; u++){
      if(!Units[u])
        continue;

      delete Units[u];
      Units[u] = 0;
    }
    _free(Units);
  }

  Units = (Unit **)_calloc(30, Unit *);
  unit_len = 30;

  char filename[4][30] = {"./testconfigs/Alg-1-1.bin", "./testconfigs/Alg-1-2.bin", "./testconfigs/Alg-1-3.bin", "./testconfigs/Alg-1-4.bin"};
  ModuleConfig config[4] = {ModuleConfig(filename[0]), ModuleConfig(filename[1]), ModuleConfig(filename[2]), ModuleConfig(filename[3])};

  config[0].read();
  config[1].read();
  config[2].read();
  config[3].read();

  // REQUIRE(config[0].parsed);
  // REQUIRE(config[1].parsed);
  // REQUIRE(config[2].parsed);
  // REQUIRE(config[3].parsed);

  new Unit(&config[0]);
  new Unit(&config[1]);
  new Unit(&config[2]);
  new Unit(&config[3]);

  Units[1]->on_layout = true;
  Units[2]->on_layout = true;
  Units[3]->on_layout = true;
  Units[4]->on_layout = true;

  /*                              --\
  //  1.0->  | --2.0-> --2.1->  |  --3.0-> --3.1-> | --4.0->
  //     C1-1 C1-1          C2-1 C1-1          C2-1 C1-2
  //     C1-2 C1-2          C2-2 C1-2          C2-2 C1-1
  // <1.1--  | <-2.2-- <-2.3--  |  <-3.2-- <-3.3-- | <-4.1--
  //                        \--
  */

  auto connectors = Algorithm_find_connectors();

  loggerf(INFO, "Have %i connectors", connectors.size());

  uint8_t x = 0;
  bool modules_linked = false;

  while(modules_linked == false){
    if(uint8_t * findResult = Algorithm_find_connectable(&connectors)){
      Algorithm_connect_connectors(&connectors, findResult);
    }

    if(connectors.size() == 0)
      break;

    if(x == 1){
      Units[1]->B[0]->blocked = 1;
      Units[2]->B[0]->blocked = 1;
      printf("\n1\n");
    }else if(x == 2){
      Units[1]->B[0]->blocked = 0;
      Units[2]->B[0]->blocked = 0;
      Units[2]->B[1]->blocked = 1;
      Units[3]->B[0]->blocked = 1;
      printf("\n2\n");
    }else if(x == 3){
      Units[2]->B[1]->blocked = 0;
      Units[3]->B[0]->blocked = 0;
      Units[3]->B[1]->blocked = 1;
      Units[4]->B[0]->blocked = 1;
      printf("\n3\n");
    }
    else if(x > 3){
      // _SYS_change(STATE_Modules_Coupled,1);
      modules_linked = true;
    }

    x++;
    //IF ALL JOINED
    //BREAK
  }
  
  Units[3]->B[1]->blocked = 0;
  Units[4]->B[0]->blocked = 0;


  link_all_blocks(Units[1]);
  link_all_blocks(Units[2]);
  link_all_blocks(Units[3]);
  link_all_blocks(Units[4]);

  // REQUIRE(connectors.size() == 0);

  // CHECK(Units[1]->B[0]->next.p.B == Units[2]->B[0]);
  // CHECK(Units[2]->B[0]->prev.p.B == Units[1]->B[0]);

  // CHECK(Units[3]->B[1]->next.p.B == Units[4]->B[0]);
  // CHECK(Units[4]->B[0]->prev.p.B == Units[3]->B[1]);
}