#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#include "modules.h"
#include "system.h"
#include "utils/logger.h"
#include "utils/mem.h"

#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/signals.h"
#include "IO.h"

#include "switchboard/station.h"


// void load_module_Configs(){
//   DIR *d;

//   struct dirent *dir;

//   d = opendir(ModuleConfigBasePath);

//   char type[5] = "";
//   int moduleID;
//   ModuleConfig * moduleID_list[255];
//   memset(moduleID_list, 0, 255 * sizeof(ModuleConfig *));

//   Units = (Unit **)_calloc(30, Unit *);
//   unit_len = 30;

//   if (d)
//   {
//     while ((dir = readdir(d)) != NULL)
//     {
//       if(sscanf(dir->d_name, "%i.%s", &moduleID, type) > 1 && strcmp(type, "bin") == 0){
//         char filename[50] = {0};
//         strcat(filename, ModuleConfigBasePath);
//         strcat(filename, dir->d_name);

//         moduleID_list[moduleID] = new ModuleConfig(filename);
//         moduleID_list[moduleID]->read();
//       }
//     }
//     closedir(d);
//   }

//   for(uint8_t i = 0; i<255; i++){
//     if(moduleID_list[i] == 0 || !moduleID_list[i]->parsed){
//       continue;
//     }

//     // read_module_Config(moduleID_list[i]);

//     new Unit(moduleID_list[i]);

//     delete moduleID_list[i];
//   }

//   SYS->modules_loaded = 1;

//   return;
// }

// void unload_module_Configs(){
//   if(SYS)
//     SYS->modules_loaded = 0;

//   for(int i = 0;i<unit_len;i++){
//     if(Units[i]){
//       delete Units[i];
//       Units[i] = 0;
//     }
//   }

//   if(Units)
//     Units = (Unit **)_free(Units);
//   if(stations)
//     stations = (Station **)_free(stations);
// }
