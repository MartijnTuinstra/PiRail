#ifndef INCLUDE_CONFIG_NEWMODULECONFIG_H
#define INCLUDE_CONFIG_NEWMODULECONFIG_H

#include "config_data.h"
#include "config/ModuleConfig.h"


struct configStruct_Unit;
struct configStruct_Node;
struct configStruct_Block;
struct configStruct_Switch;
struct configStruct_MSSwitch;
struct configStruct_Station;
struct configStruct_Signal;

class newModuleConfig {
  public:
    char filename[100];
    bool parsed;

    struct configStruct_Unit * header;

    struct configStruct_Node * Nodes;
    struct configStruct_Block * Blocks;
    struct configStruct_Switch * Switches;
    struct configStruct_MSSwitch * MSSwitches;
    struct configStruct_Station * Stations;
    struct configStruct_Signal * Signals;

    struct configStruct_WebLayout * Layout;

    // uint16_t Layout_length;
    // char * Layout;

    char * buffer;
    uint32_t buffer_len;

    newModuleConfig(char * filename);
    newModuleConfig(char * filename, ModuleConfig *);
    ~newModuleConfig();

    int read();
    void dump();
    void write();
    int calc_size();

    void newModule(uint8_t file, uint8_t connections);

    inline void print(){this->print(0,0);};
    void print(char ** cmds, uint8_t cmd_len);
};


#endif