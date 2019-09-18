#ifndef _INCLUDE_ALGORITHM_H
  #define _INCLUDE_ALGORITHM_H

  #define Block_Minimum_Size 60

  #include <pthread.h>
  #include <semaphore.h>
  #include "rail.h"

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

  void change_block_state(Algor_Block * A, enum Rail_states state);

  void scan_All();

  void * scan_All_continiously();

  void putAlgorQueue(Block * B, int enableQueue);
  void putList_AlgorQueue(struct algor_blocks AllBlocks, int enable);
  Block * getAlgorQueue();
  void processAlgorQueue();

  void * Algor_Run();

  #define lock_Algor_process() mutex_lock(&algor_mutex, "Algor_mutex")
  #define unlock_Algor_process() mutex_unlock(&algor_mutex, "Algor_mutex")

  #define algor_queue_enable(enable) int val; \
                                sem_getvalue(&AlgorQueueNoEmpty, &val); \
                                if(val == 0 && enable){sem_post(&AlgorQueueNoEmpty);}

  void process(Block * B,int debug);
  void Algor_Set_Changed(struct algor_blocks * blocks);
  void Algor_init_Blocks(struct algor_blocks * AllBlocks, Block * B);
  void Algor_free_Blocks(struct algor_blocks * AllBlocks);
  void Algor_clear_Blocks(struct algor_blocks * AllBlocks);
  void Algor_print_block_debug(struct algor_blocks AllBlocks);
  void Algor_search_Blocks(struct algor_blocks * AllBlocks, int debug);
  void Algor_Switch_Checker(struct algor_blocks AllBlocks, int debug);
  void Algor_train_following(struct algor_blocks AllBlocks, int debug);
  void Algor_GetBlocked_Blocks(struct algor_blocks AllBlocks);
  void Algor_rail_state(struct algor_blocks AllBlocks, int debug);
  void Algor_apply_rail_state(Algor_Block blocks, enum Rail_states state);
  void Algor_signal_state(struct algor_blocks AllBlocks, int debug);
  void Algor_train_control(struct algor_blocks AllBlocks, int debug);

  void procces_accessoire();

  int init_connect_Algor(struct ConnectList * List);

  int connect_Algor(struct ConnectList * List);
#endif
