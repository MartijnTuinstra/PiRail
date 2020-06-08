#include "logger.h"
#include "mem.h"
#include "train.h"
#include "websocket_stc.h"
#include "Z21.h"
#include "Z21_msg.h"

void Z21_Set_Loco_Drive_Engine(Engines * E){
  loggerf(TRACE, "Z21_Set_Loco_Drive_Engine %s", E->name);
  uint8_t data[11];
  data[0] = 0x0A;
  data[1] = 0x00;
  data[2] = 0x40;
  data[3] = 0x00;
  data[4] = 0xE4;
  data[5] = 0x10; // Speed speed_step_type
  if(E->speed_step_type == TRAIN_28_FAHR_STUFEN){ // 28 steps
    data[5] |= 2;
  }
  else if(E->speed_step_type == TRAIN_128_FAHR_STUFEN){ // 128 steps
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

void Z21_Set_Loco_Drive_Train(Trains * T){
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

  Engines * E = DCC_train[DCC_ID];

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

  bool samespeed = (speed == E->speed);
  bool samedir = (dir == E->dir);

  E->speed = speed;
  E->dir = dir;

  engine_read_speed(E);

  if(E->use){
    RailTrain * RT = E->RT;

    if(!samedir && RT->type == TRAIN_TRAIN_TYPE){
      Trains * T = (Trains *)RT->p;

      for(uint8_t i = 0; i < T->nr_engines; i++){
        if(E != T->engines[i]){
          T->engines[i]->dir = E->dir;
        }
      }
    }

    if(!samespeed){
      RT->speed = E->cur_speed;

      RT->changing_speed = RAILTRAIN_SPEED_T_DONE;

      if(RT->type == TRAIN_ENGINE_TYPE){
        engine_set_speed((Engines *)RT->p, RT->speed);
      }
      else{
        train_set_speed((Trains *)RT->p, RT->speed);
      }
    }

    if((!samedir || !samespeed) && RT->type == TRAIN_TRAIN_TYPE){
      Trains * T = (Trains *)RT->p;
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
  if(RT->type == TRAIN_ENGINE_TYPE){
    Z21_get_loco_info(((Engines *)RT->p)->DCC_ID);
  }
  else{
    Trains * T = (Trains *)RT->p;
    for(uint8_t i = 0; i < T->nr_engines; i++){
      Z21_get_loco_info(T->engines[i]->DCC_ID);
    }
  }
  
}

		train_set_speed(E->train, E->cur_speed);

		// WS_stc_UpdateTrain(E->train, TRAIN_TRAIN_TYPE);
	}
	// else{
		// WS_stc_UpdateTrain(E, TRAIN_ENGINE_TYPE);
	// }
}