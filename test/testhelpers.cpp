#include "catch.hpp"

#include "algorithm/queue.h"
#include "utils/logger.h"
#include "system.h"

#include "switchboard/manager.h"
#include "rollingstock/manager.h"


#include "config/ModuleConfig.h"
#include "switchboard/manager.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/station.h"
#include "switchboard/unit.h"
#include "switchboard/polarityGroup.h"
#include "switchboard/blockconnector.h"

#include "rollingstock/train.h"

#include "algorithm/queue.h"
#include "algorithm/core.h"
#include "algorithm/component.h"
#include "algorithm/blockconnector.h"


#include "train.h"
#include "path.h"
#include "sim.h"
#include "testhelpers.h"

struct s_systemState * SYS;

void init_test(char (* filenames)[30], int nr_files){
  logger.setlevel_stdout(INFO);
  init_main();

  loggerf(CRITICAL, "SWManager::clear");
  switchboard::SwManager->clear();
  
  loggerf(CRITICAL, "RSManager::clear");
  RSManager->clear();
  loggerf(CRITICAL, "Queue::clear");
  AlQueue.clear();
  AlQueue.cleartmp();
  AlQueue.clearTrain();
  loggerf(CRITICAL, "Pathlist::clear");
  pathlist.clear();

  logger.setlevel_stdout(CRITICAL);
  loggerf(CRITICAL, "SWManager::addFile");

  for(int i = 0; i < nr_files; i++)
    switchboard::SwManager->addFile(filenames[i]);

  switchboard::SwManager->loadFiles();
  
  if(!RSManager->continue_event)
    RSManager->initScheduler();

  RSManager->loadFile("./testconfigs/stock.bin");

  logger.setlevel_stdout(INFO);
  loggerf(INFO, "Start Test");
  logger.setlevel_stdout(TRACE);
}

TestsFixture::TestsFixture(){
  printf("====================================================================================================\n");
  auto TestName = Catch::getResultCapture().getCurrentTestName();
  printf("---%*s%*s---\n",(int)(47+TestName.length()/2),TestName.c_str(),(int)(47-TestName.length()/2),"");

  logger.setlevel_stdout(WARNING);
  scheduler = new Scheduler();
  RSManager = new RollingStock::Manager();
  switchboard::SwManager = new switchboard::Manager();

  pathlist.clear();

  // logger.setlevel_stdout(WARNING);
  init_main();
}
void TestsFixture::loadSwitchboard(char (* filenames)[30], int nr_files){
  for(int i = 0; i < nr_files; i++)
    switchboard::SwManager->addFile(filenames[i]);

  switchboard::SwManager->loadFiles();
}
void TestsFixture::loadStock(){
  RSManager->initScheduler();

  RSManager->loadFile("./testconfigs/stock.bin");
}
TestsFixture::~TestsFixture(){
  logger.setlevel_stdout(WARNING);

  for(auto g: PolarityGroupList)
    delete g;

  for(auto p: pathlist)
    delete p;

  PolarityGroupList.clear();
  pathlist.clear();
  AlQueue.clear();
  AlQueue.cleartmp();
  AlQueue.clearTrain();

  RSManager->clear();
  switchboard::SwManager->clear();

  delete RSManager;
  delete switchboard::SwManager;
  delete scheduler; // stop and destroy
  scheduler = 0;

  destroy_main();
}

void test_Algorithm_tick(){
  if(AlQueue.queue->getItems() > 0){
    Algorithm::BlockTick();
  }
  Algorithm::TrainTick();
}

void train_test_tick(struct train_sim * t, int32_t * i){
  train_sim_tick(t);
  test_Algorithm_tick();

  i[0] -= 1;
}

void train_testSim_tick(struct train_sim * t, int32_t * i){
  usleep(TRAINSIM_INTERVAL_US);

  train_test_tick(t, i);
}


void BlockTickNtimes(int I){
  Block * B = AlQueue.getWait();
  if(!B)
    return;

  do
  {
    loggerf(INFO, "Process %i:%i, %x, %x", B->module, B->id, B->IOchanged + (B->statechanged << 1) + (B->algorchanged << 2), B->state);
    Algorithm::processBlock(&B->Alg, 0);
    while(B->recalculate && I-- > 0){
      loggerf(INFO, "ReProcess");
      B->recalculate = 0;
      Algorithm::processBlock(&B->Alg, 0);
    }
  }
  while( (B = AlQueue.get()) && I-- > 0);
}

void D_printBlockStates(std::vector<Block *>& blocks){

  char debug[1000] = "|";
  char * p = &debug[1];

  for(auto b: blocks)
    p += sprintf(p, "--%2i:%2i--|", b->module, b->id);

  p += sprintf(p, "\n ");

  for(auto b: blocks){
    uint8_t colour = 0;
    char text[10] = "";
    sprintf(text, "   %c   ", b->dir ? 'R' : 'F');
    switch(b->state){
      case BLOCKED:
        strcpy(text, " train ");
        colour = 88; break;
      case DANGER:
        colour = 1; break;
      case CAUTION:
        colour = 3; break;
      case PROCEED:
        colour = 2; break;
      case RESERVED:
        colour = 32; break;
      case RESTRICTED:
        colour = 202; break;
    }

    p += sprintf(p, "\x1b[48;5;%im %s \x1b[0m ", colour, text);
  }

  p += sprintf(p, "\n ");

  for(auto b: blocks){
    uint8_t colour = 0;
    char text[10] = "";
    sprintf(text, "   %c   ", b->dir ? 'F' : 'R');
    switch(b->reverse_state){
      case BLOCKED:
        strcpy(text, " train ");
        colour = 88; break;
      case DANGER:
        colour = 1; break;
      case CAUTION:
        colour = 3; break;
      case PROCEED:
        colour = 2; break;
      case RESERVED:
        colour = 32; break;
      case RESTRICTED:
        colour = 202; break;
    }

    p += sprintf(p, "\x1b[48;5;%im %s \x1b[0m ", colour, text);
  }

  p += sprintf(p, "\n");

  printf("%s", debug);
}