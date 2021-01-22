#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

#include "system.h"

#include "train.h"
#include "config/RollingConfig.h"

#include "scheduler/scheduler.h"

#define MAX_TIMERS 10

#define ROUND(nr)  (int)(nr+0.5)

struct configStruct_Category * train_P_cat;
int train_P_cat_len = 0;
struct configStruct_Category * train_C_cat;
int train_C_cat_len = 0;

struct SchedulerEvent * railtraincontinue_event;
