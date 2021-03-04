#include "utils/logger.h"
#include "utils/mem.h"

#include "train.h"
#include "websocket/stc.h"
#include "Z21.h"
#include "Z21_msg.h"

void Z21_Set_EmergencyStop(){
  uint8_t tmp[7] = {0x07,0,0x40,0,0x21,0x80,0xA1};

  if(Z21)
    Z21->send(7, tmp);
}

void Z21_Release_EmergencyStop(){
  uint8_t tmp[7] = {0x07,0,0x40,0,0x21,0x81,0xA0};

  if(Z21)
    Z21->send(7, tmp);
}

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
  data[8] = ((E->dir & 1) << 7) | (E->Z21_set_speed & 0x7F);
  data[9] = data[4] ^ data[5] ^ data[6] ^ data[7] ^ data[8];

  if(Z21)
    Z21->send(10, data);
}

void Z21_Set_Loco_Drive_Train(Train * T){
  loggerf(TRACE, "Z21_Set_Loco_Drive_Train %s", T->name);
  for(int e = 0; e < T->engines->items; e++){
    Z21_Set_Loco_Drive_Engine(T->engines->operator[](e));
  }
}

void Z21_LAN_X_LOCO_INFO(uint8_t length, char * data){
  loggerf(TRACE, "Z21_LAN_X_LOCO_INFO");
  uint16_t DCC_ID = ((data[0] & 0x3F) << 8) + data[1];
  // uint8_t xbusRegeler = (data[2] & 0x8) >> 3;
  // uint8_t stufen = (data[2] & 0x7);
  bool dir = (data[3] & 0x80) ? true : false;
  char speed = (data[3] & 0x7F);

  Engine * E = RSManager->getEngineDCC(DCC_ID);

  if(!E){
    loggerf(ERROR, "No train with DCC address %i\n", DCC_ID);
    return;
  }

  loggerf(INFO, " - Engine %i: speed(%i), dir(i)", DCC_ID, speed, dir);
  // loggerf(INFO, " -  %02x %02x %02x %02x", data[9], data[10], data[11], data[12]);

  E->Z21_setFunctions(&data[4], length - 4);
  E->Z21_setSpeedDir(speed, dir);
}

void Z21_get_train(RailTrain * RT){
  if(RT->type == RAILTRAIN_ENGINE_TYPE){
    Z21_get_loco_info(RT->p.E->DCC_ID);
  }
  else{
    auto TrainEngines = RT->p.T->engines;
    for(uint8_t i = 0; i < TrainEngines->items; i++){
      Z21_get_loco_info((*TrainEngines)[i]->DCC_ID);
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

  if(Z21)
    Z21->send(9, data);
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

  if(Z21)
    Z21->send(10, data);
}

void Z21_KeepAliveFunc(void * args){
  if(Z21)
    Z21_GET_SERIAL;
}