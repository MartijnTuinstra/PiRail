#include "switchboard/links.h"

#include "switchboard/manager.h"
#include "switchboard/unit.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"


void * rail_link_pointer(struct rail_link link){
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

void link_all_blocks(Unit * U){
  for(int i = 0; i < U->block_len; i++){
    if(!U->B[i])
      continue;

    Block * tB = U->B[i];

    tB->next.p.p = rail_link_pointer(tB->next);
    tB->prev.p.p = rail_link_pointer(tB->prev);

    if(tB->next.type == RAIL_LINK_R && tB->type == NOSTOP && tB->next.p.B->type != NOSTOP && !tB->next.p.B->path)
      new Path(tB->next.p.B);

    if(tB->prev.type == RAIL_LINK_R && tB->type == NOSTOP && tB->prev.p.B->type != NOSTOP && !tB->prev.p.B->path)
      new Path(tB->prev.p.B);

    if((tB->prev.type == RAIL_LINK_E || tB->next.type == RAIL_LINK_E) && !tB->path)
      new Path(tB);

  }
}

void link_all_switches(Unit * U){
  for(int i = 0; i < U->switch_len; i++){
    if(!U->Sw[i])
      continue;

    Switch * tSw = U->Sw[i];

    tSw->app.p.p = rail_link_pointer(tSw->app);
    tSw->str.p.p = rail_link_pointer(tSw->str);
    tSw->div.p.p = rail_link_pointer(tSw->div);

    if(tSw->app.type == RAIL_LINK_R && tSw->app.p.B->type != NOSTOP && !tSw->app.p.B->path)
      new Path(tSw->app.p.B);
    if(tSw->str.type == RAIL_LINK_R && tSw->str.p.B->type != NOSTOP && !tSw->str.p.B->path)
      new Path(tSw->str.p.B);
    if(tSw->div.type == RAIL_LINK_R && tSw->div.p.B->type != NOSTOP && !tSw->div.p.B->path)
      new Path(tSw->div.p.B);
  }
}

void link_all_msswitches(Unit * U){
  for(int i = 0; i < U->msswitch_len; i++){
    if(!U->MSSw[i])
      continue;

    MSSwitch * tMSSw = U->MSSw[i];

    for(int s = 0; s < tMSSw->state_len; s++){
      tMSSw->sideA[s].p.p = rail_link_pointer(tMSSw->sideA[s]);
      tMSSw->sideB[s].p.p = rail_link_pointer(tMSSw->sideB[s]);

      if(tMSSw->sideA[s].type == RAIL_LINK_R && tMSSw->sideA[s].p.B->type != NOSTOP && !tMSSw->sideA[s].p.B->path)
        new Path(tMSSw->sideA[s].p.B);
      if(tMSSw->sideB[s].type == RAIL_LINK_R && tMSSw->sideB[s].p.B->type != NOSTOP && !tMSSw->sideB[s].p.B->path)
        new Path(tMSSw->sideB[s].p.B);
    }
  }
}