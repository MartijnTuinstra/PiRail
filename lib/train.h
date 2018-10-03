#ifndef INCLUDE_TRAINS_H
  #define INCLUDE_TRAINS_H

  #include <signal.h>
  #include "rail.h"
  #include "route.h"

  #define TRAIN_COMPS_CONF "./configs/train_comp.conf"
  #define CARS_CONF "./configs/cars.conf"
  #define ENGINES_CONF "./configs/engines.conf"
  #define CONF_VERSION 1

  struct engine_speed_steps;

  struct train_funcs {
    char type:6;
    char momentary:1;
    char state:1;
  };

  struct __attribute__((__packed__)) train_comp {
    uint8_t type; //engine or car
    uint16_t id; //number in list
    void * p;  //pointer to types struct
  };

  struct train_composition {
    char * name;

    char nr_stock;
    struct train_comp * composition;
  };

  typedef struct engine {
    uint16_t DCC_ID;
    
    char type:3;
    char control:2;
    char dir:1;
    char halt:2;

    _Bool use;

    char cur_speed_step;
    char max_step;
    uint16_t max_speed;

    uint8_t steps_len;
    struct engine_speed_steps * steps;

    char funcs_len;
    struct train_funcs ** funcs;

    uint16_t length;    //in mm   
    char * name;
    char * img_path;
    char * icon_path;
  } Engines;

  typedef struct car {
    uint16_t nr;
    char type:3;
    char control:2;
    char dir:1;
    char halt:2;
    int max_speed;

    char funcs_len;
    struct train_funcs ** funcs;

    int length;   //in mm
    char * name;
    char * img_path;
    char * icon_path;
  } Cars;

  struct train_comp_ws;

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
  extern Engines * DCC_train[9999];

  void init_trains();
  void alloc_trains();
  void free_trains();

  #define create_engine_from_conf(e) create_engine(e.name, e.DCC_ID, e.img_path, e.icon_path, e.type, e.length, e.config_steps, e.speed_steps)
  #define create_car_from_conf(c) create_car(c.name, c.nr, c.img_path, c.icon_path, c.type, c.length)
  #define create_train_from_conf(t) create_train(t.name, t.nr_stock, t.composition)

  void create_train(char * name, int nr_stock, struct train_comp_ws * comps);
  void create_engine(char * name,int DCC,char * img, char * icon, char type, int length, int steps_len, struct engine_speed_steps * steps);
  void create_car(char * name,int nr,char * img, char * icon, char type, int length);

  int train_read_confs();
  void train_write_confs();

  int link_train(int fid, int tid, char type);
  void unlink_train(int fid);
#endif