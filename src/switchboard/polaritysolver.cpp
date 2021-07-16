#include "switchboard/polaritysolver.h"
#include "switchboard/msswitch.h"

#include "utils/logger.h"
#include "rollingstock/train.h"

namespace PolaritySolver {

// void init();
// void free();

int solve(Train * T, Path * A, Path * B){
  loggerf(INFO, "PolaritySolver::solve(id:%i, %x, %x)", T->id, A, B);

  if(B->polarityFlippable()){
    loggerf(INFO, "B side fix");
    B->flipPolarity();
  }
  else if(A->polarityFlippable()){
    loggerf(INFO, "A side fix");
    A->flipPolarity();
  }
  else{
    Path * paths[10] = {A, 0};
    uint8_t pathsFound = 1;

    do{
      if(!A->Entrance->blocked)
        break;

      if(A->Entrance->train != T)
        break;

      Block * block = 0;
      if(A->prev->type == RAIL_LINK_R)
        block = A->prev->p.B;
      else if(A->prev->type == RAIL_LINK_S || A->prev->type == RAIL_LINK_s)
        block = A->prev->p.Sw->Detection;
      else if(A->prev->type >= RAIL_LINK_MA || A->prev->type == RAIL_LINK_MB_inside)
        block = A->prev->p.MSSw->Detection;

      Path * P = block->path;

      if(!P->Exit->blocked)
        break;

      if(P->Exit->train != T)
        break;

      if(P->polarity_type == BLOCK_FL_POLARITY_DISABLED)
        break;

      paths[pathsFound++] = P;

      A = P;
    }
    while(pathsFound < 10);
    loggerf(ERROR, "HELP %i", pathsFound);
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