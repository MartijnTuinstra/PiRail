#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#define MAX_SW 20
#define MAX_Segments 16
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

struct Seg{
	unsigned short adr;  //0bTTTM MMMB BBBB SSSS,T = RailType, M = Module, B = Block, S = Segment (No Zeros for Block and Segment!!)
	unsigned short Nadr; //0bTTTM MMMB BBBB SSSS,T = RailType, M = Module, B = Block, S = Segment
	unsigned short Padr; //0bTTTM MMMB BBBB SSSS,T = RailType, M = Module, B = Block, S = Segment
	char max_speed;      //0x00 <-> 0xFF
	unsigned short state;//0bxxxd DDSS LLLL bbbB, d = train direction, D = Direction, S = Station/StopSpot, L = Lenght, b = state, B = Blocked?
	char train;			 //0x00 <-> 0xFF
};

struct Swi{
	unsigned short adr;

	unsigned short Div;
	unsigned short Str;
	unsigned short App;
	char state;
};

int gM(unsigned short adr){
	int a = (adr & 0x1E00) >> 9;
	return a;
}
int gB(unsigned short adr){
	int a = (adr & 0x1F0) >> 4;
	return a;
}
int gS(unsigned short adr){
	int a = adr & 0xF;
	return a;
}
int tS(unsigned short adr){
	return ((adr & 0x6000) >> 13); //4/8/C <-> S_App / s_Div\Str / Special
}

short Adresses[MAX_A] = {0};
struct Seg *blocks[MAX_Modules][MAX_Blocks][MAX_Segments] = {};
struct Swi *Switch[MAX_Modules][MAX_Blocks][MAX_Segments] = {};

unsigned short C_Adr(int Module,int Block,int Segment){
	return (((Module & 0xF) << 9) + ((Block & 0x1F) << 4) + (Segment & 0xF));
}

struct Seg * C_Seg(unsigned short adr, char state){
	struct Seg *Z = (struct Seg*)malloc(sizeof(struct Seg));

	Z->adr = adr;
	Z->Nadr = 0;
	Z->Padr = 0;
	Z->max_speed = 0;
	Z->state = state;
	Z->train = 0x00;

	return Z;

}

void Create_Segment(unsigned short adr,unsigned short Nadr,unsigned short Padr,char max_speed,unsigned short state){
	struct Seg *Z = (struct Seg*)malloc(sizeof(struct Seg));

	Z->adr = adr;
	Z->Nadr = Nadr;
	Z->Padr = Padr;
	Z->max_speed = max_speed;
	Z->state = state;
	Z->train = 0x00;

	//return Z;
	printf("A Segment is created at %i:%i:%i\n",gM(adr),gB(adr),gS(adr));
	blocks[gM(adr)][gB(adr)][gS(adr)] = Z;
	for(int i = 0;i<MAX_A;i++){
		if(Adresses[i] == 0){
			Adresses[i] = adr;
			break;
		}
	}
}

void Create_Switch(unsigned short adr,unsigned short App,unsigned short Div,unsigned short Str,char state){
	struct Swi *Z = (struct Swi*)malloc(sizeof(struct Swi));

	Z->adr = adr + 0x4000;
	Z->Str = Str;
	Z->Div = Div;
	Z->App = App;
	Z->state = state;

	//return Z;
	printf("A Switch  is created at %i:%i:%i\n",gM(adr),gB(adr),gS(adr));
	if(blocks[gM(adr)][gB(adr)][1] == NULL){
		blocks[gM(adr)][gB(adr)][0] = C_Seg(C_Adr(gM(adr),gB(adr),0),0);
		for(int i = 0;i<MAX_A;i++){
			if(Adresses[i] == 0){
				Adresses[i] = C_Adr(gM(adr),gB(adr),0);
				break;
			}
		}
	}
	Switch[gM(adr)][gB(adr)][gS(adr)] = Z;
}

#include "modules2.c"

void block_state(struct Seg *S,int val){
	val &= 0b1110;

	S->state &= 0xFFF1;
	S->state |= val;
}

struct Seg * Next(short adr,int i){
	int a = 0;
	int dir = ((blocks[gM(adr)][gB(adr)][gS(adr)]->state >> 10) & 0x3);
	short NAdr = adr;
	I:{};
	if(dir == 0 || dir == 2){
		NAdr = blocks[gM(NAdr)][gB(NAdr)][gS(NAdr)]->Nadr;
	}else{
		NAdr = blocks[gM(NAdr)][gB(NAdr)][gS(NAdr)]->Padr;
	}
	R:{};

	if((NAdr & 0x6000) == 0){
	//	printf("Just a normal rail\n");
	}else if((NAdr & 0x6000) == Switch_Front){
		a++;
		if(blocks[gM(NAdr)][gB(NAdr)][0] != NULL && i == 1){
			NAdr = blocks[gM(NAdr)][gB(NAdr)][0]->adr;
		}else{
			if(Switch[gM(NAdr)][gB(NAdr)][gS(NAdr)]->state == 0){ //Straight?
				NAdr = Switch[gM(NAdr)][gB(NAdr)][gS(NAdr)]->Str;
			}else{
				NAdr = Switch[gM(NAdr)][gB(NAdr)][gS(NAdr)]->Div;
			}
		}
	}else if((NAdr & 0x6000) == Switch_Back){
		a++;
		if(blocks[gM(NAdr)][gB(NAdr)][0] != NULL && i == 1){
			NAdr = blocks[gM(NAdr)][gB(NAdr)][0]->adr;
		}else{
			NAdr = Switch[gM(NAdr)][gB(NAdr)][gS(NAdr)]->App;
		}
	}else{
	//	printf("Something unexpected happened");
	}

	if((NAdr & 0x6000) != 0){
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
	return blocks[gM(NAdr)][gB(NAdr)][gS(NAdr)];
}

struct Seg * Prev(short adr,int i){
	int a = 0;
	int dir = ((blocks[gM(adr)][gB(adr)][gS(adr)]->state >> 10) & 0x3);
	short NAdr = adr;
	I:{};
	if(dir == 0 || dir == 2){
		NAdr = blocks[gM(NAdr)][gB(NAdr)][gS(NAdr)]->Padr;
	}else{
		NAdr = blocks[gM(NAdr)][gB(NAdr)][gS(NAdr)]->Nadr;
	}
	R:{};

	if((NAdr & 0x6000) == 0){
	//	printf("Just a normal rail\n");
	}else if((NAdr & 0x6000) == Switch_Front){
		a++;
		if(blocks[gM(NAdr)][gB(NAdr)][0] != NULL && i == 1){
			NAdr = blocks[gM(NAdr)][gB(NAdr)][0]->adr;
		}else{
			if(Switch[gM(NAdr)][gB(NAdr)][gS(NAdr)]->state == 0){ //Straight?
				NAdr = Switch[gM(NAdr)][gB(NAdr)][gS(NAdr)]->Str;
			}else{
				NAdr = Switch[gM(NAdr)][gB(NAdr)][gS(NAdr)]->Div;
			}
		}
	}else if((NAdr & 0x6000) == Switch_Back){
		a++;
		if(blocks[gM(NAdr)][gB(NAdr)][0] != NULL && i == 1){
			NAdr = blocks[gM(NAdr)][gB(NAdr)][0]->adr;
		}else{
			NAdr = Switch[gM(NAdr)][gB(NAdr)][gS(NAdr)]->App;
		}
	}else{
	//	printf("Something unexpected happened");
	}

	if((NAdr & 0x6000) != 0){
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
	return blocks[gM(NAdr)][gB(NAdr)][gS(NAdr)];
}

int check_Switch(short adr){
	int dir = ((blocks[gM(adr)][gB(adr)][gS(adr)]->state >> 10) & 0x3);
	short NAdr = 0;
	if(dir == 0 || dir == 2){
		NAdr = blocks[gM(adr)][gB(adr)][gS(adr)]->Nadr;
	}else{
		NAdr = blocks[gM(adr)][gB(adr)][gS(adr)]->Padr;
	}
	int n;
	R:{};
	//printf("NAdr: 0x%x\n",NAdr);
	if((NAdr & 0x6000) == 0 || (NAdr & 0x6000) == 0x2000){
		return 1; //Passable
	}else{
		if((Switch[gM(NAdr)][gB(NAdr)][gS(NAdr)]->Div & 0x1FFF) == (adr & 0x1FFF)){
			if(Switch[gM(NAdr)][gB(NAdr)][gS(NAdr)]->state == 1){
				n = 1;
			}else{
				return 0;
			}
		}else{
			if(Switch[gM(NAdr)][gB(NAdr)][gS(NAdr)]->state == 0){
				printf("T");
				n = 1;
			}else{
				return 0;
			}
		}

		if((Switch[gM(NAdr)][gB(NAdr)][gS(NAdr)]->App & 0x6000) != 0){
		//	printf("New switch\n");
			adr = Switch[gM(NAdr)][gB(NAdr)][gS(NAdr)]->adr;
			NAdr = Switch[gM(NAdr)][gB(NAdr)][gS(NAdr)]->App;
			goto R;
		}
	}
	return n;
}

void JSON(){
	FILE *fp;

	fp = fopen("baan.json","w");


	fprintf(fp,"{\"M\" : [");
	int p = 0;

	int f_m = 0;
	int f_b = 0;
	int f_s = 0;

	for(int i_m = 1;i_m<MAX_Modules;i_m++){
		if(blocks[i_m][1][1] != NULL || blocks[i_m][1][0] != NULL){

			for(int i_b = 1;i_b<MAX_Blocks;i_b++){
				if(blocks[i_m][i_b][1] == NULL && blocks[i_m][i_b][0] != NULL){
					if(f_s != 0){
						fprintf(fp,",");
					}
					short state = blocks[i_m][i_b][0]->state;
					fprintf(fp,"[%i,%i,%i,%i,%i,%i,%i]",i_m,i_b,0,(state & 0xC0)>>6,(state & 0x30)>>4,(state & 0x7),blocks[i_m][i_b][0]->train);
				}else if(blocks[i_m][i_b][0] == NULL && blocks[i_m][i_b][1] != NULL){
					for(int i_s = 1;i_s<MAX_Segments;i_s++){
						if(blocks[i_m][i_b][i_s] != NULL){
							if(f_s == 0){
								f_s = 1;
							}else{
								fprintf(fp,",");
							}

							short state = blocks[i_m][i_b][i_s]->state;
							fprintf(fp,"[%i,%i,%i,%i,%i,%i,%i]",i_m,i_b,i_s,(state & 0xC00)>>10,(state & 0x30)>>4,(state & 0x7),blocks[i_m][i_b][i_s]->train);
						}
					}
				}
			}
			f_b = 0;
		}
	}

	fprintf(fp,"],\"SW\" : [");

	int f = 0;

	for(int i_m = 1;i_m<MAX_Modules;i_m++){
		if(blocks[i_m][1][1] != NULL || blocks[i_m][1][0] != NULL){

			for(int i_b = 1;i_b<MAX_Blocks;i_b++){
				if(Switch[i_m][i_b][1] != NULL){

					for(int i_s = 1;i_s<MAX_Segments;i_s++){
						if(Switch[i_m][i_b][i_s] != NULL){
							if(f == 0){
								f = 1;
							}else{
								fprintf(fp,",");
							}

							//short state = Switch[i_m][i_b][i_s]->state;
							fprintf(fp,"[%i,%i,%i,%i]",i_m,i_b,i_s,Switch[i_m][i_b][i_s]->state);
						}
					}
				}
			}
		}
	}


	fprintf(fp,"]}");

	fclose(fp);
}

int add_train(){
	return 0xAC;
}

void Slowdown_train(int ID){

}


void procces(short adr){
	if(gS(adr) == 0){
		//printf("Switch_Block\n");
	}else{
		
		struct Seg *BA = blocks[gM(adr)][gB(adr)][gS(adr)];
		struct Seg *BN = Next(adr,1);
		struct Seg *BNN = Next(adr,2);
		struct Seg *BNNN = Next(adr,3);
		struct Seg *BP = Prev(adr,1);

		printf("P_%i:%i:%i S_%i\t",gM(BP->adr),gB(BP->adr),gS(BP->adr),(BP->state & 0b1111));
		printf("A_%i:%i:%i S_%i\t",gM(BA->adr),gB(BA->adr),gS(BA->adr),(BA->state & 0b1111));
		printf("N_%i:%i:%i S_%i\t",gM(BN->adr),gB(BN->adr),gS(BN->adr),(BN->state & 0b1111));
		printf("NN_%i:%i:%i S_%i\t",gM(BNN->adr),gB(BNN->adr),gS(BNN->adr),(BNN->state & 0b1111));
		printf("NNN_%i:%i:%i S_%i\t",gM(BNNN->adr),gB(BNNN->adr),gS(BNNN->adr),(BNNN->state & 0b1111));

		if((BP->state & 0b1) == 1){
			BA->train = BP->train;
		}

		if((BN->state & 0b1) == 1){
			block_state(BA,0b100);
			if((BA->state & 0b1) == 1){
				printf("STOP TRAIN");
			}else{
				BA->train = 0;
			}
		}else if((BNN->state & 0b1) == 1){
			block_state(BA,0b010);
			if((BA->state & 0b1) == 1){
				printf("SLOW DOWN TRAIN");
			}else{
				BA->train = 0;
			}
		}else{
			block_state(BA,0);
		}

		if(gS(BN->adr) == 0){
			if((BNN->state & 0b1) == 1){
				block_state(BN,0b100);
			}else if((BNNN->state & 0b1) == 1){
				block_state(BN,0b010);
			}else{
				block_state(BN,0b000);
			}
		}

		printf("\n");
	}
}

void do_Magic(){
	for(int i = 0;i<MAX_Modules*MAX_Blocks*MAX_Segments;i++){
		if(Adresses[i] != 0){
			//printf("0x%x\n",Adresses[i]);
			procces(Adresses[i]);
		}
	}
	JSON();
	printf("\n\n\n");
}

void main(){
	Module_1(C_Adr(2,1,1),C_Adr(2,1,1),C_Adr(3,1,1),C_Adr(3,5,1));
	Module_3(C_Adr(1,4,1),C_Adr(1,11,1),C_Adr(4,1,1),C_Adr(4,1,1));
	//Module_5(C_Adr(3,1,1),0,C_Adr(2,1,1),0);
	Create_Segment(C_Adr(2,1,1),C_Adr(1,1,1),C_Adr(1,5,1),0xFA,0x40);
	Create_Segment(C_Adr(4,1,1),C_Adr(3,4,1),C_Adr(3,7,1),0xFA,0x40);
	short b = 0b0010011100101101;
	//printf("Module :%i\n",gM(blocks[1][1][1]->adr));
	//printf("Block  :%i\n",gB(blocks[1][1][1]->adr));
	//printf("Segment:%i\n",gS(blocks[1][1][1]->adr));
	//printf("Next type:%i\n",tS(blocks[1][1][1]->Nadr));
	JSON();
	procces(0x211);
	blocks[1][2][1]->state += 0b1;
	blocks[3][6][1]->state += 0b1;
	blocks[3][5][1]->state += 0b1;
	blocks[3][6][1]->train = 0xCA;
	printf("Test adr: %i",blocks[3][5][1]->train);
	JSON();
	usleep(2000000);
	procces(0x211);
	procces(0x211);
	int a = 0x221;
	printf("Seg %i:%i:%i\n",gM(a),gB(a),gS(a));
	int delay = 1000000;
	JSON();
	do_Magic();
	usleep(1000000);
	blocks[1][11][1]->state += 0b1;
	blocks[3][6][1]->state -= 0b1;
	do_Magic();
	usleep(1000000);
	blocks[1][10][0]->state += 0b1;
	blocks[3][5][1]->state -= 0b1;
	do_Magic();
	usleep(1000000);
	blocks[1][8][1]->state += 0b1;
	blocks[1][11][1]->state -= 0b1;
	do_Magic();
	usleep(1000000);
	blocks[1][6][0]->state += 0b1;
	blocks[1][10][0]->state -= 0b1;
	do_Magic();
	usleep(1000000);
	Switch[1][1][1]->state = 0;

	blocks[1][5][1]->state += 0b1;
	blocks[1][8][1]->state -= 0b1;
	do_Magic();
	usleep(1000000);
	blocks[2][1][1]->state += 0b1;
	blocks[1][6][0]->state -= 0b1;
	do_Magic();
	usleep(1000000);
	blocks[1][1][1]->state += 0b1;
	blocks[1][5][1]->state -= 0b1;
	do_Magic();
	usleep(1000000);
	blocks[1][3][1]->state += 0b1;
	blocks[2][1][1]->state -= 0b1;
	do_Magic();
	usleep(1000000);
	blocks[1][4][1]->state += 0b1;
	blocks[1][1][1]->state -= 0b1;
	do_Magic();
	usleep(1000000);
	blocks[3][1][1]->state += 0b1;
	blocks[1][3][1]->state -= 0b1;
	blocks[1][4][1]->state -= 0b1;
	do_Magic();
	/*
	printf("Next adr: 0x%x\n",get_NAdr(blocks[1][5][1]->adr));
	printf("Next Seg: 0x%x\n",get_NSeg(blocks[1][5][1]->adr));
	printf("\n");
	printf("Next adr: 0x%x\n",get_PAdr(blocks[1][5][1]->adr));
	printf("Next Seg: 0x%x\n",get_PSeg(blocks[1][5][1]->adr));
	printf("\n");
	printf("Next adr: 0x%x\n",get_NAdr(blocks[1][1][1]->adr));
	printf("Next adr: 0x%x\n",get_NSeg(blocks[1][1][1]->adr));
	Switch[1][10][1]->state = 0;
	printf("Pass switch:%i\n",check_Switch(blocks[1][1][1]->adr));
	printf("Pass switch:%i\n",check_Switch(blocks[1][2][1]->adr));
	printf("Pass switch:%i\n",check_Switch(blocks[1][3][1]->adr));
	printf("\n\n");
	printf("Pass switch:%i\n",check_Switch(blocks[1][7][1]->adr));
	printf("Pass switch:%i\n",check_Switch(blocks[1][8][1]->adr));
	printf("Pass switch:%i\n",check_Switch(blocks[1][9][1]->adr));
	JSON();
	/*
	int d = 1000000;
	usleep(d);
	block_state(1,1,1,RED);
	Switch[1][10][1]->state = 0;
	JSON();
	usleep(d);
	blocks[1][2][1]->state |= 0b010;
	JSON();
	usleep(d);
	blocks[1][3][1]->state |= 0b010;
	JSON();
	usleep(d);
	blocks[1][4][1]->state |= 0b010;
	JSON();
	usleep(d);
	blocks[1][5][1]->state |= 0b010;
	JSON();
	usleep(d);
	blocks[1][6][0]->state |= 0b010;
	JSON();
	usleep(d);
	block_state(1,5,1,UNKNOWN);
	blocks[1][7][1]->state |= 0b010;
	JSON();
	usleep(d);
	block_state(1,4,1,GHOST);
	blocks[1][8][1]->state |= 0b010;
	JSON();
	usleep(d);
	block_state(1,3,1,BLOCKED);
	blocks[1][9][1]->state |= 0b010;
	JSON();
	usleep(d);
	block_state(1,2,1,AMBER);
	blocks[1][10][0]->state |= 0b010;
	JSON();
	usleep(d);
	block_state(1,1,1,GREEN);
	blocks[1][11][1]->state |= 0b010;
	JSON();*/
}
