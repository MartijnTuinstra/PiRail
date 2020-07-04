#ifndef _INCLUDE_PATH_H
#define _INCLUDE_PATH_H

#include <vector>
#include "switchboard/rail.h"

class Path {
  public:
    std::vector<Block *> Blocks;

    bool direction;
    bool reserved = 0;

    struct rail_link * next;
    struct rail_link * prev;

    Block * front;
    bool front_direction;

    Block * end;
    bool end_direction;

    Path(Block * B);
    ~Path();

    void add(Block * B, bool side);
    void join(Path * P);
    void find();

    void reserve();
    void reverse();

    void sprint(char * string);
    void print();
};

extern std::vector<Path *> pathlist;
void pathlist_find();

#endif