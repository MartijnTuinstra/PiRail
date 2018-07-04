#ifndef _INCLUDE_ALGORITHM_H
  #define _INCLUDE_ALGORITHM_H

  #define Block_Minimum_Size 60

  #include "rail.h"

  typedef struct procces_block {
    _Bool blocked;
    char blocks;
    int length;
    Block * B[5];
  } Algor_Block;

  typedef struct algor_blocks {
    struct procces_block * BPPP;
    struct procces_block * BPP;
    struct procces_block * BP;
    Block * B;
    struct procces_block * BN;
    struct procces_block * BNN;
    struct procces_block * BNNN;
  } Algor_Blocks;

  void change_block_state(Algor_Block * A, enum Rail_states state);

  void scan_All();

  void * scan_All_continiously();

  void procces(Block * B,int debug);
  void Algor_search_Blocks(struct algor_blocks * AllBlocks, int debug);
  void Algor_train_following(struct algor_blocks AllBlocks, int debug);
  void Algor_rail_state(struct algor_blocks AllBlocks, int debug);
  void Algor_apply_rail_state(struct procces_block blocks, enum Rail_states state);
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
