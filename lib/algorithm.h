#ifndef _INCLUDE_ALGORITHM_H
  #define _INCLUDE_ALGORITHM_H

  #include "rail.h"

  struct procces_block {
    _Bool blocked;
    char length;
    Block * B[5];
  };

  void change_block_state(struct procces_block * A, enum Rail_states state);

  void scan_All();

  void * scan_All_continiously();

  void procces(Block * B,int debug);

  void procces_accessoire();

  struct ConnectList {
    int length;
    int list_index;
    struct rail_link ** R_L;
  };

  int init_connect_Algor(struct ConnectList * List);

  int connect_Algor(struct ConnectList * List);
#endif
