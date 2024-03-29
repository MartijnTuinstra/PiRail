#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <vector>

#include "config/ModuleConfig.h"

#include "switchboard/manager.h"
#include "switchboard/unit.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/switchsolver.h"
#include "switchboard/msswitch.h"
#include "switchboard/signals.h"
#include "switchboard/station.h"
#include "switchboard/polarityGroup.h"

#include "utils/logger.h"
#include "system.h"

namespace switchboard {

Unit * Units(uint8_t unit){
  return SwManager->Units[unit];
}

  // private:
  //   std::vector<Block *> uniqueBlock;
  //   std::vector<Switch *> uniqueSwitch;
  //   std::vector<MSSwitch *> uniqueMSSwitch;
  //   std::vector<Signal *> uniqueSignal;
  //   std::vector<Station *> uniqueStation;

  // public:
  //   Unit * Units;
  //   uint8_t unit_len;

Manager::Manager(): filenames(5) {}
Manager::~Manager(){
  clear();
}

void Manager::addUnit(Unit * U){
  Units.insertAt(U, U->module);
}
uint16_t Manager::addBlock(Block * B){
  return uniqueBlock.push_back(B);
}
uint16_t Manager::addSwitch(Switch * Sw){
  return uniqueSwitch.push_back(Sw);
}
uint16_t Manager::addMSSwitch(MSSwitch * MSSw){
  return uniqueMSSwitch.push_back(MSSw);
}
uint16_t Manager::addSignal(Signal * Si){
  return uniqueSignal.push_back(Si);
}
uint16_t Manager::addStation(Station * St){
  return uniqueStation.push_back(St);
}

Block *    Manager::getBlock(uint16_t index){
  return uniqueBlock.at(index);
}
Switch *   Manager::getSwitch(uint16_t index){
  return uniqueSwitch.at(index);
}
MSSwitch * Manager::getMSSwitch(uint16_t index){
  return uniqueMSSwitch.at(index);
}
Signal *   Manager::getSignal(uint16_t index){
  return uniqueSignal.at(index);
}
Station *  Manager::getStation(uint16_t index){
  return uniqueStation.at(index);
}

void Manager::openDir(char * path){
  struct dirent *dir;

  DIR *d = opendir(path);

  char type[10] = "";
  int moduleID;

  if (d)
  {
    while ((dir = readdir(d)) != NULL)
    {
      if(sscanf(dir->d_name, "%i.%s", &moduleID, type) > 1 && strcmp(type, "bin") == 0){
        char filename[50] = {0};
        strcat(filename, path);
        strcat(filename, dir->d_name);

        loggerf(INFO, "Adding %s", filename);

        String * str = new String((const char *)filename);
        filenames.push_back(str);
      }
    }
    closedir(d);
  }
}

void Manager::addFile(char * filename){
  loggerf(TRACE, "Adding %s", filename);
  String * str = new String((const char *)filename);
  filenames.push_back(str);
}

void Manager::loadFiles(char * path){
  openDir(path);

  loadFiles();
}

void Manager::loadFiles(){
  for(uint16_t i = 0; i < filenames.size; i++){
    if(!filenames[i])
      continue;

    openFile(filenames[i]->string);
  }

  SYS->modules_loaded = 1;
  SwitchSolver::init();
}

void Manager::openFile(char * filename){
  auto mc = new ModuleConfig(filename);
  Configs.push_back(mc);

  mc->read();

  if(!mc->parsed){
    loggerf(WARNING, "Failed to parse Module Config. Skipping....");
    return;
  }

  new Unit(mc);
}

void Manager::scanSetups(char * directory){
  struct dirent *dir;

  DIR *d = opendir(directory);

  // char type[10] = "";
  // char name[40] = "";

  if (d)
  {
    while ((dir = readdir(d)) != NULL)
    {
      uint8_t len = strlen(dir->d_name);
      if(strcmp(&dir->d_name[len-4], ".bin") == 0){
        dir->d_name[len-4] = 0; // remove everything '.bin'
        loggerf(INFO, "Setup %s added", dir->d_name);

        String * str = new String((const char *)dir->d_name);
        setups.push_back(str);
      }
    }
    closedir(d);
  }
}

void Manager::clear(){
  if(SYS)
    SYS->modules_loaded = 0;

  Units.clear();
  uniqueBlock.empty();
  uniqueSwitch.empty();
  uniqueMSSwitch.empty();
  uniqueSignal.empty();
  uniqueStation.empty();

  filenames.clear();
  Configs.clear();

  SwitchSolver::free();
}

void Manager::print(){
  printf("Switchboard manager contents:\n");
  printf("Blocks:\t");
  uniqueBlock.print();
  printf("Switch:\t");
  uniqueSwitch.print();
  printf("MSSwitch:\t");
  uniqueMSSwitch.print();
  printf("Signals:\t");
  uniqueSignal.print();
  printf("Stations:\t");
  uniqueStation.print();
}

void Manager::LinkAndMap(){
  std::vector<Unit *> tmpU = {};

  // Link all rail-links together
  //   also create paths
  for(uint16_t i = 0; i < Units.size; i++){
    if(!Units[i])
      continue;

    Units[i]->link_blocks();
    Units[i]->link_switches();
    Units[i]->link_msswitches();
    tmpU.push_back(Units[i]);
  }

  for(auto u: tmpU)
    u->link_rest(); // Signals & Paths

  // Map all polarity connections
  for(auto u: tmpU)
    u->map_all();

  for(auto PG: PolarityGroupList)
    PG->map();
}

Manager * SwManager;
  
};