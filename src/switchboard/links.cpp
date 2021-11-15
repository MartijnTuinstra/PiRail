#include "switchboard/links.h"

#include "switchboard/manager.h"
#include "switchboard/unit.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"

#include "config/LayoutStructure.h"

#include "utils/logger.h"
#include "path.h"


void railLinkExport(struct configStruct_RailLink * cfg, RailLink link){
  cfg->module = link.module;
  cfg->id = link.id;
  cfg->type = link.type;
}

RailLink::RailLink(){
  module = 0;
  id     = 0;
  type   = RAIL_LINK_R;
}

RailLink::RailLink(struct configStruct_RailLink link){
  module = link.module;
  id     = link.id;
  type   = (enum link_types)link.type;
}

Block * RailLink::getNextBlock(){
  RailLink * currentLink = this;

  while(currentLink->type != RAIL_LINK_R && currentLink->type != RAIL_LINK_E){
    if(currentLink->type == RAIL_LINK_S){
      Switch * Sw = currentLink->p.Sw;
      currentLink = (Sw->state) ? &Sw->div : &Sw->str;
    }
    else if(currentLink->type == RAIL_LINK_s){
      currentLink = &currentLink->p.Sw->app;
    }
    else if(currentLink->type == RAIL_LINK_MA || currentLink->type == RAIL_LINK_MB){
      return currentLink->p.MSSw->Detection;
    }
    else if(currentLink->type == RAIL_LINK_MA_inside){
      MSSwitch * MSSw = currentLink->p.MSSw;
      currentLink = &MSSw->sideA[MSSw->state];
    }
    else if(currentLink->type == RAIL_LINK_MA_inside){
      MSSwitch * MSSw = currentLink->p.MSSw;
      currentLink = &MSSw->sideB[MSSw->state];
    }
  }

  if(currentLink->type == RAIL_LINK_R)
    return currentLink->p.B;
  else
    return 0;
}

BlockLink::BlockLink(struct configStruct_RailLink link) : RailLink(link){};
// BlockLink::BlockLink(struct configStruct_RailLink link){
//   module = link.module;
//   id     = link.id;
//   type   = (enum link_types)link.type;
// }

void BlockLink::link(){
  p.p = rail_link_pointer(*(RailLink *)this);
}

void BlockLink::mapPolarity(void * ptr, uint64_t flags){
  BlockLink * BL;

  switch(type){
    case RAIL_LINK_MA_inside:
      BL = p.MSSw->sideA;
      break;
    case RAIL_LINK_MB_inside:
      BL = p.MSSw->sideB;
      break;
    default:
      mapPolarity(ptr, *this, flags);
      return;
  }

  // loggerf(ERROR, "BlockLink MSSw %i-%c, %x", p.MSSw->id, (type == RAIL_LINK_MA_inside) ? 'A' : 'B', (unsigned int)p.p);
  for(uint8_t i = 0; i < p.MSSw->state_len; i++){
    BL[i].mapPolarity(p.MSSw, BL[i], flags);

    // for(auto tBL: BL[i].Polarity){
    //   loggerf(ERROR, "                   %i: %2i:%2i %i", i, tBL.first->module, tBL.first->id, tBL.second);
    // }
  }
}

void BlockLink::mapPolarity(void * ptr, RailLink link, uint64_t flags){
  if(link.type == RAIL_LINK_R){
    RailLink * RL = (flags & FL_PREV_DIRECTION) ? &link.p.B->next : &link.p.B->prev;
    void * p = RL->p.p;
    // loggerf(INFO, "BlockLink %2i:%2i, %2i:%2i (%x==%x)=>%i", link.module, link.id, RL->module, RL->id, ptr, p, ptr == p);
    Polarity.insert({link.p.B, ptr == p});
  }
  else if(link.type == RAIL_LINK_S){
    Switch * Sw = link.p.Sw;
    mapPolarity((void *)Sw, Sw->str, flags);
    mapPolarity((void *)Sw, Sw->div, flags);
  }
  else if(link.type == RAIL_LINK_s){
    Switch * Sw = link.p.Sw;
    mapPolarity((void *)Sw, Sw->app, flags);
  }
}

void * rail_link_pointer(RailLink link){
  Unit * U = switchboard::Units(link.module);
  if(!U)
    return 0;
  //if(link.module == 0 || Units[link.module] == 0){
  //	  return 0;
  //}
  if(link.type == RAIL_LINK_R && U->B[link.id]){
    return U->B[link.id];
  }
  else if((link.type == RAIL_LINK_S || link.type == RAIL_LINK_s) && U->Sw[link.id]){
    return U->Sw[link.id];
  }
  else if(((link.type >= RAIL_LINK_MA && link.type <= RAIL_LINK_MB_inside) || link.type == RAIL_LINK_TT) && U->MSSw[link.id]){
    return U->MSSw[link.id];
  }
  return 0;
}
