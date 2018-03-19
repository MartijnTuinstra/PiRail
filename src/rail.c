#define _BSD_SOURCE
// #define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "./../lib/system.h"

#include "./../lib/rail.h"


#include "./../lib/modules.h"
#include "./../lib/switch.h"

//struct Seg * blocks2[MAX_Modules][MAX_Blocks*MAX_Segments] = {};

struct Station * stations[MAX_Modules*MAX_Blocks] = {};
int St_list_i = 0;

struct SegC EMPTY_BL(){
  struct SegC A;
  A.Module = 0;
  A.Adr = 0;
  A.type = 0;
  return A;
}

struct SegC CAdr(int M,int B,char type){
  struct SegC A;
  A.Module = M;
  A.Adr = B;
  A.type = type;
  return A;
}

int dir_Comp(struct Seg *A,struct Seg *B){
  if((A->dir == 2 && (B->dir == 1 || B->dir == 0)) || ((A->dir == 1 || A->dir == 0) && B->dir == 2)){
    return 1;
  }else if(A->dir == B->dir){
    return 1;
  }else if(((A->dir == 0 || A->dir == 2) && B->dir == 0b101) || (A->dir == 1 && B->dir == 0b100)){
    return 1;
  }else if(((B->dir == 0 || B->dir == 2) && A->dir == 0b101) || (B->dir == 1 && A->dir == 0b100)){
    return 1;
  }{
    return 0;
  }
    /*if(B->Adr.S == 0){
      return 1;
    }else{
      return 0;
    }
  }else{
    return 1;
  }*/
}

int Adr_Comp2(struct SegC A,struct SegC B){
  if(A.Module == B.Module && A.Adr == B.Adr && A.type == B.type){
    return 1;
  }else{
    return 0;
  }
}

void Create_Segment(int IO_Adr,struct SegC Adr ,struct SegC Next, struct SegC Prev,char max_speed,char state,char dir,char len){
	struct Seg *Z = (struct Seg*)malloc(sizeof(struct Seg));


	//Z->id = Units[Adr.Module]->B_nr++;
  Z->id = Adr.Adr;
  Z->Module = Adr.Module;
	Z->type = Adr.type;
	Z->NextC = Next;
	Z->PrevC = Prev;
	Z->max_speed = max_speed;
	Z->state = state;
	Z->dir = dir; //Clock wise or counter clock wise / Far side or closest side
	Z->length = len;
	Z->change = 1;
  Z->blocked = 0;
	Z->train = 0x00;
	Z->oneWay = FALSE;
  Z->NSi = 0;
  Z->PSi = 0;

  Z->Next.type = 0;Z->Next.ptr = 0;
  Z->Prev.type = 0;Z->Prev.ptr = 0;
	//printf("A Segment is created at %i:%i:%i\tAdr:%i\n",Adr.M,Adr.B,Adr.S,B_list_i);

  //Check if input is in range
	if(Units[Adr.Module]->InRegisters*8 <= IO_Adr){
    //Expand range
		Units[Adr.Module]->InRegisters++;

    printf("Expand to %i shift register, size(%i)\n",Units[Adr.Module]->InRegisters,8*Units[Adr.Module]->InRegisters);

    //Realloc Input array, lenght: Inregisters * sizeof()
    Units[Adr.Module]->In = (struct Rail_link **)realloc(Units[Adr.Module]->In,8*Units[Adr.Module]->InRegisters*sizeof(struct Rail_link *));

    //Clear new spaces
    for(int i = 8*Units[Adr.Module]->InRegisters-8;i<8*Units[Adr.Module]->InRegisters;i++){
      Units[Adr.Module]->In[i] = 0;
    }
	}

  //Check if ID is outside Unit array
  if(Z->id >= Units[Adr.Module]->B_L){
    //Expand size of B list
    printf("Expand B list of module %i to %i\n",Adr.Module,8*((Z->id + 8)/8));

    Units[Adr.Module]->B = (struct Seg **)realloc(Units[Adr.Module]->B,8*((Z->id + 8)/8)*sizeof(struct Seg *));

    //Clear new spaces
    for(int i = 8*((Z->id)/8);i<8*((Z->id + 8)/8);i++){
      Units[Adr.Module]->B[i] = 0;
    }
    Units[Adr.Module]->B_L = 8*((Z->id + 8)/8);
  }

	if(Units[Adr.Module]->B[Z->id] == NULL){
    printf("Module %i segment %i\n",Adr.Module,Z->id);
		Units[Adr.Module]->B[Z->id] = Z;
    //Units[Adr.Module]->B_nr++;
    if(Z->id > Units[Adr.Module]->B_nr){
      Units[Adr.Module]->B_nr = Z->id;
    }
	}else{
		printf("Double Block Number %i in Module %i\n",Z->id,Adr.Module);
	}

  struct Rail_link Y;
  Y.type = 'R';
  Y.ptr = (void *)Z;

  if(Units[Adr.Module]->In[IO_Adr] == 0){
    struct Rail_link * Y = (struct Rail_link*)calloc(1,sizeof(struct Rail_link));
    Y->type = 'R';Y->ptr = Z;
    Units[Adr.Module]->In[IO_Adr] = Y;
  }else{
		printf("Double Block IO Address %i in Module %i\n",IO_Adr,Adr.Module);
	}
}

void Create_Segment2(int IO_Adr,char M,int ID,char Type,struct SegC Next, struct SegC Prev,char max_speed,char state,char dir,char len){
  Create_Segment(IO_Adr,CAdr(M,ID,Type),Next,Prev,max_speed,state,dir,len)  ;
}

int Block_cmp(struct Seg * A,struct Seg * B){
  if(A && A == B){
  	return 1;
  }else{
    return 0;
  }
}

int Link_cmp(struct Rail_link A, struct Rail_link B){
  //printf("Link_cmp\t%c:%x:%x:%x:%x==%c:%x:%x:%x:%x\n",A.type,A.B,A.Sw,A.M,A.Si,B.type,B.B,B.Sw,B.M,B.Si);
  if(A.ptr == B.ptr){
    if(A.type == B.type || (A.type == 'S' && B.type == 's') || (A.type == 's' && B.type == 'S')){
      return 1;
    }else{return 0;}
  }else{
    return 0;
  }
}

void Connect_Segments(){
  for(int i = 0;i<MAX_Modules;i++){
    if(Units[i]){
      //printf("Unit %i ...\n",i);
      for(int j = 0;j<Units[i]->B_L;j++){
        if(Units[i]->B[j]){
          //printf(" %iB %c%i\n",Units[i]->B[j]->Module,Units[i]->B[j]->type,Units[i]->B[j]->id);

          if(Units[i]->B[j]->type != 'T'){
            //printf("  N %c%i:%i\t",Units[i]->B[j]->NextC.type,Units[i]->B[j]->NextC.Module,Units[i]->B[j]->NextC.Adr);

            if(!Adr_Comp2(Units[i]->B[j]->NextC,EMPTY_BL())){
              int nM = Units[i]->B[j]->NextC.Module;
              int nA = Units[i]->B[j]->NextC.Adr;
              char nT = Units[i]->B[j]->NextC.type;
              Units[i]->B[j]->Next.type = nT;
              if(nT == 'R'){
                Units[i]->B[j]->Next.ptr = Units[nM]->B[nA];
              }else if(nT == 'S' || nT == 's'){
                Units[i]->B[j]->Next.ptr = Units[nM]->S[nA];
              }else if(nT == 'M' || nT == 'm'){
                Units[i]->B[j]->Next.ptr = Units[nM]->M[nA];
              }
            }else{
              Units[i]->B[j]->Next.type = 0;
            }

            //printf("P %c%i:%i\n",Units[i]->B[j]->PrevC.type,Units[i]->B[j]->PrevC.Module,Units[i]->B[j]->PrevC.Adr);

            if(!Adr_Comp2(Units[i]->B[j]->PrevC,EMPTY_BL())){
              int pM = Units[i]->B[j]->PrevC.Module;
              int pA = Units[i]->B[j]->PrevC.Adr;
              char pT = Units[i]->B[j]->PrevC.type;
              Units[i]->B[j]->Prev.type = pT;
              if(pT == 'R'){
                Units[i]->B[j]->Prev.ptr = Units[pM]->B[pA];
              }else if(pT == 'S' || pT == 's'){
                Units[i]->B[j]->Prev.ptr = Units[pM]->S[pA];
              }else if(pT == 'M' || pT == 'm'){
                Units[i]->B[j]->Prev.ptr = Units[pM]->M[pA];
              }
            }else{
              Units[i]->B[j]->Next.type = 0;
            }
          }
        }
      }
      for(int j = 0;j<Units[i]->S_L;j++){
        if(Units[i]->S[j]){
      //    printf(" Sw %i\n",j);
      //    printf("  A %c%i:%i\t",Units[i]->S[j]->AppC.type,Units[i]->S[j]->AppC.Module,Units[i]->S[j]->AppC.Adr);

          int aM = Units[i]->S[j]->AppC.Module;
          int aA = Units[i]->S[j]->AppC.Adr;
          char aT = Units[i]->S[j]->AppC.type;
          Units[i]->S[j]->app.type = aT;
          if(aT == 'R'){
            Units[i]->S[j]->app.ptr = Units[aM]->B[aA];
          }else if(aT == 'S' || aT == 's'){
            Units[i]->S[j]->app.ptr = Units[aM]->S[aA];
          }else if(aT == 'M' || aT == 'm'){
            Units[i]->S[j]->app.ptr = Units[aM]->M[aA];
          }

      //    printf("D %c%i:%i\t",Units[i]->S[j]->DivC.type,Units[i]->S[j]->DivC.Module,Units[i]->S[j]->DivC.Adr);
          int dM = Units[i]->S[j]->DivC.Module;
          int dA = Units[i]->S[j]->DivC.Adr;
          char dT = Units[i]->S[j]->DivC.type;
          Units[i]->S[j]->div.type = dT;
          if(dT == 'R'){
            Units[i]->S[j]->div.ptr = Units[dM]->B[dA];
          }else if(dT == 'S' || dT == 's'){
            Units[i]->S[j]->div.ptr = Units[dM]->S[dA];
          }else if(dT == 'M' || dT == 'm'){
            Units[i]->S[j]->div.ptr = Units[dM]->M[dA];
          }

      //    printf("S %c%i:%i\n",Units[i]->S[j]->StrC.type,Units[i]->S[j]->StrC.Module,Units[i]->S[j]->StrC.Adr);
          int sM = Units[i]->S[j]->StrC.Module;
          int sA = Units[i]->S[j]->StrC.Adr;
          char sT = Units[i]->S[j]->StrC.type;
          Units[i]->S[j]->str.type = sT;
          if(sT == 'R'){
            Units[i]->S[j]->str.ptr = Units[sM]->B[sA];
          }else if(sT == 'S' || sT == 's'){
            Units[i]->S[j]->str.ptr = Units[sM]->S[sA];
          }else if(sT == 'M' || sT == 'm'){
            Units[i]->S[j]->str.ptr = Units[sM]->M[sA];
          }
        }
      }

    }
  }
}
/*
int Create_Station(struct adr Adr){
	struct Station * Z = (struct Station*)malloc(sizeof(struct Station));

	Z->Adr = Adr;

	stations[St_list_i++] = Z;
	printf("Station for %i:%i\n",Adr.M,Adr.B);

	return St_list_i - 1;
}
*/
int Create_Station(char Module,char * Name,char type,char nr,int Blocks[]){
	struct Station * Z = (struct Station*)malloc(sizeof(struct Station));

	//Z->Adr = Adr;
  Z->Module = Module;
  Z->id = Units[Module]->Station_nr;
  Z->UnID = St_list_i; //Universal ID
  Z->type = type;
  Z->Switches = 0;
  strcpy(Z->Name,Name);

  for(int i = 0;i<nr;i++){
    Z->Blocks[i] = Units[Module]->B[Blocks[i]];
    Units[Module]->B[Blocks[i]]->Station = Z;
  }

  Units[Module]->St[Units[Module]->Station_nr++] = Z;

	stations[St_list_i++] = Z;
	printf("Station for %i ('%s')\n",Module,Name);

	return St_list_i - 1;
}

void Station_Add_Switch(struct Station * St,struct Swi * Sw,char state){
  struct Switch_state_link * Z = (struct Switch_state_link *)malloc(sizeof(struct Switch_state_link));
  Z->type = 'S';
  Z->state = state;
  Z->Sw = Sw;

  if(St->Switches == 0){
    St->Sw = (struct Switch_state_link **)malloc(sizeof(struct Switch_state_link *));
    St->Sw[0] = Z;
    St->Switches++;
  }else{
    St->Sw = (struct Switch_state_link **)realloc(St->Sw,++St->Switches * sizeof(struct Switch_state_link *));
    St->Sw[St->Switches-1] = Z;
  }
}

void Station_Add_MSwitch(struct Station * St,struct Mod * MSw, char state){
  struct Switch_state_link * Z = (struct Switch_state_link *)malloc(sizeof(struct Switch_state_link));
  Z->type = 'M';
  Z->state = state;
  Z->MSw = MSw;

  if(St->Switches == 0){
    St->Sw = (struct Switch_state_link **)malloc(sizeof(struct Switch_state_link *));
    St->Sw[0] = Z;
    St->Switches++;
  }else{
    St->Sw = (struct Switch_state_link **)realloc(St->Sw,++St->Switches * sizeof(struct Switch_state_link *));
    St->Sw[St->Switches-1] = Z;
  }
}


struct Seg * Next2(struct Seg * B,int i){
  if(!B){printf("Empty Next2");return 0;}
  //  else{printf("Begin at %i:%i\t%i ticks\n",B->Module,B->id,i);}
	struct Rail_link NAdr,SNAdr;
  struct Swi * S;struct Mod * M;
	int Search_len = i;
	int a = 0;
	//printf("\n%i\n",i);
	//int direct = blocks[Adr.M][Adr.B][Adr.S]->dir;

	char prev_dir = B->dir;
  //printf("Prev_dir:\t%i\t",prev_dir);

	I:{};
  /*
	if(B && (!B->Next.B && !B->Next.Sw && !B->Next.M)){
		goto J;
	}
  */
  if(!B){
    //printf("Empty Next2");
    return 0;
  }
	char dir = B->dir;

	if(prev_dir == 0 && dir == 1 || prev_dir == 129 && dir == 0){
		prev_dir ^= 0x80;	//Reverse
		NAdr = B->Next;
	}
	else if(prev_dir == 1 && dir == 0 || prev_dir == 128 && dir == 1){
		prev_dir ^= 0x80;	//Reverse
		NAdr = B->Prev;
	}
	else if((prev_dir >> 7) == 1){
		if(dir == 0 || dir == 2 || dir == 5){
			NAdr = B->Prev;
		}else{
			NAdr = B->Next;
		}
	}
	else if(dir == 0 || dir == 2 || dir == 5){
		NAdr = B->Next;
	}
	else{
		NAdr = B->Prev;
	}

	prev_dir = (prev_dir & 0xC0) + (dir & 0xF);

	J:{};

	if(i <= 0){
		//printf("\n");
		//printf("Done: R  %c%i:%i\n",B->type,B->Module,B->id);
		return B;
	}

  /*
  printf("NAdr type:%c\t",NAdr.type);
  if(NAdr.type == 'R'){
    printf("%iR   %i:%i\n",i,NAdr.B->Module,NAdr.B->id);
  }else if(NAdr.type == 'S' || NAdr.type == 's'){
    printf("%iSw  %i:%i\n",i,NAdr.Sw->Module,NAdr.Sw->id);
  }else if(NAdr.type == 'M' || NAdr.type == 'm'){
    printf("%iMSw %i:%i\n",i,NAdr.M->Module,NAdr.M->id);
  }*/

	//printf("\ni:%i",i);
	//printf("\t%i:%i:%i\ttype:%c\n",NAdr.M,NAdr.B,NAdr.S,NAdr.type);
	if(NAdr.type == 'R'){
		i--;
		B = (block *)NAdr.ptr;
		goto I;
	}else if(NAdr.type == 'S' || NAdr.type == 's' || NAdr.type == 'm' || NAdr.type == 'M'){
		R:{};
    /*
    printf("R:%c\t",NAdr.type);
    if(NAdr.type == 'S' || NAdr.type == 's'){
      printf("%iSw  %i:%i\n",i,NAdr.Sw->Module,NAdr.Sw->id);
    }else if(NAdr.type == 'M' || NAdr.type == 'm'){
      printf("%iMSw %i:%i\n",i,NAdr.M->Module,NAdr.M->id);
    }*/

		if((NAdr.type == 'S' || NAdr.type=='s') && ((Switch *)NAdr.ptr)->Detection_Block){
      if(((Switch *)NAdr.ptr)->Detection_Block->type == 'T'){
        if(i == 1){
          //printf("Switch has a Detection_Block\n");
    			i--;
    			B = ((Switch *)NAdr.ptr)->Detection_Block;
    			goto J;
        }else{
          a++;
        }
      }
		}
		if(NAdr.type == 'S'){
			if((((Switch *)NAdr.ptr)->state & 0x3F) == 0){ //Straight?
				SNAdr = ((Switch *)NAdr.ptr)->str;
			}else{
				SNAdr = ((Switch *)NAdr.ptr)->div;
			}
		}
		else if(NAdr.type == 's'){
			SNAdr = ((Switch *)NAdr.ptr)->app;
		}
		else if(NAdr.type == 'M'){
			//printf("m\n");
			int s = ((mswitch *)NAdr.ptr)->state;
			SNAdr = ((mswitch *)NAdr.ptr)->m_Adr[s];
			//printf("\tm:%i:%i:%i\ttype:%c\n",SNAdr.M,SNAdr.B,SNAdr.S,SNAdr.type);
		}
		else if(NAdr.type == 'm'){
			//printf("M\n");
			int s = ((mswitch *)NAdr.ptr)->state;
			SNAdr = ((mswitch *)NAdr.ptr)->M_Adr[s];
			//printf("\t%i:%i:%i\ttype:%c\n",Adr.M,Adr.B,Adr.S,Adr.type);
		}
		//printf("SN %i:%i:%i%c\tN %i:%i:%i\ti:%i\n",SNAdr.M,SNAdr.B,SNAdr.S,SNAdr.type,NAdr.M,NAdr.B,NAdr.S,i);
		if(SNAdr.type == 'S' || SNAdr.type == 's'){
			if(!Block_cmp(((Switch *)SNAdr.ptr)->Detection_Block,((Switch *)SNAdr.ptr)->Detection_Block)){
      //  printf("Second Detection_Block\n");
				i--;
			}
			NAdr = SNAdr;
			goto R;
    }else if(SNAdr.type == 'M' || SNAdr.type == 'm'){
			if(SNAdr.ptr != NAdr.ptr){
				i--;
			}
			NAdr = SNAdr;
			goto R;
		}else{
			B = (block *)SNAdr.ptr;
			//usleep(20000);
			//printf("\n%i:%i:%i type:%c\n",NAdr.M,NAdr.B,NAdr.S,NAdr.type);
      if(NAdr.type == 'S' || NAdr.type == 's'){
        if(Block_cmp((block *)SNAdr.ptr,((Switch *)NAdr.ptr)->Detection_Block)){
          i++;
        }
      }
			i--;
      if(a>0){
        i--;
      }
			goto I;
		}
	}
	return B;
}

struct Seg * Prev2(struct Seg * B,int i){
  if(!B){printf("Empty Next2");return 0;}
  //  else{printf("Begin at %i:%i\t%i ticks\n",B->Module,B->id,i);}
	struct Rail_link NAdr,SNAdr;
  struct Swi * S;struct Mod * M;
	int Search_len = i;
	int a = 0;
	//printf("\n%i\n",i);
	//int direct = blocks[Adr.M][Adr.B][Adr.S]->dir;

	char prev_dir = B->dir;
  //printf("Prev_dir:\t%i\t",prev_dir);

	I:{};
  /*
	if(B && (!B->Next.B && !B->Next.Sw && !B->Next.M)){
		goto J;
	}
  */
  if(!B){
    //printf("Empty Next2");
    return 0;
  }
	char dir = B->dir;

	if(prev_dir == 0 && dir == 1 || prev_dir == 129 && dir == 0){
		prev_dir ^= 0x80;	//Reverse
		NAdr = B->Prev;
	}
	else if(prev_dir == 1 && dir == 0 || prev_dir == 128 && dir == 1){
		prev_dir ^= 0x80;	//Reverse
		NAdr = B->Next;
	}
	else if((prev_dir >> 7) == 1){
		if(dir == 0 || dir == 2 || dir == 5){
			NAdr = B->Next;
		}else{
			NAdr = B->Prev;
		}
	}
	else if(dir == 0 || dir == 2 || dir == 5){
		NAdr = B->Prev;
	}
	else{
		NAdr = B->Next;
	}

	prev_dir = (prev_dir & 0xC0) + (dir & 0xF);

	J:{};

	if(i <= 0){
		//printf("\n");
		//printf("Done: R  %c%i:%i\n",B->type,B->Module,B->id);
		return B;
	}
  /*
  printf("NAdr type:%c\t",NAdr.type);
  if(NAdr.type == 'R'){
    printf("%iR   %i:%i\n",i,NAdr.B->Module,NAdr.B->id);
  }else if(NAdr.type == 'S' || NAdr.type == 's'){
    printf("%iSw  %i:%i\n",i,NAdr.Sw->Module,NAdr.Sw->id);
  }else if(NAdr.type == 'M' || NAdr.type == 'm'){
    printf("%iMSw %i:%i\n",i,NAdr.M->Module,NAdr.M->id);
  }*/

	//printf("\ni:%i",i);
	//printf("\t%i:%i:%i\ttype:%c\n",NAdr.M,NAdr.B,NAdr.S,NAdr.type);
  if(NAdr.type == 'R'){
		i--;
		B = (block *)NAdr.ptr;
		goto I;
	}else if(NAdr.type == 'S' || NAdr.type == 's' || NAdr.type == 'm' || NAdr.type == 'M'){
		R:{};
    /*
    printf("R:%c\t",NAdr.type);
    if(NAdr.type == 'S' || NAdr.type == 's'){
      printf("%iSw  %i:%i\n",i,NAdr.Sw->Module,NAdr.Sw->id);
    }else if(NAdr.type == 'M' || NAdr.type == 'm'){
      printf("%iMSw %i:%i\n",i,NAdr.M->Module,NAdr.M->id);
    }*/

		if((NAdr.type == 'S' || NAdr.type=='s') && ((Switch *)NAdr.ptr)->Detection_Block){
      if(((Switch *)NAdr.ptr)->Detection_Block->type == 'T'){
        if(i == 1){
          //printf("Switch has a Detection_Block\n");
    			i--;
    			B = ((Switch *)NAdr.ptr)->Detection_Block;
    			goto J;
        }else{
          a++;
        }
      }
		}
		if(NAdr.type == 'S'){
			if((((Switch *)NAdr.ptr)->state & 0x3F) == 0){ //Straight?
				SNAdr = ((Switch *)NAdr.ptr)->str;
			}else{
				SNAdr = ((Switch *)NAdr.ptr)->div;
			}
		}
		else if(NAdr.type == 's'){
			SNAdr = ((Switch *)NAdr.ptr)->app;
		}
		else if(NAdr.type == 'M'){
			//printf("m\n");
			int s = ((mswitch *)NAdr.ptr)->state;
			SNAdr = ((mswitch *)NAdr.ptr)->m_Adr[s];
			//printf("\tm:%i:%i:%i\ttype:%c\n",SNAdr.M,SNAdr.B,SNAdr.S,SNAdr.type);
		}
		else if(NAdr.type == 'm'){
			//printf("M\n");
			int s = ((mswitch *)NAdr.ptr)->state;
			SNAdr = ((mswitch *)NAdr.ptr)->M_Adr[s];
			//printf("\t%i:%i:%i\ttype:%c\n",Adr.M,Adr.B,Adr.S,Adr.type);
		}
		//printf("SN %i:%i:%i%c\tN %i:%i:%i\ti:%i\n",SNAdr.M,SNAdr.B,SNAdr.S,SNAdr.type,NAdr.M,NAdr.B,NAdr.S,i);
		if(SNAdr.type == 'S' || SNAdr.type == 's'){
			if(!Block_cmp(((Switch *)SNAdr.ptr)->Detection_Block,((Switch *)NAdr.ptr)->Detection_Block)){
      //  printf("Second Detection_Block\n");
				i--;
			}
			NAdr = SNAdr;
			goto R;
    }else if(SNAdr.type == 'M' || SNAdr.type == 'm'){
			if(SNAdr.ptr != NAdr.ptr){
				i--;
			}
			NAdr = SNAdr;
			goto R;
		}else{
			B = (block *)SNAdr.ptr;
			//usleep(20000);
			//printf("\n%i:%i:%i type:%c\n",NAdr.M,NAdr.B,NAdr.S,NAdr.type);
      if(NAdr.type == 'S' || NAdr.type == 's'){
        if(Block_cmp((block *)SNAdr.ptr,((Switch *)NAdr.ptr)->Detection_Block)){
          i++;
        }
      }
			i--;
      if(a>0){
        i--;
      }
			goto I;
		}
	}
	return B;
}

struct Rail_link NADR2(struct Seg * B){
  struct Rail_link NAdr;
  if(!B){printf("Empty NADR2");return NAdr;}
	int dir = B->dir;

	if(dir == 0 || dir == 2 || dir == 0b101){
		NAdr = B->Next;
	}else{
		NAdr = B->Prev;
	}
	return NAdr;
}

struct Rail_link PADR2(struct Seg * B){
  struct Rail_link NAdr;
  NAdr.type = 0;NAdr.ptr = 0;
  if(!B){printf("Empty PADR2");return NAdr;}
	int dir = B->dir;

	if(dir == 0 || dir == 2 || dir == 0b101){
		NAdr = B->Prev;
	}else{
		NAdr = B->Next;
	}
	return NAdr;
}

/*
struct adr NADR(struct adr Adr){
	struct adr NAdr;
	int dir = blocks[Adr.M][Adr.B][Adr.S]->dir;

	if(dir == 0 || dir == 2 || dir == 0b101){
		NAdr = blocks[Adr.M][Adr.B][Adr.S]->NAdr;
	}else{
		NAdr = blocks[Adr.M][Adr.B][Adr.S]->PAdr;
	}
	return NAdr;
}

struct adr PADR(struct adr Adr){
	struct adr PAdr;
	int dir = blocks[Adr.M][Adr.B][Adr.S]->dir;

	if(dir == 0 || dir == 2){
		PAdr = blocks[Adr.M][Adr.B][Adr.S]->PAdr;
	}else{
		PAdr = blocks[Adr.M][Adr.B][Adr.S]->NAdr;
	}
	return PAdr;
}

struct Seg * Next(struct adr Adr,int i){
	struct adr NAdr,SNAdr;
	int Search_len = i;
	int a = 0;
	//printf("\n%i\n",i);
	//int direct = blocks[Adr.M][Adr.B][Adr.S]->dir;

	char prev_dir = blocks[Adr.M][Adr.B][Adr.S]->dir;
	I:{};

	if(Adr.type == 'e'){
		goto J;
	}

	char dir = blocks[Adr.M][Adr.B][Adr.S]->dir;

	if(prev_dir == 0 && dir == 1){
		prev_dir ^= 0x80;	//Reverse
		NAdr = blocks[Adr.M][Adr.B][Adr.S]->NAdr;
	}
	else if(prev_dir == 1 && dir == 0){
		prev_dir ^= 0x80;	//Reverse
		NAdr = blocks[Adr.M][Adr.B][Adr.S]->PAdr;
	}
	else if(prev_dir == 129 && dir == 0){
		prev_dir ^= 0x80;	//Reverse
		NAdr = blocks[Adr.M][Adr.B][Adr.S]->NAdr;
	}
	else if(prev_dir == 128 && dir == 1){
		prev_dir ^= 0x80;	//Reverse
		NAdr = blocks[Adr.M][Adr.B][Adr.S]->PAdr;
	}
	else if((prev_dir >> 7) == 1){
		if(dir == 0 || dir == 2 || dir == 5){
			NAdr = blocks[Adr.M][Adr.B][Adr.S]->PAdr;
		}else{
			NAdr = blocks[Adr.M][Adr.B][Adr.S]->NAdr;
		}
	}
	else if(dir == 0 || dir == 2 || dir == 5){
		NAdr = blocks[Adr.M][Adr.B][Adr.S]->NAdr;
	}
	else{
		NAdr = blocks[Adr.M][Adr.B][Adr.S]->PAdr;
	}

	prev_dir = (prev_dir & 0xC0) + (dir & 0xF);

	J:{};

	if(i <= 0){
		//printf("\n");
		//printf("\nA:%i:%i:%i type:%c\n",Adr.M,Adr.B,Adr.S,Adr.type);
		return blocks[Adr.M][Adr.B][Adr.S];
	}

	//printf("\ni:%i",i);
	//printf("\t%i:%i:%i\ttype:%c\n",NAdr.M,NAdr.B,NAdr.S,NAdr.type);
	if(NAdr.type == 'R'){
		i--;
		Adr = NAdr;
		goto I;
	}else if(NAdr.type == 'e'){
		return blocks[0][0][0];

	}else if(NAdr.type == 'S' || NAdr.type == 's' || NAdr.type == 'm' || NAdr.type == 'M'){
		R:{};
		//printf("\ni:%i\tC:%i:%i:%i type:%c\n",i,NAdr.M,NAdr.B,NAdr.S,NAdr.type);
		if(i == 1 && blocks[NAdr.M][NAdr.B][0] != NULL){
			i--;
			Adr = C_Adr(NAdr.M,NAdr.B,0);
			goto J;
		}
		if(blocks[NAdr.M][NAdr.B][0] != NULL){
			a++;
		}
		if(NAdr.type == 'S'){
			if(Switch[NAdr.M][NAdr.B][NAdr.S]->state == 0){ //Straight?
				SNAdr = Switch[NAdr.M][NAdr.B][NAdr.S]->Str;
			}else{
				SNAdr = Switch[NAdr.M][NAdr.B][NAdr.S]->Div;
			}
		}
		else if(NAdr.type == 's'){
			SNAdr = Switch[NAdr.M][NAdr.B][NAdr.S]->App;
		}
		else if(NAdr.type == 'M'){
			//printf("m\n");
			int s = Moduls[NAdr.M][NAdr.B][NAdr.S]->state;
			SNAdr = Moduls[NAdr.M][NAdr.B][NAdr.S]->mAdr[s];
			//printf("\tm:%i:%i:%i\ttype:%c\n",SNAdr.M,SNAdr.B,SNAdr.S,SNAdr.type);
		}
		else if(NAdr.type == 'm'){
			//printf("M\n");
			int s = Moduls[NAdr.M][NAdr.B][NAdr.S]->state;
			SNAdr = Moduls[NAdr.M][NAdr.B][NAdr.S]->MAdr[s];
			//printf("\t%i:%i:%i\ttype:%c\n",Adr.M,Adr.B,Adr.S,Adr.type);
		}
		//printf("SN %i:%i:%i%c\tN %i:%i:%i\ti:%i\n",SNAdr.M,SNAdr.B,SNAdr.S,SNAdr.type,NAdr.M,NAdr.B,NAdr.S,i);
		if(SNAdr.type == 'S' || SNAdr.type == 's' || SNAdr.type == 'M' || SNAdr.type == 'm'){
			if(SNAdr.B != NAdr.B){
				i--;
			}
			NAdr = SNAdr;
			goto R;
		}else{
			Adr = SNAdr;
			NAdr = Adr;
			//usleep(20000);
			//printf("\n%i:%i:%i type:%c\n",NAdr.M,NAdr.B,NAdr.S,NAdr.type);
			i--;
			if(a>0){
				i--;
			}
			goto I;
		}
	}
	return blocks[Adr.M][Adr.B][Adr.S];
}

struct Seg * Prev(struct adr Adr,int i){
	struct adr PAdr,SPAdr;
	int a = 0;
	//printf("\n%i\n",i);
	int direct = blocks[Adr.M][Adr.B][Adr.S]->dir;
	I:{};
	int dir = blocks[Adr.M][Adr.B][Adr.S]->dir;

	if(direct == 0 || direct == 2 || direct == 5){
		if(dir == 0 || dir == 2 || dir == 5){
			PAdr = blocks[Adr.M][Adr.B][Adr.S]->PAdr;
		}else{
			PAdr = blocks[Adr.M][Adr.B][Adr.S]->PAdr;
		}
	}else{
		if(dir == 0 || dir == 2 || dir == 5){
			PAdr = blocks[Adr.M][Adr.B][Adr.S]->NAdr;
		}else{
			PAdr = blocks[Adr.M][Adr.B][Adr.S]->NAdr;
		}
	}

	J:{};

	if(i <= 0){
		//printf("\n");
		//printf("\nA:%i:%i:%i type:%c\n",Adr.M,Adr.B,Adr.S,Adr.type);
		return blocks[Adr.M][Adr.B][Adr.S];
	}

	//printf("i:%i",i);
	//printf("\t%i:%i:%i\ttype:%c\n",NAdr.M,NAdr.B,NAdr.S,NAdr.type);
	if(PAdr.type == 'R'){
		i--;
		Adr = PAdr;
		goto I;
	}else if(PAdr.type == 'e'){
		return blocks[0][0][0];

	}else if(PAdr.type == 'S' || PAdr.type == 's' || PAdr.type == 'm' || PAdr.type == 'M'){
		R:{};
		//printf("\ni:%i\tC:%i:%i:%i type:%c\n",i,PAdr.M,PAdr.B,PAdr.S,PAdr.type);
		if(i == 1 && blocks[PAdr.M][PAdr.B][0] != NULL){
			i--;
			Adr = C_Adr(PAdr.M,PAdr.B,0);
			goto J;
		}
		if(blocks[PAdr.M][PAdr.B][0] != NULL){
			a++;
		}
		if(PAdr.type == 'S'){
			if(Switch[PAdr.M][PAdr.B][PAdr.S]->state == 0){ //Straight?
				SPAdr = Switch[PAdr.M][PAdr.B][PAdr.S]->Str;
			}else{
				SPAdr = Switch[PAdr.M][PAdr.B][PAdr.S]->Div;
			}
		}
		else if(PAdr.type == 's'){
			SPAdr = Switch[PAdr.M][PAdr.B][PAdr.S]->App;
		}
		else if(PAdr.type == 'M'){
			//printf("m\n");
			int s = Moduls[PAdr.M][PAdr.B][PAdr.S]->state;
			SPAdr = Moduls[PAdr.M][PAdr.B][PAdr.S]->mAdr[s];
			//printf("\tm:%i:%i:%i\ttype:%c\n",SNAdr.M,SNAdr.B,SNAdr.S,SNAdr.type);
		}
		else if(PAdr.type == 'm'){
			//printf("M\n");
			int s = Moduls[PAdr.M][PAdr.B][PAdr.S]->state;
			SPAdr = Moduls[PAdr.M][PAdr.B][PAdr.S]->MAdr[s];
			//printf("\t%i:%i:%i\ttype:%c\n",Adr.M,Adr.B,Adr.S,Adr.type);
		}
		//printf("SN %i:%i:%i%c\tN %i:%i:%i\ti:%i\n",SNAdr.M,SNAdr.B,SNAdr.S,SNAdr.type,NAdr.M,NAdr.B,NAdr.S,i);
		if(SPAdr.type == 'S' || SPAdr.type == 's' || SPAdr.type == 'M' || SPAdr.type == 'm'){
			if(SPAdr.B != PAdr.B && blocks[Adr.M][Adr.B][0] != NULL){
				i--;
			}
			PAdr = SPAdr;
			goto R;
		}else{
			Adr = SPAdr;
			PAdr = Adr;
			//usleep(20000);
			//printf("\n%i:%i:%i type:%c\n",NAdr.M,NAdr.B,NAdr.S,NAdr.type);
			i--;
			if(a>0){
				i--;
			}
			goto I;
		}
	}
	return blocks[Adr.M][Adr.B][Adr.S];
}
*/
