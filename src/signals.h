#ifndef t_signal
  #define t_signal
  struct signal{
    int id;
    int MAdr;
    int UAdr;
    int state;
    int type; // 0 = 2-state / 1 = 4-state / 2 = 8-state
    uint8_t length;

    short adr[6];
    char states[BLOCK_STATES];
    char flash[BLOCK_STATES];
  };

  struct signal *signals[MAX_A] = {};

  struct Unit;
#endif
