#ifndef INCLUDE_CONFIG_MODULECONFIG_H
#define INCLUDE_CONFIG_MODULECONFIG_H

#include "config_data.h"

class ModuleConfig {
  public:
    char filename[100];
    bool parsed;

    struct s_unit_conf header;

    struct node_conf * Nodes;

    struct s_block_conf * Blocks;
    struct switch_conf * Switches;
    struct ms_switch_conf * MSSwitches;
    struct station_conf * Stations;
    struct signal_conf * Signals;

    uint16_t Layout_length;
    char * Layout;

    char * buffer;
    uint32_t buffer_len;

    ModuleConfig(char * filename);
    ~ModuleConfig();

    int read();
    void dump();
    void write();
    int calc_size();

    void newModule(uint8_t file, uint8_t connections);

    inline void print(){this->print(0,0);};
    void print(char ** cmds, uint8_t cmd_len);
};


void print_link(char ** debug, struct s_link_conf link);
void print_Node(struct node_conf node);
void print_Block(struct s_block_conf block);
void print_Switch(struct switch_conf Switch);
void print_MSSwitch(struct ms_switch_conf Switch);
void print_Signals(struct signal_conf signal);
void print_Stations(struct station_conf stations);
void print_Layout(struct ModuleConfig * config);

#endif