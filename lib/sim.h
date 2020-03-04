#include "rail.h"
#include "algorithm.h"

void change_Block(Block * B, enum Rail_states state);

void * TRAIN_SIMA();
void * TRAIN_SIMB();

int init_connect_Algor(struct ConnectList * List);
_Bool find_and_connect(uint8_t ModuleA, char anchor_A, uint8_t ModuleB, char anchor_B);
int connect_Algor(struct ConnectList * list);
void * rail_link_pointer(struct rail_link link);

void SIM_JoinModules();
void SIM_Connect_Rail_links();
void SIM_Client_Connect_cb();