#ifndef INCLUDE_TRAINS_H
#define INCLUDE_TRAINS_H

#include <signal.h>
#include <pthread.h>
#include "rail.h"
// #include "route.h"
#include "config_data.h"
#include "scheduler/event.h"

#define TRAIN_COMPS_CONF "./configs/train_comp.conf"
#define CARS_CONF "./configs/cars.conf"
#define ENGINES_CONF "./configs/engines.conf"
#define CONF_VERSION 1

struct engine_speed_steps;
typedef struct trains Trains;


struct train_funcs {
  uint8_t type;
  uint8_t button:4;
  uint8_t state:4;
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

  bool use;
  Trains * train;
  RailTrain * RT;

  char speed; // Z21 Speed
  char speed_step_type;
  uint16_t max_speed; // Real Speed
  uint16_t cur_speed; // Real Speed

  uint8_t steps_len;
  struct engine_speed_steps * steps;

  struct train_funcs function[29];

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

  struct train_funcs function[29];

  uint16_t length;   //in mm
  char * name;
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

  Block * B;

  char timer;
  int timer_id;
} Trains;

struct TrainSpeedEventData;

typedef struct rail_train {
  void * p;
  char type;

  Block * B;

  uint8_t link_id;

  uint16_t speed;        // Real speed
  uint16_t max_speed;    // Real max speed

  uint16_t target_speed;
  uint16_t target_distance;

  uint8_t changing_speed:3;  // RAILTRAIN_SPEED_T_(INIT / CHANGING / UPDATE / DONE / FAIL)
  uint8_t control:2;         // TRAIN_(MANUAL / SEMI_AUTO / FULL_AUTO)
  uint8_t route:1;           //
  uint8_t stop:1;            // 
  uint8_t dir:1;             // TRAIN_FORWARD / TRAIN_REVERSE

  struct SchedulerEvent * speed_event;
  struct TrainSpeedEventData * speed_event_data;

  uint8_t category;

  // struct pathinstruction * instructions;
} RailTrain;

struct train_speed_timer {
  RailTrain * T;
  uint16_t target_speed;
  uint16_t length;
};

struct TrainSpeedEventData {
  RailTrain * T;

  uint16_t startSpeed;
  float acceleration;
  float time;
  uint16_t steps;
  uint16_t stepCounter;
  uint32_t stepTime;

  struct timespec starttime;
};

#define RAILTRAIN_SPEED_T_INIT 0
#define RAILTRAIN_SPEED_T_CHANGING 1
#define RAILTRAIN_SPEED_T_UPDATE 2
#define RAILTRAIN_SPEED_T_DONE 3
#define RAILTRAIN_SPEED_T_FAIL 4

#define IMMEDIATE_SPEED 0
#define GRADUAL_FAST_SPEED 1
#define GRADUAL_SLOW_SPEED 2

#define TRAIN_14_FAHR_STUFEN 0
#define TRAIN_28_FAHR_STUFEN 1
#define TRAIN_128_FAHR_STUFEN 2

#define TRAIN_MANUAL 0
#define TRAIN_SEMI_AUTO 1
#define TRAIN_FULL_AUTO 2

#define TRAIN_FORWARD 0
#define TRAIN_REVERSE 1

#define TRAIN_ENGINE_TYPE 0
#define TRAIN_TRAIN_TYPE 1

#define TRAIN_FUNCTION_MOMENTARY 0
#define TRAIN_FUNCTION_TOGGLE 1
#define TRAIN_FUNCTION_SHORT 2
#define TRAIN_FUNCTION_LONG 3

#define TRAIN_FUNCTION_UNDEFINED 0
#define TRAIN_FUNCTION_HEADLIGHTS 2
#define TRAIN_FUNCTION_CABLIGHTS 3
#define TRAIN_FUNCTION_LOW_HORN 4
#define TRAIN_FUNCTION_HIGH_HORN 5
#define TRAIN_FUNCTION_UNUSED 0x3F

extern Trains ** trains;
extern int trains_len;
extern Engines ** engines;
extern int engines_len;
extern Cars ** cars;
extern int cars_len;
extern RailTrain ** train_link;
extern int train_link_len;
extern Engines * DCC_train[9999];

extern struct cat_conf * train_P_cat;
extern int train_P_cat_len;
extern struct cat_conf * train_C_cat;
extern int train_C_cat_len;

int read_rolling_Configs();
void write_rolling_Configs();
int load_rolling_Configs();
void unload_rolling_Configs();

#define create_engine_from_conf(e) Create_Engine(e.name, e.DCC_ID, e.img_path, e.icon_path, e.type, e.length, e.config_steps, e.speed_steps, e.functions)
#define create_car_from_conf(c) Create_Car(c.name, c.nr, c.icon_path, c.type, c.length, c.max_speed)
#define create_train_from_conf(t) Create_Train(t.name, t.nr_stock, t.composition, t.catagory, 1)

void Create_Train(char * name, int nr_stock, struct train_comp_ws * comps, uint8_t catagory, uint8_t save);
void Clear_Train(Trains ** E);
void Create_Engine(char * name,int DCC,char * img, char * icon, char type, int length, int steps_len, struct engine_speed_steps * steps, uint8_t functions[28]);
void Clear_Engine(Engines ** E);
void Create_Car(char * name,int nr, char * icon, char type, uint16_t length, uint16_t speed);
void Clear_Car(Cars ** E);

RailTrain * new_railTrain();

int link_train(int fid, int tid, char type);
void unlink_train(int fid);

void engine_set_speed(Engines * E, uint16_t speed);
void engine_read_speed(Engines * E);

void train_speed_event_create(RailTrain * T, uint16_t targetSpeed, uint16_t distance);
void train_speed_event_calc(struct TrainSpeedEventData * data);
void train_speed_event_init(RailTrain * T);
void train_speed_event_tick(struct TrainSpeedEventData * data);

void train_set_speed(Trains * T, uint16_t speed);
void train_calc_speed(Trains * T);
void train_change_speed(RailTrain * T, uint16_t target_speed, uint8_t type);

void train_set_route(RailTrain * T, Block * dest);
#endif
