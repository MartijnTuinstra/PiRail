#ifndef _INCLUDE_ROLLINGSTOCK_ENGINE_H
#define _INCLUDE_ROLLINGSTOCK_ENGINE_H

#include <stdint.h>
#include "config_data.h"

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

    Engine(struct engines_conf);
    Engine(uint16_t, char * name);
    ~Engine();

    void setName(char * name);
    void setImagePath(char *);
    void setIconPath(char *);

    void setSpeedSteps(uint8_t, struct engine_speed_steps *);

    void constructor(struct engines_conf);

    void setSpeed(uint16_t speed);
    void readSpeed();
};


#endif