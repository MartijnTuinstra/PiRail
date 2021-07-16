#ifndef _INCLUDE_SWITCHBOARD_LINK_H 
#define _INCLUDE_SWITCHBOARD_LINK_H

#include <stdint.h>
#include <map>

#include "switchboard/declares.h"

enum link_types {
  RAIL_LINK_R,
  RAIL_LINK_S,
  RAIL_LINK_s,
  RAIL_LINK_MA,
  RAIL_LINK_MB,
  RAIL_LINK_MA_inside,
  RAIL_LINK_MB_inside,
  RAIL_LINK_TT = 0x10, // Turntable
  RAIL_LINK_C  = 0xfe,
  RAIL_LINK_E  = 0xff
};

class RailLink {
  public:
    uint8_t  module;
    uint16_t id;
    enum link_types type;

    union {
        Block * B;
        Switch * Sw;
        MSSwitch * MSSw;
        void * p;
    } p;

    RailLink();
    RailLink(struct configStruct_RailLink link);
};

class Block;
class BlockLink : public RailLink {
  public:
    std::map<Block *, uint8_t> Polarity;
    // std::vector<Block * [10]> Polarity;

    BlockLink(struct configStruct_RailLink link);

    void link();
    void mapPolarity(void *, uint64_t);
    void mapPolarity(void *, RailLink, uint64_t);
};

struct configStruct_RailLink;

void railLinkExport(struct configStruct_RailLink * cfg, RailLink link);

void * rail_link_pointer(RailLink link);
void link_all_blocks(Unit * U);
void link_all_switches(Unit * U);
void link_all_msswitches(Unit * U);

#endif