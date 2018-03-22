#define _BSD_SOURCE
// #define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>


#include "./../lib/system.h"

#include "./../lib/rail.h"
#include "./../lib/switch.h"


#include "./../lib/trains.h"

#include "./../lib/websocket.h"
#include "./../lib/pathfinding.h"
#include "./../lib/modules.h"
#include "./../lib/com.h"


struct adr Switch_list[MAX_A] = {};
struct adr MS_Switch_list[MAX_A] = {};

int throw_switch(struct Swi * S){
	int linked = 0;
  for(int i = 0;i<MAX_SWITCH_LINK;i++){
    if(S->L_Swi[i]){
			linked = 1;
      struct SegC A = S->L_Swi[i]->Adr;

			if(A.type == 'S' || A.type == 's'){
	      if(Units[A.Module]->S[A.Adr]->Detection_Block && (Units[A.Module]->S[A.Adr]->Detection_Block->state == RESERVED || 
	      				Units[A.Module]->S[A.Adr]->Detection_Block->blocked)){
					printf("Linked switches blocked\n");
	        return 0;
	      }
			}else if(A.type == 'M' || A.type == 'm'){

			}
    }
  }
  if(S->Detection_Block && (S->Detection_Block->state != RESERVED && !S->Detection_Block->blocked) || !S->Detection_Block){
    S->state = 0x80 + !(S->state & 0x7F);

    char buf[40];
    buf[0] = WSopc_BroadSwitch;
		int index = 1;

		buf[index++] = S->Module;
		buf[index++] = S->id & 0x7F;
		buf[index++] = Units[S->Module]->S[S->id]->state & 0x7F;

    for(int i = 0;i<MAX_SWITCH_LINK;i++){
      if(S->L_Swi[i]){
        struct SegC A = S->L_Swi[i]->Adr;
        printf("Linked switching (%c%i:%i",A.type,A.Module,A.Adr);

        Units[A.Module]->S[A.Adr]->state = 0x80 + (S->L_Swi[i]->states[S->state&0x7F] & 0x7F);
        printf(" => %i)\n",Units[A.Module]->S[A.Adr]->state);

				buf[index++] = A.Module;
				buf[index++] = A.Adr & 0x7F;
				buf[index++] = Units[A.Module]->S[A.Adr]->state & 0x7F;
      }
    }
    printf("Throw Switch %s\n\n",buf);
	COM_change_switch(S->Module);
    send_all(buf,index,2); //Websocket
    return 1;
  }else{
		printf("Switch blocked\n");
    return 0;
  }
}

int set_switch(struct Swi * S,char state){
	int linked = 0;
	for(int i = 0;i<MAX_SWITCH_LINK;i++){
		if(S->L_Swi[i]){
			linked = 1;
			struct SegC A = S->L_Swi[i]->Adr;

			if(A.type == 'S' || A.type == 's'){
				if(Units[A.Module]->S[A.Adr]->Detection_Block && (Units[A.Module]->S[A.Adr]->Detection_Block->state == RESERVED || 
							Units[A.Module]->S[A.Adr]->Detection_Block->blocked)){
						printf("Linked switches blocked\n");
				return 0;
			}
		}else if(A.type == 'M' || A.type == 'm'){

		}
    }
  }
  if(S->Detection_Block && (S->Detection_Block->state != RESERVED && !S->Detection_Block->blocked) || !S->Detection_Block){
    S->state = state + 0x80;

    char buf[40];
    buf[0] = WSopc_BroadTrack;
		int index = 1;

		buf[index++] = S->Module;
		buf[index++] = S->id;
		buf[index++] = Units[S->Module]->S[S->id]->state;

    for(int i = 0;i<MAX_SWITCH_LINK;i++){
      if(S->L_Swi[i]){
        struct SegC A = S->L_Swi[i]->Adr;
        printf("Linked switching (%c%i:%i",A.type,A.Module,A.Adr);

        Units[A.Module]->S[A.Adr]->state = S->L_Swi[i]->states[S->state];
        printf(" => %i)\n",Units[A.Module]->S[A.Adr]->state);

				buf[index++] = A.Module;
				buf[index++] = A.Adr;
				buf[index++] = Units[A.Module]->S[A.Adr]->state;
      }
    }
    printf("Throw Switch %s\n\n",buf);
	COM_change_switch(S->Module);
    send_all(buf,index,2);
    return 1;
  }else{
		printf("Switch blocked\n");
    return 0;
  }
}

int throw_ms_switch(struct Mod * M, char c){ //Multi state object
  /*if((blocks[M->Adr.M][M->Adr.B][0] != NULL && blocks[M->Adr.M][M->Adr.B][0]->state != RESERVED) ||
        (blocks[M->Adr.M][M->Adr.B][1] != NULL && blocks[M->Adr.M][M->Adr.B][1]->state != RESERVED)){
    M->state = c;
    char buf[30];
    sprintf(buf,"{\"Mod\" : [[%i,%i,%i,%i,%i]]}",M->Adr.M,M->Adr.B,M->Adr.S,M->state,M->length);
    printf("Throw ms Switch %s\n\n",buf);
    send_all(buf,strlen(buf),2);
    return 1;
  }else{*/
    return 0;
}


void Create_Switch(struct SegC Adr,struct SegC App,struct SegC Div,struct SegC Str,int * adr,char state){
	struct Swi *Z = (struct Swi*)malloc(sizeof(struct Swi));

	Adr.type = 'S';

	Z->id = Adr.Adr;
	Z->Module = Adr.Module;
	Z->StrC = Str;
	Z->DivC = Div;
	Z->AppC = App;
	Z->state = state + 0x80;
	Z->default_state = state;
	Z->len = 1;

	//Check and copy addresses
	for(char i = 0;i<2;i++){
		if(Units[Adr.Module]->OutRegisters*8 < adr[i]){
			printf("Expansion needed\t");
			printf("Address %i doesn't fit\n",adr[i]);

			//Expand range
			Units[Adr.Module]->OutRegisters++;

			printf("Expanded to: %i bytes\n",Units[Adr.Module]->OutRegisters);

			//Realloc Input array, lenght: Inregisters * sizeof()
			Units[Adr.Module]->Out = (struct Rail_link **)realloc(Units[Adr.Module]->Out,8*Units[Adr.Module]->OutRegisters*sizeof(struct Rail_link *));

			//Clear new spaces
			for(int i = 8*Units[Adr.Module]->OutRegisters-8;i<8*Units[Adr.Module]->OutRegisters;i++){
				Units[Adr.Module]->Out[i] = 0;
			}
		}
		Z->Out[i] = adr[i];
	}

	//Check if ID is outside Unit array
	if(Z->id >= Units[Adr.Module]->S_L){
		//Expand size of B list
		printf("Expand S list of module %i to %i\n",Adr.Module,8*((Z->id + 8)/8));

		Units[Adr.Module]->S = (struct Swi **)realloc(Units[Adr.Module]->S,8*((Z->id + 8)/8)*sizeof(struct Swi *));

		//Clear new spaces
		for(int i = 8*((Z->id)/8);i<8*((Z->id + 8)/8);i++){
		  Units[Adr.Module]->S[i] = 0;
		}
		Units[Adr.Module]->S_L = 8*((Z->id + 8)/8);
	}

	for(int i = 0;i<MAX_SWITCH_LINK;i++){
		Z->L_Swi[i] = NULL;
	}

	for(int i = 0;i<MAX_SWITCH_PREFFERENCE;i++){
		Z->pref[i] = NULL;
	}
	//printf("A Switch  is created at %i:%i:%i\tAdr:%i\n",Adr.M,Adr.B,Adr.S,Adress);
	Units[Adr.Module]->S[Adr.Adr] = Z;

  if(!Units[Adr.Module]->S[Adr.Adr]){
    Units[Adr.Module]->S[Adr.Adr] = Z;
		Units[Adr.Module]->S_L++;
  }else{
    printf("WARNING - Double switch number %i in Module %i\n",Adr.Adr,Adr.Module);
  }
}

void Create_Moduls(int Unit_Adr, struct adr Adr,struct adr mAdr[10],struct adr MAdr[10],char length){
	struct Mod *Z = (struct Mod*)malloc(sizeof(struct Mod));

	Adr.type = 'M';

	Z->Adr = Adr;
	for(int i = 0;i<length;i++){
		printf("i:%i\n",i);
		Z->mAdr[i] = mAdr[i];
		Z->MAdr[i] = MAdr[i];
	}
	Z->length = length;
	Z->state = 0;
	Z->s_length = 1;

	/*if(Adr.S > 1){
		Z->s_length = Adr.S;
		for(int i = 1;i<Adr.S;i++){
			if(Units[Adr.M]->S[Adr.B] != NULL){
				Units[Adr.M]->S[Adr.B]->len = Adr.S;
			}else if(Moduls[Adr.M][Adr.B][i] != NULL){
				Moduls[Adr.M][Adr.B][i]->s_length = Adr.S;
			}
		}
	}*/
	printf("NOT SUPPORTED YET\n\n");

	//return Z;
	//if(blocks[Adr.M][Adr.B][1] == NULL && blocks[Adr.M][Adr.B][0] == NULL){
	//	printf("Needs 0 block\n");
		//C_Seg(C_Adr(Adr.M,Adr.B,0),0);
	//}
	//printf("A Moduls is created at %i:%i:%i\tAdr:%i\n",Adr.M,Adr.B,Adr.S,Adress);
	//Moduls[Adr.M][Adr.B][Adr.S] = Z;



	if(Units[Adr.M]->M[Unit_Adr] == NULL){
		Units[Adr.M]->M[Unit_Adr] = Z;
	}else{
		printf("Double Switch adress %i in Module %i\n",Unit_Adr,Adr.M);
	}

	//MS_Switch_list[M_list_i] = Adr;
	//M_list_i++;
}


int check_Switch_state(struct Rail_link NAdr){
	if(((NAdr.type == 's' || NAdr.type == 'S') && ((Switch *)NAdr.ptr)->Detection_Block && 
					((Switch *)NAdr.ptr)->Detection_Block->state != RESERVED) ||
		 ((NAdr.type == 'm' || NAdr.type == 'M') &&  ((mswitch *)NAdr.ptr)->Detection_Block && 
		 			((mswitch *)NAdr.ptr)->Detection_Block->state != RESERVED)){

		return 1;
	}else{
	 return 0;
	}
}

int check_Switch(struct Seg * B, int direct, _Bool incl_pref){
	struct Rail_link Adr,NAdr,SNAdr;
	struct Swi * S;

	int debug = 0;

	//if(B->Module == 4 && B->id == 18){
		//debug = 1;
	//}

	Adr.type = 'R';Adr.ptr = 0;

	if(!B){printf("Empty ChSw2");return 0;}
	int dir = B->dir;

	Adr.type = 'R';
	Adr.ptr = B;

	if(direct == 0){
		if(dir == 0 || dir == 2 || dir == 5){
			NAdr = B->Next;
		}else{
			NAdr = B->Prev;
		}
	}else{
		if(dir == 0 || dir == 2 || dir == 5){
			NAdr = B->Prev;
		}else{
			NAdr = B->Next;
		}
	}

	int n;
	R:{};
	if(debug){
	  printf("Switch P type:%c\t",NAdr.type);
	  if(NAdr.type == 'R'){
	    printf("R   %i:%i\n",((block *)NAdr.ptr)->Module,((block *)NAdr.ptr)->id);
	  }else if(NAdr.type == 'S' || NAdr.type == 's'){
	    printf("Sw  %i:%i\n",((Switch *)NAdr.ptr)->Module,((Switch *)NAdr.ptr)->id);
	  }else if(NAdr.type == 'M' || NAdr.type == 'm'){
	    printf("MSw %i:%i\n",((mswitch *)NAdr.ptr)->Module,((mswitch *)NAdr.ptr)->id);
	  }
	}
	//printf("NAdr %i:%i:%i:%c\n",NAdr.M,NAdr.B,NAdr.S,NAdr.type);
	if(NAdr.type == 0){
		return 0;
	}else if(NAdr.type == 'R'){
		//printf("Return 1, Just rail\n");
		return 1; //Passable
	}
	else if(NAdr.type == 'S'){
		S = NAdr.ptr;
		if(incl_pref == TRUE && S->pref[0] && S->pref[0]->type == 0 && S->pref[0]->state != (S->state & 0x7F)){
			if(debug)printf("Wrong state for the preference\n");
			return 0; //Wrong state for the preference setting
		}
		//printf("Switch approach\n");
		uint8_t SwState = S->state & 0x7F;
		if(SwState == 0 && S->str.type != 0){ //Straight?
			Adr = NAdr;
			NAdr = S->str;
		}else if(SwState == 1 && S->div.type != 0){
			Adr = NAdr;
			NAdr = S->div;
		}else{
			//printf("Unknown state (%x)\n",SwState);
			return 0;
		}

		goto R;
	}
	else if(NAdr.type == 'M'){
		int s = ((mswitch *)NAdr.ptr)->state;
		if(Link_cmp(((mswitch *)NAdr.ptr)->M_Adr[s],Adr)){
			NAdr = ((mswitch *)NAdr.ptr)->m_Adr[s];
			goto R;
		}else{
			return 0;
		}
	}
	else if(NAdr.type == 'm'){
		int s = ((mswitch *)NAdr.ptr)->state;
		if(Link_cmp(((mswitch *)NAdr.ptr)->m_Adr[s],Adr)){
			NAdr = ((mswitch *)NAdr.ptr)->M_Adr[s];
			goto R;
		}else{
			return 0;
		}
	}
	else if(NAdr.type == 's'){
		Switch * S = NAdr.ptr;
		struct Rail_link Div = S->div;
		struct Rail_link Str = S->str;
		//printf("Div %c %i==%c %i\n",Div.type,Div.Id,Div.B,Div.S,adr.M,adr.B,adr.S);
		//printf("Str %c %i==%c %i\n",Str.M,Str.B,Str.S,adr.M,adr.B,adr.S);
		if(Link_cmp(Div,Adr)){
			if((S->state & 0x7F) == 1){
				if(debug)printf("Diverging\n");
				n = 1;
			}else{
				if(debug)printf("Wrong Diverging\n");
				return 0;
			}
		}else if(Link_cmp(Str,Adr)){
			if((S->state & 0x7F) == 0){
				if(debug)printf("Straight\n");
				n = 1;
			}else{
				if(debug)printf("Wrong Diverging\n");
				return 0;
			}
		}else{
			if(debug)printf("Failed Link_cmp\n");
			return 0;
		}

		//	printf("New switch\n");
		Adr = NAdr;
		NAdr = S->app;
		goto R;

	}
	//printf("Retrun %i\n",n);
	return n;
}

int free_Switch(struct Seg *B, int direct){
	struct Rail_link Adr,NAdr,SNAdr;

	int debug = 0;

	// if(B->Module == 4 && B->id == 17){
	// 	debug = 1;
	// }

	Adr.type = 'R';Adr.ptr = 0;
	int return_Value = 1;
	int dir = B->dir;

	if(direct == 0){
		if(dir == 0 || dir == 2 || dir == 5){
			NAdr = B->Next;
		}else{
			NAdr = B->Prev;
		}
	}else{
		if(dir == 0 || dir == 2 || dir == 5){
			NAdr = B->Prev;
		}else{
			NAdr = B->Next;
		}
	}
	//printf("NAdr %i:%i:%i\n",NAdr.M,NAdr.B,NAdr.S);
	R:{};
	if(debug){
	  printf("Switch P type:%c\t",NAdr.type);
	  if(NAdr.type == 'R'){
	    printf("R   %i:%i\n",((block *)NAdr.ptr)->Module,((block *)NAdr.ptr)->id);
	  }else if(NAdr.type == 'S' || NAdr.type == 's'){
	    printf("Sw  %i:%i\n",((Switch *)NAdr.ptr)->Module,((Switch *)NAdr.ptr)->id);
	  }else if(NAdr.type == 'M' || NAdr.type == 'm'){
	    printf("MSw %i:%i\n",((mswitch *)NAdr.ptr)->Module,((mswitch *)NAdr.ptr)->id);
	  }
	}
	//printf("NAdr: %i:%i:%i\t",NAdr.M,NAdr.B,NAdr.S);
	if(return_Value == 0){
		return 0;
	}

	if(NAdr.type == 'S'){
		Switch * S = NAdr.ptr;
		uint8_t SwState = S->state & 0x3F;
		if(S->pref[0] && S->pref[0]->type == 0 && SwState != S->pref[0]->state){
			throw_switch(S);
		}
		if(SwState == 0 && S->str.type != 0){ //Straight?
			Adr = NAdr;
			NAdr = S->str;
		}else if(SwState == 1 && S->div.type != 0){
			Adr = NAdr;
			NAdr = S->div;
		}else{
			throw_switch(S);
		}
		goto R;
	}else if(NAdr.type == 's'){
		Switch * S = NAdr.ptr;
		struct Rail_link Div = S->div;
		struct Rail_link Str = S->str;
		uint8_t SwState = S->state & 0x3F;
		if(Link_cmp(Div,Adr)){
			if(SwState == 0){
				return_Value = throw_switch(S);
			}
		}else if(Link_cmp(Str,Adr)){
			if(SwState == 1){
				return_Value = throw_switch(S);
			}
		}

		//	printf("New switch\n");
		Adr = NAdr;
		NAdr = S->app;
		goto R;

	}else if(NAdr.type == 'M'){
		mswitch * M = NAdr.ptr;
		int s = M->state;
		if(Link_cmp(M->M_Adr[s],Adr)){
			NAdr = M->m_Adr[s];
		}else{
			for(int i = 0;i<M->length;i++){
				if(Link_cmp(M->M_Adr[i],Adr)){
					return_Value = throw_ms_switch(M,i);
					break;
				}
			}
		}
		Adr = NAdr;
		NAdr = M->m_Adr[M->state];
		goto R;
	}else if(NAdr.type == 'm'){
		mswitch * M = NAdr.ptr;
		int s = M->state;
		if(Link_cmp(M->m_Adr[s],Adr)){
			NAdr = M->M_Adr[s];
		}else{
			for(int i = 0;i<M->length;i++){
				if(Link_cmp(M->m_Adr[i],Adr)){
					return_Value = throw_ms_switch(M,i);
					break;
				}
			}
		}
		Adr = NAdr;
		NAdr = M->M_Adr[M->state];
		goto R;
	}
	return 1;
}

int free_Route_Switch(struct Seg *B, int direct, struct train * T){
	struct Rail_link Adr,NAdr,SNAdr;

	int debug = 0;

	if(B->Module == 4 && B->id == 17){
		debug = 1;
	}

	Adr.type = 'R';Adr.ptr = 0;
	int return_Value = 1;
	int dir = B->dir;

	if(direct == 0){
		if(dir == 0 || dir == 2 || dir == 5){
			NAdr = B->Next;
		}else{
			NAdr = B->Prev;
		}
	}else{
		if(dir == 0 || dir == 2 || dir == 5){
			NAdr = B->Prev;
		}else{
			NAdr = B->Next;
		}
	}
	//printf("NAdr %i:%i:%i\n",NAdr.M,NAdr.B,NAdr.S);
	R:{};
	//printf("NAdr: %i:%i:%i\t",NAdr.M,NAdr.B,NAdr.S);
	if(return_Value == 0){
		return 0;
	}

	if(NAdr.type == 'S'){
		Switch * S = (Switch *)NAdr.ptr;
		Adr = NAdr;
		for(int x = 0;x<T->Sw_len;x++){
			if(T->Route[x]->adr.ptr == S && T->Route[x]->states > 0){
				char r = (rand() % T->Route[x]->states);
				printf("Random selected nr %i (state %i)\n",r,T->Route[x]->suc[r]);
				char state = T->Route[x]->suc[r];

				if(!set_switch(S,state)) //If failing to set switches to state
					return 0;
				if(state == 0){
					printf("Straight\t");
					NAdr = S->str;
				}else if(state == 1){
					printf("Diverging\t");
					NAdr = S->div;
				}
			}
		}
		goto R;
	}
	else if(NAdr.type == 's'){
		struct Rail_link Div = ((Switch *)NAdr.ptr)->div;
		struct Rail_link Str = ((Switch *)NAdr.ptr)->str;
		uint8_t SwState = ((Switch *)NAdr.ptr)->state & 0x3F;
		if(Link_cmp(Div,Adr)){
			if(SwState == 0){
				return_Value = throw_switch((Switch *)NAdr.ptr);
			}
		}else if(Link_cmp(Str,Adr)){
			if(SwState == 1){
				return_Value = throw_switch((Switch *)NAdr.ptr);
			}
		}

		//	printf("New switch\n");
		Adr = NAdr;
		NAdr = ((Switch *)NAdr.ptr)->app;
		goto R;

	}
	else if(NAdr.type == 'M'){
		mswitch * M = (mswitch *)NAdr.ptr;
		Adr = NAdr;
		for(int x = 0;x<T->Sw_len;x++){
			if(T->Route[x]->adr.ptr == M && T->Route[x]->states > 0){
				char state = T->Route[x]->suc[0];

				//if(!set_switch(S,state)) //If failing to set switches to state
					//return 0;
				M->state = state;
				NAdr = M->m_Adr[state];
			}
		}
		goto R;
	}
	else if(NAdr.type == 'm'){
		mswitch * M = (mswitch *)NAdr.ptr;
		Adr = NAdr;
		for(int x = 0;x<T->Sw_len;x++){
			if(T->Route[x]->adr.ptr == M && T->Route[x]->states > 0){
				char state = T->Route[x]->suc[0];

				//if(!set_switch(S,state)) //If failing to set switches to state
					//return 0;
				M->state = state;
				NAdr = M->M_Adr[state];
			}
		}
		goto R;
	}
	return 1;
}
