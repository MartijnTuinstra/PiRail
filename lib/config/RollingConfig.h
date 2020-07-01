#ifndef INCLUDE_CONFIG_ROLLINGCONFIG_H
#define INCLUDE_CONFIG_ROLLINGCONFIG_H

#include "config_data.h"
#include "rollingstock/declares.h"

class RollingConfig {
  public:
    char filename[100];
    bool parsed;

    struct s_train_header_conf header;

    struct cat_conf * P_Cat;
    struct cat_conf * C_Cat;

    struct engines_conf * Engines;
    struct cars_conf * Cars;
    struct trains_conf * Trains;

    RollingConfig(const char * filename);
    RollingConfig(char * filename);
    ~RollingConfig();

    void addTrain(Train * T);
    void addEngine(Engine * E);
    void addCar(Car * C);

    int read();
    void write();
    int calc_size();

    inline void print(){this->print(0,0);};
    void print(char ** cmds, uint8_t cmd_len);
};


void print_Cars(struct cars_conf car);
void print_Engines(struct engines_conf engine);
void print_Trains(struct trains_conf train);
void print_Catagories(struct train_config * config);

#endif