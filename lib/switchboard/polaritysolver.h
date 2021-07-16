#ifndef _INCLUDE_SWICHBOARD_POLARITYSOLVER_H
#define _INCLUDE_SWICHBOARD_POLARITYSOLVER_H


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <vector>

#include "switch.h"
#include "switchboard/unit.h"
#include "switchboard/rail.h"
#include "path.h"

namespace PathFinding { class Route; };

namespace PolaritySolver {

// void init();
// void free();

int solve(Train *, Path *, Path *);

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

#endif