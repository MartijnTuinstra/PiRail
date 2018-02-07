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

  #define SIG_GREEN      0
  #define SIG_AMBER      1
  #define SIG_RED        2
  #define SIG_RESTRICTED 3
  #define SIG_CAUTION    4
#endif
