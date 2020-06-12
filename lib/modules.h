#ifndef _INCLUDE_MODULE_H
#define _INCLUDE_MODULE_H

#include <stdint.h>

#include "switchboard/unit.h"

#define ModuleConfigBasePath "./configs/units/"

#define Unit_Blocks_changed 0x1
#define Unit_Switch_changed 0x2
#define Unit_MSSwitch_changed 0x4
#define Unit_Signal_changed 0x8

void read_module_Config(uint16_t M);
void write_module_Config(uint16_t M);
void load_module_Configs();
void unload_module_Configs();

#endif
