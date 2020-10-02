#ifndef INCLUDE_CONFIG_DATA_H
#define INCLUDE_CONFIG_DATA_H

#include <stdint.h>

#define MODULE_CONF_VERSION 3
#define TRAIN_CONF_VERSION 3

#define F_CAR_DETECTABLE 0x01

#define TRAIN_CONF_PATH "configs/stock.bin"

#define SWITCH_FEEDBACK_FLAG 0x80

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
  uint16_t id;
  uint8_t type;
};

struct __attribute__((__packed__)) s_IO_port_conf {
  uint8_t Node;
  uint16_t Adr;
};

#include "switchboard/links.h"

struct __attribute__((__packed__)) s_node_conf {
  uint8_t Node;
  uint8_t size;
};

struct node_conf {
  uint8_t Node;
  uint8_t size;
  uint8_t * data;
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

// struct __attribute__((__packed__)) s_switch_state_conf {
//   uint8_t Node;
//   uint8_t Adr;
//   enum IO_event on_state_set;
//   uint8_t speed;
// };

struct __attribute__((__packed__)) s_switch_conf {
  uint8_t id;
  uint8_t det_block;
  struct s_link_conf App;
  struct s_link_conf Str;
  struct s_link_conf Div;
  uint8_t IO;
  uint8_t speed_Str;
  uint8_t speed_Div;
  uint8_t feedback_len;
};

struct switch_conf {
  uint8_t id;
  uint8_t det_block;

  struct s_link_conf App;
  struct s_link_conf Str;
  struct s_link_conf Div;

  uint8_t IO_len:4;
  uint8_t IO_type:4;
  uint8_t speed_Str;
  uint8_t speed_Div;
  uint8_t feedback_len;

  struct s_IO_port_conf * IO_Ports;
  uint8_t * IO_events;

  struct s_IO_port_conf * FB_Ports;
  uint8_t * FB_events;
};


struct s_ms_switch_state_conf {
  struct s_link_conf sideA;
  struct s_link_conf sideB;
  uint16_t speed;
  uint8_t dir;
  uint8_t output_sequence;
};

struct __attribute__((__packed__)) s_ms_switch_conf {
  uint8_t id;
  uint8_t det_block;
  uint8_t type;

  uint8_t nr_states;
  uint8_t IO;
};

struct __attribute__((__packed__)) ms_switch_conf {
  uint8_t id;
  uint8_t det_block;
  uint8_t type;

  uint8_t nr_states;
  uint8_t IO;

  struct s_ms_switch_state_conf * states;
  struct s_IO_port_conf * IO_Ports;  
};


struct __attribute__((__packed__)) s_station_conf {
  uint8_t type;
  uint8_t nr_blocks;
  uint8_t name_len;
  uint8_t reserved;
  uint16_t parent;
};

struct  station_conf {
    uint8_t type;
    uint8_t nr_blocks;
    uint8_t name_len;
    uint8_t reserved;
    uint16_t parent;

    uint8_t * blocks;
    char * name;
};

struct __attribute__((__packed__)) s_Signal_DependentSwitch {
  uint8_t type;
  uint8_t Sw;
  uint8_t state;
};

struct __attribute__((__packed__)) signal_conf {
  uint16_t direction:1;
  uint16_t id:15;
  // uint16_t blockId;
  struct s_link_conf Block;
  uint8_t output_len;
  uint8_t Switch_len;

  struct s_IO_port_conf * output;
  struct s_IO_signal_event_conf * stating;
  struct s_Signal_DependentSwitch * Switches;
};

struct __attribute__((__packed__)) s_IO_signal_event_conf {
  uint8_t event[8];
};

struct __attribute__((__packed__)) s_signal_conf {
  uint16_t direction:1;
  uint16_t id:15;
  struct s_link_conf Block;
  uint8_t output_len;
  uint8_t Switch_len;
};


struct module_config {
  struct s_unit_conf header;

  struct node_conf * Nodes;

  struct s_block_conf * Blocks;
  struct switch_conf * Switches;
  struct ms_switch_conf * MSSwitches;
  struct station_conf * Stations;
  struct signal_conf * Signals;

  uint16_t Layout_length;
  char * Layout;
};

struct __attribute__((__packed__)) s_train_header_conf {
  uint8_t P_Catagories;
  uint8_t C_Catagories;
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
  uint8_t functions[29];
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
  uint8_t functions[29];
  char * name;
  char * img_path;
  char * icon_path;
  struct engine_speed_steps * speed_steps;
};

struct __attribute__((__packed__)) s_r_car_conf {
  uint16_t nr;
  uint16_t max_speed;
  uint16_t length; // in mm
  uint8_t flags;
  uint8_t type; 
  uint8_t name_len;
  uint8_t icon_path_len;
  uint8_t functions[29];
};

struct __attribute__((__packed__)) s_car_conf {
  uint16_t nr;
  uint16_t max_speed;
  uint16_t length; // in mm
  uint8_t flags;
  uint8_t type;   
  uint8_t name_len;
  uint8_t icon_path_len;
  uint8_t functions[29];
};

struct cars_conf {
  uint16_t nr;
  uint16_t max_speed;
  uint16_t length; // in mm
  uint8_t flags;
  uint8_t type;   
  uint8_t name_len;
  uint8_t icon_path_len;
  uint8_t functions[29];
  char * name;
  char * icon_path;
};

struct __attribute__((__packed__)) train_comp_ws {
  uint8_t type;
  uint16_t id;
};

struct __attribute__((__packed__)) s_train_conf {
  uint8_t name_len;
  uint8_t nr_stock;
  uint8_t catagory;
};

struct trains_conf {
  uint8_t name_len;
  uint8_t nr_stock;
  uint8_t catagory;
  char * name;
  struct train_comp_ws * composition;
};

struct __attribute__((__packed__)) s_cat_conf {
  uint8_t name_len;
};

struct cat_conf {
  uint8_t name_len;
  char * name;
};

struct train_config {
  struct s_train_header_conf header;

  struct cat_conf * P_Cat;
  struct cat_conf * C_Cat;

  struct engines_conf * Engines;
  struct cars_conf * Cars;
  struct trains_conf * Trains;
};

#endif
