#include "mem.h"
#include "logger.h"
#include "system.h"

#include "sim.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/blockconnector.h"
#include "train.h"
#include "modules.h"

#include "submodule.h"
#include "algorithm.h"
#include "websocket/stc.h"
#include "pathfinding.h"

extern pthread_mutex_t mutex_lockA;

#define delayA 5000000
#define delayB 5000000
#define OneSec 1000000

#define TRAIN_A_LEN   5 //cm
#define TRAIN_A_SPEED 5 //cm/s

#define TRAIN_B_LEN   5 //cm
#define TRAIN_B_SPEED 5 //cm/s

#define TRAINSIM_INTERVAL_US 50000
#define TRAINSIM_INTERVAL_SEC 0.05

#define JOIN_SIM_INTERVAL 1000

void change_Block(Block * B, enum Rail_states state){
  B->IOchanged = 1;
  B->statechanged = 1;
  Units[B->module]->block_state_changed = 1;
  if (state == BLOCKED)
    B->blocked = 1;
  else
    B->blocked = 0;

  putAlgorQueue(B, 1);
  // process(B, 3);
}

struct train_sim {
  char sim;

  RailTrain * T;
  uint16_t train_length;

  uint8_t dir;
  float posFront;
  float posRear;

  Block * Front;
  uint8_t FrontSpecialCounter;

  uint8_t blocks;
  Block ** B;
};

void train_sim_tick(struct train_sim * t){
  // loggerf(INFO, "train_sim_tick speed %i\t%f\t%f", t->T->speed,t->posFront, t->posRear);

  if(t->posFront <= 0 && t->blocks < 10){
    // Add block
    for(uint8_t i = t->blocks - 1; i >= 0 && i < 10; i--){
      t->B[i + 1] = t->B[i];
    }
    t->blocks++;
    if(t->FrontSpecialCounter){
      t->B[0] = t->Front->_Next(NEXT, ++t->FrontSpecialCounter);
    }
    else if(t->B[1]->Alg.next){
      t->B[0] = t->B[1]->Alg.N[0];
    }
    else{
      SYS_set_state(&SYS->SimA.state, Module_Fail);
      return;
    }
    loggerf(INFO, "%c  Step %02i:%02i", t->sim, t->B[0]->module, t->B[0]->id);
    t->posFront += t->B[0]->length;
    change_Block(t->B[0], BLOCKED);
  }

  if(t->posRear <= 0){
    // Remove block
    t->blocks--;
    change_Block(t->B[t->blocks], PROCEED);
    t->B[t->blocks] = 0;
    t->posRear += t->B[t->blocks - 1]->length;
  }

  // Advance train (km/h -> cm/s) / scale * tick interval (in sec)
  t->posFront -= (t->T->speed / 3.6) * 100 / 160 * TRAINSIM_INTERVAL_SEC;
  t->posRear  -= (t->T->speed / 3.6) * 100 / 160 * TRAINSIM_INTERVAL_SEC;
}

void *TRAIN_SIMA(void * args){
  if(!SYS_wait_for_state(&SYS->LC.state, Module_Run)){
    SYS_set_state(&SYS->SimA.state, Module_Fail);
    return 0;
  }

  usleep(100000);

  Block *B = Units[25]->B[3];

  struct train_sim train;
  train.B = (Block **)_calloc(10, Block *);
  train.sim = 'A';
  train.posFront = 0;
  train.posRear = B->length;
  train.FrontSpecialCounter = 0;
  train.Front = 0;

  train.blocks = 0;
  // train.B[0] = B;

  while(!B->Alg.N[0] || !B->Alg.P[0]){}
  while(B->Alg.N[0]->blocked || B->blocked || B->Alg.P[0]->blocked){} // Wait for space

  change_Block(B, BLOCKED);

  usleep(100000);

  while(!B->train){
      usleep(10000);
  }

  B->train->control = TRAIN_SEMI_AUTO;

  while(!B->train->p.p){
    usleep(10000);
  }

  train.T = B->train;

  if(train.T->type == RAILTRAIN_ENGINE_TYPE){
    //Engine only
    train.train_length = train.T->p.E->length / 10;
  }
  else{
    //Train
    train.train_length = train.T->p.T->length / 10;
  }
  loggerf(INFO, "train length %icm", train.train_length);

  int32_t len = train.train_length;
  while(len > 0){
    len -= B->length;

    for(uint8_t i = train.blocks - 1; i >= 0 && i < 10; i--){
      train.B[i + 1] = train.B[i];
    }
    train.blocks++;

    change_Block(B, BLOCKED);
    loggerf(INFO, "Add block %i (%02i:%02i)", train.blocks, B->module, B->id);

    if(B->Alg.next){
      train.B[0] = B;
      B = B->Alg.N[0];
    }
  }

  train.posFront -= len;

  SYS_set_state(&SYS->SimA.state, Module_Run);

  while(SYS->SimA.state & Module_Run){
    train_sim_tick(&train);
    usleep(TRAINSIM_INTERVAL_US);
  }

  _free(train.B);
  SYS_set_state(&SYS->SimA.state, Module_STOP);

  return 0;
}

void *TRAIN_SIMB(void * args){
  if(!SYS_wait_for_state(&SYS->LC.state, Module_Run)){
    SYS_set_state(&SYS->SimA.state, Module_Fail);
    return 0;
  }

  usleep(11000000);

  Block *B = Units[25]->B[3];

  struct train_sim train;
  train.B = (Block **)_calloc(10, Block *);
  train.sim = 'B';
  train.posFront = 0;
  train.posRear = B->length;
  train.FrontSpecialCounter = 0;
  train.Front = 0;

  train.blocks = 0;
  // train.B[0] = B;

  while(!B->Alg.N[0] || !B->Alg.P[0]){}
  while(B->Alg.N[0]->blocked || B->blocked || B->Alg.P[0]->blocked){} // Wait for space

  change_Block(B, BLOCKED);

  usleep(100000);

  while(!B->train){
      usleep(10000);
  }

  B->train->control = TRAIN_SEMI_AUTO;

  while(!B->train->p.p){
    usleep(10000);
  }

  train.T = B->train;

  if(train.T->type == RAILTRAIN_ENGINE_TYPE){
    //Engine only
    train.train_length = train.T->p.E->length / 10;
  }
  else{
    //Train
    train.train_length = train.T->p.T->length / 10;
  }
  loggerf(INFO, "train length %icm", train.train_length);

  int32_t len = train.train_length;
  while(len > 0){
    len -= B->length;

    for(uint8_t i = train.blocks - 1; i >= 0 && i < 10; i--){
      train.B[i + 1] = train.B[i];
    }
    train.blocks++;

    change_Block(B, BLOCKED);
    loggerf(INFO, "Add block %i (%02i:%02i)", train.blocks, B->module, B->id);

    if(B->Alg.next){
      train.B[0] = B;
      B = B->Alg.N[0];
    }
  }

  train.posFront -= len;

  SYS_set_state(&SYS->SimB.state, Module_Run);

  while(SYS->SimB.state & Module_Run){
    train_sim_tick(&train);
    usleep(TRAINSIM_INTERVAL_US);
  }

  _free(train.B);

  return 0;
}


void SIM_JoinModules(){
  Units[10]->on_layout = 1;
  Units[20]->on_layout = 1;
  Units[21]->on_layout = 1;
  Units[22]->on_layout = 1;
  Units[23]->on_layout = 1;
  Units[25]->on_layout = 1;

  WS_stc_Track_Layout(0);
  printf("Ready to join modules\n");

  auto connectors = Algorithm_find_connectors();
  uint16_t maxConnectors = connectors.size();

  loggerf(INFO, "Have %i connectors", connectors.size());

  uint8_t x = 0;
  char data[20];

  while(SYS->modules_linked == 0){
    WS_stc_trackUpdate(0);
    
    if(uint8_t * findResult = Algorithm_find_connectable(&connectors)){
      Algorithm_connect_connectors(&connectors, findResult);

      data[0] = 0x82;
      data[1] = (char)connectors.size();
      data[2] = maxConnectors;
      int k = 3;
      for(int j = 0;j<unit_len;j++){
        if(Units[j]){
          data[k++] = j;
        }
      }
      WSServer->send_all(data, k, 0x10);
    }

    if(connectors.size() == 0)
      break;

    usleep(JOIN_SIM_INTERVAL);

    if(x == 1){
      Units[20]->block_state_changed = 1;
      Units[20]->B[5]->IOchanged = 1;
      Units[25]->block_state_changed = 1;
      Units[25]->B[0]->IOchanged = 1;

      Units[20]->B[5]->blocked = 1;
      Units[25]->B[0]->blocked = 1;
      Units[20]->B[5]->state = BLOCKED;
      Units[25]->B[0]->state = BLOCKED;
      printf("\n1\n");
    }else if(x == 2){
      Units[20]->block_state_changed = 1;
      Units[20]->B[5]->IOchanged = 1;
      Units[25]->block_state_changed = 1;
      Units[25]->B[0]->IOchanged = 1;
      
      Units[25]->block_state_changed = 1;
      Units[25]->B[3]->IOchanged = 1;
      Units[22]->block_state_changed = 1;
      Units[22]->B[0]->IOchanged = 1;

      Units[25]->B[3]->blocked = 1;
      Units[22]->B[0]->blocked = 1;
      Units[25]->B[3]->state = BLOCKED;
      Units[22]->B[0]->state = BLOCKED;

      Units[20]->B[5]->blocked = 0;
      Units[25]->B[0]->blocked = 0;
      Units[20]->B[5]->state = PROCEED;
      Units[25]->B[0]->state = PROCEED;
      printf("\n2\n");
    }else if(x == 3){
      Units[25]->block_state_changed = 1;
      Units[25]->B[3]->IOchanged = 1;
      Units[22]->block_state_changed = 1;
      Units[22]->B[0]->IOchanged = 1;

      Units[22]->block_state_changed = 1;
      Units[22]->B[1]->IOchanged = 1;
      Units[10]->block_state_changed = 1;
      Units[10]->B[0]->IOchanged = 1;

      Units[22]->B[1]->blocked = 1;
      Units[10]->B[0]->blocked = 1;
      Units[22]->B[1]->state = BLOCKED;
      Units[10]->B[0]->state = BLOCKED;

      Units[25]->B[3]->blocked = 0;
      Units[22]->B[0]->blocked = 0;
      Units[25]->B[3]->state = PROCEED;
      Units[22]->B[0]->state = PROCEED;
      printf("\n3\n");
    }else if(x == 4){
      Units[22]->block_state_changed = 1;
      Units[22]->B[1]->IOchanged = 1;
      Units[10]->block_state_changed = 1;
      Units[10]->B[0]->IOchanged = 1;

      Units[10]->block_state_changed = 1;
      Units[10]->B[3]->IOchanged = 1;
      Units[21]->block_state_changed = 1;
      Units[21]->B[0]->IOchanged = 1;

      Units[10]->B[3]->blocked = 1;
      Units[21]->B[0]->blocked = 1;
      Units[10]->B[3]->state = BLOCKED;
      Units[21]->B[0]->state = BLOCKED;

      Units[22]->B[1]->blocked = 0;
      Units[10]->B[0]->blocked = 0;
      Units[22]->B[1]->state = PROCEED;
      Units[10]->B[0]->state = PROCEED;
      printf("\n4\n");
    }else if(x == 5){
      Units[10]->block_state_changed = 1;
      Units[10]->B[3]->IOchanged = 1;
      Units[21]->block_state_changed = 1;
      Units[21]->B[0]->IOchanged = 1;
      
      Units[21]->block_state_changed = 1;
      Units[21]->B[3]->IOchanged = 1;
      Units[23]->block_state_changed = 1;
      Units[23]->B[0]->IOchanged = 1;

      Units[21]->B[3]->blocked = 1;
      Units[23]->B[0]->blocked = 1;
      Units[21]->B[3]->state = BLOCKED;
      Units[23]->B[0]->state = BLOCKED;

      Units[10]->B[3]->blocked = 0;
      Units[21]->B[0]->blocked = 0;
      Units[10]->B[3]->state = PROCEED;
      Units[21]->B[0]->state = PROCEED;
      printf("\n5\n");
    }else if(x == 6){
      Units[21]->block_state_changed = 1;
      Units[21]->B[3]->IOchanged = 1;
      Units[23]->block_state_changed = 1;
      Units[23]->B[0]->IOchanged = 1;

      Units[23]->block_state_changed = 1;
      Units[23]->B[1]->IOchanged = 1;
      Units[20]->block_state_changed = 1;
      Units[20]->B[0]->IOchanged = 1;

      Units[23]->B[1]->blocked = 1;
      Units[20]->B[0]->blocked = 1;
      Units[23]->B[1]->state = BLOCKED;
      Units[20]->B[0]->state = BLOCKED;

      Units[21]->B[3]->blocked = 0;
      Units[23]->B[0]->blocked = 0;
      Units[21]->B[3]->state = PROCEED;
      Units[23]->B[0]->state = PROCEED;
      printf("\n6\n");
    }else if(x == 7){
      Units[23]->block_state_changed = 1;
      Units[23]->B[1]->IOchanged = 1;
      Units[20]->block_state_changed = 1;
      Units[20]->B[0]->IOchanged = 1;

      Units[23]->B[1]->blocked = 0;
      Units[20]->B[0]->blocked = 0;
      Units[23]->B[1]->state = PROCEED;
      Units[20]->B[0]->state = PROCEED;
      printf("\n7\n");
    }else if(x == 6){
      printf("\nend\n");
    }
    else if(x == 10){
      // _SYS_change(STATE_Modules_Coupled,1);
      SYS->modules_linked = 1;
    }

    x++;
    //IF ALL JOINED
    //BREAK
  }
  
  Units[21]->B[0]->blocked = 0;
  Units[22]->B[1]->blocked = 0;

  for(uint8_t i = 0; i < unit_len; i++){
    if(!Units[i])
      continue;

    for(uint8_t j = 0; j < Units[i]->block_len; j++){
      if(Units[i]->B[j]){
        Units[i]->B[j]->blocked = 0;
      }
    }
  }

  // WS_stc_Track_Layout();
}

void SIM_Connect_Rail_links(){
  // add pointer to the rail_link
  pathlist.clear();

  for(int m = 0; m<unit_len; m++){
    if(!Units[m]){
      continue;
    }

    loggerf(INFO, "LINKING UNIT %i", m);

    Unit * tU = Units[m];

    link_all_blocks(tU);
    link_all_switches(tU);
    link_all_msswitches(tU);
  }

  pathlist_find();
}

void SIM_Client_Connect_cb(){
  // SimA_start();
  // SimB_start();
  // Algor_start();
  // Z21_start();
  // if(SYS->LC.state == Module_STOP){
  //   Algor_start();
  //   while(SYS->LC.state != Module_Run){}
  //   struct paths return_value = pathfinding(U_B(20,8), U_B(20,14));
  //   if(return_value.forward || return_value.reverse)
  //     printf("CHEERS");
  //   // pathfinding_print(instr);
  //   free_pathinstructions(return_value.forward);
  //   free_pathinstructions(return_value.reverse);
  // }
  loggerf(INFO, "Done SIM_Client_Connect_cb");
}
