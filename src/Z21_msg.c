#include "logger.h"
#include "mem.h"
#include "train.h"
#include "websocket_stc.h"
#include "Z21.h"
#include "Z21_msg.h"

void Z21_Set_Loco_Drive_Engine(Engine * E){
  loggerf(TRACE, "Z21_Set_Loco_Drive_Engine %s", E->name);
  uint8_t data[11];
  data[0] = 0x0A;
  data[1] = 0x00;
  data[2] = 0x40;
  data[3] = 0x00;
  data[4] = 0xE4;
  data[5] = 0x10; // Speed speed_step_type
  if(E->speed_step_type == ENGINE_28_FAHR_STUFEN){ // 28 steps
    data[5] |= 2;
  }
  else if(E->speed_step_type == ENGINE_128_FAHR_STUFEN){ // 128 steps
    data[5] |= 3;
  }
  data[6] = (E->DCC_ID & 0x3F00) >> 8;
  
  if(E->DCC_ID >= 128)
    data[6] |= 0x80;

  data[7] = E->DCC_ID & 0xFF;
  data[8] = ((E->dir & 1) << 7) | (E->speed & 0x7F);
  data[9] = data[4] ^ data[5] ^ data[6] ^ data[7] ^ data[8];

  Z21_send_data(data, 10);
}

void Z21_Set_Loco_Drive_Train(Train * T){
  loggerf(TRACE, "Z21_Set_Loco_Drive_Train %s", T->name);
  for(int e = 0; e < T->nr_engines; e++){
    Z21_Set_Loco_Drive_Engine(T->engines[e]);
  }
}

void Z21_LAN_X_LOCO_INFO(uint8_t length, char * data){
  loggerf(TRACE, "Z21_LAN_X_LOCO_INFO");
  uint16_t DCC_ID = ((data[0] & 0x3F) << 8) + data[1];
  // uint8_t xbusRegeler = (data[2] & 0x8) >> 3;
  // uint8_t stufen = (data[2] & 0x7);
  uint8_t dir = (data[3] & 0x80) >> 7;
  uint8_t speed = (data[3] & 0x7F);

  Engine * E = DCC_train[DCC_ID];

  if(!E){
    loggerf(ERROR, "No train with DCC address %i\n", DCC_ID);
    return;
  }

  loggerf(INFO, " - Engine %i", DCC_ID);
  for(uint8_t i = 0; i < length; i++){
    printf("%02x ", data[i]);
  }
  // loggerf(INFO, " -  %02x %02x %02x %02x", data[9], data[10], data[11], data[12]);

  //Functions ....
  E->function[0].state = (data[4] >> 4) & 0x1;
  E->function[1].state = data[4] & 0x1;
  E->function[2].state = (data[4] >> 1) & 0x1;
  E->function[3].state = (data[4] >> 2) & 0x1;
  E->function[4].state = (data[4] >> 3) & 0x1;

  for(uint8_t i = 0; i < 3; i++){
    if ((i + 4) > length) {
      break;
    }

    for(uint8_t j = 0; j < 8; j++){
      E->function[i * 8 + j + 5].state = (data[5 + i] >> j) & 0x1;
    }
  }

  bool samespeed = (speed == E->speed);
  bool samedir = (dir == E->dir);

  E->speed = speed;
  E->dir = dir;

  E->readSpeed();

  if(E->use){
    RailTrain * RT = E->RT;

    if(!samedir && RT->type == RAILTRAIN_TRAIN_TYPE){
      Train * T = RT->p.T;

      for(uint8_t i = 0; i < T->nr_engines; i++){
        if(E != T->engines[i]){
          T->engines[i]->dir = E->dir;
        }
      }
    }

    if(!samespeed){
      RT->speed = E->cur_speed;

      RT->changing_speed = RAILTRAIN_SPEED_T_DONE;

      RT->setSpeed(RT->speed);
    }

    if((!samedir || !samespeed) && RT->type == RAILTRAIN_TRAIN_TYPE){
      Train * T = RT->p.T;
      // Set all other engines coupled to this train to new parameters
      for(uint8_t i = 0; i < T->nr_engines; i++){
        if(E != T->engines[i]){
          Z21_Set_Loco_Drive_Engine(T->engines[i]);
        }
      }
    }

    WS_stc_UpdateTrain(RT);
  }
  else{
    WS_stc_DCCEngineUpdate(E);
  }
}

void Z21_get_train(RailTrain * RT){
  if(RT->type == RAILTRAIN_ENGINE_TYPE){
    Z21_get_loco_info(RT->p.E->DCC_ID);
  }
  else{
    Train * T = RT->p.T;
    for(uint8_t i = 0; i < T->nr_engines; i++){
      Z21_get_loco_info(T->engines[i]->DCC_ID);
    }
  }
  
}

void Z21_get_loco_info(uint16_t DCC_id){
  uint8_t data[10];

  data[0] = 0x09;
  data[1] = 0x00;
  data[2] = 0x40;
  data[3] = 0x00;
  data[4] = 0xE3;
  data[5] = 0xF0;
  data[6] = (DCC_id & 0x3F00) >> 8;
  data[7] = (DCC_id & 0xFF);
  data[8] = 0x13 ^ data[6] ^ data[7];

  Z21_send_data(data, 9);
}

void Z21_setLocoFunction(Engine * E, uint8_t function, uint8_t type){
  uint8_t data[11];

  loggerf(INFO, "Set function %i of train %i", function, E->DCC_ID);

  data[0] = 0x0A;
  data[1] = 0x00;
  data[2] = 0x40;
  data[3] = 0x00;
  data[4] = 0xE4;
  data[5] = 0xF8;
  data[6] = (E->DCC_ID & 0x3F00) >> 8;

  if(E->DCC_ID >= 128)
    data[6] |= 0x80;

  data[7] = (E->DCC_ID & 0xFF);
  data[8] = ((type & 0x3) << 6) | (function & 0x3F);
  data[9] = 0x1C ^ data[6] ^ data[7] ^ data[8];

  Z21_send_data(data, 10);
}

void Z21_KeepAliveFunc(void * args){
  Z21_send(4, 0x10);
}