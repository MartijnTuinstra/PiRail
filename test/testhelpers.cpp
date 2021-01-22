
#include "algorithm/queue.h"
#include "utils/logger.h"
#include "system.h"

#include "switchboard/manager.h"
#include "rollingstock/manager.h"

#include "train.h"
#include "path.h"

void init_test(char (* filenames)[30], int nr_files){
  // logger.setlevel_stdout(MEMORY);
  init_main();

  switchboard::SwManager->clear();
  
  RSManager->clear();
  AlQueue.clear();
  pathlist.clear();

  for(int i = 0; i < nr_files; i++)
    switchboard::SwManager->addFile(filenames[i]);

  switchboard::SwManager->loadFiles();
  
  RSManager->loadFile("./testconfigs/stock.bin");

  logger.setlevel_stdout(INFO);
  loggerf(INFO, "Start Test");
  logger.setlevel_stdout(CRITICAL);
}
