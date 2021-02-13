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
#include "switchboard/blockconnector.h"

#include "rollingstock/railtrain.h"

#include "algorithm/queue.h"
#include "algorithm/core.h"
#include "algorithm/component.h"
#include "algorithm/blockconnector.h"


#include "train.h"
#include "path.h"

void init_test(char (* filenames)[30], int nr_files){
  logger.setlevel_stdout(INFO);
  init_main();

  loggerf(CRITICAL, "SWManager::clear");
  switchboard::SwManager->clear();
  
  loggerf(CRITICAL, "RSManager::clear");
  RSManager->clear();
  loggerf(CRITICAL, "Queue::clear");
  AlQueue.clear();
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

class TestsFixture {
public:
  TestsFixture();
  void loadSwitchboard(char (* filenames)[30], int nr_files);
  void loadStock();
  ~TestsFixture();
};

TestsFixture::TestsFixture(){
  printf("====================================================================================================\n");
  auto TestName = Catch::getResultCapture().getCurrentTestName();
  printf("---%*s%*s---\n",47+TestName.length()/2,TestName.c_str(),47-TestName.length()/2,"");

  logger.setlevel_stdout(WARNING);
  scheduler = new Scheduler();
  RSManager = new RollingStock::Manager();
  switchboard::SwManager = new switchboard::Manager();

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

  for(auto p: pathlist)
    delete p;

  pathlist.clear();
  AlQueue.clear();
  AlQueue.cleartmp();

  RSManager->clear();
  switchboard::SwManager->clear();

  delete RSManager;
  delete switchboard::SwManager;
  delete scheduler;

  destroy_main();
}

