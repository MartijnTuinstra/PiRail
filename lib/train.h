#ifndef INCLUDE_TRAINS_H
  #define INCLUDE_TRAINS_H

  #include <signal.h>
  #include "rail.h"
  #include "route.h"
  #include "config_data.h"

  #define TRAIN_COMPS_CONF "./configs/train_comp.conf"
  #define CARS_CONF "./configs/cars.conf"
  #define ENGINES_CONF "./configs/engines.conf"
  #define CONF_VERSION 1

  struct engine_speed_steps;
  typedef struct trains Trains;

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
    uint16_t id;
    
    uint8_t type;
    uint8_t control;
    uint8_t dir;
    uint8_t halt;

    _Bool use;
    Trains * train;

    char speed; // Z21 Speed
    char speed_step_type;
    uint16_t max_speed; // Real Speed
    uint16_t cur_speed; // Real Speed

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
    uint8_t type;
    uint8_t control;
    uint8_t dir;
    uint8_t halt;
    uint16_t max_speed;

    char funcs_len;
    struct train_funcs ** funcs;

    uint16_t length;   //in mm
    char * name;
    char * img_path;
    char * icon_path;
  } Cars;

  struct train_comp_ws;

  typedef struct trains {
    uint16_t id;
    char * name;

    uint8_t nr_engines;
    Engines ** engines;

    uint8_t nr_stock;
    struct train_comp * composition; //One block memory for all nr_stocks

    uint16_t length; //in mm

    uint16_t max_speed;
    uint16_t cur_speed;

    uint8_t type;

    uint8_t in_use:1;
    uint8_t control:2;
    uint8_t dir:1;
    uint8_t halt:2;
    uint8_t save:1;

    Route route;

    Block * Block;

    char timer;
    int timer_id;
  } Trains;

  #define TRAIN_14_FAHR_STUFEN 0
  #define TRAIN_28_FAHR_STUFEN 1
  #define TRAIN_128_FAHR_STUFEN 2

  #define TRAIN_MANUAL 0
  #define TRAIN_SEMI_AUTO 1
  #define TRAIN_FULL_AUTO 2

  #define TRAIN_ENGINE_TYPE 0
  #define TRAIN_TRAIN_TYPE 1

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

  extern struct cat_conf * train_P_cat;
  extern int train_P_cat_len;
  extern struct cat_conf * train_C_cat;
  extern int train_C_cat_len;

  void init_trains();
  void alloc_trains();
  void free_trains();

  #define create_engine_from_conf(e) create_engine(e.name, e.DCC_ID, e.img_path, e.icon_path, e.type, e.length, e.config_steps, e.speed_steps)
  #define create_car_from_conf(c) create_car(c.name, c.nr, c.img_path, c.icon_path, c.type, c.length, c.max_speed)
  #define create_train_from_conf(t) create_train(t.name, t.nr_stock, t.composition, t.catagory, 1)

  void create_train(char * name, int nr_stock, struct train_comp_ws * comps, uint8_t catagory, uint8_t save);
  void clear_train(Trains ** E);
  void create_engine(char * name,int DCC,char * img, char * icon, char type, int length, int steps_len, struct engine_speed_steps * steps);
  void clear_engine(Engines ** E);
  void create_car(char * name,int nr,char * img, char * icon, char type, uint16_t length, uint16_t speed);
  void clear_car(Cars ** E);

  int train_read_confs();
  void train_write_confs();

  int link_train(int fid, int tid, char type);
  void unlink_train(int fid);

  void engine_calc_speed(Engines * E);
  void engine_calc_real_speed(Engines * E);
  void train_calc_speed(Trains * T);
#endif