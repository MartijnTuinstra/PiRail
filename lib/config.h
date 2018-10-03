#ifndef INCLUDE_CONFIG_H
#define INCLUDE_CONFIG_H

#include "rail.h"
#include "IO.h"

#define MODULE_CONF_VERSION 1
#define TRAIN_CONF_VERSION 1

#define TRAIN_CONF_PATH "configs/stock.bin"

struct __attribute__((__packed__)) s_unit_conf {
  uint8_t module;
  uint8_t connections;
  uint8_t IO_Nodes;
  uint16_t Blocks;
  uint16_t Switches;
  uint16_t MSSwitches;
  uint16_t Signals;
  uint8_t Stations;
};

struct __attribute__((__packed__)) s_link_conf {
  uint8_t module;
  uint8_t id;
  uint8_t type;
};

struct __attribute__((__packed__)) s_IO_port_conf {
  uint8_t Node;
  uint8_t Adr;
};

#ifndef RAIL_LINK_TYPES
#define RAIL_LINK_TYPES 
enum link_types {
  RAIL_LINK_R,
  RAIL_LINK_S,
  RAIL_LINK_s,
  RAIL_LINK_M,
  RAIL_LINK_m,
  RAIL_LINK_C = 0xfe,
  RAIL_LINK_E = 0xff
};
#endif

struct __attribute__((__packed__)) s_node_conf {
  uint8_t Node;
  uint8_t size;
};

struct __attribute__((__packed__)) s_block_conf {
  uint8_t id;
  uint8_t type;
  struct s_link_conf next;
  struct s_link_conf prev;
  struct s_IO_port_conf IO_In;
  struct s_IO_port_conf IO_Out;
  uint8_t speed;
  uint16_t length;
  uint8_t fl; //FLAGS, 0x1 = OneWay, 0x6 = dir, 0x8 = out enable
};

struct __attribute__((__packed__)) s_switch_state_conf {
  uint8_t Node;
  uint8_t Adr;
  enum IO_event on_state_set;
  uint8_t speed;
};

struct __attribute__((__packed__)) s_switch_conf {
  uint8_t id;
  uint8_t det_block;
  struct s_link_conf App;
  struct s_link_conf Str;
  struct s_link_conf Div;
  uint8_t IO;
  uint8_t speed_Str;
  uint8_t speed_Div;
};

struct switch_conf {
  uint8_t id;
  uint8_t det_block;

  struct s_link_conf App;
  struct s_link_conf Str;
  struct s_link_conf Div;

  uint8_t IO;        // 0xf0 type, 0x0f length
  uint8_t speed_Str;
  uint8_t speed_Div;

  struct s_IO_port_conf * IO_Ports;
};


struct s_ms_switch_state_conf {
  struct s_link_conf sideA;
  struct s_link_conf sideB;
  uint8_t speed;
  uint16_t output_sequence;
};

struct __attribute__((__packed__)) s_ms_switch_conf {
  uint8_t id;
  uint8_t det_block;

  uint8_t nr_states;
  uint8_t IO;
};

struct ms_switch_conf {
  uint8_t id;
  uint8_t det_block;

  uint8_t nr_states;
  uint8_t IO;

  struct s_ms_switch_state_conf * states;
  struct s_IO_port_conf * IO_Ports;  
};


struct __attribute__((__packed__)) s_station_conf {
  uint8_t type;
  uint8_t nr_blocks;
  uint8_t name_len;
};

struct station_conf {
    uint8_t type;
    uint8_t nr_blocks;
    uint8_t name_len;
    uint8_t * blocks;
    char * name;
};


struct module_config {
  struct s_unit_conf header;

  struct s_node_conf * Nodes;

  struct s_block_conf * Blocks;
  struct switch_conf * Switches;
  struct ms_switch_conf * MSSwitches;
  struct station_conf * Stations;
};

struct __attribute__((__packed__)) s_train_header_conf {
  uint16_t Engines;
  uint16_t Cars;
  uint16_t Trains;
};

struct __attribute__((__packed__)) s_engine_conf {
  uint16_t DCC_ID;
  uint16_t length;
  uint8_t type;   //in mm
  uint8_t config_steps;
  uint8_t name_len;
  uint8_t img_path_len;
  uint8_t icon_path_len;
};

struct __attribute__((__packed__)) engine_speed_steps {
  uint16_t speed;
  uint8_t step;
};

struct engines_conf {
  uint16_t DCC_ID;
  uint16_t length;
  uint8_t type;   //in mm
  uint8_t config_steps;
  uint8_t name_len;
  uint8_t img_path_len;
  uint8_t icon_path_len;
  char * name;
  char * img_path;
  char * icon_path;
  struct engine_speed_steps * speed_steps;
};

struct __attribute__((__packed__)) s_car_conf {
  uint16_t nr;
  uint16_t length;
  uint8_t type;   //in mm   
  uint8_t name_len;
  uint8_t img_path_len;
  uint8_t icon_path_len;
};

struct cars_conf {
  uint16_t nr;
  uint16_t length;
  uint8_t type;   //in mm   
  uint8_t name_len;
  uint8_t img_path_len;
  uint8_t icon_path_len;
  char * name;
  char * img_path;
  char * icon_path;
};

struct __attribute__((__packed__)) train_comp_ws {
  uint8_t type;
  uint16_t id;
};

struct __attribute__((__packed__)) s_train_conf {
  uint8_t name_len;
  uint8_t nr_stock;
};

struct trains_conf {
  uint8_t name_len;
  uint8_t nr_stock;
  char * name;
  struct train_comp_ws * composition;
};

struct train_config {
  struct s_train_header_conf header;

  struct engines_conf * Engines;
  struct cars_conf * Cars;
  struct trains_conf * Trains;
};

uint8_t read_byte_conf(uint8_t ** p);

int calc_module_write_size(struct module_config * config);

int calc_train_write_size(struct train_config * config);


void print_hex(char * data, int size);

void write_module_from_conf(struct module_config * config, char * filename);

void write_train_from_conf(struct train_config * config, char * filename);

int check_Spacing(uint8_t ** p);

struct s_node_conf read_s_node_conf(uint8_t ** p);

struct s_unit_conf read_s_unit_conf(uint8_t ** p);

struct s_block_conf read_s_block_conf(uint8_t ** p);

struct switch_conf read_s_switch_conf(uint8_t ** p);

struct ms_switch_conf read_s_ms_switch_conf(uint8_t ** p);

struct station_conf read_s_station_conf(uint8_t ** p);

struct s_train_header_conf read_s_train_header_conf(uint8_t ** p);
struct cars_conf read_cars_conf(uint8_t ** p);
struct engines_conf read_engines_conf(uint8_t ** p);
struct trains_conf read_trains_conf(uint8_t ** p);

#endif
