#ifndef INCLUDE_CONFIG_H
#define INCLUDE_CONFIG_H

#include "config_data.h"

uint8_t read_byte_conf(uint8_t ** p);

int calc_module_write_size(struct module_config * config);

int calc_train_write_size(struct train_config * config);


void print_hex(char * data, int size);

void write_module_from_conf(struct module_config * config, const char * filename);

void write_train_from_conf(struct train_config * config, const char * filename);

int check_Spacing(uint8_t ** p);

struct node_conf read_s_node_conf(uint8_t ** p);
struct s_unit_conf read_s_unit_conf(uint8_t ** p);

struct s_block_conf read_s_block_conf(uint8_t ** p);
struct switch_conf read_s_switch_conf(uint8_t ** p);
struct ms_switch_conf read_s_ms_switch_conf(uint8_t ** p);
struct station_conf read_s_station_conf(uint8_t ** p);
struct signal_conf read_s_signal_conf(uint8_t ** p);

struct s_train_header_conf read_s_train_header_conf(uint8_t ** p);
struct cars_conf read_cars_conf(uint8_t ** p);
struct engines_conf read_engines_conf(uint8_t ** p);
struct trains_conf read_trains_conf(uint8_t ** p);
struct cat_conf read_cat_conf(uint8_t ** p);

#endif
