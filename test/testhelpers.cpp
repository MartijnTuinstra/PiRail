
#include "algorithm/queue.h"
#include "utils/logger.h"
#include "system.h"

#include "switchboard/manager.h"

#include "train.h"
#include "modules.h"
#include "path.h"

void init_test(char (* filenames)[30], int nr_files){
  init_main();

  switchboard::SwManager->clear();
  unload_rolling_Configs();
  AlQueue.clear();
  pathlist.clear();

  for(int i = 0; i < nr_files; i++)
    switchboard::SwManager->addFile(filenames[i]);

  switchboard::SwManager->loadFiles();
  
  load_rolling_Configs("./testconfigs/stock.bin");

  logger.setlevel_stdout(CRITICAL);
}