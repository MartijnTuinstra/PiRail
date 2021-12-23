#ifndef _INCLUDE_SWICHBOARD_POLARITYGROUP_H
#define _INCLUDE_SWICHBOARD_POLARITYGROUP_H

#include "switchboard/rail.h"

#include "flags.h"
#include "config/LayoutStructure.h"

class PolarityGroup;

extern std::vector<PolarityGroup *> PolarityGroupList;


namespace PathFinding { class Route; };

namespace PolaritySolver {
int solve(Train *, std::vector<PolarityGroup *>, PolarityGroup *);
}

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

    // Test if the group is flippable
    // Returns:
    //  0 - Success
    //  1 - ends[0] is blocked
    //  2 - ends[1] is blocked
    //  3 - Group has a DISABLED polarity type
    uint8_t flippable();
    
    // Test if the group is flippable
    // Returns:
    //  0 - Success
    //  1 - ends[0] is blocked
    //  2 - ends[1] is blocked
    //  3 - Group has a DISABLED polarity type
    uint8_t flippable(PolarityGroup * PG);
    bool flippableTest(PolarityGroup * PG);
};


#endif