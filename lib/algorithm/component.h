#ifndef _INCLUDE_ALGORITHM_COMPONENT_H
#define _INCLUDE_ALGORITHM_COMPONENT_H

namespace Algorithm {
void * Run(void * args);

typedef void (*initfuncs)(void);

extern initfuncs InitFuncs[5];
extern initfuncs InitSimFuncs[5];

extern uint8_t InitFuncsLength;
extern uint8_t InitSimFuncsLength;

extern int InitStates[5];
extern int InitSimStates[5];

int InitFindModules(void);
int InitConnectModules(void);
int InitProcess(void);

void BlockTick(void);
void TrainTick(void);

};

#endif