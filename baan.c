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
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <openssl/sha.h>
#include "./src/b64.c"

#define MAX_SW 8
#define MAX_Segments 8
#define MAX_Blocks 16
#define MAX_Modules 16
#define MAX_A MAX_Modules*MAX_Blocks*MAX_Segments
#define MAX_TRAINS 25
#define MAX_ROUTE 20
#define MAX_TIMERS 5
#define MAX_WEB_CLIENTS 5

#define MAX_SWITCH_LINK 5
#define MAX_SWITCH_PREFFERENCE 5

#define GREEN 0
#define AMBER 1
#define RED 2
#define BLOCKED 3
#define PARKED 4
#define RESERVED 5
#define UNKNOWN 6

#define Arr_Count(array) (sizeof(array)/sizeof((array)[0]))
#define ROUND(nr)  (int)(nr+0.5)

#define TRACK_SCALE 160

int stop = 0;
int delayA = 6000000;
int delayB = 6000000;
int initialise = 1;
char setup_data[100];
int status_st[20] = {0};

pthread_mutex_t mutex_lockA;
pthread_mutex_t mutex_lockB;
pthread_t timer_thread[MAX_TIMERS];
int timers[MAX_TIMERS] = {0}; //0 = Free, 1 = Busy, 2 = Done

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
	char max_speed;		// 5 times the speed (25 => 125km/h )
	char state;
	char dir;		//0xAABB, A = Current travel direction, B = Travel direction
	char length;
	char train;		 //0x00 <-> 0xFF
	_Bool blocked;
	_Bool change; //If block changed state;
	/*
	//Running timers
	char t_Flag;  /*	1 = Speed timer
										2 = Signal timer
										4-128 = Empty timers
								*/
	//Rules
	//Speed Rules
	//Train Type preference

	//Signals
	char signals; // AAAA BBBB, A = Prev Signal, B = Next Signal
};

struct Mod{
	struct adr Adr;

	struct adr mAdr[10];
	struct adr MAdr[10];
	char length;
	char s_length;
	char state;
};

struct link{
	struct adr Adr1;
	struct adr Adr2;
	struct adr Adr3;
	struct adr Adr4;
};

struct train{
	char DCC_ID;
	char type;
	char name[21];
	char cur_speed;
	long max_speed;
	char accelerate_speed; //divide by 50
	char break_speed; //divide by 50
	char control;
	char use;
	struct adr Route[MAX_ROUTE];
	int timer;
	int timer_id;
};

//Include Custom Headers
#include "./src/Web.h"
#include "./src/COM.h"


struct train *trains[MAX_TRAINS] = {};
struct train *train_link[MAX_TRAINS];
int iTrain = 0;
int bTrain = 0;

struct adr Adresses[MAX_A] = {};
struct adr StartAdr;
int Adress = 0;
struct Seg *blocks[MAX_Modules][MAX_Blocks][MAX_Segments] = {};

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
	Z->change = 0;
	Z->train = 0x00;

	printf("A Segment is created at %i:%i:%i\tAdr:%i\n",Adr.M,Adr.B,Adr.S,Adress);
	blocks[Adr.M][Adr.B][Adr.S] = Z;

	//return Z;
	Adresses[Adress] = Adr;
	Adress++;
}

#include "./src/switch.c"

int Adr_Comp(struct adr A,struct adr B){
	if(A.M == B.M && A.B == B.B && A.S == B.S){
		return 1;
	}else{
		return 0;
	}
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
	}else if(B->Adr.S == 0){
		return 1;
	}else{
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
	int direct = blocks[Adr.M][Adr.B][Adr.S]->dir;
	I:{};
	int dir = blocks[Adr.M][Adr.B][Adr.S]->dir;

	if(direct == 0 || direct == 2 || direct == 5){
		if(dir == 0 || dir == 2 || dir == 5){
			NAdr = blocks[Adr.M][Adr.B][Adr.S]->NAdr;
		}else{
			NAdr = blocks[Adr.M][Adr.B][Adr.S]->NAdr;
		}
	}else{
		if(dir == 0 || dir == 2 || dir == 5){
			NAdr = blocks[Adr.M][Adr.B][Adr.S]->PAdr;
		}else{
			NAdr = blocks[Adr.M][Adr.B][Adr.S]->PAdr;
		}
	}

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

void change_block_state(struct Seg * Block,int State){
	if(Block->state != State){
		Block->change = 1;
		Block->state = State;
	}
}

void change_signal(struct Seg * Block, char State, char side){
	if(side == 'N'){
		if((Block->signals & 0xF) != (State & 0xF)){
			Block->signals = Block->signals & 0xF0 + State & 0xF;
			//COM_change_signal(Block);
		}
	}else if(side == 'P'){
		if((Block->signals >> 4) != (State & 0xF)){
			Block->signals = Block->signals & 0xF + State << 4;
			//COM_change_signal(Block);
		}
	}
}

void JSON_new_client();

struct timer_thread_data{
   int  thread_id;
	 int  t;
};

struct timer_thread_data a[MAX_TIMERS];

#include "./src/modules.c"
#include "./src/status.c"
#include "./src/signals.c"
#include "./src/trainlist.c"
#include "./src/train_sim.c"
#include "./src/Web.c"
#include "./src/COM.c"

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

	int n;
	R:{};
	//printf("NAdr %i:%i:%i:%c\n",NAdr.M,NAdr.B,NAdr.S,NAdr.type);
	if(NAdr.type == 'R'){
		//printf("Return 1\n");
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
	}else if(NAdr.type == 'M'){
		int s = Moduls[NAdr.M][NAdr.B][NAdr.S]->state;
		if(Adr_Comp(Moduls[NAdr.M][NAdr.B][NAdr.S]->MAdr[s],adr)){
			NAdr = Moduls[NAdr.M][NAdr.B][NAdr.S]->mAdr[s];
			goto R;
		}else{
			return 0;
		}
	}else if(NAdr.type == 'm'){
		int s = Moduls[NAdr.M][NAdr.B][NAdr.S]->state;
		if(Adr_Comp(Moduls[NAdr.M][NAdr.B][NAdr.S]->mAdr[s],adr)){
			NAdr = Moduls[NAdr.M][NAdr.B][NAdr.S]->MAdr[s];
			goto R;
		}else{
			return 0;
		}
	}else{
		struct adr Div = Switch[NAdr.M][NAdr.B][NAdr.S]->Div;
		struct adr Str = Switch[NAdr.M][NAdr.B][NAdr.S]->Str;
		//printf("Div %i:%i:%i==%i:%i:%i\n",Div.M,Div.B,Div.S,adr.M,adr.B,adr.S);
		//printf("Str %i:%i:%i==%i:%i:%i\n",Str.M,Str.B,Str.S,adr.M,adr.B,adr.S);
		if(Adr_Comp(Div,adr)){
			if(Switch[NAdr.M][NAdr.B][NAdr.S]->state == 1){
				n = 1;
			}else{
				return 0;
			}
		}else if(Adr_Comp(Str,adr)){
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
	printf("Retrun %i\n",n);
	return n;
}

int free_Switch(struct adr adr, int direct){
	struct adr NAdr;
	int return_Value = 1;
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
	R:{};
	printf("NAdr: %i:%i:%i\t",NAdr.M,NAdr.B,NAdr.S);
	printf("%i\n",return_Value);
	if(return_Value == 0){
		return 0;
	}

	if(NAdr.type == 'S'){
		if(Switch[NAdr.M][NAdr.B][NAdr.S]->state == 0){ //Straight?
			adr = NAdr;
			NAdr = Switch[NAdr.M][NAdr.B][NAdr.S]->Str;
		}else{
			adr = NAdr;
			NAdr = Switch[NAdr.M][NAdr.B][NAdr.S]->Div;
		}
		goto R;
	}else if(NAdr.type == 's'){
		struct adr Div = Switch[NAdr.M][NAdr.B][NAdr.S]->Div;
		struct adr Str = Switch[NAdr.M][NAdr.B][NAdr.S]->Str;
		if(Adr_Comp(Div,adr)){
			if(Switch[NAdr.M][NAdr.B][NAdr.S]->state == 0){
				return_Value = throw_switch(Switch[NAdr.M][NAdr.B][NAdr.S]);
			}
		}else if(Adr_Comp(Str,adr)){
			if(Switch[NAdr.M][NAdr.B][NAdr.S]->state == 1){
				return_Value = throw_switch(Switch[NAdr.M][NAdr.B][NAdr.S]);
			}
		}

		//	printf("New switch\n");
		adr = Switch[NAdr.M][NAdr.B][NAdr.S]->Adr;
		NAdr = Switch[NAdr.M][NAdr.B][NAdr.S]->App;
		goto R;

	}else if(NAdr.type == 'M'){
		struct Mod * M = Moduls[NAdr.M][NAdr.B][NAdr.S];
		int s = M->state;
		if(Adr_Comp(M->MAdr[s],adr)){
			NAdr = M->mAdr[s];
		}else{
			for(int i = 0;i<M->length;i++){
				if(Adr_Comp(M->MAdr[i],adr)){
					return_Value = throw_ms_switch(M,i);
					break;
				}
			}
		}
		adr = M->Adr;
		NAdr = M->mAdr[M->state];
		goto R;
	}else if(NAdr.type == 'm'){
		struct Mod * M = Moduls[NAdr.M][NAdr.B][NAdr.S];
		int s = M->state;
		if(Adr_Comp(M->mAdr[s],adr)){
			NAdr = M->MAdr[s];
		}else{
			for(int i = 0;i<M->length;i++){
				if(Adr_Comp(M->mAdr[i],adr)){
					return_Value = throw_ms_switch(M,i);
					break;
				}
			}
		}
		adr = M->Adr;
		NAdr = M->MAdr[M->state];
		goto R;
	}
	return 1;
}

void JSON(){
	pthread_mutex_lock(&mutex_lockB);
	char buf[4096];
	memset(buf,0,4096);

	sprintf(buf,"%s{\"M\" : [",buf);
	int p = 0;


	for(int i = 0;i<Adress;i++){

		struct adr A = Adresses[i];
		if(A.type == 'R' && blocks[A.M][A.B][A.S]->change != 0){
			blocks[A.M][A.B][A.S]->change = 0;
			if(p != 0){
				sprintf(buf,"%s,",buf);
			}else{
				p = 1;
			}

			sprintf(buf,"%s[%i,%i,%i,%i,%i,%i,%i]",buf,A.M,A.B,A.S,blocks[A.M][A.B][A.S]->dir,blocks[A.M][A.B][A.S]->blocked,blocks[A.M][A.B][A.S]->state,blocks[A.M][A.B][A.S]->train);
		}
	}

	sprintf(buf,"%s]}",buf);

	if(strlen(buf) > 10){
		send_all(buf);
	}

	pthread_mutex_unlock(&mutex_lockB);
}

void JSON_new_client(){
	pthread_mutex_lock(&mutex_lockB);
	char buf[4096];
	memset(buf,0,4096);

	sprintf(buf,"%s{\"M\" : [",buf);
	int p = 0;


	for(int i = 0;i<Adress;i++){

		struct adr A = Adresses[i];
		if(A.type == 'R'){
			if(p != 0){
				sprintf(buf,"%s,",buf);
			}else{
				p = 1;
			}

			sprintf(buf,"%s[%i,%i,%i,%i,%i,%i,%i]",buf,A.M,A.B,A.S,blocks[A.M][A.B][A.S]->dir,blocks[A.M][A.B][A.S]->blocked,blocks[A.M][A.B][A.S]->state,blocks[A.M][A.B][A.S]->train);
		}
	}

	sprintf(buf,"%s]}",buf);

	if(strlen(buf) > 10){
		send_all(buf);
	}
	memset(buf,0,4096);

	sprintf(buf,"{\"SW\" : [");

	p = 0;

	for(int i = 0;i<Adress;i++){

		struct adr A = Adresses[i];
		if(A.type == 'S'){
			if(p != 0){
				sprintf(buf,"%s,",buf);
			}else{
				p = 1;
			}

			sprintf(buf,"%s[%i,%i,%i,%i,%i,%i]",buf,A.M,A.B,A.S,Switch[A.M][A.B][A.S]->state);
		}
	}
	sprintf(buf,"%s]}",buf);

	if(strlen(buf) > 11){
		send_all(buf);
	}
	memset(buf,0,4096);

	sprintf(buf,"{\"Mod\" : [");
	p = 0;
	for(int i = 0;i<Adress;i++){

		struct adr A = Adresses[i];
		if(A.type == 'M'){
			if(p != 0){
				sprintf(buf,"%s,",buf);
			}else{
				p = 1;
			}

			sprintf(buf,"%s[%i,%i,%i,%i,%i]",buf,A.M,A.B,A.S,Moduls[A.M][A.B][A.S]->state,Moduls[A.M][A.B][A.S]->length);
		}
	}
	sprintf(buf,"%s]}",buf);

	if(strlen(buf) > 11){
		send_all(buf);
	}

	pthread_mutex_unlock(&mutex_lockB);
}

void *Test(void *threadArg){
	struct timer_thread_data *my_data;
	my_data = (struct timer_thread_data *) threadArg;
	int i = my_data->thread_id;
	int t = my_data->t;
	printf("\t%i Sleep %i\n",i,t);
	usleep(t);
	timers[i] = 2;
	printf("%i done\n",i);
}

void STOP_train(char train){
	usleep(500000);
	//timers;
}

void create_timer(){
	int i = 0;
	while(1){
		if(timers[i] == 0){
			timers[i] = 1;
			a[i].thread_id = i;
			a[i].t = (rand() % 50) * 100000 + 400000;
			printf("Create time %i, sleep %i\n",i,a[i].t);
			pthread_create(&timer_thread[i], NULL, Test, (void *) &a[i]);
			break;
		}
		i++;
		if(i==MAX_TIMERS){
			i = 0;
			printf("Not enought timers!!!!!\n\n");
			usleep(100000);
		}
	}
}

void procces(struct adr adr,int debug){
	if(adr.type != 'R'){

	}else{
		if(adr.S == 0){
			if(blocks[adr.M][adr.B][adr.S]->blocked == 0){
				blocks[adr.M][adr.B][adr.S]->train = 0;
			}
			//if(debug){
				struct Seg *BA = blocks[adr.M][adr.B][adr.S];
				if(BA->train != 0){
					//printf("ID: %i\t%i:%i:%i\n",BA->train,BA->Adr.M,BA->Adr.B,BA->Adr.S);
				}
				//printf("\t\t          \tA  %i:%i:%i;T%iR%i",BA->Adr.M,BA->Adr.B,BA->Adr.S,BA->train,BA->dir);
				//if(BA->blocked){
				//	printf("B");
				//}
				//printf("\n");
			//}
		}else{
			//printf("B\n");

			struct adr bl[4] = {0};
			struct adr bp,bp2;
			bl[0] = adr;
			int i = 0;
			int p = 0;
			struct Seg B;
			//Get blocks in avalable path
			for(i = 0;i<4;i){
				B = *blocks[bl[i].M][bl[i].B][bl[i].S];
				i++;
				//printf("i%i\t%i:%i:%i:%c\t%i:%i:%i:%c\n",i,B.Adr.M,B.Adr.B,B.Adr.S,B.Adr.type,NADR(B.Adr).M,NADR(B.Adr).B,NADR(B.Adr).S,NADR(B.Adr).type);
				if(NADR(B.Adr).type == 'e' && B.Adr.S != 0){
					break;
				}else if(NADR(B.Adr).type == 's' || NADR(B.Adr).type == 'S' || NADR(B.Adr).type == 'm' || NADR(B.Adr).type == 'M'){
					//printf("Check_switch\n");
					if(!check_Switch(B.Adr,0)){
						//printf("Switch checked\n");
						break;
					}
				}
				bl[i] = Next(adr,i)->Adr;
			}
			i--;

			//Setup previous address
			if(check_Switch(adr,1)){
				bp = Prev(adr,1)->Adr;
				p  = 1;
			}
			if(bp.S != 0 && p == 1 && check_Switch(bp,1) == 1){
				bp2 = Prev(adr,2)->Adr;
				p = 2;
			}

			struct Seg *BA = blocks[bl[0].M][bl[0].B][bl[0].S];
			struct Seg *BPP;
			struct Seg *BP;
			struct Seg *BN;
			struct Seg *BNN;
			struct Seg *BNNN;

			if(i > 0){
				BN = blocks[bl[1].M][bl[1].B][bl[1].S];
			}
			//printf("|");
			if(i > 1){
				BNN = blocks[bl[2].M][bl[2].B][bl[2].S];
			}
			//printf("|");
			if(i > 2){
				BNNN = blocks[bl[3].M][bl[3].B][bl[3].S];
			}
			//printf("|");
			if(p > 0){
				BP = blocks[bp.M][bp.B][bp.S];
			}
			//printf("|");
			if(p > 1){
				BPP = blocks[bp2.M][bp2.B][bp2.S];
			}
			//printf("|\n");

			//SPEEDUP/ if all blocks are not blocked skip!!
			/*
			if(i > 2 && !BA->blocked && !BN->blocked && !BNN->blocked && !BNNN->blocked){
				if(p > 0 && !BP->blocked){
					return;
				}else if(p == 0){
					return;
				}
			}*/

			/*Train ID following*/
				if(!BA->blocked && BA->train != 0){
					//Reset
					//printf("Reset");
					BA->train = 0;
				}
				if(i > 0 && BN->train && !BA->train && BA->blocked && !BP->blocked){
					//printf("Reverse\n");
					BA->dir ^= 0b100;
					//REVERSED
				}else if(p > 0 && i > 0 && BN->train == 0 && BP->train == 0 && BA->train == 0 && BA->blocked){
						//NEW TRAIN
						BA->train = ++bTrain;
						req_train(bTrain,BA->Adr);
				}
				if(p > 0 && BP->blocked && BA->blocked && BA->train == 0){
					BA->train = BP->train;
				}
				if(i > 0 && BN->train == 0 && BN->blocked && BA->blocked){
					BN->train = BA->train;
				}
				if(i > 1 && BNN->train == 0 && BNN->blocked && BN->blocked){
					BNN->train = BN->train;
				}
			/**/
			/**/
			/*Check switch*/
				//
				if(i > 0 && (NADR(BN->Adr).type == 's' || NADR(BN->Adr).type == 'S' || NADR(BN->Adr).type == 'm' || NADR(BN->Adr).type == 'M') && BA->blocked){
					if(!check_Switch(BN->Adr,0)){
						if(free_Switch(BN->Adr,0)){
							if(i < 2){
								BNN = Next(BN->Adr,1);
								BNNN = Next(BN->Adr,2);
								i = 3;
							}
							if(BNN->state != RESERVED){
								if(Adr_Comp(NADR(BN->Adr),BN->Adr)){
									change_block_state(BN,RESERVED);
									if(i > 1 && BNN->Adr.S == 0){
										change_block_state(BNN,RESERVED);
									}
								}else{
									change_block_state(BNN,RESERVED);
									if(i > 2 && BNNN->Adr.S == 0){
										change_block_state(BNNN,RESERVED);
									}
								}
							}
						}
					}else{
						if(BNN->state != RESERVED){
							if(Adr_Comp(NADR(BN->Adr),BN->Adr)){
								//printf("asdfjkkkkkkkk\n");
								change_block_state(BN,RESERVED);
								if(i > 1 && BNN->Adr.S == 0){
									change_block_state(BNN,RESERVED);
								}
							}else{
								change_block_state(BNN,RESERVED);
								if(i > 2 && BNNN->Adr.S == 0){
									change_block_state(BNNN,RESERVED);
								}
							}
						}
					}
				}
				else if(i > 0 && p > 0 && (NADR(BA->Adr).type == 's' || NADR(BA->Adr).type == 'S' || NADR(BA->Adr).type == 'm' || NADR(BA->Adr).type == 'M') && BP->blocked){
					if(!check_Switch(BA->Adr,0)){
						if(free_Switch(BA->Adr,0)){
							if(i < 2){
								BN = Next(BA->Adr,1);
								BNN = Next(BA->Adr,2);
								i = 2;
							}
							if(BN->state != RESERVED){
								if(Adr_Comp(NADR(BA->Adr),BA->Adr)){
									change_block_state(BA,RESERVED);
									if(i > 1 && BN->Adr.S == 0){
										change_block_state(BN,RESERVED);
									}
								}else{
									change_block_state(BN,RESERVED);
									if(i > 2 && BNN->Adr.S == 0){
										change_block_state(BNN,RESERVED);
									}
								}
							}
						}
					}else{
						if(BN->state != RESERVED){
							if(Adr_Comp(NADR(BA->Adr),BA->Adr)){
								//printf("asdfjkkkkkkkk\n");
								change_block_state(BA,RESERVED);
								if(i > 1 && BN->Adr.S == 0){
									change_block_state(BN,RESERVED);
								}
							}else{
								change_block_state(BN,RESERVED);
								if(i > 2 && BNN->Adr.S == 0){
									change_block_state(BNN,RESERVED);
								}
							}
						}
					}
				}
			/**/
			/**/
			/**/
			/*Reverse block after one or two zero-blocks*/
				//If Next block is a Switch-block and the block after that is a normal block and that block is in reversed direction
				if(i > 1 && BA->blocked && !dir_Comp(BA,BNN) && BN->Adr.S == 0 && BNN->Adr.S != 0){
					printf("%i:%i:%i:%i <-> %i:%i:%i:%i\t",BA->Adr.M,BA->Adr.B,BA->Adr.S,BA->dir,BNN->Adr.M,BNN->Adr.B,BNN->Adr.S,BNN->dir);
					printf("Reverse in advance 1\n");
					if(BNN->Adr.S == 1){
						for(int a = 1;a<MAX_Segments;a++){
							if(blocks[BNN->Adr.M][BNN->Adr.B][a] != NULL){
								if(blocks[BNN->Adr.M][BNN->Adr.B][a]->blocked){
									break;
								}
								blocks[BNN->Adr.M][BNN->Adr.B][a]->dir ^= 0b100;
							}
						}
					}else{
						//Reverse whole block
						for(int a = MAX_Segments;0<a;a--){
							if(blocks[BNN->Adr.M][BNN->Adr.B][a] != NULL){
								//Break if there is a Train in the Block
								if(blocks[BNN->Adr.M][BNN->Adr.B][a]->blocked){
									break;
								}
								blocks[BNN->Adr.M][BNN->Adr.B][a]->dir ^= 0b100;
							}
						}
					}
				}
				//If the next block and the block after it are both a Switch-block and the block after that is a normal block and that block is in reversed direction
				if(i > 2 && BA->blocked && !dir_Comp(BA,BNNN) && BN->Adr.S == 0 && BNN->Adr.S == 0 && BNNN->Adr.S != 0){
					printf("%i:%i:%i:%i <-> %i:%i:%i:%i\t",BA->Adr.M,BA->Adr.B,BA->Adr.S,BA->dir,BNNN->Adr.M,BNNN->Adr.B,BNNN->Adr.S,BNNN->dir);
					printf("Reverse in advance 2\n");
					if(BNNN->Adr.S == 1){
						for(int a = 1;a<MAX_Segments;a++){
							if(blocks[BNNN->Adr.M][BNNN->Adr.B][a] != NULL){
								if(blocks[BNNN->Adr.M][BNNN->Adr.B][a]->blocked){
									break;
								}
								blocks[BNNN->Adr.M][BNNN->Adr.B][a]->dir ^= 0b100;
							}
						}
					}else{
						//Reverse whole block
						for(int a = MAX_Segments;0<a;a--){
							if(blocks[BNNN->Adr.M][BNNN->Adr.B][a] != NULL){
								//Break if there is a Train in the Block
								if(blocks[BNNN->Adr.M][BNNN->Adr.B][a]->blocked){
									break;
								}
								blocks[BNNN->Adr.M][BNNN->Adr.B][a]->dir ^= 0b100;
							}
						}
					}
				}
				//If the next block is reversed, and not blocked
				if(i > 0 && BA->blocked && !dir_Comp(BA,BN) && BN->Adr.S != 0 && !BN->blocked){
					printf("%i:%i:%i:%i <-> %i:%i:%i:%i\t",BA->Adr.M,BA->Adr.B,BA->Adr.S,BA->dir,BN->Adr.M,BN->Adr.B,BN->Adr.S,BN->dir);
					printf("Reverse next\n");
					BN->dir ^= 0b100;
				}
			/**/
			/**/
			/*State coloring*/
				//Self
				if(i > 0 && p > 1 && !dir_Comp(BA,BN) && BNN->blocked && BPP->blocked){
					change_block_state(BA,RED);
					change_block_state(BN,AMBER);
					change_block_state(BP,AMBER);
				}
				else if(i > 0 && p > 1 && !dir_Comp(BA,BN) && !BNN->blocked && !BPP->blocked){
					change_block_state(BA,GREEN);
				}
				else if(i > 1 && !dir_Comp(BA,BNN) && BN->Adr.S == 0 && !BNN->blocked && BNN->state == GREEN){
					change_block_state(BA,GREEN);
				}
				else if(i > 0 && BN->blocked){
					change_block_state(BA,RED);
				}
				else if(i > 1 && dir_Comp(BA,BNN) && BNN->blocked && !BN->blocked){
					change_block_state(BA,AMBER);
				}
				else if(i > 2 && dir_Comp(BA,BNN) && !BNN->blocked && !BN->blocked){
					if(BA->state != RESERVED){
						change_block_state(BA,GREEN);
					}
				}
				else if(i == 0){
					change_block_state(BA,AMBER);
				}
				else if(i == 1){
					change_block_state(BA,GREEN);
				}

				//Next
				if(i > 2 && dir_Comp(BA,BNNN) && BNN->blocked && !BN->blocked){
					change_block_state(BN,RED);
				}
				else if(i > 2 && dir_Comp(BA,BNNN) && BNNN->blocked && !BNN->blocked){
					change_block_state(BN,AMBER);
				}
				else if(i > 2 && dir_Comp(BA,BNNN) && !BN->blocked && !BNN->blocked && !BNNN->blocked){
					if(BN->state != RESERVED){
						change_block_state(BN,GREEN);
					}
				}

				//Next Next
				if(i > 2 && dir_Comp(BA,BNNN) && BNNN->blocked && !BNN->blocked){
					change_block_state(BNN,RED);
				}

				//Prev
				if(i > 2 && p > 0 && BP->blocked && BNNN->blocked && !dir_Comp(BP,BNN)){
					change_block_state(BNN,AMBER);
					change_block_state(BN,RED);
					change_block_state(BA,AMBER);
					debug = 1;
					//printf("SPECIAL\n");
				}
				else if(p > 0 && i > 0 && dir_Comp(BN,BP) && !BA->blocked && BN->blocked){
					change_block_state(BP,AMBER);
				}
				else if(p > 0 && i > 0 && dir_Comp(BN,BP) && !BA->blocked && !BN->blocked){
					if(BP->state != RESERVED){
						change_block_state(BP,GREEN);
					}
				}
			/**/
			/**/
			/**/
			/**/
			/*Signals*/
			struct adr Z = {4,10,4,'R'};

			if((BA->signals & 128) == 128){
				//Wrong Switch
				//printf("Signal at %i:%i:%i\n",BA->Adr.M,BA->Adr.B,BA->Adr.S);
				if((BA->dir == 0 || BA->dir == 2 || BA->dir == 5) && !check_Switch(BA->Adr,0) || BA->dir == 1){
					set_signal(BA,4,RED);
					//printf("%i:%i:%i\tRed signal L1\n",BA->Adr.M,BA->Adr.B,BA->Adr.S);
				}

				if(BA->dir != 1 && check_Switch(BA->Adr,0) && i > 0){
					//Next block is RED/Blocked
					if(BN->blocked || BN->state == RED || BN->state == PARKED){
						set_signal(BA,4,RED);
					}else if(BN->state == AMBER){	//Next block AMBER
						set_signal(BA,4,AMBER);
					}else{ // //Next block AMBER	if(BN->state == GREEN)
						set_signal(BA,4,GREEN);
					}
				}
			}

			if((BA->signals & 0x8) == 8){
				//printf("Signal at %i:%i:%i\t0x%x\n",BA->Adr.M,BA->Adr.B,BA->Adr.S,BA->signals);
				if((BA->dir == 1 || BA->dir == 4 || BA->dir == 6) && !check_Switch(BA->Adr,0) || BA->dir == 5){
					set_signal(BA,0,RED);
					//printf("%i:%i:%i\tRed signal R2\n",BA->Adr.M,BA->Adr.B,BA->Adr.S);
				}

				if(BA->dir != 5 && check_Switch(BA->Adr,0) && i > 0){
					//Next block is RED/Blocked
					if(BN->blocked || BN->state == RED || BN->state == PARKED){
						set_signal(BA,0,RED);
					}else if(BN->state == AMBER){	//Next block AMBER
						set_signal(BA,0,AMBER);
					}else{ // //Next block AMBER	if(BN->state == GREEN)
						set_signal(BA,0,GREEN);
					}
				}
			}
			/**/
			/**/
			/*TRAIN control*/
				//Check if current and next block are blocked, and have different trainIDs
				if(BA->blocked && BN->blocked && BA->train != BN->train){
					//Kill train
					printf("COLLISION PREVENTION\n\t");
					train_stop(train_link[BA->train]);
				}
				//Check if next block is a RED block
				if(((BA->blocked && !BN->blocked && BN->state == RED) || (i == 0 && BA->blocked)) && train_link[BA->train]->timer != 1){
					//Fire stop timer
					printf("NEXT SIGNAL: RED\n\tSTOP TRAIN:");
					train_signal(BA,train_link[BA->train],RED);
				}
				//Check if next block is a AMBER block
				if(((BA->blocked && !BN->blocked && BN->state == AMBER) || (i == 1 && BA->blocked && !BN->blocked)) && train_link[BA->train]->timer != 1){
					//Fire slowdown timer
					printf("NEXT SIGNAL: AMBER\n\tSLOWDOWN TRAIN:\t");
					train_signal(BA,train_link[BA->train],AMBER);
				}

				//If the next 2 blocks are free, accelerate
				/*
				//If the next block has a higher speed limit than the current
				if(i > 0 && !BN->blocked && BA->train != 0 && train_link[BA->train] != NULL && train_link[BA->train]->timer != 2 && train_link[BA->train]->timer != 1){
					if((BN->state == GREEN || BN->state == RESERVED) && train_link[BA->train]->cur_speed < BA->max_speed && BN->max_speed >= BA->max_speed){
						printf("Next block has a higher speed limit (%i > %i)",BN->max_speed,BA->max_speed);
						train_speed(BA,train_link[BA->train],BA->max_speed);
					}
				}

				//If the next block has a lower speed limit than the current
				if(BA->train != 0 && train_link[BA->train] != NULL && train_link[BA->train]->timer != 2){
					if((BN->state == GREEN || BN->state == RESERVED) && train_link[BA->train]->cur_speed > BN->max_speed && BN->Adr.S != 0){
						printf("Next block has a lower speed limit");
						train_speed(BN,train_link[BA->train],BN->max_speed);
					}else if(i > 1 && BN->Adr.S == 0 && BNN->Adr.S != 0 && (BNN->state == GREEN || BNN->state == RESERVED) && train_link[BA->train]->cur_speed > BNN->max_speed){
						printf("Block after Switches has a lower speed limit");
						train_speed(BNN,train_link[BA->train],BNN->max_speed);
					}else if(train_link[BA->train]->cur_speed != BN->max_speed && BN->Adr.S != 0){
						printf("%i <= %i\n",train_link[BA->train]->cur_speed,BN->max_speed && BN->Adr.S != 0);
					}
				}
			/**/
			/**/
			/*Debug info*/
				if(BA->train != 0){
					//printf("ID: %i\t%i:%i:%i\n",BA->train,BA->Adr.M,BA->Adr.B,BA->Adr.S);
				}
				if(debug){
					if(p > 0){
						printf("\t\tP %i:%i:%i;B:%i\t",BP->Adr.M,BP->Adr.B,BP->Adr.S,BP->blocked);
					}else{
						printf("\t\t          \t");
					}
					printf("A%i %i:%i:%i;T%iD%i",i,adr.M,adr.B,adr.S,BA->train,BA->dir);
					if(BA->blocked){
						printf("B");
					}
					printf("\t");
					if(i > 0){
						printf("N %i:%i:%i;B%iDC%i\t",BN->Adr.M,BN->Adr.B,BN->Adr.S,BN->blocked,dir_Comp(BA,BN));
					}
					if(i > 1){
						printf("NN %i:%i:%i;B%iDC%i\t",BNN->Adr.M,BNN->Adr.B,BNN->Adr.S,BNN->blocked,dir_Comp(BA,BNN));
					}
					if(i > 2){
						printf("NNN %i:%i:%i;B%iDC%i\t",BNNN->Adr.M,BNNN->Adr.B,BNNN->Adr.S,BNNN->blocked,dir_Comp(BA,BNNN));
					}
					//if(i == 0){
						printf("\n");
					//}
				}
		}
	}
}
/*
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
*/
void *do_Magic(){
	while(!stop){
		//printf("\n\n\n");
		clock_t t;
		t = clock();
		pthread_mutex_lock(&mutex_lockA);
		for(int i = 0;i<Adress;i++){
			//printf("%i: %i:%i:%i\n",i,Adresses[i].M,Adresses[i].B,Adresses[i].S);
			procces(Adresses[i],0);
		}
		JSON();
		pthread_mutex_unlock(&mutex_lockA);
		t = clock() - t;
		//printf ("It took me %d clicks (%f seconds).\n",t,((float)t)/CLOCKS_PER_SEC);
		//printf("\n\n\n\n\n\n");

		//FILE *data;
		//data = fopen("data.txt", "a");
		//fprintf(data,"%d\n",t);
		//fclose(data);

		usleep(1000000);
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

void *clear_timers(){
	while(!stop){
		for(int i = 0;i<MAX_TIMERS;i++){
			if(timers[i] == 2){
				pthread_join(timer_thread[i], NULL);
				timers[i] = 0;
				printf("Reset time %i\n",i);
			}
		}
		usleep(10000);
	}
}

void main(){
	setbuf(stdout,NULL);
	setbuf(stderr,NULL);

	pthread_t thread_web_server;
	pthread_create(&thread_web_server, NULL, web_server, NULL);

	//Define empty block
	blocks[0][0][0] = C_Seg(C_AdrT(0,0,0,'e'),3);
	blocks[0][0][0]->NAdr.type = 'e';
	blocks[0][0][0]->PAdr.type = 'e';

	//Initialising two link object with empty blocks
	struct link LINK;
	struct link LINK2;
	LINK.Adr1 = END_BL;
	LINK.Adr2 = END_BL;
	LINK.Adr3 = END_BL;
	LINK.Adr4 = END_BL;

	int setup[MAX_Modules] = {1,8,4,12,2,0};
	int setup2[5] = {6,7,0};

	for(int i = 0;i<5;i++){
		LINK = Modules(setup[i],LINK);
		//if(!Adr_Comp(LINK.Adr3,END_BL)){
		//	LINK2.Adr1 = LINK.Adr3;
		//	LINK2.Adr2 = LINK.Adr4;
		//}
	}
	/*
	for(int i = 0;i<2;i++){
		LINK2 = Modules(setup2[i],LINK2);
	}*/
	setup_JSON(setup,setup2,5,0);

	init_trains();

	//blocks[11][1][1]->blocked = 1;
	//throw_switch(Switch[5][5][1]);
	//blocks[5][5][0]->state = RESERVED;
	//procces(Adresses[75],1);

	//Done with setup when there is at least one client

	if(connected_clients == 0){
		printf("\n\nWaiting until a client connects\n");
	}
	while(connected_clients == 0){
		usleep(1000000);
	}
	initialise = 0;

	usleep(400000);

	pthread_t tid[10];
	//throw_switch(Switch[5][2][1]);
	//throw_switch(Switch[6][3][1]);


	//printf("Create clear thread\n");
	//pthread_create(&tid[0], NULL, clear_timers, NULL);
	/*
	printf("Create timer threads\n");
	for(int j = 0;j<100;j++){
		if(j == 3){
			status_add(4,"[0,5,2,3,6,3,4,6]");
		}else if(j == 47){
			status_rem(0,4);
		}
		printf("J:%i\t",j);
		create_timer();
		usleep(75000);
	}
	int i = 0;
	int k = 0;
	while(1){
		if(timers[i] != 0){
			k = 1;
		}
		i++;
		if(i==MAX_TIMERS){
			if(k == 0){
				break;
			}
			k = 0;
			i = 0;
		}
	}
	stop = 1;
	pthread_join(tid[0],NULL);
	pthread_join(tid[1],NULL);
	*/



	//usleep(5000000);

	printf("Creating Threads\n");
	pthread_create(&tid[0], NULL, do_Magic, NULL);
	pthread_create(&tid[1], NULL, clear_timers, NULL);
	blocks[8][3][1]->blocked = 1;
	blocks[8][3][1]->change  = 1;
	JSON();
	usleep(5000000);
	//procces(Adresses[63],1);
	//JSON();
	//throw_switch(Switch[4][4][1]);
	//pthread_create(&tid[1], NULL, STOP_FUNC, NULL);
	//StartAdr = C_Adr(1,4,1);
	//blocks[1][4][1]->blocked = 1;
	pthread_create(&tid[3], NULL, TRAIN_SIMA, NULL);
	//StartAdr = C_Adr(8,5,1);
	//blocks[8][6][1]->blocked = 1;
	//blocks[7][4][5]->blocked = 1;

  //pthread_create(&tid[4], NULL, TRAIN_SIMB, NULL);
	//blocks[7][3][5]->blocked = 1;
	/*usleep(22000000);

  //pthread_create(&tid[5], NULL, TRAIN_SIMC, NULL);
	//blocks[7][2][5]->blocked = 1;
	usleep(22000000);

  //pthread_create(&tid[6], NULL, TRAIN_SIMD, NULL);

	usleep(22000000);

	new_message(1,"[]");

	usleep(22000000);

	new_message(3,"[]");

	pthread_mutex_lock(&mutex_lockB);
	//throw_switch(Switch[4][6][1]);
	//throw_switch(Switch[4][6][2]);
	pthread_mutex_unlock(&mutex_lockB);
	*/
	pthread_join(tid[0],NULL);

	pthread_join(thread_web_server,NULL);
	//procces(C_Adr(6,2,1),1);

	printf("STOPPED");
	/**/
	pthread_exit(NULL);
	//do_Magic();
}
