#include "switchboard/rail.h"


void change_Block(Block * B, enum Rail_states state);

void * TRAIN_SIMA(void * args);
void * TRAIN_SIMB(void * args);

int init_connect_Algor(struct ConnectList * List);
bool find_and_connect(uint8_t ModuleA, char anchor_A, uint8_t ModuleB, char anchor_B);
int connect_Algor(struct ConnectList * list);

void SIM_JoinModules();
void SIM_Connect_Rail_links();
void SIM_Client_Connect_cb();