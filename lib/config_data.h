#ifndef INCLUDE_CONFIG_DATA_H
#define INCLUDE_CONFIG_DATA_H

#define MODULE_CONF_VERSION 3
#define TRAIN_CONF_VERSION 2

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
  uint16_t id;
  uint8_t type;
};

struct __attribute__((__packed__)) s_IO_port_conf {
  uint8_t Node;
  uint16_t Adr;
};

#ifndef RAIL_LINK_TYPES
#define RAIL_LINK_TYPES 
enum link_types {
  RAIL_LINK_R,
  RAIL_LINK_S,
  RAIL_LINK_s,
  RAIL_LINK_MA,
  RAIL_LINK_MB,
  RAIL_LINK_ma,
  RAIL_LINK_mb,
  RAIL_LINK_TT = 0x10, // Turntable
  RAIL_LINK_C  = 0xfe,
  RAIL_LINK_E  = 0xff
};
#endif

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


struct __attribute__((__packed__)) signal_conf {
  uint16_t side:1;
  uint16_t id:15;
  uint16_t blockId;
  uint8_t output_len;

  struct s_IO_port_conf * output;
  struct s_IO_signal_event_conf * stating;
};

struct __attribute__((__packed__)) s_IO_signal_event_conf {
  uint8_t event[8];
};

struct __attribute__((__packed__)) s_signal_conf {
  uint16_t side:1;
  uint16_t id:15;
  uint16_t blockId;
  uint8_t output_len;
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

struct __attribute__((__packed__)) s_r_car_conf {
  uint16_t nr;
  uint16_t length;
  uint8_t type;   //in mm   
  uint8_t name_len;
  uint8_t icon_path_len;
};

struct __attribute__((__packed__)) s_car_conf {
  uint16_t nr;
  uint16_t max_speed;
  uint16_t length;
  uint8_t type;   //in mm   
  uint8_t name_len;
  uint8_t icon_path_len;
};

struct cars_conf {
  uint16_t nr;
  uint16_t max_speed;
  uint16_t length;
  uint8_t type;   //in mm   
  uint8_t name_len;
  uint8_t icon_path_len;
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
