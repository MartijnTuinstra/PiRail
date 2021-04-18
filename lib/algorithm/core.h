#ifndef _INCLUDE_ALGORITHM_CORE_H
#define _INCLUDE_ALGORITHM_CORE_H

#include <pthread.h>
#include "switchboard/rail.h"

#define _DEBUG 1
#define _FORCE 2
#define _LOCK  0x80


#define SpeedToDistance_A(s, a) 0.173625 * (s * s) / (2 * -a)


namespace Algorithm {

extern pthread_mutex_t process_mutex;

void process(Block * B,int flags);

void Set_Changed(struct algor_blocks * blocks);

void Check_Algor_Stating(Block * B, uint8_t flags);

void Switch_to_rail(Block *, void *, enum link_types, uint8_t);
void print_block_debug(Block * B);

void Switch_Checker(Algor_Blocks * ABs, int debug);
void rail_state(struct algor_blocks * ABs, int debug);
void ApplyRailState(uint8_t blocks, Block * B, Block * BL[10], enum Rail_states state[3], uint8_t prevGroup[3], uint8_t j, bool Dir);

void train_following(Algor_Blocks * ABs, int debug);
// void GetBlocked_Blocks(struct algor_blocks AllBlocks);
// void apply_rail_state(Algor_Block blocks, enum Rail_states state);

void train_control(RailTrain *);

void Connect_Rails();

// void procces_accessoire();

// int init_connect_Algor(struct ConnectList * List);

// int connect_Algor(struct ConnectList * List);
};

#endif