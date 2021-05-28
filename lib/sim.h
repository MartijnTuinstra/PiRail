#ifndef _INCLUDE_SIM_H
#define _INCLUDE_SIM_H

#include "switchboard/rail.h"

#define TRAINSIM_INTERVAL_US 50000
#define TRAINSIM_INTERVAL_SEC 0.05

struct engine_sim {
  uint16_t offset;
  uint16_t length;
};

struct train_sim {
  char sim;

  Train * T;
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

void change_Block(Block * B, enum Rail_states state);
void train_sim_tick(struct train_sim * t);

void * TRAIN_SIMA(void * args);
void * TRAIN_SIMB(void * args);

int init_connect_Algor(struct ConnectList * List);
bool find_and_connect(uint8_t ModuleA, char anchor_A, uint8_t ModuleB, char anchor_B);
int connect_Algor(struct ConnectList * list);

void SIM_JoinModules();
void SIM_Connect_Rail_links();
void SIM_Client_Connect_cb();

#endif