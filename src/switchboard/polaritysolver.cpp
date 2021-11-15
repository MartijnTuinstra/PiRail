#include "switchboard/polaritysolver.h"
#include "switchboard/msswitch.h"

#include "utils/logger.h"
#include "rollingstock/train.h"

#include "switchboard/polarityGroup.h"

namespace PolaritySolver {

// void init();
// void free();

int solve(Train * T, PolarityGroup * near, PolarityGroup * far){
  loggerf(INFO, "PolaritySolver::solve(id:%i, %x, %x)", T->id, near, far);

  if(far->flippable()){
    loggerf(INFO, "B/far side fix");
    far->flip();
    return true;
  }
  else if(near->flippable()){
    loggerf(INFO, "A/near side fix");
    near->flip();
    return true;
  }
  /* FIXME
  else{
    Path * paths[10] = {near, 0};
    uint8_t pathsFound = 1;
    uint8_t pathsFlippable = (near->polarity_type != BLOCK_FL_POLARITY_DISABLED);

    do{
      if(!near->Entrance->blocked)
        break;

      if(near->Entrance->train != T)
        break;

      Block * block = near->getBlockAtEdge(near->prev);
      Path * P = block->path;
      loggerf(INFO, "test path %i", P->Blocks[0]->id);

      if(!P->Exit->blocked)
        break;

      if(P->Exit->train != T)
        break;

      if(P->polarity_type == BLOCK_FL_POLARITY_DISABLED)
        break;

      paths[pathsFound++] = P;
      pathsFlippable += (P->polarity_type != BLOCK_FL_POLARITY_DISABLED);
      loggerf(INFO, "add path %i", P->Blocks[0]->id);

      near = P;
    }
    while(pathsFound < 10);
    loggerf(ERROR, "HELP %i/%i", pathsFound, pathsFlippable);

    if(pathsFlippable < pathsFound)
      return 0;

    if(pathsFlippable == pathsFound){
      printf("Need to flip: ");
      for(uint8_t i = 0; i < pathsFound; i++){
        for(Block * b: paths[i]->Blocks){
          printf("%02i:%02i ", b->module, b->id);
        }
        paths[i]->flipPolarityNow();
      }
      printf("\n");
    }
  }
  */

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