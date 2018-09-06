#ifndef INCLUDE_TRAINS_H
  #define INCLUDE_TRAINS_H

  #include <signal.h>
  #include "rail.h"
  #include "route.h"

  #define TRAIN_COMPS_CONF "./configs/train_comp.conf"
  #define CARS_CONF "./configs/cars.conf"
  #define ENGINES_CONF "./configs/engines.conf"
  #define CONF_VERSION 1

  struct train_funcs {
    char type:6;
    char momentary:1;
    char state:1;
  };

  struct train_comp {
    char type; //engine or car
    void * p;  //pointer to types struct
    uint16_t id; //number in list
  };

  struct __attribute__((__packed__)) train_comp_ws {
    char type;
    uint16_t ID;
  };

  struct train_composition {
    char * name;

    char nr_stock;
    struct train_comp * composition;
  };

  struct __attribute__((__packed__)) train_comp_conf {
    char name_len;
    char nr_stock;
    char check;
  };

  typedef struct engine {
    uint16_t DCC_ID;
    
    char type:3;
    char control:2;
    char dir:1;
    char halt:2;

    _Bool use;

    char cur_speed_step;
    char max_speed_step;
    uint16_t max_speed;

    char funcs_len;
    struct train_funcs ** funcs;

    uint16_t length;    //in mm   
    char * name;
    char * img_path;
    char * icon_path;
  } Engines;

  struct __attribute__((__packed__)) engine_conf {
    uint16_t DCC_ID;
    uint16_t max_spd;
    uint16_t length;
    uint8_t type;   //in mm   
    uint8_t name_len;
    uint8_t img_path_len;
    uint8_t icon_path_len;
    uint8_t check;
  };

  typedef struct car {
    int nr;
    char type:3;
    char control:2;
    char dir:1;
    char halt:1;
    int max_speed;

    char funcs_len;
    struct train_funcs ** funcs;

    int length;   //in mm
    char * name;
    char * img_path;
    char * icon_path;
  } Cars;

  struct __attribute__((__packed__)) car_conf {
    uint16_t nr;
    uint16_t length;
    uint8_t type;   //in mm   
    uint8_t name_len;
    uint8_t img_path_len;
    uint8_t icon_path_len;
    uint8_t check;
  };

  typedef struct trains {
    char * name;

    char nr_engines;
    Engines ** engines;

    char nr_stock;
    struct train_comp * composition; //One block memory for all nr_stocks

    int length; //in mm

    int max_speed;
    int cur_speed;

    char type:2;
    char in_use:1;
    char control:2;
    char dir:1;
    char halt:2;

    Route route;

    Block * Block;

    char timer;
    int timer_id;
  } Trains;

  extern Trains ** trains;
  extern int trains_len;
  extern Engines ** engines;
  extern int engines_len;
  extern Cars ** cars;
  extern int cars_len;
  extern struct train_composition ** trains_comp;
  extern int trains_comp_len;
  extern Trains ** train_link;
  extern int train_link_len;
  extern Trains * DCC_train[9999];

  void init_trains();
  void alloc_trains();
  void free_trains();

  void create_train(char * name, int nr_stock, struct train_comp_ws * comps);
  void create_engine(char * name,int DCC,char * img, char * icon, int max, char type, int length);
  void create_car(char * name,int nr,char * img, char * icon, char type, int length);

  int train_read_confs();

  int link_train(int fid, int tid, char type);
  void unlink_train(int fid);
#endif