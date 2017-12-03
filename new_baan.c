#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#define MAX_SW 20
#define MAX_Segments 8
#define MAX_Blocks 32
#define MAX_Modules 16
#define MAX_A MAX_Modules*MAX_Blocks*MAX_Segments

#define GREEN 0
#define AMBER 1
#define RED 2
#define BLOCKED 3
#define GHOST 4
#define RESERVED 5
#define UNKNOWN 6

#define Switch_Front 0x2000	
#define Switch_Back  0x4000

#define Arr_Count(array) (sizeof(array)/sizeof((array)[0])) 

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
	char dir;		//0xAB, A = Travel direction, B = Current travel direction
	char lenght;
	char train;		 //0x00 <-> 0xFF
	char blocked;
};

struct Swi{
	struct adr Adr;

	struct adr Div;
	struct adr Str;
	struct adr App;
	char state;
};

struct adr Adresses[MAX_A] = {};
int Adress = 0;
struct Seg *blocks[MAX_Modules][MAX_Blocks][MAX_Segments] = {};
struct Swi *Switch[MAX_Modules][MAX_Blocks][MAX_Segments] = {};

struct adr C_Adr(char M,char B,char S){
	struct adr Z;

	Z.M = M;
	Z.B = B;
	Z.S = S;
	Z.type = 'R';
	
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

struct Seg * C_Seg(struct adr Adr, char state){
	struct Seg *Z = (struct Seg*)malloc(sizeof(struct Seg));

	Adr.type = 'R';

	Z->Adr = Adr;
	Z->NAdr = C_Adr(0,0,0);
	Z->PAdr = C_Adr(0,0,0);
	Z->max_speed = 0;
	Z->state = state;
	Z->train = 0x00;

	printf("A Segment is created at %i:%i:%i\tAdr:%i\n",Adr.M,Adr.B,Adr.S,Adress);
	blocks[Adr.M][Adr.B][Adr.S] = Z;

	Adresses[Adress] = Adr;
	Adress++;

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
	Z->lenght = len;
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

	//return Z;
	if(blocks[Adr.M][Adr.B][1] == NULL && blocks[Adr.M][Adr.B][0] == NULL){
		C_Seg(C_Adr(Adr.M,Adr.B,0),0);
	}
	printf("A Switch  is created at %i:%i:%i\tAdr:%i\n",Adr.M,Adr.B,Adr.S,Adress);
	Switch[Adr.M][Adr.B][Adr.S] = Z;

	Adresses[Adress] = Adr;
	Adress++;
}

#include "modules.c"

struct Seg * Next(struct adr Adr,int i){
	int a = 0;
	struct adr NAdr = Adr;
	printf("<%i:%i:%i\n",Adr.M,Adr.B,Adr.S);

	I:{};

	int dir = blocks[Adr.M][Adr.B][Adr.S]->dir;

	if(dir == 0 || dir == 2){
		NAdr = blocks[Adr.M][Adr.B][Adr.S]->NAdr;
	}else{
		NAdr = blocks[Adr.M][Adr.B][Adr.S]->PAdr;
	}

	//Adr = NAdr;
	R:{};

	if((NAdr.type) == 'R'){
		i--;
		//	printf("Just a normal rail\n");
	}else if(NAdr.type == 'S'){
		a++;
		if(blocks[NAdr.M][NAdr.B][0] != NULL && i == 1){
			NAdr = blocks[NAdr.M][NAdr.B][0]->Adr;
		}else{
			if(Switch[NAdr.M][NAdr.B][NAdr.S]->state == 0){ //Straight?
				NAdr = Switch[NAdr.M][NAdr.B][NAdr.S]->Str;
			}else{
				NAdr = Switch[NAdr.M][NAdr.B][NAdr.S]->Div;
			}
		}
	}else if(NAdr.type == 's'){
		if(blocks[NAdr.M][NAdr.B][0] != NULL && i == 1){
			a++;
			NAdr = blocks[NAdr.M][NAdr.B][0]->Adr;
		}else{
			NAdr = Switch[NAdr.M][NAdr.B][NAdr.S]->App;
		}
	}else{
		//	printf("Something unexpected happened");
	}
	printf(">i:%i\ta:%i\n",i,a);
	printf(" %i:%i:%i %c\n",NAdr.M,NAdr.B,NAdr.S,NAdr.type);

	if(a > 0 && NAdr.B != Adr.B && (NAdr.type != 'R')){
		i--;
		goto R;
	}
	if(NAdr.type != 'R'){
		Adr = NAdr;
		i--;
		goto R;
	}
	if(a > 0){
		i--;
		a = 0;
		Adr = NAdr;
		goto I;
	}

	if(i > 0){
		Adr = NAdr;
		i--;
		goto I;
	}
	printf("\n");
	return blocks[NAdr.M][NAdr.B][NAdr.S];
}

struct Seg * Next2(struct adr Adr,int i){
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
		return blocks[Adr.M][Adr.B][Adr.S];
	}
	//usleep(20000);
	//printf("i:%i",i);
	//printf("\t%i:%i:%i\ttype:%c\n",NAdr.M,NAdr.B,NAdr.S,NAdr.type);
	if(NAdr.type == 'R'){
		i--;
		Adr = NAdr;
		goto I;
	}else if(NAdr.type == 'S' || NAdr.type == 's'){
		R:{};
		if(i == 1 && blocks[NAdr.M][NAdr.B][0] != NULL){
			i--;
			Adr = C_Adr(NAdr.M,NAdr.B,0);
			goto I;
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
		}else{
			SNAdr = Switch[NAdr.M][NAdr.B][NAdr.S]->App;
		}
		//printf("SN %i:%i:%i%c\tN %i:%i:%i\ti:%i\n",SNAdr.M,SNAdr.B,SNAdr.S,SNAdr.type,NAdr.M,NAdr.B,NAdr.S,i);
		if(SNAdr.type == 'S' || SNAdr.type == 's'){
			if(SNAdr.B != NAdr.B){
				i--;
			}
			NAdr = SNAdr;
			goto R;
		}else{
			Adr = SNAdr;
			NAdr = Adr;
			//printf("\n%i:%i:%i\n",NAdr.M,NAdr.B,NAdr.S);
			i--;
			if(a>0){
				i--;
			}
			goto I;
		}
	}
}

struct Seg * Prev(struct adr adr,int i){
	int a = 0;
	int dir = blocks[adr.M][adr.B][adr.S]->dir;
	struct adr NAdr = adr;
	I:{};
	if(dir == 0 || dir == 2){
		NAdr = blocks[NAdr.M][NAdr.B][NAdr.S]->PAdr;
	}else{
		NAdr = blocks[NAdr.M][NAdr.B][NAdr.S]->NAdr;
	}
	R:{};

	if(NAdr.type == 'R'){
	//	printf("Just a normal rail\n");
	}else if(NAdr.type == 'S'){
		a++;
		if(blocks[NAdr.M][NAdr.B][0] != NULL && i == 1){
			NAdr = blocks[NAdr.M][NAdr.B][0]->Adr;
		}else{
			if(Switch[NAdr.M][NAdr.B][NAdr.S]->state == 0){ //Straight?
				NAdr = Switch[NAdr.M][NAdr.B][NAdr.S]->Str;
			}else{
				NAdr = Switch[NAdr.M][NAdr.B][NAdr.S]->Div;
			}
		}
	}else if(NAdr.type == 's'){
		a++;
		if(blocks[NAdr.M][NAdr.B][0] != NULL && i == 1){
			NAdr = blocks[NAdr.M][NAdr.B][0]->Adr;
		}else{
			NAdr = Switch[NAdr.M][NAdr.B][NAdr.S]->App;
		}
	}else{
	//	printf("Something unexpected happened");
	}

	if(NAdr.type != 'R'){
		goto R;
	}
	if(a > 0){
		i--;
		a = 0;
		goto R;
	}

	if(i > 1){
		i--;
		goto I;
	}
	return blocks[NAdr.M][NAdr.B][NAdr.S];
}

int check_Switch(struct adr adr){
	struct adr NAdr;
	int dir = blocks[adr.M][adr.B][adr.S]->dir;

	if(dir == 0 || dir == 2){
		NAdr = blocks[adr.M][adr.B][adr.S]->NAdr;
	}else{
		NAdr = blocks[adr.M][adr.B][adr.S]->PAdr;
	}
	//printf("NAdr %i:%i:%i\n",NAdr.M,NAdr.B,NAdr.S);
	int n;
	R:{};
	//printf("NAdr: %i:%i:%i\n",NAdr.M,NAdr.B,NAdr.S);
	if(NAdr.type == 'R'){
		return 1; //Passable
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

void JSON(){
	FILE *fp;

	fp = fopen("baan.json","w");


	fprintf(fp,"{\"M\" : [");
	int p = 0;


	for(int i = 0;i<Adress;i++){
		printf("+++");

		struct adr A = Adresses[i];
		if(A.type != 'S'){
			if(p != 0){
				fprintf(fp,",");
			}else{
				p = 1;
			}

			fprintf(fp,"[%i,%i,%i,%i,%i,%i,%i]",A.M,A.B,A.S,blocks[A.M][A.B][A.S]->dir,blocks[A.M][A.B][A.S]->blocked,blocks[A.M][A.B][A.S]->state,blocks[A.M][A.B][A.S]->train);
		}
	}

	fprintf(fp,"],\"SW\" : [");
	printf("\n");

	p = 0;

	for(int i = 0;i<Adress;i++){
		printf("---");

		struct adr A = Adresses[i];
		if(A.type == 'S'){
			if(p != 0){
				fprintf(fp,",");
			}else{
				p = 1;
			}

			fprintf(fp,"[%i,%i,%i,%i]",A.M,A.B,A.S,Switch[A.M][A.B][A.S]->state);
		}
	}
	fprintf(fp,"]}");
	printf("\n");
	fclose(fp);
}

int add_train(){
	return 0xAC;
}

void Slowdown_train(int ID){

}


void procces(struct adr adr,int i){
	//printf("adr.S:%i\ttype:%c\n",adr.S,adr.type);
	if(adr.type != 'R'){
		//printf("Switch_Block\n");
	}else if(adr.S == 0){
		if(blocks[adr.M][adr.B][adr.S]->blocked == 0){
			blocks[adr.M][adr.B][adr.S]->train = 0;
		}
		struct Seg *BA = blocks[adr.M][adr.B][adr.S];
		if(i){
			printf("\t\t\tA_%i:%i:%i \tB%iid%x\n",BA->Adr.M,BA->Adr.B,BA->Adr.S,BA->blocked,BA->train);
		}
	}else{
		
		struct Seg *BA = blocks[adr.M][adr.B][adr.S];
		struct Seg *BN = Next2(adr,1);
		struct Seg *BNN = Next2(adr,2);
		struct Seg *BNNN = Next2(adr,3);
		//struct Seg *BN4 = Next2(adr,4);
		struct Seg *BP = Prev(adr,1);

		if(i){
			printf("P_%i:%i:%i \tB%iid%x\t",BP->Adr.M,BP->Adr.B,BP->Adr.S,BP->blocked,BP->train);
			printf("A_%i:%i:%i \tB%iid%x\t",BA->Adr.M,BA->Adr.B,BA->Adr.S,BA->blocked,BA->train);
			printf("N1_%i:%i:%i \tB%iid%x\t",BN->Adr.M,BN->Adr.B,BN->Adr.S,BN->blocked,BN->train);
			printf("N2_%i:%i:%i \tB%iid%x\t",BNN->Adr.M,BNN->Adr.B,BNN->Adr.S,BNN->blocked,BNN->train);
			printf("N3_%i:%i:%i \tB%iid%x\t",BNNN->Adr.M,BNNN->Adr.B,BNNN->Adr.S,BNNN->blocked,BNNN->train);
			//printf("N4_%i:%i:%i \tB%iid%x\t",BN4->Adr.M,BN4->Adr.B,BN4->Adr.S,BN4->blocked,BN4->train);
		}

		if(BP->blocked == 1 && BA->blocked == 1){
			BA->train = BP->train;
		}

		if(BN->Adr.S == 0 && BNN->Adr.S == 0){
			if(BN->blocked == 1 && BNN->blocked == 1 && BNN->train == 0){
				BNN->train = BN->train;
			}
		}

		if(BA->blocked == 0){
			BA->train = 0;
		}

		if(BN->blocked){
			BA->state = RED;
		}else{
			if(BNN->blocked){
				BA->state = AMBER;
			}else{
				BA->state = GREEN;
			}
		}

		if(BNN->Adr.S == 0 && BN->Adr.S == 0){
			if(BNN->train == 0 && BNN->blocked == 1 && BN->blocked == 1){
				BNN->train = BN->train;
			}
			if(BNNN->blocked){
				BNN->state = RED;
				BN->state = AMBER;
			}
		}

		if(BN->Adr.S == 0){
			if(BN->train == 0 && BN->blocked == 1 && BA->blocked == 1){
				BN->train = BA->train;
			}

			if(BNN->blocked == 1){
				BN->state = RED;
			}else{
				if(BNNN->blocked == 1){
					BN->state = AMBER;
				}else{
					BN->state = GREEN;
				}
			}
		}


		if(BP->Adr.S == 0){
			if(BN->blocked && BA->blocked == 0){
				BP->state = AMBER;
			}
			if(BNN->blocked && BN->blocked == 0){
				BP->state = GREEN;
			}
		}


		int dir = blocks[BA->Adr.M][BA->Adr.B][BA->Adr.S]->dir;
		struct adr NAdr;
		if(dir == 0 || dir == 2){
			NAdr = blocks[BA->Adr.M][BA->Adr.B][BA->Adr.S]->NAdr;
		}else{
			NAdr = blocks[BA->Adr.M][BA->Adr.B][BA->Adr.S]->PAdr;
		}

		if(NAdr.type == 's' || NAdr.type == 'S'){
			//printf("Test: %i\n",check_Switch(BA->Adr));
			if(!check_Switch(BA->Adr)){
				BA->state = RESERVED;
			}
		}

		printf("\n");
	}
}

void do_Magic(){
	for(int i = 0;i<Adress;i++){
		//printf("0x%x\n",Adresses[i]);
		procces(Adresses[i],1);
	}
	JSON();
	printf("\n\n\n");
}

void main(){
	/*
	Module_1(C_Adr(3,5,1),C_Adr(3,5,1),C_Adr(3,1,1),C_Adr(3,5,1));
	Module_2(C_Adr(5,4,1),C_Adr(5,8,1),C_AdrT(2,3,1,'s'),C_AdrT(2,2,1,'s'));
	Module_3(C_Adr(1,1,1),C_Adr(1,4,1),C_Adr(5,1,1),C_Adr(5,5,1));
	Module_5(C_Adr(3,4,1),C_Adr(3,11,1),C_Adr(2,1,1),C_Adr(2,4,1));
	/**/
	Module_1(C_Adr(7,5,1),C_Adr(7,5,1),C_Adr(7,1,1),C_Adr(7,5,1));
	Module_2(C_Adr(5,4,1),C_Adr(5,8,1),C_AdrT(2,3,1,'s'),C_AdrT(2,2,1,'s'));
	Module_7(C_Adr(1,1,1),C_Adr(1,4,1),C_Adr(5,1,1),C_Adr(5,5,1));
	Module_5(C_Adr(7,4,1),C_Adr(7,11,1),C_Adr(2,1,1),C_Adr(2,4,1));
	/**/
	//Module_5(C_Adr(3,1,1),0,C_Adr(2,1,1),0);
	short b = 0b0010011100101101;


	//printf("Module :%i\n",gM(blocks[1][1][1]->Adr));
	//printf("Block  :%i\n",gB(blocks[1][1][1]->Adr));
	//printf("Segment:%i\n",gS(blocks[1][1][1]->Adr));
	//printf("Next type:%i\n",tS(blocks[1][1][1]->NAdr));
	printf("T");
	usleep(200000);
	JSON();
	printf("T");
	//procces(0x211);
	//blocks[1][2][1]->state += 0b1;

	blocks[7][3][1]->blocked = 1;
	/*
	blocks[3][1][1]->blocked = 1;
	blocks[3][2][1]->train = 0xCA;
	blocks[3][1][1]->train = 0xCA;
	blocks[3][3][1]->blocked = 1;
	blocks[3][3][1]->train = 168;
*/
	struct Seg *B = blocks[5][3][1];
	struct Seg *N = blocks[5][2][1];
	struct Seg *A[3] = {};
	struct Seg *B2 = blocks[5][3][1];
	struct Seg *N2 = blocks[5][2][1];
	struct Seg *A2[3] = {};
	int i = 0,i2 = 0,a = 0,trains = 1;

	int delay = 500000;
	JSON();

	while(1){
		a++;
		printf("%i\n",a);
			int m = 7;

		/**/
		if(a == 38){
			B = blocks[m][3][1];
			Switch[m][1][1]->state = !Switch[m][1][1]->state;
			Switch[m][4][1]->state = !Switch[m][4][1]->state;
			JSON();
			usleep(delay);

		}else if(a == 79){
			B2 = blocks[m][2][1];
			trains = 2;
			Switch[m][1][1]->state = !Switch[m][1][1]->state;
			Switch[m][4][1]->state = !Switch[m][4][1]->state;
			JSON();
			usleep(delay);
		}
		/**/
/*
		if(a == 30){
			Switch[3][1][1]->state = !Switch[3][1][1]->state;
			Switch[3][4][1]->state = !Switch[3][4][1]->state;
		}else if(a == 40){
			Switch[3][6][1]->state = !Switch[3][6][1]->state;
			Switch[3][10][1]->state = !Switch[3][10][1]->state;
		}else if(a == 70){
			Switch[3][6][1]->state = !Switch[3][6][1]->state;
			Switch[3][10][1]->state = !Switch[3][10][1]->state;
			Switch[3][6][2]->state = !Switch[3][6][2]->state;
			Switch[3][10][2]->state = !Switch[3][10][2]->state;
		}
*/
		N = Next2(B->Adr,1+i);
		if(i > 0){
			A[i] = Next2(B->Adr,i);
		}

		if(trains == 2){
			N2 = Next2(B2->Adr,1+i2);
			if(i2 > 0){
				A2[i2] = Next2(B2->Adr,i2);
			}
			N2->blocked = 1;
		}

		N->blocked = 1;
		do_Magic();
		usleep(delay);
		printf("%i\n",a);
		if(i>0){
			A[i]->blocked = 0;
		}else{
			B->blocked = 0;
		}
		if(trains != 1){
			if(i2>0){
				A2[i2]->blocked = 0;
			}else{
				B2->blocked = 0;
			}
		}
		do_Magic();
		usleep(delay);
		if(N->Adr.S == 0){
			i++;
		}else{
			B = N;
			i = 0;
		}
		if(trains == 2){
			if(N2->Adr.S == 0){
				i2++;
			}else{
				B2 = N2;
				i2 = 0;
			}
		}
	}

	/**/
}
