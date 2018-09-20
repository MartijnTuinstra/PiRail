#ifndef INCLUDE_CONFIG_H
#define INCLUDE_CONFIG_H

#include "rail.h"
#include "IO.h"

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


struct config {
  struct s_unit_conf header;

  struct s_node_conf * Nodes;

  struct s_block_conf * Blocks;
  struct switch_conf * Switches;
  struct ms_switch_conf * MSSwitches;
  struct station_conf * Stations;
};

uint8_t read_byte_conf(uint8_t ** p);

int calc_write_size(struct config * config);


void print_hex(char * data, int size);

void write_from_conf(struct config * config, char * filename);

int check_Spacing(uint8_t ** p);

struct s_node_conf read_s_node_conf(uint8_t ** p);

struct s_unit_conf read_s_unit_conf(uint8_t ** p);

struct s_block_conf read_s_block_conf(uint8_t ** p);

struct switch_conf read_s_switch_conf(uint8_t ** p);

struct ms_switch_conf read_s_ms_switch_conf(uint8_t ** p);

struct station_conf read_s_station_conf(uint8_t ** p);

#endif
