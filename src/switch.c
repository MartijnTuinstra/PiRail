struct L_Swi_t{
	struct adr Adr;
	int states[5];
};

struct P_Swi_t{
	char type;
	int state;
};

struct Swi{
	struct adr Adr;

	struct adr Div;
	struct adr Str;
	struct adr App;
	char state;
	char len;

	struct L_Swi_t * L_Swi[MAX_SWITCH_LINK]; //Linked switches

	struct P_Swi_t * pref[MAX_SWITCH_PREFFERENCE];//Switch preference
};

struct Swi *Switch[MAX_Modules][MAX_Blocks][MAX_SW] = {};
struct Mod *Moduls[MAX_Modules][MAX_Blocks][MAX_SW/4] = {};

int throw_switch(struct Swi * S){
  for(int i = 0;i<MAX_SWITCH_LINK;i++){
    if(S->L_Swi[i] != NULL){
      struct adr A = S->L_Swi[i]->Adr;

      if((blocks[A.M][A.B][0] != NULL && blocks[A.M][A.B][0]->state != RESERVED) ||
            (blocks[A.M][A.B][1] != NULL && blocks[A.M][A.B][1]->state != RESERVED)){
      }else{
        return 0;
      }
    }
  }
  if((blocks[S->Adr.M][S->Adr.B][0] != NULL && blocks[S->Adr.M][S->Adr.B][0]->state != RESERVED) ||
        (blocks[S->Adr.M][S->Adr.B][1] != NULL && blocks[S->Adr.M][S->Adr.B][1]->state != RESERVED)){
    S->state = !S->state;

    char buf[40];
    sprintf(buf,"{\"SW\" : [[%i,%i,%i,%i]",S->Adr.M,S->Adr.B,S->Adr.S,S->state);

    for(int i = 0;i<MAX_SWITCH_LINK;i++){
      if(S->L_Swi[i] != NULL){
        struct adr A = S->L_Swi[i]->Adr;
        printf("Linked switching (%i:%i:%i",A.M,A.B,A.S);

        Switch[A.M][A.B][A.S]->state = S->L_Swi[i]->states[S->state];
        printf(" => %i)\n",Switch[A.M][A.B][A.S]->state);

        sprintf(buf,"%s,[%i,%i,%i,%i]",buf,A.M,A.B,A.S,Switch[A.M][A.B][A.S]->state);
      }
    }
    sprintf(buf,"%s]}",buf);
    printf("Throw Switch %s\n\n",buf);
    send_all(buf);
    return 1;
  }else{
    return 0;
  }
}

int throw_ms_switch(struct Mod * M, char c){ //Multi state object
  if((blocks[M->Adr.M][M->Adr.B][0] != NULL && blocks[M->Adr.M][M->Adr.B][0]->state != RESERVED) ||
        (blocks[M->Adr.M][M->Adr.B][1] != NULL && blocks[M->Adr.M][M->Adr.B][1]->state != RESERVED)){
    M->state = c;
    char buf[30];
    sprintf(buf,"{\"Mod\" : [[%i,%i,%i,%i,%i]]}",M->Adr.M,M->Adr.B,M->Adr.S,M->state,M->length);
    printf("Throw ms Switch %s\n\n",buf);
    send_all(buf);
    return 1;
  }else{
    return 0;
  }
}

void Create_Switch(struct adr Adr,struct adr  App,struct adr Div,struct adr Str,char state){
	struct Swi *Z = (struct Swi*)malloc(sizeof(struct Swi));

	Adr.type = 'S';

	Z->Adr = Adr;
	Z->Str = Str;
	Z->Div = Div;
	Z->App = App;
	Z->state = state;
	Z->len = 1;

	if(Adr.S > 1){
		Z->len = Adr.S;
		for(int i = 1;i<Adr.S;i++){
			if(Switch[Adr.M][Adr.B][i] != NULL){
				Switch[Adr.M][Adr.B][i]->len = Adr.S;
			}else if(Moduls[Adr.M][Adr.B][i] != NULL){
				Moduls[Adr.M][Adr.B][i]->s_length = Adr.S;
			}
		}
	}
	//return Z;
	if(blocks[Adr.M][Adr.B][1] == NULL && blocks[Adr.M][Adr.B][0] == NULL){
		C_Seg(C_Adr(Adr.M,Adr.B,0),0);
	}
	printf("A Switch  is created at %i:%i:%i\tAdr:%i\n",Adr.M,Adr.B,Adr.S,Adress);
	Switch[Adr.M][Adr.B][Adr.S] = Z;

	Adresses[Adress] = Adr;
	Adress++;
}

void Create_Moduls(struct adr Adr,struct adr mAdr[10],struct adr MAdr[10],char length){
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

	if(Adr.S > 1){
		Z->s_length = Adr.S;
		for(int i = 1;i<Adr.S;i++){
			if(Switch[Adr.M][Adr.B][i] != NULL){
				Switch[Adr.M][Adr.B][i]->len = Adr.S;
			}else if(Moduls[Adr.M][Adr.B][i] != NULL){
				Moduls[Adr.M][Adr.B][i]->s_length = Adr.S;
			}
		}
	}

	//return Z;
	if(blocks[Adr.M][Adr.B][1] == NULL && blocks[Adr.M][Adr.B][0] == NULL){
		C_Seg(C_Adr(Adr.M,Adr.B,0),0);
	}
	printf("A Moduls is created at %i:%i:%i\tAdr:%i\n",Adr.M,Adr.B,Adr.S,Adress);
	Moduls[Adr.M][Adr.B][Adr.S] = Z;

	Adresses[Adress] = Adr;
	Adress++;
}
/*
void Switch_Link(struct Seg * S,int M,int B,int S,){

}*/
