#ifndef INCLUDE_TRAINS_H
#define INCLUDE_TRAINS_H

#include <signal.h>
#include <pthread.h>

#include "config/RollingStructure.h"

extern struct configStruct_Category * train_P_cat;
extern int train_P_cat_len;
extern struct configStruct_Category * train_C_cat;
extern int train_C_cat_len;

#endif
