#ifndef _INCLUDE_SWICHBOARD_SWITCHSOLVER_H
#define _INCLUDE_SWICHBOARD_SWITCHSOLVER_H


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "switch.h"
#include "switchboard/unit.h"
#include "switchboard/rail.h"
#include "pathfinding.h"

namespace SwitchSolver {

int solve(RailTrain *, Block *, Block *, struct rail_link, int);

struct find {
  int possible;
  int allreadyCorrect;
};

struct find findPath(RailTrain *, PathFinding::Route *, void *, struct rail_link, int);

int setPath(RailTrain *, PathFinding::Route *, void *, struct rail_link, int);

void setWrong(PathFinding::Route *, void *, struct rail_link, int);

void dereservePath(RailTrain * T, PathFinding::Route * r, void * p, struct rail_link link, int flags);
int reservePath(RailTrain *, PathFinding::Route *, void *, struct rail_link, int);

};

#endif