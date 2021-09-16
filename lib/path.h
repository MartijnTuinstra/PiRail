#ifndef _INCLUDE_PATH_H
#define _INCLUDE_PATH_H

#include <vector>
#include "switchboard/rail.h"
#include "rollingstock/declares.h"
#include "flags.h"

class Path {
  public:
    std::vector<Block *> Blocks;

    bool direction = false;
    bool reserved  = false;

    bool    polarity      = POLARITY_NORMAL;
    uint8_t polarity_type = BLOCK_FL_POLARITY_DISABLED;
    uint16_t maxLength    = 0; // Maximum train length allowed through this path
    uint16_t length       = 0; // Lenght of this path

    Block * Entrance;
    RailLink * prev; // Link to block before Entrance

    Block * Exit;
    RailLink * next; // Link to block after Exit

    Block * front;
    bool front_direction;

    Block * end;
    bool end_direction;

    std::vector<Train *> trains;
    std::vector<Train *> reservedTrains;

    Path(Block * B);
    ~Path();

    void updateEntranceExit();
    void add(Block * B, bool side);
    void join(Path * P);
    void join(Path * P, Block ** side, Block * Pside, bool * sideDirection, bool PsideDirection, RailLink ** link, RailLink * Plink);
    void find();
    void find(RailLink ** link, Block ** side, uint8_t SearchDir);

    void setMaxLength();

    void reserve(Train *);
    void reserve(Train *, Block *);
    void dereserve(Train *);
    void trainAtEnd(Train *);

    void trainEnter(Train *); // When the train enters the path
    void trainExit(Train *);  // When the train leaves the path
    void analyzeTrains();     // Find all trains inside path

    void reverse();
    void reverse(Train *);
    bool reversable();

    inline void flipPolarity(){ flipPolarity(0); };
    void flipPolarity(bool _reverse);
    bool polarityFlippable();

    void sprint(uint8_t detail, char * string);
    void print();
};

extern std::vector<Path *> pathlist;
void pathlist_find();

#endif