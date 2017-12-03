struct signal{
  int id;
  int MAdr;
  int UAdr;
  int state;
  int type; // 0 = 2-state / 1 = 4-state / 2 = 8-state
};

struct signal *signals[MAX_A] = {};
