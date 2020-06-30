
#include "switchboard/rail.h"

class Path {
  public:
    std::vector<Block *> Blocks;

    bool direction;
    bool reserved;

    struct rail_link * next;
    struct rail_link * prev;
}