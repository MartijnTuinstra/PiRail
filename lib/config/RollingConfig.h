#ifndef INCLUDE_CONFIG_ROLLINGCONFIG_H
#define INCLUDE_CONFIG_ROLLINGCONFIG_H

#include <stdint.h>

#include "rollingstock/declares.h"

struct configStruct_TrainHeader;
struct configStruct_Engine;
struct configStruct_Car;
struct configStruct_Train;
struct configStruct_Category;

class RollingConfig {
  public:
    char filename[100];
    bool parsed;

    struct configStruct_RollingStockHeader * header;

    struct configStruct_Category * P_Cat;
    struct configStruct_Category * C_Cat;

    struct configStruct_Engine * Engines;
    struct configStruct_Car * Cars;
    struct configStruct_TrainSet * TrainSets;

    char * buffer;
    uint32_t buffer_len;

    RollingConfig(const char * filename);
    RollingConfig(char * filename);
    ~RollingConfig();

    void addTrainSet(TrainSet * T);
    void addEngine(Engine * E);
    void addCar(Car * C);

    int read();
    void dump();
    void write();
    int calc_size();

    inline void print(){this->print(0,0);};
    void print(char ** cmds, uint8_t cmd_len);
};


void print_Cars(struct configStruct_Car car);
void print_Engines(struct configStruct_Engine engine);
void print_TrainSets(struct configStruct_TrainSet train);
void print_Catagories(struct configStruct_Category * config);

#endif