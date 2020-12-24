#include "catch.hpp"
#include "algorithm/queue.h"
#include "utils/logger.h"
#include "system.h"

#include "switchboard/manager.h"
#include "rollingstock/manager.h"

#include "train.h"
#include "modules.h"
#include "path.h"

void init_test(char (* filenames)[30], int nr_files){
  logger.setlevel_stdout(WARNING);
  init_main();

  switchboard::SwManager->clear();
  
  RSManager->clear();
  AlQueue.clear();
  pathlist.clear();

  logger.setlevel_stdout(INFO);
  loggerf(INFO, "Start Test \"%s\"", Catch::getResultCapture().getCurrentTestName().c_str());
  logger.setlevel_stdout(WARNING);

  for(int i = 0; i < nr_files; i++)
    switchboard::SwManager->addFile(filenames[i]);

  switchboard::SwManager->loadFiles();
  
  RSManager->loadFile("./testconfigs/stock.bin");
}
