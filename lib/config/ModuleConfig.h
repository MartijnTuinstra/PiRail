#ifndef INCLUDE_CONFIG_MODULECONFIG_H
#define INCLUDE_CONFIG_MODULECONFIG_H

#include <stdint.h>

#define ModuleConfigBasePath "./configs/units/"
#define LayoutSetupBasePath "./configs/setups/"

struct configStruct_Unit;
struct configStruct_Node;
struct configStruct_Block;
struct configStruct_Switch;
struct configStruct_MSSwitch;
struct configStruct_Station;
struct configStruct_Signal;

class ModuleConfig {
  public:
    char filename[100];
    bool parsed;

  private:
    struct configStruct_Unit * header;
  public:

    struct configStruct_Node * Nodes;
    struct configStruct_Block * Blocks;
    struct configStruct_PolarityGroup * PolarityGroup;
    struct configStruct_Switch * Switches;
    struct configStruct_MSSwitch * MSSwitches;
    struct configStruct_Station * Stations;
    struct configStruct_Signal * Signals;

    struct configStruct_WebLayout * Layout;

    char * buffer;
    uint32_t buffer_len;

    ModuleConfig(char * filename);
    ModuleConfig(char * filename, ModuleConfig *);
    ~ModuleConfig();

    int read();
    void dump();
    bool write();
    int calc_size();

    void newModule(uint8_t file, uint8_t connections);

    inline void print(){this->print(0,0);};
    void print(char ** cmds, uint8_t cmd_len);

    inline struct configStruct_Unit * const getHeader(){return header;};
};

void print_link(char ** debug, struct configStruct_RailLink link);
void print_Node(struct configStruct_Node node);
void print_Block(struct configStruct_Block block);
void print_PolarityGroup(struct configStruct_PolarityGroup PG);
void print_Switch(struct configStruct_Switch Switch);
void print_MSSwitch(struct configStruct_MSSwitch Switch, uint8_t printLevel);
void print_Signals(struct configStruct_Signal signal);
void print_Stations(struct configStruct_Station stations);
void print_Layout(struct configStruct_WebLayout * config);

#endif