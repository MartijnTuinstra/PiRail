#ifndef _INCLUDE_SWICHBOARD_POLARITYGROUP_H
#define _INCLUDE_SWICHBOARD_POLARITYGROUP_H

#include "switchboard/rail.h"

#include "flags.h"
#include "config/LayoutStructure.h"

class PolarityGroup;

extern std::vector<PolarityGroup *> PolarityGroupList;

class PolarityGroup {
  public:
    uint8_t type;
    bool status = POLARITY_NORMAL;
    bool train;

    std::vector<Block *> blocks;

    Block * ends[2];
    uint8_t direction[2];

    PolarityGroup(uint8_t, struct configStruct_PolarityGroup *);
    ~PolarityGroup();

    void updateDetection();
    void map();

    bool flip();
    bool flip(PolarityGroup *);

    uint8_t flippable();
    uint8_t flippable(PolarityGroup * PG);
};


#endif