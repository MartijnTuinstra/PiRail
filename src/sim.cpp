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

#include "algorithm/core.h"
#include "algorithm/queue.h"
#include "algorithm/blockconnector.h"

#include "submodule.h"
#include "websocket/stc.h"
#include "path.h"

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

#define JOIN_SIM_INTERVAL 1000

void change_Block(Block * B, enum Rail_states state){
  if((state != BLOCKED && B->detectionBlocked) || (state == BLOCKED && !B->detectionBlocked)){
    B->setDetection(state == BLOCKED);

    if(state == BLOCKED)
      loggerf(INFO, "SIM set block %2i:%2i %i%i%i", B->module, B->id, B->blocked, B->detectionBlocked, B->virtualBlocked);
    else
      loggerf(INFO, "SIM unset block %2i:%2i %i%i%i", B->module, B->id, B->blocked, B->detectionBlocked, B->virtualBlocked);

    AlQueue.puttemp(B);
  }
}

void train_sim_tick(struct train_sim * t){

  const std::lock_guard<std::mutex> lock(Algorithm::processMutex);

  if(t->posFront <= 0 && t->blocks < 10){
    // Add block
    for(uint8_t i = t->blocks - 1; i >= 0 && i < 10; i--){
      t->B[i + 1] = t->B[i];
    }
    t->blocks++;

    if(t->blocks > 1 && t->B[1]->Alg.N->group[3]){
      t->B[0] = t->B[1]->Alg.N->B[0];
    }
    else{
      loggerf(WARNING, "train_sim_tick Failed to get blocks %i  %02i:%02i > %i", t->blocks, t->B[1]->module, t->B[1]->id, t->B[1]->Alg.N->group[3]);
      SYS_set_state(&SYS->SimA.state, Module_Fail);
      return;
    }
    loggerf(INFO, "%c  Step %02i:%02i", t->sim, t->B[0]->module, t->B[0]->id);
    t->posFront += t->B[0]->length;
  }

  if(t->posRear <= 0){
    // Remove block
    t->blocks--;

    if(t->B[t->blocks]->detectionBlocked){
      t->B[t->blocks]->setDetection(0);
      AlQueue.puttemp(t->B[t->blocks]);
    }

    t->B[t->blocks] = 0;
    t->posRear += t->B[t->blocks - 1]->length;
  }

  // Advance train (km/h -> cm/s) / scale * tick interval (in sec)
  float distance = ((t->T->speed / 3.6) * 100.0 / 160.0) * TRAINSIM_INTERVAL_SEC;
  t->posFront -= distance;
  t->posRear  -= distance;

  uint16_t blockoffset = 0;

  uint8_t stockid = 0;

  // char debug[100] = " ";
  // char * p = &debug[0];

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

    if(blocktheblock && !t->B[i]->detectionBlocked){
      t->B[i]->setDetection(1);
      AlQueue.puttemp(t->B[i]);
    }
    else if(!blocktheblock && t->B[i]->detectionBlocked){
      t->B[i]->setDetection(0);
      AlQueue.puttemp(t->B[i]);
    }

    // p += sprintf(p, "%c%02i:%02i%c ", t->B[i]->detectionBlocked ? 'B' : ' ', t->B[i]->module, t->B[i]->id, blocktheblock ? 'B' : ' ');

    blockoffset += t->B[i]->length;
  }

  // loggerf(INFO, "SIM blocks (%i): %s", t->blocks, debug);

  AlQueue.cpytmp();
}

void *TRAIN_SIMA(void * args){
  if(!SYS_wait_for_state(&SYS->LC.state, Module_Run)){
    SYS_set_state(&SYS->SimA.state, Module_Fail);
    return 0;
  }

  usleep(100000);

  Block *B = Units(25)->B[6];

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

  while(!B->Alg.N->B[0] || !B->Alg.P->B[0]){usleep(10000);}
  while(B->Alg.N->B[0]->blocked || B->blocked || B->Alg.P->B[0]->blocked){usleep(10000);} // Wait for space

  // B->train = new Train(B);

  change_Block(B, BLOCKED);
  // algor_queue_enable(1);
  AlQueue.cpytmp();
  // B->setDetection(1);
  

  // usleep(100000);

  while(!B->train){
    usleep(10000);
  }

  // B->train = new Train(B);

  B->train->link(0, TRAIN_ENGINE_TYPE);
  struct s_opc_LinkTrain msg = {
    .follow_id=B->train->id,
    .real_id=0,
    .message_id_H=0,
    .type=TRAIN_ENGINE_TYPE,
    .message_id_L=0
  };
  WS_stc_LinkTrain(&msg);

  if(B->train->type == TRAIN_TRAIN_TYPE)
    loggerf(INFO, "SIMTrain linked %s", B->train->p.T->name);
  else
    loggerf(INFO, "SIMTrain linked %s", B->train->p.E->name);

  B->train->setControl(TRAIN_MANUAL);

  train.train_length = B->train->length;

  train.T = B->train;

  train.T->changeSpeed(10, 0);
 
  AlQueue.put(B);

  if(train.T->type == TRAIN_ENGINE_TYPE){
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
    SYS_set_state(&SYS->SimB.state, Module_Fail);
    return 0;
  }

  usleep(100000);

  Block *B = Units(25)->B[3];

  struct train_sim train = {
    .sim = 'B',
    .train_length = 0,
    .posFront = 0.0,
    .posRear = 124.0,
    .Front = 0,
    .B = (Block **)_calloc(10, Block *),
    .blocks = 1,
  };
  train.B[0] = B;

  while(!B->Alg.N->B[0] || !B->Alg.P->B[0]){usleep(10000);}
  while(B->Alg.N->B[0]->blocked || B->blocked || B->Alg.P->B[0]->blocked){usleep(10000);} // Wait for space

  // B->train = new Train(B);

  change_Block(B, BLOCKED);
  // algor_queue_enable(1);
  AlQueue.cpytmp();
  // B->setDetection(1);
  

  // usleep(100000);

  while(!B->train){
    usleep(10000);
  }

  // B->train = new Train(B);

  B->train->link(1, TRAIN_ENGINE_TYPE);
  struct s_opc_LinkTrain msg = {
    .follow_id=B->train->id,
    .real_id=1,
    .message_id_H=0,
    .type=TRAIN_ENGINE_TYPE,
    .message_id_L=1
  };
  WS_stc_LinkTrain(&msg);

  if(B->train->type == TRAIN_TRAIN_TYPE)
    loggerf(INFO, "SIMTrain linked %s", B->train->p.T->name);
  else
    loggerf(INFO, "SIMTrain linked %s", B->train->p.E->name);

  B->train->setControl(TRAIN_MANUAL);

  train.train_length = B->train->length;

  train.T = B->train;

  train.T->changeSpeed(10, 0);
 
  AlQueue.put(B);

  if(train.T->type == TRAIN_ENGINE_TYPE){
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

  SYS_set_state(&SYS->SimB.state, Module_Run);

  while(SYS->SimB.state & Module_Run){
    train_sim_tick(&train);
    usleep(TRAINSIM_INTERVAL_US);
  }

  _free(train.B);
  SYS_set_state(&SYS->SimB.state, Module_STOP);

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

  uint16_t msgID = WS_stc_ScanStatus(-1, 0, maxConnectors);

  while(SYS->modules_linked == 0){
    WS_stc_trackUpdate(0);
    
    if(uint8_t * findResult = Algorithm::find_connectable(&connectors)){
      Algorithm::connect_connectors(&connectors, findResult);

      WS_stc_ScanStatus(msgID, maxConnectors - connectors.size(), maxConnectors);
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

  switchboard::SwManager->LinkAndMap();

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
