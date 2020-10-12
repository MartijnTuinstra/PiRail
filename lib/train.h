#ifndef INCLUDE_TRAINS_H
#define INCLUDE_TRAINS_H

#include <signal.h>
#include <pthread.h>
#include "switchboard/rail.h"
// #include "route.h"
#include "config_data.h"
#include "scheduler/event.h"

#include "rollingstock/functions.h"
#include "rollingstock/car.h"
#include "rollingstock/engine.h"
#include "rollingstock/train.h"
#include "rollingstock/railtrain.h"


#define TRAIN_COMPS_CONF "./configs/train_comp.conf"
#define CARS_CONF "./configs/cars.conf"
#define ENGINES_CONF "./configs/engines.conf"
#define CONF_VERSION 1

struct engine_speed_steps;

#define RAILTRAIN_SPEED_T_INIT 0
#define RAILTRAIN_SPEED_T_CHANGING 1
#define RAILTRAIN_SPEED_T_UPDATE 2
#define RAILTRAIN_SPEED_T_DONE 3
#define RAILTRAIN_SPEED_T_FAIL 4

#define IMMEDIATE_SPEED 0
#define GRADUAL_FAST_SPEED 1
#define GRADUAL_SLOW_SPEED 2

#define TRAIN_FORWARD 0
#define TRAIN_REVERSE 1


extern struct cat_conf * train_P_cat;
extern int train_P_cat_len;
extern struct cat_conf * train_C_cat;
extern int train_C_cat_len;

// int read_rolling_Configs();
// void write_rolling_Configs();
// int load_rolling_Configs(const char * filename);
// void unload_rolling_Configs();

// void unlink_train(int fid);
#endif
