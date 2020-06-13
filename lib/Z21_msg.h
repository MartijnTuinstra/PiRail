#ifndef _INCLUDE_Z21_MSG_H
#define _INCLUDE_Z21_MSG_H

void Z21_Set_Loco_Drive_Engine(Engine * E);
void Z21_Set_Loco_Drive_Train(Train * T);

void Z21_Set_Emergency();

void Z21_LAN_X_LOCO_INFO(uint8_t length, char * data);

void Z21_get_train(RailTrain * RT);
void Z21_get_loco_info(uint16_t DCC_id);

void Z21_setLocoFunction(Engine * E, uint8_t function, uint8_t type);

void Z21_KeepAliveFunc(void * args);
#endif