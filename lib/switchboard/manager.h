#ifndef _INCLUDE_SWITCHBOARD_MANAGER_H
#define _INCLUDE_SWITCHBOARD_MANAGER_H

#include <vector>
#include <stdint.h>
#include <cstring>
#include "utils/mem.h"
#include "utils/dynArray.h"

#include "switchboard/declares.h"

class ModuleConfig;

namespace switchboard {

Unit * Units(uint8_t);

class String {
  public:
    char * string;

    String(const char * str){
        int len = strlen(str) + 1;
        string = (char *)_calloc(len, char);
        strcpy(string, str);
    }
    ~String(){
        _free(string);
    }
};

class Manager {

  public:
    ::dynArray<Unit *>     Units;
    ::dynArray<Block *>    uniqueBlock;
    ::dynArray<Switch *>   uniqueSwitch;
    ::dynArray<MSSwitch *> uniqueMSSwitch;
    ::dynArray<Signal *>   uniqueSignal;
    ::dynArray<Station *>  uniqueStation;

    ::dynArray<String *>  filenames;
    ::dynArray<ModuleConfig *> Configs;

    Manager();
    ~Manager();

    void     addUnit(Unit *);
    uint16_t addBlock(Block *);
    uint16_t addSwitch(Switch *);
    uint16_t addMSSwitch(MSSwitch *);
    uint16_t addSignal(Signal *);
    uint16_t addStation(Station *);

    Block *    getBlock(uint16_t);
    Switch *   getSwitch(uint16_t);
    MSSwitch * getMSSwitch(uint16_t);
    Signal *   getSignal(uint16_t);
    Station *  getStation(uint16_t);

    void print();

    void openDir(char *);
    void addFile(char *);
    void loadFiles(char *);
    void loadFiles();
    void openFile(char *);

    void clear();

    void LinkAndMap();
};

extern Manager * SwManager;
}; //namespace


#endif