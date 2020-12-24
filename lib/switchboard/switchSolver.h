#ifndef _INCLUDE_SWICHBOARD_SWITCHSOLVER_H
#define _INCLUDE_SWICHBOARD_SWITCHSOLVER_H


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "config/ModuleConfig.h"
#include "switchboard/switch.h"
#include "switchboard/rail.h"

#include "IO.h"
#include "pathfinding.h"

#include "rollingstock/declares.h"

#define U_Sw(U, A) Units(U)->Sw[A]

#define STRAIGHT_SWITCH 0
#define DIVERGING_SWITCH 1


namespace SwitchSolver {

int solve(RailTrain *, Block *, Block *, struct rail_link, int);

struct find {
  int possible;
  int allreadyCorrect;
};

struct find findPath(RailTrain *, PathFinding::Route *, void *, struct rail_link, int);
int setPath(RailTrain *, PathFinding::Route *, void *, struct rail_link, int);

int setWrong(PathFinding::Route *, void *, struct rail_link, int);

void dereservePath(RailTrain * T, PathFinding::Route * r, void * p, struct rail_link link, int flags);
int reservePath(RailTrain *, PathFinding::Route *, void *, struct rail_link, int);

};

#endif