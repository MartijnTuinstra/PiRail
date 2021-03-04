#ifndef _INCLUDE_ROLLINGSTOCK_ENGINE_H
#define _INCLUDE_ROLLINGSTOCK_ENGINE_H

#include <stdint.h>

#include "rollingstock/declares.h"
#include "rollingstock/functions.h"

#include "config/RollingStructure.h"


#define ENGINE_14_FAHR_STUFEN 0
#define ENGINE_28_FAHR_STUFEN 1
#define ENGINE_128_FAHR_STUFEN 2


struct EngineSpeedSteps {
  uint16_t speed;
  uint8_t step;
};

class Engine {
  public:
    uint16_t DCC_ID;
    uint16_t id;
    
    uint8_t type;
    uint8_t control;
    uint8_t halt;

    bool use;
    Train * train;
    RailTrain * RT;

    bool dirZ21; // Z21 Direction
    bool dir;    // Disired Direction for the Z21

    char Z21_get_speed;   // Z21 Speed (1-128/0-100%)
    char Z21_set_speed;   // Disired Speed for the Z21 (1-128/0-100%)
    char speed_step_type; // Z21 FAHR_STUFEN 14/28/128
    uint16_t max_speed;   // Real Speed (km/h)
    uint16_t cur_speed;   // Real Speed (km/h)

    uint8_t configSteps_len;
    struct EngineSpeedSteps * configSteps;
    uint8_t steps_len;
    struct EngineSpeedSteps steps[129];

    struct train_funcs function[29] = {{0, 0, 0}};

    uint16_t length;    //in mm   
    char * name;
    char * img_path;
    char * icon_path;

    Engine(struct configStruct_Engine);
    Engine(uint16_t, char * name);
    ~Engine();

    void setName(char * name);
    void setImagePath(char *);
    void setIconPath(char *);

    void setSpeedSteps(uint8_t, struct EngineSpeedSteps *);

    void constructor(struct configStruct_Engine);

  private:
    void convertRealSpeedtoZ21(uint16_t);
    void convertZ21toRealSpeed(uint16_t);

  public:
    void Z21_setFunctions(char * functions, uint8_t length);
    void Z21_setSpeedDir(char _speed, bool _dir);

    void setSpeed(uint16_t);
    void setSpeed(uint16_t, bool);
};


#endif