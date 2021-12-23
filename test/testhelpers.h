#ifndef _INCLUDE_TESTHELPERS_
#define _INCLUDE_TESTHELPERS_

#include <stdint.h>
#include "sim.h"

void init_test(char (* filenames)[30], int nr_files);

class TestsFixture {
public:
  TestsFixture();
  void loadSwitchboard(char (* filenames)[30], int nr_files);
  void loadStock();
  ~TestsFixture();
};

void test_Algorithm_tick();
void train_test_tick(struct train_sim * t, int32_t * i);
void train_testSim_tick(struct train_sim * t, int32_t * i);

void BlockTickNtimes(int I);

void D_printBlockStates(std::vector<Block *>&);

#endif