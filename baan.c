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


#define MAX_SW 16
#define MAX_Segments 8
#define MAX_Blocks 32
#define MAX_Modules 16
#define MAX_A MAX_Modules*MAX_Blocks*MAX_Segments
#define MAX_TRAINS 20
#define MAX_ROUTE 20
#define MAX_TIMERS 50
#define MAX_WEB_CLIENTS 20

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
	char max_speed;		//0x00 <-> 0xFF
	char state;
	char dir;		//0xAABB, A = Current travel direction, B = Travel direction
	char length;
	char train;		 //0x00 <-> 0xFF
	char blocked;
	char change; //If block changed state;
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
	char s_length;
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

int Adr_Comp(struct adr A,struct adr B){
	if(A.M == B.M && A.B == B.B && A.S == B.S){
		return 1;
	}else{
		return 0;
	}
}

int dir_Comp(struct Seg *A,struct Seg *B){
	if((A->dir == 0 && (B->dir == 1 || B->dir == 0b100)) || ((A->dir == 1 || A->dir == 0b100) && B->dir == 0)){
		if(B->Adr.S == 0){
			return 1;
		}else{
			return 0;
		}
	}else{
		return 1;
	}
}

#include "./src/modules.c"
#include "./src/status.c"
#include "./src/trainlist.c"

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
	int a = 0;
	//printf("\n%i\n",i);
	int dir = blocks[Adr.M][Adr.B][Adr.S]->dir;
	I:{};

	if(dir == 0 || dir == 2 || dir == 5){
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
	return blocks[Adr.M][Adr.B][Adr.S];
}

struct Seg * Prev(struct adr Adr,int i){
	struct adr PAdr,SPAdr;
	int a = 0;
	//printf("\n%i\n",i);
	I:{};

	PAdr = PADR(Adr);

	if(i == 0){
		//printf("\n");
		return blocks[Adr.M][Adr.B][Adr.S];
	}
	//usleep(20000);
	//printf("i:%i",i);
	//printf("\t%i:%i:%i\ttype:%c\n",Adr.M,Adr.B,Adr.S,Adr.type);
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
	return blocks[Adr.M][Adr.B][Adr.S];
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
	}else if(NAdr.type == 'M'){
		int s = Moduls[NAdr.M][NAdr.B][NAdr.S]->state;
		NAdr = Moduls[NAdr.M][NAdr.B][NAdr.S]->mAdr[s];
	}else if(NAdr.type == 'm'){
		NAdr = Moduls[NAdr.M][NAdr.B][NAdr.S]->MAdr[Moduls[NAdr.M][NAdr.B][NAdr.S]->state];
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
	R:{};
	//printf("NAdr: %i:%i:%i\n",NAdr.M,NAdr.B,NAdr.S);
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
		if(Div.M == adr.M && Div.B == adr.B && Div.S == adr.S){
			if(Switch[NAdr.M][NAdr.B][NAdr.S]->state == 0){
				pthread_mutex_lock(&mutex_lockB);
				Switch[NAdr.M][NAdr.B][NAdr.S]->state = 1;
				pthread_mutex_unlock(&mutex_lockB);
			}
		}else if(Str.M == adr.M && Str.B == adr.B && Str.S == adr.S){
			if(Switch[NAdr.M][NAdr.B][NAdr.S]->state == 1){
				pthread_mutex_lock(&mutex_lockB);
				Switch[NAdr.M][NAdr.B][NAdr.S]->state = 0;
				pthread_mutex_unlock(&mutex_lockB);
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
	pthread_mutex_lock(&mutex_lockB);
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

			fprintf(fp,"[%i,%i,%i,%i,%i,%i]",A.M,A.B,A.S,Switch[A.M][A.B][A.S]->state,2,Switch[A.M][A.B][A.S]->len);
		}
	}

	for(int i = 0;i<Adress;i++){

		struct adr A = Adresses[i];
		if(A.type == 'M'){
			if(p != 0){
				fprintf(fp,",");
			}else{
				p = 1;
			}

			fprintf(fp,"[%i,%i,%i,%i,%i,%i]",A.M,A.B,A.S,Moduls[A.M][A.B][A.S]->state,Moduls[A.M][A.B][A.S]->length,Moduls[A.M][A.B][A.S]->s_length);
		}
	}
	fprintf(fp,"], \"Message\" : [");

	p = 0;

	for(int i = 0;i<20;i++){
		if(status_st[i] != NULL){
			if(p == 1){
				fprintf(fp,",");
			}else{
				p = 1;
			}
			//printf("Write available at i:%i\n",i);

			//printf("Write [%i,[%s]]",status_st[i]->type,status_st[i]->data);
			fprintf(fp,"[%i,%i,%s]",status_st[i]->type,status_st[i]->id,status_st[i]->data);

		}
	}

	fprintf(fp,"]}");
	fclose(fp);
	pthread_mutex_unlock(&mutex_lockB);
}

char * JSON2(){
	pthread_mutex_lock(&mutex_lockB);
	char buf[4096];
	memset(buf,0,4096);

	sprintf(buf,"%s{\"M\" : [",buf);
	int p = 0;


	for(int i = 0;i<Adress;i++){

		struct adr A = Adresses[i];
		if(A.type == 'R' && blocks[A.M][A.B][A.S]->change == 1){
			if(p != 0){
				sprintf(buf,"%s,",buf);
			}else{
				p = 1;
			}

			sprintf(buf,"%s[%i,%i,%i,%i,%i,%i,%i]",buf,A.M,A.B,A.S,blocks[A.M][A.B][A.S]->dir,blocks[A.M][A.B][A.S]->blocked,blocks[A.M][A.B][A.S]->state,blocks[A.M][A.B][A.S]->train);
		}
	}

	sprintf(buf,"%s],\"SW\" : [",buf);

	p = 0;

	for(int i = 0;i<Adress;i++){

		struct adr A = Adresses[i];
		if(A.type == 'S'){
			if(p != 0){
				sprintf(buf,"%s,",buf);
			}else{
				p = 1;
			}

			sprintf(buf,"%s[%i,%i,%i,%i,%i,%i]",buf,A.M,A.B,A.S,Switch[A.M][A.B][A.S]->state,2,Switch[A.M][A.B][A.S]->len);
		}
	}

	for(int i = 0;i<Adress;i++){

		struct adr A = Adresses[i];
		if(A.type == 'M'){
			if(p != 0){
				sprintf(buf,"%s,",buf);
			}else{
				p = 1;
			}

			sprintf(buf,"%s[%i,%i,%i,%i,%i,%i]",buf,A.M,A.B,A.S,Moduls[A.M][A.B][A.S]->state,Moduls[A.M][A.B][A.S]->length,Moduls[A.M][A.B][A.S]->s_length);
		}
	}
	sprintf(buf,"%s], \"Message\" : [",buf);

	p = 0;

	for(int i = 0;i<20;i++){
		if(status_st[i] != NULL){
			if(p == 1){
				sprintf(buf,"%s,",buf);
			}else{
				p = 1;
			}
			//printf("Write available at i:%i\n",i);

			//printf("Write [%i,[%s]]",status_st[i]->type,status_st[i]->data);
			sprintf(buf,"%s[%i,%i,%s]",buf,status_st[i]->type,status_st[i]->id,status_st[i]->data);

		}
	}

	sprintf(buf,"%s]}",buf);
	pthread_mutex_unlock(&mutex_lockB);
	return buf;
}

struct timer_thread_data{
   int  thread_id;
   int  t;
};

struct timer_thread_data a[MAX_TIMERS];

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
			if(debug){
				struct Seg *BA = blocks[adr.M][adr.B][adr.S];
				printf("\t\t          \tA  %i:%i:%i;T%iR%i",BA->Adr.M,BA->Adr.B,BA->Adr.S,BA->train,BA->dir);
				if(BA->blocked){
					printf("B");
				}
				printf("\n");
			}
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
				}else if(NADR(B.Adr).type == 's'){
					//printf("Check_switch\n");
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
			if(bp.S != 0 && p == 1 && check_Switch(bp,1)){
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
			if(i > 1){
				BNN = blocks[bl[2].M][bl[2].B][bl[2].S];
			}
			if(i > 2){
				BNNN = blocks[bl[3].M][bl[3].B][bl[3].S];
			}
			if(p > 0){
				BP = blocks[bp.M][bp.B][bp.S];
			}
			if(p > 1){
				BPP = blocks[bp2.M][bp2.B][bp2.S];
			}
			if(!BA->blocked){
				//debug = 0;
			}

			//SPEEDUP/ if all blocks are not blocked skip!!
			/*
			if(i > 2 && !BA->blocked && !BN->blocked && !BNN->blocked && !BNNN->blocked){
				if(p > 0 && !BP->blocked){
					return;
				}else if(p == 0){
					return;
				}
			}*/

			//Train ID following
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
			//Check switch
				if(i > 0 && (NADR(BN->Adr).type == 's' || NADR(BN->Adr).type == 'S') && BA->blocked && BN->state != RESERVED){
					if(!check_Switch(BN->Adr,0)){
						free_Switch(BN->Adr,0);
						if(i < 2){
							BNN = Next(BN->Adr,1);
							BNNN = Next(BN->Adr,2);
							i++;i++;
						}
					}
					if(Adr_Comp(NADR(BN->Adr),BN->Adr)){
						printf("asdfjkkkkkkkk\n");
						BN->state = RESERVED;
						if(i > 1 && BNN->Adr.S == 0){
							BNN->state = RESERVED;
						}
					}else{
						BNN->state = RESERVED;
						if(i > 2 && BNNN->Adr.S == 0){
							BNNN->state = RESERVED;
						}
					}
				}else	if(i > 0 && p > 0 && (NADR(BA->Adr).type == 's' || NADR(BA->Adr).type == 'S') && BP->blocked && BN->state != RESERVED){
					if(!check_Switch(BA->Adr,0)){
						free_Switch(BA->Adr,0);
						if(i < 1){
							BN = Next(BA->Adr,1);
							BNN = Next(BA->Adr,2);
							i++;
						}
					}
					if(Adr_Comp(NADR(BA->Adr),BA->Adr)){
						BA->state = RESERVED;
						if(i > 0 && BN->Adr.S == 0){
							BN->state = RESERVED;
						}
					}else{
						BN->state = RESERVED;
						if(BNN->Adr.S == 0){
							BNN->state = RESERVED;
						}
					}
				}

			//Reverse block after one or two zero-blocks
				if(i > 2 && BA->blocked && !dir_Comp(BA,BNN) && BN->Adr.S == 0 && BNN->Adr.S != 0){
					//printf("%i:%i:%i:%i <-> %i:%i:%i:%i\t",BA->Adr.M,BA->Adr.B,BA->Adr.S,BA->dir,BNN->Adr.M,BNN->Adr.B,BNN->Adr.S,BNN->dir);
					//printf("Reverse in advance 1");
					BNN->dir ^= 0b100;
				}
				if(i > 2 && BA->blocked && !dir_Comp(BA,BNNN) && BN->Adr.S == 0 && BNN->Adr.S == 0 && BNNN->Adr.S != 0){
					//printf("%i:%i:%i:%i <-> %i:%i:%i:%i\t",BA->Adr.M,BA->Adr.B,BA->Adr.S,BA->dir,BNNN->Adr.M,BNNN->Adr.B,BNNN->Adr.S,BNNN->dir);
					//printf("Reverse in advance 2");
					BNNN->dir ^= 0b100;
				}
			//Reverse block
				if(i > 0 && BA->blocked && !dir_Comp(BA,BN) && BN->Adr.S != 0){
					//printf("%i:%i:%i:%i <-> %i:%i:%i:%i\t",BA->Adr.M,BA->Adr.B,BA->Adr.S,BA->dir,BN->Adr.M,BN->Adr.B,BN->Adr.S,BN->dir);
					//printf("Reverse next");
					BN->dir ^= 0b100;
				}

			//State coloring
				//Self
				if(i > 0 && p > 1 && !dir_Comp(BA,BN) && BNN->blocked && BPP->blocked){
					BA->state = RED;
					BN->state = AMBER;
					BP->state = AMBER;
				}else if(i > 0 && p > 1 && !dir_Comp(BA,BN) && !BNN->blocked && !BPP->blocked){
					BA->state = GREEN;
				}else if(i> 0 && BN->blocked){
					BA->state = RED;
				}else if(i > 1 && dir_Comp(BA,BNN) && BNN->blocked && !BN->blocked){
					BA->state = AMBER;
				}else if(i > 2 && dir_Comp(BA,BNN) && !BNN->blocked && !BN->blocked){
					if(BA->state != RESERVED){
						BA->state = GREEN;
					}
				}

				//Next
				if(i > 1 && dir_Comp(BA,BNNN) && BNN->blocked && !BN->blocked){
					BN->state = RED;
				}else if(i > 2 && dir_Comp(BA,BNNN) && BNNN->blocked && !BNN->blocked){
					BN->state = AMBER;
				}else if(i > 2 && dir_Comp(BA,BNNN) && !BN->blocked && !BNN->blocked && !BNNN->blocked){
					if(BN->state != RESERVED){
						BN->state = GREEN;
					}
				}

				//Next Next
				if(i > 2 && dir_Comp(BA,BNNN) && BNNN->blocked && !BNN->blocked){
					BNN->state = RED;
				}

				//Prev
				if(i > 2 && p > 0 && BP->blocked && BNNN->blocked && !dir_Comp(BP,BNN)){
					BNN->state = AMBER;
					BN->state  = RED;
					BA->state  = AMBER;
					debug = 1;
					//printf("SPECIAL\n");
				}else if(p > 0 && i > 0 && dir_Comp(BN,BP) && !BA->blocked && BN->blocked){
					BP->state = AMBER;
				}else if(p > 0 && i > 0 && dir_Comp(BN,BP) && !BA->blocked && !BN->blocked){
					if(BP->state != RESERVED){
						BP->state = GREEN;
					}
				}

				//Debug info
				if(debug){
					if(p > 0){
						printf("\t\tP %i:%i:%i;B:%i\t",BP->Adr.M,BP->Adr.B,BP->Adr.S,BP->blocked);
					}else{
						printf("\t\t          \t");
					}
					printf("A%i %i:%i:%i;T:%i",i,adr.M,adr.B,adr.S,BA->train);
					if(BA->blocked){
						printf("B");
					}
					printf("%i",BA->dir);
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
			if(debug){
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
				if((BA->blocked && !BN->blocked && BA->train != 0 && BN->state == GREEN) || (BN->max_speed > BA->max_speed && BA->blocked && !BN->blocked && BN->state ==GREEN)){
					printf("Speedup to track speed %i km/h, train:%i",BA->max_speed,BA->train);
				}
				if(BN->max_speed < BA->max_speed && BA->blocked && !BN->blocked && BN->state == GREEN){
					printf("Slowdown to track speed %i km/h, train:%i",BA->max_speed,BA->train);
				}
				printf("\n");
			}

			if(BA->train >= 5){
				printf("STPOPPED\n");
				stop = 1;
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
		//printf("Procces loop \t");
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
		printf ("It took me %d clicks (%f seconds).\n",t,((float)t)/CLOCKS_PER_SEC);

		//FILE *data;
		//data = fopen("data.txt", "a");
		//fprintf(data,"%d\n",t);
		//fclose(data);

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

void *clear_timers(){
	while(!stop){
		for(int i = 0;i<MAX_TIMERS;i++){
			if(timers[i] == 2){
				pthread_join(timer_thread[i], NULL);
				timers[i] = 0;
				printf("Reset time %i\n",i);
			}
		}
	}
}

#include "./src/train_sim.c"
#include "./src/Web2.c"

void main(){

	blocks[0][0][0] = C_Seg(C_AdrT(0,0,0,'e'),3);
	blocks[0][0][0]->NAdr.type = 'e';
	blocks[0][0][0]->PAdr.type = 'e';

	struct link LINK;
	struct link LINK2;
	LINK.Adr1 = END_BL;
	LINK.Adr2 = END_BL;
	LINK.Adr3 = END_BL;
	LINK.Adr4 = END_BL;

	int setup[MAX_Modules] = {1,4,8,5,10,9,2,0};
	int setup2[5] = {11,6,7,0};

	for(int i = 0;i<7;i++){
		LINK = Modules(setup[i],LINK);
		if(!Adr_Comp(LINK.Adr3,END_BL)){
			LINK2.Adr1 = LINK.Adr3;
			LINK2.Adr2 = LINK.Adr4;
		}
	}

	for(int i = 0;i<3;i++){
		LINK2 = Modules(setup2[i],LINK2);
	}
	setup_JSON(setup,setup2,7,3);

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

	//printf("Next: %i:%i:%i:%c\n",blocks[4][11][1]->PAdr.M,blocks[4][11][1]->PAdr.B,blocks[4][11][1]->PAdr.S,blocks[4][11][1]->PAdr.type);
	//do_Magic();

	printf("TEST\n");
	//do_Magic();

	//blocks[6][2][1]->blocked = 1;
	blocks[7][4][5]->blocked = 1;
	blocks[7][3][5]->blocked = 1;
	blocks[7][2][5]->blocked = 1;
	//blocks[3][7][1]->blocked = 1;
	//do_Magic();
*/
	pthread_t tid[10];

	printf("Creating Threads");
	Switch[5][2][1]->state = !Switch[5][2][1]->state;
	Switch[6][3][1]->state = !Switch[6][3][1]->state;


//	printf("Create clear thread\n");
//	pthread_create(&tid[0], NULL, clear_timers, NULL);
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

	//pthread_create(&tid[0], NULL, do_Magic, NULL);
	//pthread_create(&tid[1], NULL, status_write, NULL);
	pthread_create(&tid[2], NULL, web_server, NULL);/*
	//pthread_create(&tid[1], NULL, STOP_FUNC, NULL);
	//StartAdr = C_Adr(1,4,1);
	//blocks[1][4][1]->blocked = 1;
	pthread_create(&tid[3], NULL, TRAIN_SIMA, NULL);
	//StartAdr = C_Adr(8,5,1);
	//blocks[8][6][1]->blocked = 1;
	blocks[7][4][5]->blocked = 1;
	usleep(20000000);

  pthread_create(&tid[4], NULL, TRAIN_SIMB, NULL);
	blocks[7][3][5]->blocked = 1;
	usleep(20000000);

  pthread_create(&tid[5], NULL, TRAIN_SIMC, NULL);
	blocks[7][2][5]->blocked = 1;
	usleep(20000000);

  pthread_create(&tid[6], NULL, TRAIN_SIMD, NULL);

	usleep(20000000);

	status_add(1,"[]");

	usleep(20000000);

	status_add(2,"[]");

	pthread_mutex_lock(&mutex_lockB);
	Switch[4][6][1]->state = !Switch[4][6][1]->state;
	Switch[4][6][2]->state = !Switch[4][6][1]->state;
	pthread_mutex_unlock(&mutex_lockB);

	pthread_join(tid[0],NULL);
	//pthread_join(tid[1],NULL);
	pthread_join(tid[2],NULL);
	//pthread_join(tid[3],NULL);
	pthread_join(tid[4],NULL);

	//procces(C_Adr(6,2,1),1);

	printf("STOPPED");
	/**/
	pthread_exit(NULL);
	//do_Magic();
}
