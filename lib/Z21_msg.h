#ifndef _INCLUDE_Z21_MSG_H
#define _INCLUDE_Z21_MSG_H

void Z21_Set_Loco_Drive_Engine(Engines * E);
void Z21_Set_Loco_Drive_Train(Trains * T);

void Z21_LAN_X_LOCO_INFO(uint8_t length, char * data);

#endif