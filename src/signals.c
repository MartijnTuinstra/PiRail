#define _BSD_SOURCE
// #define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "./../lib/system.h"

#include "./../lib/signals.h"

#include "./../lib/rail.h"

#include "./../lib/modules.h"
#include "./../lib/com.h"

/*
void create_signal(int Unit_Adr,struct Seg * B,int type, int side){ //Side = 0 => NSi, 1 => PSi
  struct signal *Z = (struct signal*)malloc(sizeof(struct signal));

  Z->state = 0;
  Z->id = Unit_Adr;
  Z->MAdr = B->Adr.M;
  Z->type = type;

  printf("Signal #%i\n",Si_list_i);

  signals[Si_list_i] = Z;

  if(side == 0){
    B->NSi = Z;
  }else if(side == 1){
    B->PSi = Z;
  }

  if(Units[B->Adr.M]->Signals[Unit_Adr] == NULL){
    Units[B->Adr.M]->Signals[Unit_Adr] = Z;
    Z->UAdr = Unit_Adr;
    if(Unit_Adr > Units[B->Adr.M]->Si_L){
      Units[B->Adr.M]->Si_L = Unit_Adr;
    };
  }else{
    printf("Double signal adress %i in Module %i\n",Unit_Adr,B->Adr.M);
  }


  Si_list_i++;
}
*/

void create_signal2(struct Seg * B,char adr_nr, uint8_t addresses[adr_nr], char state[BLOCK_STATES], char flash[BLOCK_STATES], char side){
  /*Block*/
  /*Number of output pins/addresses*/
  /*State output relation*/
  /*Side*/

  struct signal *Z = (struct signal*)malloc(sizeof(struct signal));

  Z->state = 0;

  Z->id = Units[B->Module]->Signal_nr;
  long Unit_Adr = Units[B->Module]->Signal_nr++;

  Z->MAdr = B->Module;
  Z->type = 1;
  Z->length = adr_nr;

  for(char i = 0;i<adr_nr;i++){
    if(Units[Z->MAdr]->OutRegisters*8 < addresses[i]){
      printf("Expansion needed\t");
      printf("Address %i doesn't fit\n",addresses[i]);

      //Expand range
      Units[Z->MAdr]->OutRegisters++;

      printf("Expanded to: %i bytes\n",Units[Z->MAdr]->OutRegisters);

      //Realloc Input array, lenght: Inregisters * sizeof()
      Units[Z->MAdr]->Out = (struct Rail_link **)realloc(Units[Z->MAdr]->Out,8*Units[Z->MAdr]->OutRegisters*sizeof(struct Rail_link *));

      //Clear new spaces
      for(int i = 8*Units[Z->MAdr]->OutRegisters-8;i<8*Units[Z->MAdr]->OutRegisters;i++){
        Units[Z->MAdr]->Out[i] = 0;
      }
    }
    Z->adr[i] = addresses[i];
  }

  memcpy(Z->states,state,BLOCK_STATES);
  memcpy(Z->flash,flash,BLOCK_STATES);

  if(side == 0){
    B->NSi = Z;
  }else if(side == 1){
    B->PSi = Z;
  }

  if(!Units[B->Module]->Signals[Unit_Adr]){
    Units[B->Module]->Signals[Unit_Adr] = Z;
    Z->UAdr = Unit_Adr;
    if(Unit_Adr > (Units[B->Module]->Si_L-1)){
      Units[B->Module]->Si_L = Unit_Adr + 1;
    }
  }else{
    printf("Double signal adress %i in Module %i\n",Unit_Adr,B->Module);
  }
}

void set_signal(struct signal *Si,int state){
  if(!(Si->state == state || Si->state == (0x80 + state))){
    /*printf("Module %i Signal #%i change from %x to ",Si->MAdr,Si->id,Si->state);

    if(state == SIG_RED){
      printf("SIG_RED");
    }else if(state == SIG_AMBER){
      printf("SIG_AMBER");
    }else if(state == SIG_GREEN){
      printf("SIG_GREEN");
    }

    printf("  (%x)\n",state);*/
    Si->state = 0x80 + state;
  }
}
