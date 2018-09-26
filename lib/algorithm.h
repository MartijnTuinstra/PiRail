#ifndef _INCLUDE_ALGORITHM_H
  #define _INCLUDE_ALGORITHM_H

  #define Block_Minimum_Size 60

  #include <pthread.h>
  #include "rail.h"

  extern pthread_mutex_t algor_mutex;

  typedef struct proces_block {
    _Bool blocked;
    uint8_t blocks;
    int length;
    Block * B[5];
  } Algor_Block;

  typedef struct algor_blocks {
    Algor_Block * BPPP;
    Algor_Block * BPP;
    Algor_Block * BP;
    Block * B;
    Algor_Block * BN;
    Algor_Block * BNN;
    Algor_Block * BNNN;
  } Algor_Blocks;

  void change_block_state(Algor_Block * A, enum Rail_states state);

  void scan_All();

  void * scan_All_continiously();

  void process(Block * B,int debug);
  void Algor_print_block_debug(struct algor_blocks AllBlocks);
  void Algor_search_Blocks(struct algor_blocks * AllBlocks, int debug);
  void Algor_Switch_Checker(struct algor_blocks AllBlocks, int debug);
  void Algor_train_following(struct algor_blocks AllBlocks, int debug);
  void Algor_rail_state(struct algor_blocks AllBlocks, int debug);
  void Algor_apply_rail_state(Algor_Block blocks, enum Rail_states state);
  void Algor_signal_state(struct algor_blocks AllBlocks, int debug);

  void procces_accessoire();

  struct ConnectList {
    int length;
    int list_index;
    struct rail_link ** R_L;
  };

  int init_connect_Algor(struct ConnectList * List);

  int connect_Algor(struct ConnectList * List);
#endif
