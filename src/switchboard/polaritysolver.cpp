#include "switchboard/polaritysolver.h"
#include "switchboard/msswitch.h"

#include "utils/logger.h"
#include "rollingstock/train.h"

namespace PolaritySolver {

// void init();
// void free();

int solve(Train * T, Path * near, Path * far){
  loggerf(INFO, "PolaritySolver::solve(id:%i, %x, %x)", T->id, near, far);

  if(far->polarityFlippable()){
    loggerf(INFO, "B/far side fix");
    far->flipPolarity();
    return true;
  }
  else if(near->polarityFlippable()){
    loggerf(INFO, "A/near side fix");
    near->flipPolarity();
    return true;
  }
  else{
    Path * paths[10] = {near, 0};
    uint8_t pathsFound = 1;
    uint8_t pathsFlippable = (near->polarity_type != BLOCK_FL_POLARITY_DISABLED);

    do{
      if(!near->Entrance->blocked)
        break;

      if(near->Entrance->train != T)
        break;

      Block * block = 0;
      if(near->prev->type == RAIL_LINK_R)
        block = near->prev->p.B;
      else if(near->prev->type == RAIL_LINK_S || near->prev->type == RAIL_LINK_s)
        block = near->prev->p.Sw->Detection;
      else if(near->prev->type >= RAIL_LINK_MA || near->prev->type == RAIL_LINK_MB_inside)
        block = near->prev->p.MSSw->Detection;

      Path * P = block->path;

      if(!P->Exit->blocked)
        break;

      if(P->Exit->train != T)
        break;

      if(P->polarity_type == BLOCK_FL_POLARITY_DISABLED)
        break;

      paths[pathsFound++] = P;
      pathsFlippable += (P->polarity_type != BLOCK_FL_POLARITY_DISABLED);

      near = P;
    }
    while(pathsFound < 10);
    loggerf(ERROR, "HELP %i/%i", pathsFound, pathsFlippable);

    if(pathsFlippable < pathsFound)
      return 0;

    loggerf(ERROR, "FLIP Some paths please");
  }

  return 0;
}

// struct instruction {
//   uint8_t type;
//   union {
//     void * p;
//     Switch * Sw;
//     MSSwitch * MSSw;
//   } p;

//   uint8_t nrOptions;
//   uint8_t * options;
//   uint16_t * length;
//   uint8_t * possible;

//   struct instruction ** next;
// };

// struct find {
//   bool possible;
//   bool allreadyCorrect;
//   bool polarityWrong;
// };

// struct switchFind {
//   struct find f[2];
// };

// struct msswitchFind {
//   struct find f[64];
// };

// extern struct switchFind * switches;
// extern struct msswitchFind * msswitches;

// struct SwSolve {
//   Train * train;
//   int16_t trainLength;
//   PathFinding::Route * route;
//   Block * prevBlock;
//   void * prevPtr;
//   RailLink * link;
//   int flags;

//   std::vector<struct instruction> * Instructions;
// };

// struct find findPath(struct SwSolve);

// int setPath(struct SwSolve);

// void setWrong(PathFinding::Route *, void *, RailLink, int);

// void dereservePath(Train * T, PathFinding::Route * r, void * p, RailLink link, int flags);
// int reservePath(struct SwSolve);

};