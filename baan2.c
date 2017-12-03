#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <wiringPi.h>

#define MAX_SW 16
#define MAX_Segments 8
#define MAX_Blocks 32
#define MAX_Modules 16
#define MAX_A MAX_Modules*MAX_Blocks*MAX_Segments
#define MAX_TRAINS 20
#define MAX_ROUTE 20

#define GREEN 0
#define AMBER 1
#define RED 2
#define BLOCKED 3
#define GHOST 4
#define RESERVED 5
#define UNKNOWN 6

#define Arr_Count(array) (sizeof(array)/sizeof((array)[0]))

int stop = 0;
int delayA = 1000000;
int delayB = 1000000;

pthread_mutex_t mutex_lockA;

struct adr{
	int M;		// Module
	int B;		// Block
	int S;		// Section
	int type;	// Type
};

struct Seg{
	struct adr Adr;
	struct adr NAdr;
	struct adr PAdr;
	char max_speed;		//0x00 <-> 0xFF
	char state;
	char dir;		//0xAABB, A = Current travel direction, B = Travel direction
	char length;
	char train;		 //0x00 <-> 0xFF
	char blocked;
};

struct Swi{
	struct adr Adr;

	struct adr Div;
	struct adr Str;
	struct adr App;
	char state;
	char len;
};

struct Mod{
	struct adr Adr;

	struct adr mAdr[10];
	struct adr MAdr[10];
	char length;
	char state;
};

struct link{
	struct adr Adr1;
	struct adr Adr2;
	struct adr Adr3;
	struct adr Adr4;
};

struct adr Adresses[MAX_A] = {};
struct adr StartAdr;
int Adress = 0;
struct Seg *blocks[MAX_Modules][MAX_Blocks][MAX_Segments] = {};
struct Swi *Switch[MAX_Modules][MAX_Blocks][MAX_SW] = {};
struct Mod *Moduls[MAX_Modules][MAX_Blocks][MAX_SW/4] = {};

struct adr C_Adr(char M,char B,char S){
	struct adr Z;

	Z.M = M;
	Z.B = B;
	Z.S = S;
	Z.type = 'R';

	return Z;
}

struct adr * c_Adr(char M,char B,char S){
	struct adr *Z = (struct adr*)malloc(sizeof(struct adr));

	Z->M = M;
	Z->B = B;
	Z->S = S;
	Z->type = 'R';

	return Z;
}

struct adr C_AdrT(char M,char B,char S,char T){
	struct adr Z;

	Z.M = M;
	Z.B = B;
	Z.S = S;
	Z.type = T;

	return Z;
}

struct adr * c_AdrT(char M,char B,char S,char T){
	struct adr *Z = (struct adr*)malloc(sizeof(struct adr));

	Z->M = M;
	Z->B = B;
	Z->S = S;
	Z->type = T;

	return Z;
}

#define END_BL C_AdrT(0,0,0,'e')

struct Seg * C_Seg(struct adr Adr, char state){
	struct Seg *Z = (struct Seg*)malloc(sizeof(struct Seg));

	Z->Adr = Adr;
	Z->NAdr = C_AdrT(0,0,0,'e');
	Z->PAdr = C_AdrT(0,0,0,'e');
	Z->max_speed = 0;
	Z->state = state;
	Z->train = 0x00;

	printf("A Segment is created at %i:%i:%i\t",Adr.M,Adr.B,Adr.S);
	blocks[Adr.M][Adr.B][Adr.S] = Z;

	if(!(Adr.M == 0 && Adr.B == 0 && Adr.S == 0)){
		printf("Adr:%i\n",Adress);
		Adresses[Adress] = Adr;
		Adress++;
	}else{
		printf("\n");
	}

	return Z;
}

void Create_Segment(struct adr Adr,struct adr NAdr,struct adr PAdr,char max_speed,char state,char dir,char len){
	struct Seg *Z = (struct Seg*)malloc(sizeof(struct Seg));

	Z->Adr = Adr;
	Z->NAdr = NAdr;
	Z->PAdr = PAdr;
	Z->max_speed = max_speed;
	Z->state = state;
	Z->dir = dir;
	Z->length = len;
	Z->train = 0x00;

	printf("A Segment is created at %i:%i:%i\tAdr:%i\n",Adr.M,Adr.B,Adr.S,Adress);
	blocks[Adr.M][Adr.B][Adr.S] = Z;

	//return Z;
	Adresses[Adress] = Adr;
	Adress++;
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
			Switch[Adr.M][Adr.B][i]->len = Adr.S;
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

void Create_Moduls(struct adr Adr,struct adr * NAdr[10],struct adr * PAdr[10],char length){
	struct Mod *Z = (struct Mod*)malloc(sizeof(struct Mod));

	Adr.type = 'M';

	Z->Adr = Adr;
	for(int i = 0;i<length;i++){
		printf("i:%i\n",i);
		Z->mAdr[i] = *NAdr[i];
		Z->MAdr[i] = *PAdr[i];
	}
	Z->length = length;
	Z->state = 0;

	//return Z;
	if(blocks[Adr.M][Adr.B][1] == NULL && blocks[Adr.M][Adr.B][0] == NULL){
		C_Seg(C_Adr(Adr.M,Adr.B,0),0);
	}
	printf("A Moduls is created at %i:%i:%i\tAdr:%i\n",Adr.M,Adr.B,Adr.S,Adress);
	Moduls[Adr.M][Adr.B][Adr.S] = Z;

	Adresses[Adress] = Adr;
	Adress++;
}

struct Seg * block(struct adr Adr){
	return blocks[Adr.M][Adr.B][Adr.S];
}

#include "modules2.c"
#include "trainlist.c"

int Adr_Comp(struct adr A,struct adr B){
	if(A.M == B.M && A.B == B.B && A.S == B.S){
		return 1;
	}else{
		return 0;
	}
}

struct Seg * Next(struct adr Adr,int i){
	struct adr NAdr,SNAdr;
	int a = 0;
	//printf("\n%i\n",i);
	I:{};

	int dir = blocks[Adr.M][Adr.B][Adr.S]->dir;

	if(dir == 0 || dir == 2){
		NAdr = blocks[Adr.M][Adr.B][Adr.S]->NAdr;
	}else{
		NAdr = blocks[Adr.M][Adr.B][Adr.S]->PAdr;
	}

	J:{};

	if(i == 0){
		//printf("\n");
		//printf("\nA:%i:%i:%i type:%c\n",Adr.M,Adr.B,Adr.S,Adr.type);
		return blocks[Adr.M][Adr.B][Adr.S];
	}
	//usleep(20000);
	//printf("i:%i",i);
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
		}else if(NAdr.type == 's'){
			SNAdr = Switch[NAdr.M][NAdr.B][NAdr.S]->App;
		}else if(NAdr.type == 'M'){
			//printf("m\n");
			int s = Moduls[NAdr.M][NAdr.B][NAdr.S]->state;
			SNAdr = Moduls[NAdr.M][NAdr.B][NAdr.S]->mAdr[s];
			//printf("\tm:%i:%i:%i\ttype:%c\n",SNAdr.M,SNAdr.B,SNAdr.S,SNAdr.type);
		}else if(NAdr.type == 'm'){
			//printf("M\n");
			int s = Moduls[NAdr.M][NAdr.B][NAdr.S]->state;
			SNAdr = Moduls[NAdr.M][NAdr.B][NAdr.S]->MAdr[s];
			//printf("\t%i:%i:%i\ttype:%c\n",Adr.M,Adr.B,Adr.S,Adr.type);
		}
		//printf("SN %i:%i:%i%c\tN %i:%i:%i\ti:%i\n",SNAdr.M,SNAdr.B,SNAdr.S,SNAdr.type,NAdr.M,NAdr.B,NAdr.S,i);
		if(SNAdr.type == 'S' || SNAdr.type == 's'){
			if(SNAdr.B != NAdr.B){
				i--;
			}
			NAdr = SNAdr;
			goto R;
		}else if(SNAdr.type == 'M' || SNAdr.type == 'm'){
			if(SNAdr.B != NAdr.B){
				i--;
			}
			NAdr = SNAdr;
			//usleep(20000);
			//printf("\n%i:%i:%i type:%c\n",NAdr.M,NAdr.B,NAdr.S,NAdr.type);
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
}

struct Seg * Prev(struct adr Adr,int i){
	struct adr PAdr,SPAdr;
	int a = 0;
	//printf("\n%i\n",i);
	I:{};

	int dir = blocks[Adr.M][Adr.B][Adr.S]->dir;

	if(dir == 0 || dir == 2){
		PAdr = blocks[Adr.M][Adr.B][Adr.S]->PAdr;
	}else{
		PAdr = blocks[Adr.M][Adr.B][Adr.S]->NAdr;
	}

	J:{};

	if(i == 0){
		//printf("\n");
		return blocks[Adr.M][Adr.B][Adr.S];
	}
	//usleep(20000);
	//printf("i:%i",i);
	//printf("\t%i:%i:%i\ttype:%c\n",PAdr.M,PAdr.B,PAdr.S,PAdr.type);
	if(PAdr.type == 'R'){
		i--;
		Adr = PAdr;
		goto I;
	}else if(PAdr.type == 'e'){
		return blocks[0][0][0];

	}else if(PAdr.type == 'S' || PAdr.type == 's'){
		R:{};
		if(i == 1 && blocks[PAdr.M][PAdr.B][0] != NULL){
			i--;
			Adr = C_Adr(PAdr.M,PAdr.B,0);
			goto I;
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
		}else{
			SPAdr = Switch[PAdr.M][PAdr.B][PAdr.S]->App;
		}
		//printf("SN %i:%i:%i%c\tN %i:%i:%i\ti:%i\n",SPAdr.M,SPAdr.B,SPAdr.S,SPAdr.type,PAdr.M,PAdr.B,PAdr.S,i);
		if(SPAdr.type == 'S' || SPAdr.type == 's'){
			if(SPAdr.B != PAdr.B){
				i--;
			}
			PAdr = SPAdr;
			goto R;
		}else{
			Adr = SPAdr;
			PAdr = Adr;
			//printf("\n%i:%i:%i\n",PAdr.M,PAdr.B,PAdr.S);
			i--;
			if(a>0){
				i--;
			}
			goto I;
		}
	}
}

struct adr NADR(struct adr Adr){
	struct adr NAdr;
	int dir = blocks[Adr.M][Adr.B][Adr.S]->dir;

	if(dir == 0 || dir == 2){
		NAdr = blocks[Adr.M][Adr.B][Adr.S]->NAdr;
	}else{
		NAdr = blocks[Adr.M][Adr.B][Adr.S]->PAdr;
	}
	return NAdr;
}

int check_Switch(struct adr adr, int direct){
	struct adr NAdr;
	int dir = blocks[adr.M][adr.B][adr.S]->dir;

	if(direct == 0){
		if(dir == 0 || dir == 2){
			NAdr = blocks[adr.M][adr.B][adr.S]->NAdr;
		}else{
			NAdr = blocks[adr.M][adr.B][adr.S]->PAdr;
		}
	}else{
		if(dir == 0 || dir == 2){
			NAdr = blocks[adr.M][adr.B][adr.S]->PAdr;
		}else{
			NAdr = blocks[adr.M][adr.B][adr.S]->NAdr;
		}
	}
	//printf("NAdr %i:%i:%i\n",NAdr.M,NAdr.B,NAdr.S);
	int n;
	R:{};
	//printf("NAdr: %i:%i:%i\n",NAdr.M,NAdr.B,NAdr.S);
	if(NAdr.type == 'R'){
		return 1; //Passable
	}else if(NAdr.type == 'e'){
		return 0; //Passable
	}else if(NAdr.type == 'S'){
		if(Switch[NAdr.M][NAdr.B][NAdr.S]->state == 0){ //Straight?
			adr = NAdr;
			NAdr = Switch[NAdr.M][NAdr.B][NAdr.S]->Str;
		}else{
			adr = NAdr;
			NAdr = Switch[NAdr.M][NAdr.B][NAdr.S]->Div;
		}
		goto R;
	}else{
		struct adr Div = Switch[NAdr.M][NAdr.B][NAdr.S]->Div;
		struct adr Str = Switch[NAdr.M][NAdr.B][NAdr.S]->Str;
		//printf("Div %i:%i:%i==%i:%i:%i\n",Div.M,Div.B,Div.S,adr.M,adr.B,adr.S);
		//printf("Str %i:%i:%i==%i:%i:%i\n",Str.M,Str.B,Str.S,adr.M,adr.B,adr.S);
		if(Div.M == adr.M && Div.B == adr.B && Div.S == adr.S){
			if(Switch[NAdr.M][NAdr.B][NAdr.S]->state == 1){
				n = 1;
			}else{
				return 0;
			}
		}else if(Str.M == adr.M && Str.B == adr.B && Str.S == adr.S){
			if(Switch[NAdr.M][NAdr.B][NAdr.S]->state == 0){
				n = 1;
			}else{
				return 0;
			}
		}else{
			return 0;
		}

		//	printf("New switch\n");
		adr = Switch[NAdr.M][NAdr.B][NAdr.S]->Adr;
		NAdr = Switch[NAdr.M][NAdr.B][NAdr.S]->App;
		goto R;

	}
	return n;
}

void free_Switch(struct adr adr, int direct){
	struct adr NAdr;
	int dir = blocks[adr.M][adr.B][adr.S]->dir;

	if(direct == 0){
		if(dir == 0 || dir == 2){
			NAdr = blocks[adr.M][adr.B][adr.S]->NAdr;
		}else{
			NAdr = blocks[adr.M][adr.B][adr.S]->PAdr;
		}
	}else{
		if(dir == 0 || dir == 2){
			NAdr = blocks[adr.M][adr.B][adr.S]->PAdr;
		}else{
			NAdr = blocks[adr.M][adr.B][adr.S]->NAdr;
		}
	}
	//printf("NAdr %i:%i:%i\n",NAdr.M,NAdr.B,NAdr.S);
	int n;
	R:{};
	//printf("NAdr: %i:%i:%i\n",NAdr.M,NAdr.B,NAdr.S);
	if(NAdr.type == 's'){
		struct adr Div = Switch[NAdr.M][NAdr.B][NAdr.S]->Div;
		struct adr Str = Switch[NAdr.M][NAdr.B][NAdr.S]->Str;
		if(Div.M == adr.M && Div.B == adr.B && Div.S == adr.S){
			if(Switch[NAdr.M][NAdr.B][NAdr.S]->state == 0){
				Switch[NAdr.M][NAdr.B][NAdr.S]->state = 1;
			}
		}else if(Str.M == adr.M && Str.B == adr.B && Str.S == adr.S){
			if(Switch[NAdr.M][NAdr.B][NAdr.S]->state == 1){
				Switch[NAdr.M][NAdr.B][NAdr.S]->state = 0;
			}
		}

		//	printf("New switch\n");
		adr = Switch[NAdr.M][NAdr.B][NAdr.S]->Adr;
		NAdr = Switch[NAdr.M][NAdr.B][NAdr.S]->App;
		goto R;

	}
	return;
}

void JSON(){
	FILE *fp;

	fp = fopen("baan.json","w");

	if(fp == NULL){
		printf("Opening failed!!!!\n");
		//exit(1);
		return;
	}
	fprintf(fp,"{\"M\" : [");
	int p = 0;


	for(int i = 0;i<Adress;i++){

		struct adr A = Adresses[i];
		if(A.type == 'R'){
			if(p != 0){
				fprintf(fp,",");
			}else{
				p = 1;
			}

			fprintf(fp,"[%i,%i,%i,%i,%i,%i,%i]",A.M,A.B,A.S,blocks[A.M][A.B][A.S]->dir,blocks[A.M][A.B][A.S]->blocked,blocks[A.M][A.B][A.S]->state,blocks[A.M][A.B][A.S]->train);
		}
	}

	fprintf(fp,"],\"SW\" : [");

	p = 0;

	for(int i = 0;i<Adress;i++){

		struct adr A = Adresses[i];
		if(A.type == 'S'){
			if(p != 0){
				fprintf(fp,",");
			}else{
				p = 1;
			}

			fprintf(fp,"[%i,%i,%i,%i,%i]",A.M,A.B,A.S,Switch[A.M][A.B][A.S]->state,Switch[A.M][A.B][A.S]->len);
		}
	}
	fprintf(fp,"],\"MO\" : [");

	p = 0;

	for(int i = 0;i<Adress;i++){

		struct adr A = Adresses[i];
		if(A.type == 'M'){
			if(p != 0){
				fprintf(fp,",");
			}else{
				p = 1;
			}

			fprintf(fp,"[%i,%i,%i,%i,%i]",A.M,A.B,A.S,Moduls[A.M][A.B][A.S]->length,Moduls[A.M][A.B][A.S]->state);
		}
	}
	fprintf(fp,"]}");
	fclose(fp);
}

void Slowdown_train(char train){
	if(delayA == 1500000){
		delayA -= 1000000;
	}
}

void STOP_train(char train){
	if(delayA == 1500000){
		delayA -= 1000000;
	}
}

void procces(struct adr adr,int debug){
	if(adr.type != 'R'){

	}else{
		if(adr.S == 0){
			if(blocks[adr.M][adr.B][adr.S]->blocked == 0){
				blocks[adr.M][adr.B][adr.S]->train = 0;
			}
			if(debug){
				struct Seg *BA = blocks[adr.M][adr.B][adr.S];
				printf("\t\t          \tA  %i:%i:%i;T:%i",BA->Adr.M,BA->Adr.B,BA->Adr.S,BA->train);
				if(BA->blocked){
					printf("B");
				}
				printf("\n");
			}
		}else{

			struct adr bl[4] = {0};
			struct adr bp = {0};
			bl[0] = adr;
			int i = 0;
			int p = 0;
			struct Seg B;
			//Get blocks in avalable path
			for(i = 0;i<4;i){
				B = *blocks[bl[i].M][bl[i].B][bl[i].S];
				i++;
				if(NADR(B.Adr).type == 'e' && B.Adr.S != 0){
					break;
				}else if(NADR(B.Adr).type == 's'){
					if(!check_Switch(B.Adr,0)){
						break;
					}
				}
				bl[i] = Next(adr,i)->Adr;
			}
			i--;
			//printf("I: %i\n",i);

			//Setup previous address
			if(check_Switch(adr,1)){
				bp = Prev(adr,1)->Adr;
				p  = 1;
			}

			struct Seg *BA = blocks[bl[0].M][bl[0].B][bl[0].S];
			struct Seg *BP;
			struct Seg *BN;
			struct Seg *BNN;
			struct Seg *BNNN;

			if(i > 0){
				BN = blocks[bl[1].M][bl[1].B][bl[1].S];
			}
			if(i > 1){
				BNN = blocks[bl[2].M][bl[2].B][bl[2].S];
			}
			if(i > 2){
				BNNN = blocks[bl[3].M][bl[3].B][bl[3].S];
			}
			if(p == 1){
				BP = blocks[bp.M][bp.B][bp.S];
			}

			//Train ID following
				if(!BA->blocked && BA->train != 0){
					//Reset
					printf("Reset");
					BA->train = 0;
				}
				if(i > 0 && BN->train && !BA->train && BA->blocked){
					//REVERSED
				}else if(p == 1 && i > 0 && BN->train == 0 && BP->train == 0 && BA->train == 0 && BA->blocked){
						//NEW TRAIN
						BA->train = ++bTrain;
						//printf("New train");
				}
				if(p == 1 && BP->blocked && BA->blocked && BA->train == 0){
					BA->train = BP->train;
				}
				if(i > 0 && BN->train == 0 && BN->blocked && BA->blocked){
					BN->train = BA->train;
				}
				if(i > 1 && BNN->train == 0 && BNN->blocked && BN->blocked){
					BNN->train = BN->train;
				}
			//Check switch
				if(i > 0 && NADR(BN->Adr).type == 's' && BA->blocked){
					if(!check_Switch(BN->Adr,0)){
						free_Switch(BN->Adr,0);
						if(i < 2){
							BNN = Next(BN->Adr,1);
							i++;
						}
					}
					BNN->state = RESERVED;
				}

			//State coloring
				//Self
				if(i > 0 && BN->blocked){
					BA->state = RED;
				}else if(i > 1 && BNN->blocked && !BN->blocked){
					BA->state = AMBER;
				}else if(i > 2 && !BNN->blocked && !BN->blocked){
					if(BA->state != RESERVED){
						BA->state = GREEN;
					}
				}

				//Next
				if(i > 1 && BNN->blocked && !BN->blocked){
					BN->state = RED;
				}else if(i > 2 && BNNN->blocked && !BNN->blocked){
					BN->state = AMBER;
				}else if(i > 2 && !BN->blocked && !BNN->blocked && !BNNN->blocked){
					if(BN->state != RESERVED){
						BN->state = GREEN;
					}
				}

				//Next Next
				if(i > 2 && BNNN->blocked && !BNN->blocked){
					BNN->state = RED;
				}

				//Prev
				if(p > 0 && i > 0 && !BA->blocked && BN->blocked){
					BP->state = AMBER;
				}else if(p > 0 && i > 0 && !BA->blocked && !BN->blocked){
					if(BP->state != RESERVED){
						BP->state = GREEN;
					}
				}

				//Debug info
				if(debug){
					if(p == 1){
						printf("\t\tP %i:%i:%i;B:%i\t",BP->Adr.M,BP->Adr.B,BP->Adr.S,BP->blocked);
					}else{
						printf("\t\t          \t");
					}
					printf("A%i %i:%i:%i;T:%i",i,adr.M,adr.B,adr.S,BA->train);
					if(BA->blocked){
						printf("B");
					}
					printf("\t");
					if(i == 3){
						printf("N %i:%i:%i;B:%i\tNN %i:%i:%i;B:%i\tNNN %i:%i:%i;B:%i\t",BN->Adr.M,BN->Adr.B,BN->Adr.S,BN->blocked,BNN->Adr.M,BNN->Adr.B,BNN->Adr.S,BNN->blocked,BNNN->Adr.M,BNNN->Adr.B,BNNN->Adr.S,BNNN->blocked);
					}
					if(i == 2){
						printf("N %i:%i:%i;B:%i\tNN %i:%i:%i;B:%i\t",BN->Adr.M,BN->Adr.B,BN->Adr.S,BN->blocked,BNN->Adr.M,BNN->Adr.B,BNN->Adr.S,BNN->blocked);
					}
					if(i == 1){
						printf("N %i:%i:%i;B:%i\t",BN->Adr.M,BN->Adr.B,BN->Adr.S,BN->blocked);
					}
					if(i == 0){
						printf("\n");
					}
				}


			//TRAIN control
				if(BA->blocked && BN->blocked && BA->train != BN->train){
					//Kill train
					printf("KILL TRAIN:%i",BA->train);
				}
				if((BA->blocked && !BN->blocked && BN->state == RED) || (i == 0 && BA->blocked)){
					//Fire stop timer
					printf("STOP TRAIN:%i",BA->train);
				}
				if((BA->blocked && !BN->blocked && BN->state == AMBER) || (i == 1 && BA->blocked)){
					//Fire slowdown timer
					printf("SLOWDOWN TRAIN:%i",BA->train);
				}
				if((BA->blocked && !BN->blocked && BA->train != 0) || (BN->max_speed > BA->max_speed && BA->blocked && !BN->blocked)){
					printf("Speedup to track speed %i km/h, train:%i",BA->max_speed,BA->train);
				}
				if(BN->max_speed < BA->max_speed && BA->blocked && !BN->blocked){
					printf("Slowdown to track speed %i km/h, train:%i",BA->max_speed,BA->train);
				}
				printf("\n");
			if(BA->train >= 4){
				printf("STPOPPED\n");
				stop = 1;
			}
		}
	}
}

int pathFinding(struct adr Begin, struct adr End){
	struct adr NAdr,SNAdr,Adr = Begin;
	struct PATH {
		struct adr adr;
		struct adr App;
		int suc;
		int Sstate[10];
		int state;
	};
	struct PATH PATH[MAX_ROUTE] = {{{0,0,0,'e'},0}};
	int a = 0;
	int flip = 0;
	int switches = 0;
	int pathid = 0;

	printf("S\t%i:%i:%i\ttype:%c\n",Adr.M,Adr.B,Adr.S,Adr.type);
	//printf("\n%i\n",i);
	I:{};
	if(Adr.type == 'R'){
		int dir = blocks[Adr.M][Adr.B][Adr.S]->dir;

		if(dir == 0 || dir == 2){
			NAdr = blocks[Adr.M][Adr.B][Adr.S]->NAdr;
		}else{
			NAdr = blocks[Adr.M][Adr.B][Adr.S]->PAdr;
		}
	}

	J:{};

	//printf("%i:%i:%i\n",NAdr.M,NAdr.B,NAdr.S);

	if(NAdr.M == End.M && NAdr.B == End.B && NAdr.S == End.S){
		//printf("\n");
		printf("FOUND\nSwitches: %i\n\n",switches);
		return 1;//blocks[Adr.M][Adr.B][Adr.S];
	}
	usleep(20000);
	//printf("i:%i",i);
	printf("\t%i:%i:%i\ttype:%c\t==\t%i:%i:%i\ttype:%c\n",NAdr.M,NAdr.B,NAdr.S,NAdr.type,Begin.M,Begin.B,Begin.S,Begin.type);
	if(NAdr.type == 'e' || NAdr.M == Begin.M && NAdr.B == Begin.B && NAdr.S == Begin.S){
		//printf("Switch back: %i:%i:%i:%c\n",PATH[switches-1].adr.M,PATH[switches-1].adr.B,PATH[switches-1].adr.S,PATH[switches-1].adr.type);
		//if(PATH[switches-1].adr.type == 'S'){
			if(flip == 0){
				PATH[switches-1].suc = 1;
				PATH[switches-1].state = !PATH[switches-1].state;
				printf("Change switch %i:%i:%i\n",PATH[switches-1].adr.M,PATH[switches-1].adr.B,PATH[switches-1].adr.S);

				if(PATH[switches-1].state == 0){ //Straight?
					printf("Str");
					Adr = Switch[PATH[switches-1].adr.M][PATH[switches-1].adr.B][PATH[switches-1].adr.S]->Str;
				}else{
					printf("Str");
					Adr = Switch[PATH[switches-1].adr.M][PATH[switches-1].adr.B][PATH[switches-1].adr.S]->Div;
				}
				NAdr = Adr;
				printf("New adr %i:%i:%i:%c",Adr.M,Adr.B,Adr.S,Adr.type);
				flip = switches - 1;
				printf("Flip: %i\n",flip);
				usleep(2000000);
				goto I;
			}else if((switches - 1) > flip){
				printf("New Flip\n");
			}else if((switches - 1) == flip){
				printf("ReFlip\n");
				flip--;
				printf("Flip: %i\n",flip);
				PATH[flip-1].suc = 1;
				PATH[flip-1].state = !PATH[switches-1].state;
				printf("Change switch %i:%i:%i\n",PATH[flip-1].adr.M,PATH[flip-1].adr.B,PATH[flip-1].adr.S);
				if(PATH[flip-1].state == 0){ //Straight?
					Adr = Switch[PATH[flip-1].adr.M][PATH[flip-1].adr.B][PATH[flip-1].adr.S]->Str;
				}else{
					Adr = Switch[PATH[flip-1].adr.M][PATH[flip-1].adr.B][PATH[flip-1].adr.S]->Div;
				}
				NAdr = Adr;
				usleep(2000000);
				printf("Adr %i:%i:%i:%c\n",Adr.M,Adr.B,Adr.S,Adr.type);
				goto I;
			}
		//}
		printf("STOP\nBack to start\n\n",Adr.M,Adr.B,Adr.S,Adr.type);
		return 0;//blocks[Adr.M][Adr.B][Adr.S];

	}else if(NAdr.type == 'R'){
		Adr = NAdr;
		goto I;
	}else if(NAdr.type == 'S' || NAdr.type == 's' || NAdr.type == 'm' || NAdr.type == 'M'){
		R:{};
		//printf("\ni:%i\tC:%i:%i:%i type:%c\n",i,NAdr.M,NAdr.B,NAdr.S,NAdr.type);
		if(blocks[NAdr.M][NAdr.B][0] != NULL){
			a++;
		}
		if(NAdr.type == 'S'){
			PATH[switches].adr = NAdr;
			PATH[switches].App = Adr;
			PATH[switches].state = Switch[NAdr.M][NAdr.B][NAdr.S]->state;
			printf("N%i %i:%i:%i\n",switches,NAdr.M,NAdr.B,NAdr.S);
			switches++;

			if(Switch[NAdr.M][NAdr.B][NAdr.S]->state == 0){ //Straight?
				SNAdr = Switch[NAdr.M][NAdr.B][NAdr.S]->Str;
			}else{
				SNAdr = Switch[NAdr.M][NAdr.B][NAdr.S]->Div;
			}
		}else if(NAdr.type == 's'){
			SNAdr = Switch[NAdr.M][NAdr.B][NAdr.S]->App;
		}else if(NAdr.type == 'M'){
			//printf("m\n");
			int s = Moduls[NAdr.M][NAdr.B][NAdr.S]->state;
			SNAdr = Moduls[NAdr.M][NAdr.B][NAdr.S]->mAdr[s];
			//printf("\tm:%i:%i:%i\ttype:%c\n",SNAdr.M,SNAdr.B,SNAdr.S,SNAdr.type);
		}else if(NAdr.type == 'm'){
			//printf("M\n");
			int s = Moduls[NAdr.M][NAdr.B][NAdr.S]->state;
			SNAdr = Moduls[NAdr.M][NAdr.B][NAdr.S]->MAdr[s];
			//printf("\t%i:%i:%i\ttype:%c\n",Adr.M,Adr.B,Adr.S,Adr.type);
		}
		//printf("SN %i:%i:%i%c\tN %i:%i:%i\ti:%i\n",SNAdr.M,SNAdr.B,SNAdr.S,SNAdr.type,NAdr.M,NAdr.B,NAdr.S,i);
		if(SNAdr.type == 'S' || SNAdr.type == 's'){
			NAdr = SNAdr;
			goto R;
		}else if(SNAdr.type == 'M' || SNAdr.type == 'm'){
			NAdr = SNAdr;
			goto R;

		}else{
			Adr = SNAdr;
			NAdr = Adr;
			goto I;
		}
	}
}

void *do_Magic(){
	while(!stop){
		//printf("Procces loop \t");
		clock_t t;
		t = clock();
		pthread_mutex_lock(&mutex_lockA);
		for(int i = 0;i<Adress;i++){
			//printf("%i: %i:%i:%i\n",i,Adresses[i].M,Adresses[i].B,Adresses[i].S);
			procces(Adresses[i],1);
		}
		JSON();
		pthread_mutex_unlock(&mutex_lockA);
		t = clock() - t;
		printf ("It took me %d clicks (%f seconds).\n",t,((float)t)/CLOCKS_PER_SEC);
		usleep(500000);
	}
}

void *STOP_FUNC(){
	//char str[10];
	while(!stop){
		printf("Type q{Enter} to stop\n");
		if (feof(stdin)){
			char c = getc(stdin);
			if(c == 'q'){
				//stop = 1;
			}
		}
		usleep(1000000);
	}
}

void *TRAIN_SIMA(){
	struct Seg *B = blocks[8][5][1];
	struct Seg *N = blocks[8][5][1];
	struct Seg *A = blocks[0][0][0];
	int i = 0;
	pthread_mutex_lock(&mutex_lockA);

	while(!stop){
		printf("Train Sim Step (id:%i)\t",pthread_self());
		N = Next(B->Adr,1+i);
		if(i > 0){
			A = Next(B->Adr,i);
		}
		printf(" %i:%i:%i\n",N->Adr.M,N->Adr.B,N->Adr.S);
		N->blocked = 1;
		pthread_mutex_unlock(&mutex_lockA);
		usleep(delayA);
		pthread_mutex_lock(&mutex_lockA);
		if(i>0){
			A->blocked = 0;
		}else{
			B->blocked = 0;
		}
		pthread_mutex_unlock(&mutex_lockA);
		usleep(delayA);
		pthread_mutex_lock(&mutex_lockA);
		if(N->Adr.S == 0){
			i++;
		}else{
			B = N;
			i = 0;
		}
	}
}

void *TRAIN_SIMB(){
	struct Seg *B2 = blocks[1][4][1];
	struct Seg *N2 = blocks[1][4][1];
	struct Seg *A2[3] = {blocks[0][0][0],blocks[0][0][0],blocks[0][0][0]};
	int i2 = 0;
	pthread_mutex_lock(&mutex_lockA);

	while(!stop){
		printf("Train Sim Step (id:%i)\t",pthread_self());
		N2 = Next(B2->Adr,1+i2);
		if(i2 > 0){
			A2[i2] = Next(B2->Adr,i2);
		}
		printf(" %i:%i:%i\n",N2->Adr.M,N2->Adr.B,N2->Adr.S);
		N2->blocked = 1;
		pthread_mutex_unlock(&mutex_lockA);
		usleep(delayB);
		pthread_mutex_lock(&mutex_lockA);
		if(i2>0){
			A2[i2]->blocked = 0;
		}else{
			B2->blocked = 0;
		}
		pthread_mutex_unlock(&mutex_lockA);
		usleep(delayB);
		pthread_mutex_lock(&mutex_lockA);
		if(N2->Adr.S == 0){
			i2++;
		}else{
			B2 = N2;
			i2 = 0;
		}
	}
}

void main(){
	blocks[0][0][0] = C_Seg(C_AdrT(0,0,0,'e'),3);
	blocks[0][0][0]->NAdr.type = 'e';
	blocks[0][0][0]->PAdr.type = 'e';

	struct link LINK;
	LINK.Adr1 = END_BL;
	LINK.Adr2 = END_BL;
	LINK.Adr3 = END_BL;
	LINK.Adr4 = END_BL;

	int setup[MAX_Modules] = {1,4,8,2,0};

	for(int i = 0;i<4;i++){
		LINK = Modules(setup[i],LINK);
	}
	setup_JSON(setup,4);

	//Module_5(C_Adr(3,1,1),0,C_Adr(2,1,1),0);
	printf("Adress: %i\n",Adress);

//	Switch[5][2][1]->state = 0;
//	Switch[5][5][1]->state = 0;
//	Switch[6][3][1]->state = 0;

	char ID = 0xFA;
/*
	struct adr A = {3,5,1,'R'};
	struct adr B = {3,7,1,'R'};

	pathFinding(A,B);
*/
	add_train(ID);

	JSON();
	printf("TEST\n");
	//do_Magic();

	//blocks[1][1][1]->blocked = 1;
	//blocks[3][7][1]->blocked = 1;
	//do_Magic();

	pthread_t tid[4];

	printf("Creating Threads");

	pthread_create(&tid[0], NULL, do_Magic, NULL);
	pthread_create(&tid[1], NULL, STOP_FUNC, NULL);
	StartAdr = C_Adr(3,5,1);
	blocks[1][4][1]->blocked = 1;
	pthread_create(&tid[2], NULL, TRAIN_SIMA, NULL);
	StartAdr = C_Adr(8,5,1);
	blocks[8][6][1]->blocked = 1;
	pthread_create(&tid[3], NULL, TRAIN_SIMB, NULL);

	usleep(20000000);
	Switch[4][6][1]->state = !Switch[4][6][1]->state;
	Switch[4][6][2]->state = !Switch[4][6][1]->state;


	pthread_join(tid[0],NULL);
	pthread_join(tid[1],NULL);
	pthread_join(tid[2],NULL);
	pthread_join(tid[3],NULL);

	//procces(C_Adr(6,2,1),1);

	printf("STOPPED");

	//do_Magic();
}
