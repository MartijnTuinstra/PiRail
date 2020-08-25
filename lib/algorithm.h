#ifndef _INCLUDE_ALGORITHM_H
#define _INCLUDE_ALGORITHM_H

#define Block_Minimum_Size 60

#include <pthread.h>
#include <semaphore.h>
#include "switchboard/rail.h"

#define AlgorQueueLength 100

struct s_AlgorQueue {
  Block * B[AlgorQueueLength];
  uint16_t writeIndex;
  uint16_t readIndex;
};

struct ConnectList {
  int length;
  int list_index;
  struct rail_link ** R_L;
};

extern pthread_mutex_t algor_mutex;

extern pthread_mutex_t AlgorQueueMutex;
extern sem_t AlgorQueueNoEmpty;
extern struct s_AlgorQueue AlgorQueue;

void putAlgorQueue(Block * B, int enableQueue);
void putList_AlgorQueue(struct algor_blocks AllBlocks, int enable);
Block * getAlgorQueue();
void processAlgorQueue();

void * Algor_Run(void * args);

#define lock_Algor_process() mutex_lock(&algor_mutex, "Algor_mutex")
#define unlock_Algor_process() mutex_unlock(&algor_mutex, "Algor_mutex")

#define algor_queue_enable(enable) int val; \
                              sem_getvalue(&AlgorQueueNoEmpty, &val); \
                              if(val == 0 && enable){sem_post(&AlgorQueueNoEmpty);}

#define _DEBUG 1
#define _FORCE 2
#define _LOCK  0x80

void Algor_process(Block * B,int flags);

void Algor_Set_Changed(struct algor_blocks * blocks);
void Algor_init_Blocks(Algor_Blocks * ABs, Block * B);
void Algor_free_Blocks(Algor_Blocks * ABs);
void Algor_clear_Blocks(Algor_Blocks * ABs);

void Algor_Check_Algor_Stating(Block * B, uint8_t flags);

void Algor_print_block_debug(Block * B);
void Algor_search_Blocks(Block * B, int debug);
void Algor_special_search_Blocks(Block * B, int flags);
void Algor_turntable_search_Blocks(Block * B, int debug);
void Algor_Switch_Checker(Algor_Blocks * ABs, int debug);
void Algor_train_following(Algor_Blocks * ABs, int debug);
// void Algor_GetBlocked_Blocks(struct algor_blocks AllBlocks);
void Algor_rail_state(struct algor_blocks * ABs, int debug);
// void Algor_apply_rail_state(Algor_Block blocks, enum Rail_states state);

void Algor_train_control(Algor_Blocks * ABs, int debug);

void Algor_Connect_Rails();
void Algor_save_setup();

// void procces_accessoire();

// int init_connect_Algor(struct ConnectList * List);

// int connect_Algor(struct ConnectList * List);
#endif
