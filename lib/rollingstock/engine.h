#ifndef _INCLUDE_ROLLINGSTOCK_ENGINE_H
#define _INCLUDE_ROLLINGSTOCK_ENGINE_H

#include <stdint.h>
#include "rollingstock/declares.h"
#include "rollingstock/functions.h"


#define ENGINE_14_FAHR_STUFEN 0
#define ENGINE_28_FAHR_STUFEN 1
#define ENGINE_128_FAHR_STUFEN 2


class Engine {
  public:
    uint16_t DCC_ID;
    uint16_t id;
    
    uint8_t type;
    uint8_t control;
    uint8_t dir;
    uint8_t halt;

    bool use;
    Train * train;
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

    Engine(char * name,int DCC,char * img, char * icon, char type, int length, int steps_len, struct engine_speed_steps * steps, uint8_t functions[28]);
    ~Engine();

    void setSpeed(uint16_t speed);
    void readSpeed();
};

extern Engine ** engines;
extern int engines_len;
extern Engine * DCC_train[9999];

#define create_engine_from_conf(e) new Engine(e.name, e.DCC_ID, e.img_path, e.icon_path, e.type, e.length, e.config_steps, e.speed_steps, e.functions)


// void engine_set_speed(Engine * E, uint16_t speed);
// void engine_read_speed(Engine * E);


#endif