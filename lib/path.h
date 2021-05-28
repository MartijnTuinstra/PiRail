#ifndef _INCLUDE_PATH_H
#define _INCLUDE_PATH_H

#include <vector>
#include "switchboard/rail.h"
#include "rollingstock/declares.h"

class Path {
  public:
    std::vector<Block *> Blocks;

    bool direction = false;
    bool reserved  = false;

    Block * Entrance;
    struct rail_link * prev; // Link to block before Entrance

    Block * Exit;
    struct rail_link * next; // Link to block after Exit

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
    void find();

    void reserve(Train *);
    void reserve(Train *, Block *);
    void dereserve(Train *);
    void trainAtEnd(Train *);

    void reg(Train *);    // When the train enters the path
    void unreg(Train *);  // When the train leaves the path

    void reverse();
    void reverse(Train *);
    bool reversable();

    void sprint(char * string);
    void print();
};

extern std::vector<Path *> pathlist;
void pathlist_find();

#endif