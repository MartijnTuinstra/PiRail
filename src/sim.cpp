#include "utils/mem.h"
#include "utils/logger.h"
#include "system.h"

#include "sim.h"
#include "switchboard/manager.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
// #include "switchboard/blockconnector.h"
#include "train.h"

// #include "algorithm/core.h"
#include "algorithm/queue.h"
#include "algorithm/blockconnector.h"

#include "submodule.h"
#include "websocket/stc.h"
// #include "pathfinding.h"

#include "websocket/message_structure.h"

using namespace switchboard;

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
  if(state != B->state){
    B->setDetection(state == BLOCKED);

    loggerf(WARNING, "SIM set block %2i:%2i %i%i%i - %s>%s", B->module, B->id, B->blocked, B->detectionblocked, B->virtualblocked, rail_states_string[B->state], rail_states_string[state]);

    AlQueue.puttemp(B);
  }
}

struct engine_sim {
  uint16_t offset;
  uint16_t length;
};

struct train_sim {
  char sim;

  RailTrain * T;
  uint16_t train_length;

  uint8_t dir;
  float posFront;
  float posRear;

  Block * Front;
  Block ** B;

  struct engine_sim * engines;
  uint8_t engines_len;

  uint8_t blocks;
};

void train_sim_tick(struct train_sim * t){
  // loggerf(INFO, "train_sim_tick speed %i\t%f\t%f", t->T->speed,t->posFront, t->posRear);

  if(t->posFront <= 0 && t->blocks < 10){
    // Add block
    for(uint8_t i = t->blocks - 1; i >= 0 && i < 10; i--){
      t->B[i + 1] = t->B[i];
    }
    t->blocks++;

    if(t->blocks > 1 && t->B[1]->Alg.next){
      t->B[0] = t->B[1]->Alg.N[0];
    }
    else{
      SYS_set_state(&SYS->SimA.state, Module_Fail);
      return;
    }
    loggerf(INFO, "%c  Step %02i:%02i", t->sim, t->B[0]->module, t->B[0]->id);
    t->posFront += t->B[0]->length;
  }

  if(t->posRear <= 0){
    // Remove block
    t->blocks--;
    t->B[t->blocks] = 0;
    t->posRear += t->B[t->blocks - 1]->length;
  }

  // Advance train (km/h -> cm/s) / scale * tick interval (in sec)
  float distance = (t->T->speed / 3.6) * 100.0 / 160.0 * TRAINSIM_INTERVAL_SEC;
  t->posFront -= distance;
  t->posRear  -= distance;

  uint16_t blockoffset = 0;

  uint8_t stockid = 0;

  for(uint8_t i = 0; i < t->blocks; i++){
    bool blocktheblock = false;

    Block * tB = t->B[i];

    for(; stockid < t->engines_len; stockid++){
      struct engine_sim * E = &t->engines[stockid];
      uint16_t trainoffset = ((uint16_t)t->posFront) + E->offset;

      // Either:
      //   front side is in block
      //   rear  side is in block
      //   front and rear side are around block
      if((blockoffset <= trainoffset && blockoffset + tB->length > trainoffset) ||
         (blockoffset <= trainoffset + (E->length / 10) && blockoffset + tB->length > trainoffset + (E->length / 10)) || 
         (blockoffset >= trainoffset && blockoffset + tB->length > trainoffset &&
          blockoffset <= trainoffset + (E->length / 10) && blockoffset + tB->length < trainoffset + (E->length / 10)) ){
        blocktheblock = true;
        break;
      }
      else if(blockoffset + tB->length < trainoffset)
        break; // Detectable is not in this block but maybe in the next.
    }

    if(blocktheblock){
      change_Block(t->B[i], BLOCKED);
    }
    else
      change_Block(t->B[i], PROCEED);

    blockoffset += t->B[i]->length;
  }

  AlQueue.cpytmp();
}

void *TRAIN_SIMA(void * args){
  if(!SYS_wait_for_state(&SYS->LC.state, Module_Run)){
    SYS_set_state(&SYS->SimA.state, Module_Fail);
    return 0;
  }

  usleep(100000);

  Block *B = Units(25)->B[3];

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

  while(!B->Alg.N[0] || !B->Alg.P[0]){}
  while(B->Alg.N[0]->blocked || B->blocked || B->Alg.P[0]->blocked){} // Wait for space

  // B->train = new RailTrain(B);

  change_Block(B, BLOCKED);
  // algor_queue_enable(1);
  AlQueue.cpytmp();
  // B->setDetection(1);
  

  // usleep(100000);

  while(!B->train){
      usleep(10000);
  }

  // B->train = new RailTrain(B);

  B->train->link(2, RAILTRAIN_TRAIN_TYPE);
  struct s_opc_LinkTrain msg = {
    .follow_id=B->train->id,
    .real_id=2,
    .message_id_H=0,
    .type=RAILTRAIN_ENGINE_TYPE,
    .message_id_L=0
  };
  WS_stc_LinkTrain(&msg);

  loggerf(INFO, "SIMTrain linked %s", B->train->p.T->name);

  B->train->setControl(TRAIN_MANUAL);

  train.train_length = B->train->length;

  train.T = B->train;

  train.T->changeSpeed(50, IMMEDIATE_SPEED);
 
  AlQueue.put(B);

  if(train.T->type == RAILTRAIN_ENGINE_TYPE){
    //Engine only
    train.train_length = train.T->p.E->length / 10;

    train.engines_len = 1;
    train.engines = (struct engine_sim *)_calloc(1, struct engine_sim);
    train.engines[0].offset = 0;
    train.engines[0].length = train.T->p.E->length;
  }
  else{
    //Train
    train.train_length = train.T->p.T->length / 10;

    uint16_t offset = 0;

    train.engines_len = train.T->p.T->detectables;
    train.engines = (struct engine_sim *)_calloc(train.T->p.T->detectables, struct engine_sim);

    uint8_t j = 0;

    for(uint8_t i = 0; i < train.T->p.T->nr_stock; i++){
      if(train.T->p.T->composition[i].type == 0){
        train.engines[j].offset = offset;
        train.engines[j++].length = ((Engine *)train.T->p.T->composition[i].p)->length;
      }
      else if(train.T->p.T->composition[i].type == 1 && ((Car *)train.T->p.T->composition[i].p)->detectable){
        train.engines[j].offset = offset;
        train.engines[j++].length = ((Car *)train.T->p.T->composition[i].p)->length;
      }

      if(train.T->p.T->composition[i].type == 0)
        offset += ((Engine *)train.T->p.T->composition[i].p)->length / 10;
      else
        offset += ((Car *)train.T->p.T->composition[i].p)->length / 10;
    }
  }
  train.posFront = train.B[0]->length - (train.engines[0].length / 10);
  train.posRear = train.B[0]->length;
  loggerf(INFO, "train length %icm", train.train_length);

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

  Block *B = Units(25)->B[3];

  struct train_sim train;
  train.B = (Block **)_calloc(10, Block *);
  train.sim = 'B';
  train.posFront = 0;
  train.posRear = B->length;
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

  B->train->setControl(TRAIN_MANUAL);

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
  Units(10)->on_layout = 1;
  Units(20)->on_layout = 1;
  Units(21)->on_layout = 1;
  Units(22)->on_layout = 1;
  Units(23)->on_layout = 1;
  Units(25)->on_layout = 1;

  WS_stc_Track_Layout(0);
  printf("Ready to join modules\n");

  auto connectors = Algorithm::find_connectors();
  uint16_t maxConnectors = connectors.size();

  loggerf(INFO, "Have %i connectors", connectors.size());

  uint8_t x = 0;
  char data[20];

  while(SYS->modules_linked == 0){
    WS_stc_trackUpdate(0);
    
    if(uint8_t * findResult = Algorithm::find_connectable(&connectors)){
      Algorithm::connect_connectors(&connectors, findResult);

      data[0] = 0x82;
      data[1] = (char)connectors.size();
      data[2] = maxConnectors;
      int k = 3;
      for(int j = 0;j< SwManager->Units.size;j++){
        if(Units(j)){
          data[k++] = j;
        }
      }
      WSServer->send_all(data, k, 0x10);
    }

    if(connectors.size() == 0)
      break;

    usleep(JOIN_SIM_INTERVAL);

    if(x == 1){
      Units(20)->block_state_changed = 1;
      Units(25)->block_state_changed = 1;

      Units(20)->B[5]->setDetection(1);
      Units(25)->B[0]->setDetection(1);
      Units(20)->B[5]->state = BLOCKED;
      Units(25)->B[0]->state = BLOCKED;
    }else if(x == 2){
      Units(20)->block_state_changed = 1;
      Units(25)->block_state_changed = 1;
      
      Units(25)->block_state_changed = 1;
      Units(22)->block_state_changed = 1;

      Units(25)->B[3]->setDetection(1);
      Units(22)->B[0]->setDetection(1);
      Units(25)->B[3]->state = BLOCKED;
      Units(22)->B[0]->state = BLOCKED;

      Units(20)->B[5]->setDetection(0);
      Units(25)->B[0]->setDetection(0);
      Units(20)->B[5]->state = PROCEED;
      Units(25)->B[0]->state = PROCEED;
    }else if(x == 3){
      Units(25)->block_state_changed = 1;
      Units(22)->block_state_changed = 1;

      Units(22)->block_state_changed = 1;
      Units(10)->block_state_changed = 1;

      Units(22)->B[1]->setDetection(1);
      Units(10)->B[0]->setDetection(1);
      Units(22)->B[1]->state = BLOCKED;
      Units(10)->B[0]->state = BLOCKED;

      Units(25)->B[3]->setDetection(0);
      Units(22)->B[0]->setDetection(0);
      Units(25)->B[3]->state = PROCEED;
      Units(22)->B[0]->state = PROCEED;
    }else if(x == 4){
      Units(22)->block_state_changed = 1;
      Units(10)->block_state_changed = 1;

      Units(10)->block_state_changed = 1;
      Units(21)->block_state_changed = 1;

      Units(10)->B[3]->setDetection(1);
      Units(21)->B[0]->setDetection(1);
      Units(10)->B[3]->state = BLOCKED;
      Units(21)->B[0]->state = BLOCKED;

      Units(22)->B[1]->setDetection(0);
      Units(10)->B[0]->setDetection(0);
      Units(22)->B[1]->state = PROCEED;
      Units(10)->B[0]->state = PROCEED;
    }else if(x == 5){
      Units(10)->block_state_changed = 1;
      Units(21)->block_state_changed = 1;
      
      Units(21)->block_state_changed = 1;
      Units(23)->block_state_changed = 1;

      Units(21)->B[3]->setDetection(1);
      Units(23)->B[0]->setDetection(1);
      Units(21)->B[3]->state = BLOCKED;
      Units(23)->B[0]->state = BLOCKED;

      Units(10)->B[3]->setDetection(0);
      Units(21)->B[0]->setDetection(0);
      Units(10)->B[3]->state = PROCEED;
      Units(21)->B[0]->state = PROCEED;
    }else if(x == 6){
      Units(21)->block_state_changed = 1;
      Units(23)->block_state_changed = 1;

      Units(23)->block_state_changed = 1;
      Units(20)->block_state_changed = 1;

      Units(23)->B[1]->setDetection(1);
      Units(20)->B[0]->setDetection(1);
      Units(23)->B[1]->state = BLOCKED;
      Units(20)->B[0]->state = BLOCKED;

      Units(21)->B[3]->setDetection(0);
      Units(23)->B[0]->setDetection(0);
      Units(21)->B[3]->state = PROCEED;
      Units(23)->B[0]->state = PROCEED;
    }else if(x == 7){
      Units(23)->block_state_changed = 1;
      Units(20)->block_state_changed = 1;

      Units(23)->B[1]->setDetection(0);
      Units(20)->B[0]->setDetection(0);
      Units(23)->B[1]->state = PROCEED;
      Units(20)->B[0]->state = PROCEED;
    }
    else if(x == 10){
      SYS->modules_linked = 1; // break condition
    }

    x++;
  }

  for(uint8_t i = 0; i < SwManager->Units.size; i++){
    Unit * U = Units(i);
    if(!U)
      continue;

    for(uint8_t j = 0; j < U->block_len; j++){
      if(U->B[j]){
        U->B[j]->setDetection(0);
        U->B[j]->setVirtualDetection(0);
        U->B[j]->state = PROCEED;
      }
    }
  }


  WS_stc_Track_Layout(0);
}

void SIM_Connect_Rail_links(){
  // add pointer to the rail_link
  pathlist.clear();

  for(int m = 0; m< SwManager->Units.size; m++){
    if(!Units(m)){
      continue;
    }

    loggerf(INFO, "LINKING UNIT %i", m);

    Units(m)->link_all();
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
