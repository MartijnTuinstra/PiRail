#include "./signals.h"

#ifndef H_COM
  #include "./COM.h"
#endif

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

void create_signal2(struct Seg * B,char adr_nr, short addresses[adr_nr], char state[BLOCK_STATES], char flash[BLOCK_STATES], char side){
  /*Block*/
  /*Number of output pins/addresses*/
  /*State output relation*/
  /*Side*/

  struct signal *Z = (struct signal*)malloc(sizeof(struct signal));

  Z->state = 10;

  Z->id = Units[B->Module]->Signal_nr;
  long Unit_Adr = Units[B->Module]->Signal_nr++;

  Z->MAdr = B->Module;
  Z->type = 1;
  Z->length = adr_nr;

  for(char i = 0;i<adr_nr;i++){
    if(Units[Z->MAdr]->Out_length < addresses[i]){
      printf("Expansion needed\t");
      printf("Address %i doesn't fit\n",addresses[i]);

      char expand = ((addresses[i]-Units[Z->MAdr]->Out_length) + 8) / 8;

    	Units[Z->MAdr]->Out_length += (expand*8);

      printf("Expanded to: %i bytes\n\n",(Units[Z->MAdr]->Out_length/8));
    }
    Z->adr[i] = addresses[i];
  }
  for(char i = 0;i<BLOCK_STATES;i++){ Z->states[i] = state[i];    }
  for(char i = 0;i<BLOCK_STATES;i++){  Z->flash[i] = flash[i];    }

  printf("Signal2 #%i\n",Si_list_i);

  signals[Si_list_i] = Z;

  if(side == 0){
    B->NSi = Z;
  }else if(side == 1){
    B->PSi = Z;
  }

  if(Units[B->Module]->Signals[Unit_Adr] == NULL){
    Units[B->Module]->Signals[Unit_Adr] = Z;
    Z->UAdr = Unit_Adr;
    if(Unit_Adr > Units[B->Module]->Si_L){
      Units[B->Module]->Si_L = Unit_Adr;
    };
  }else{
    printf("Double signal adress %i in Module %i\n",Unit_Adr,B->Module);
  }


  Si_list_i++;
}

void set_signal(struct signal *Si,int state){  //0 = NSi, 1 = PSi
  /*
  if(state == GREEN){
    state = 1;
  }else if(state == AMBER){
    state = 2;
  }else if(state == RED){
    state = 4;
  }else{
    state = 0b111;
  }*/
  if(Si->state != 10){
    if(Si->type == 0){
      if(state == GREEN_S){
        state = 1;
      }else{
        state = 0;
      }
    }else if(Si->type == 1){
      if(state == GREEN_S){
        state = 1;
      }else if(state == AMBER_S){
        state = 2;
      }else{
        state = 0;
      }
    }

    if(Si->state != state){
      printf("Module %i Signal #%i change to %i\n",Si->MAdr,Si->id,state);
      Si->state = state;

      Units[Si->MAdr]->Sig_change = TRUE;
      //if(startup == 1){
        //COM_change_signal(Si);
      //}
    }
  }else{
    Si->state = state;
    Units[Si->MAdr]->Sig_change = TRUE;
  }
}
