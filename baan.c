#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <wiringPi.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <openssl/sha.h>
#include <errno.h>
#include <signal.h>
#include "./src/b64.c"

#include "settings.h"

#define MAX_A MAX_Modules*MAX_Blocks*MAX_Segments

#define GREEN 0
#define AMBER 1
#define RED 2
#define BLOCKED 3
#define PARKED 4
#define RESERVED 5
#define UNKNOWN 6
#define BLOCK_STATES 4

//#define En_UART

#define Arr_Count(array) (sizeof(array)/sizeof((array)[0]))
#define ROUND(nr)  (int)(nr+0.5)

#define TRACK_SCALE 160

int stop = 0;
int startup = 0;

struct adr{
	int M;		// Module
	int B;		// Block
	int S;		// Section
	int type;	// Type
};

struct adr C_Adr(char M,char B,char S);
int Adr_Comp(struct adr A,struct adr B);

int B_list_i = 0, St_list_i = 0, S_list_i = 0, M_list_i = 0, Si_list_i = 0;

#include "./src/rail.c"
#include "./src/signals.c"
#include "./src/switch.c"
#include "./src/COM.h"

int delayA = 5000000;
int delayB = 5000000;
int initialise = 1;
char setup_data[100];
char setup_data_l = 0;
int status_st[20] = {0};

_Bool digital_track = 0;

pthread_mutex_t mutex_lockA;
pthread_mutex_t mutex_lockB;
pthread_t timer_thread[MAX_TIMERS];
int timers[MAX_TIMERS] = {0}; //0 = Free, 1 = Busy, 2 = Done

struct adr Adresses[MAX_A] = {};

char List_of_Modules[MAX_Modules] = {0};

struct adr StartAdr;

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
#define EMPTY_BL EMPTY_BL()

struct Seg * C_Seg(int Unit_Adr, struct SegC Adr, char state);

#include "./src/modules.c"

#include "./src/Z21.h"

struct Seg * C_Seg(int Unit_Adr, struct SegC Adr, char state){
	struct Seg *Z = (struct Seg*)malloc(sizeof(struct Seg));

	Z->Module = Adr.Module;
	Z->id = Adr.Adr;
	Z->type = Adr.type;
	Z->NextC = EMPTY_BL;
	Z->PrevC = EMPTY_BL;
	Z->max_speed = 0;
	Z->state = state;
	Z->train = 0x00;

	//printf("A Segment is created at %i:%i:%i\t",Adr.M,Adr.B,Adr.S);
	blocks2[Adr.Module][Adr.Adr] = Z;

	return Z;
}

int Adr_Comp(struct adr A,struct adr B){
	if(A.M == B.M && A.B == B.B && A.S == B.S){
		return 1;
	}else{
		return 0;
	}
}

int Adr_Comp2(struct SegC A,struct SegC B){
	if(A.Module == B.Module && A.Adr == B.Adr && A.type == B.type){
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

void setup_JSON(int arr[], int arr2[], int size, int size2){
	char buf[100];

	setup_data[0] = 2;
	setup_data_l = 2 + size + size2;

	int i = 2;

	for(i;(i-2)<size;i++){
		setup_data[i] = arr[i-2];
	}

	if(size2 != 0){
		setup_data[1] = size;

		for(i;(i-2-size)<size2;i++){
			setup_data[i] = arr2[i-2-size];
		}
	}
}

void change_block_state(struct Seg * Block,int State){
	if(Block->state != State){
		Block->change = 1;
		Block->state = State;
	}
}

void JSON_new_client(int Client_fd);

struct timer_thread_data{
   int  thread_id;
	 int  t;
};

struct timer_thread_data a[MAX_TIMERS];

#include "./src/status.c"
#include "./src/trains.c"
#include "./src/train_sim.c"
#include "./src/Web.c"
#include "./src/COM.c"

#include "./src/Z21.c"

void JSON(){
	pthread_mutex_lock(&mutex_lockB);
	char buf[4096];
	memset(buf,0,4096);

	buf[0] = 3;
	int buf_l;
	_Bool data = FALSE;

	int q = 1;

	for(int i = 0;i<MAX_Modules;i++){
		if(Units[i]){
			for(int j = 0;j<Units[i]->B_nr;j++){
				struct Seg * B = Units[i]->B[j];
				if(B && B->change){
					data = 1;
					buf[(q-1)*4+1] = B->Module;
					buf[(q-1)*4+2] = B->id;
					buf[(q-1)*4+3] = (B->dir << 7) + (B->blocked << 4) + B->state;
					buf[(q-1)*4+4] = B->train;
					B->change = 0;
					q++;
				}
			}
		}
	}

	buf_l = (q-1)*4+1;

	if(data == 1){
		send_all(buf,++buf_l,8);
	}

	pthread_mutex_unlock(&mutex_lockB);
}

void JSON_new_client(int Client_fd){
	pthread_mutex_lock(&mutex_lockB);
	char buf[4096];
	memset(buf,0,4096);

	/*Track*/

		buf[0] = 3;
		int buf_l;
		_Bool data = 0;

		int q = 1;

		for(int i = 0;i<MAX_Modules;i++){
			if(Units[i]){
				data = 1;
				for(int j = 0;j<Units[i]->B_nr;j++){
					struct Seg * B = Units[i]->B[j];
					if(B){
						buf[(q-1)*4+1] = B->Module;
						buf[(q-1)*4+2] = B->id;
						buf[(q-1)*4+3] = (B->dir << 7) + (B->blocked << 4) + B->state;
						buf[(q-1)*4+4] = B->train;
						q++;
					}
				}
			}
		}

		buf_l = (q-1)*4+1;

		if(data == 1){
			send_packet(Client_fd,buf,buf_l,8);
		}

		memset(buf,0,4096);

	/*Switches*/

		buf[0] = 4;
		buf_l = 0;
		data = 0;

		q = 1;
		printf("\n\n3");

		for(int i = 0;i<MAX_Modules;i++){
			if(Units[i]){
				data = 1;
				for(int j = 0;j<Units[i]->Swi_nr;j++){
					struct Swi * S = Units[i]->S[j];
					if(S){
						buf[(q-1)*3+1] = S->Module;
						buf[(q-1)*3+2] = S->id;
						buf[(q-1)*3+3] = S->state;
						printf(",%i,%i,%i",S->Module,S->id,S->state);
						q++;
					}
				}
			}
		}

		buf_l = (q-1)*3+1;

		printf("\tbuf_l:%i\n\n",buf_l);

		if(data == 1){
			send_packet(Client_fd,buf,buf_l,8);
		}

		memset(buf,0,4096);

	/*Moduls*/

		buf[0] = 5;
		buf_l = 0;
		data = 0;

		q = 1;

		for(int i = 0;i<MAX_Modules;i++){
			if(Units[i]){
				data = 1;
				for(int j = 0;j<Units[i]->Mod_nr;j++){
					struct Mod * M = Units[i]->M[j];
					if(M){
						buf[(q-1)*4+1] = M->Module;
						buf[(q-1)*4+2] = M->id;
						buf[(q-1)*4+3] = M->state;
						buf[(q-1)*4+4] = M->length;
						q++;
					}
				}
			}
		}

		buf_l = (q-1)*4+1;

		if(data == 1){
			send_packet(Client_fd,buf,buf_l,8);
		}

		memset(buf,0,4096);

	/*Stations*/

	buf[0] = 6;
	buf_l = 1;
	data = 0;

	if(St_list_i>0){
		data = 1;
	}
	for(int i = 1;(i-1)<St_list_i;i++){
		printf("entry %i\tStation %i:%i\t%s\tbuf_l: %i\n",i,stations[i-1]->Module,stations[i-1]->id,stations[i-1]->Name,buf_l);

		buf[buf_l]   = stations[i-1]->Module;
		buf[buf_l+1] = stations[i-1]->id;
		buf[buf_l+2] = strlen(stations[i-1]->Name);
		strcpy(&buf[buf_l+3],stations[i-1]->Name);

		buf_l+=3+strlen(stations[i-1]->Name);
	}

	if(data == 1){
		send_packet(Client_fd,buf,buf_l,8);
	}

	memset(buf,0,4096);

	for(int i = 1;i<MAX_TRAINS;i++){
		if(train_link[i]){
			printf("Recall #%i\n",train_link[i]->DCC_ID);
			Z21_GET_LOCO_INFO(train_link[i]->DCC_ID);
		}
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

#include "./src/algorithm.c"
#include "./src/pathfinding.c"

void *do_Magic(){
	while(!stop){
		//printf("\n\n\n");
		clock_t t;
		t = clock();
		pthread_mutex_lock(&mutex_lockA);
		#ifdef En_UART
		for(int i = 0;i<strlen(List_of_Modules);i++){
			printf("R%i ",List_of_Modules[i]);
			struct COM_t C;
			memset(C.Data,0,32);
			C.Adr = List_of_Modules[i];
			C.Opcode = 6;
			C.Length = 0;

			pthread_mutex_lock(&mutex_UART);
			COM_Send(C);
			char COM_data[20];
			memset(COM_data,0,20);
			COM_Recv(COM_data);
			pthread_mutex_unlock(&mutex_UART);
			usleep(10);
		}
		printf("\n");
		#endif
		_Bool debug;
		for(int i = 0;i<MAX_Modules;i++){
			//printf("%i: %i:%i:%i\n",i,Adresses[i].M,Adresses[i].B,Adresses[i].S);
			for(int j = 0;j<100;j++){
				if(blocks2[i][j]){
					procces(blocks2[i][j],0);
				}
			}
		}
		JSON();
		pthread_mutex_unlock(&mutex_lockA);
		t = clock() - t;
		//printf ("It took me %d clicks (%f seconds).\n",t,((float)t)/CLOCKS_PER_SEC);
		//printf("\n\n\n\n\n\n");

		procces_accessoire();

		//FILE *data;
		//data = fopen("data.txt", "a");
		//fprintf(data,"%d\n",t);
		//fclose(data);

		usleep(1000000);
	}
}

void do_once_Magic(){
	pthread_mutex_lock(&mutex_lockA);
	for(int i = 0;i<MAX_Modules;i++){
		//printf("%i: %i:%i:%i\n",i,Adresses[i].M,Adresses[i].B,Adresses[i].S);
		//int i = 4;
		for(int j = 0;j<100;j++){
			if(blocks2[i][j]){
				procces(blocks2[i][j],1);
			}
		}
	}
	COM_change_A_signal(4);
	COM_change_A_switch(4);
	JSON();
	pthread_mutex_unlock(&mutex_lockA);
}

void *STOP_FUNC(){
	//char str[10];
	while(!stop){
		printf("Type q{Enter} to stop\n");

		int r;
		unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0) {
        continue;
    } else {
        if(c == 'q'){
					stop = 1;
					break;
				}
    }
	}
	printf("STOPPING...\n");
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
	signal(SIGPIPE, SIG_IGN);
	srand(time(NULL));
	/*Starting wiringPi*/
		wiringPiSetup();

		pinMode(0, OUTPUT); //GPIO17
		pinMode(1, OUTPUT); //GPIO17
		digitalWrite(0,LOW);
		digitalWrite(1,LOW);
	/*Splash screen*/
		printf("\n\n                o  o   o\n");
		printf("            O  o  o\n");
		printf("        o oO  o\n");
		printf("      OoO\n");
		printf("     oOo ___             __________  ___________  ___________  __-----__\n");
	 printf("    _\\/__|_|_  _,o—o.,_  | |.\\/.| |  |         |  | EXPRESS |  ||_| |_||\n");
		printf("   [=      _|--|______|--|_|_/\\_|_|--|_________|--|_________|--|_______|\n");
		printf("   //o--=OOO-  ‘o’  ‘o’  ‘o’    ‘o’  ‘o’     ‘o’  ‘o’     ‘o’  ‘o’   ‘o’\n");

		printf("----------------------------------------------------------------------------\n");
		printf("|                                                                          |\n");
		printf("|                         RASPBERRY RAIL SOFTWARE                          |\n");
		printf("|                               is booting                                 |\n");
		printf("|                                                                          |\n");
		printf("----------------------------------------------------------------------------\n");
		printf("|                                                                          |\n");
	/**/
	/*Start web server*/
		printf("----------------------------------------------------------------------------\n");
		printf("|                                                                          |\n");
		printf("|                               Web server                                 |\n");
		printf("|                                                                          |\n");
		pthread_t thread_web_server;
		pthread_create(&thread_web_server, NULL, web_server, NULL);
		usleep(100000);
	/*Start UART*/
		printf("|                                  UART                                    |\n");
		printf("|                                                                          |\n");
		//Start UART port
		pthread_t thread_UART;
		pthread_create(&thread_UART, NULL, UART, NULL);
		usleep(100000);
	/*Z21 Client*/
		printf("|                          Z21@%s:%i\t                   |\n",Z21_IP,Z21_PORT);
		printf("|                                                                          |\n");
		//Start UART port
		pthread_t thread_Z21_client;
		//pthread_create(&thread_Z21_client, NULL, Z21_client, NULL);
		//Z21_client();
		usleep(100000);
	/*Define empty block*/
		C_Seg(0,CAdr(0,0,'e'),3);
	/*Search all blocks*/
		printf("|                              BLOCK LINKING                               |\n");
		printf("|                                                                          |\n");
		//Initialising two link object with empty blocks
		struct link LINK;
		struct link LINK2;
		LINK.Adr1 = EMPTY_BL;
		LINK.Adr2 = EMPTY_BL;
		LINK.Adr3 = EMPTY_BL;
		LINK.Adr4 = EMPTY_BL;
		LINK2.Adr1 = EMPTY_BL;
		LINK2.Adr2 = EMPTY_BL;
		int nr_Modules = 0;
		usleep(100000);
		digitalWrite(1,HIGH);

		int setup[MAX_Modules] = {1,8,4,2,0};
		int setup2[5] = {11,6,7,0};

		for(int i = 0;i<4;i++){
			#ifdef En_UART
				char Line_Data[25];
				memset(Line_Data,0,25);
				int L = COM_Recv(Line_Data);
				if(L == 3 && Line_Data[0] == 0 && (Line_Data[1] & 0xF) == 0){
					printf("|                       UART Module %i\t found                             |\n",Line_Data[2]);
					//printf("Module %i\tfound\n",setup[i]);
					LINK = Modules(Line_Data[2],LINK);
					if(!Adr_Comp2(LINK.Adr3,EMPTY_BL)){
						printf("Branch needed\n");
						LINK2.Adr1 = LINK.Adr3;
						LINK2.Adr2 = LINK.Adr4;
					}

					setup[i] = Line_Data[2];
					List_of_Modules[nr_Modules++] = Line_Data[2];
				}else{
					printf("Wrong data length recieved: %i\n",L);
					for(int i = 0;i<L;i++){
						printf("[%i]",Line_Data[i]);
					}
					printf("\n");
				}
			#endif
			#ifndef En_UART
				printf("|                            Module %i\t found                             |\n",setup[i]);
				//printf("Module %i\tfound\n",setup[i]);
				LINK = Modules(setup[i],LINK);
				if(!Adr_Comp2(LINK.Adr3,EMPTY_BL)){
					printf("Branch needed\n");
					printf("%i==%i\t%i==%i\t%c==%c\n",LINK.Adr3.Module,EMPTY_BL.Module,LINK.Adr3.Adr,EMPTY_BL.Adr,LINK.Adr3.type,EMPTY_BL.type);
					LINK2.Adr1 = LINK.Adr3;
					LINK2.Adr2 = LINK.Adr4;
				}
			#endif
		}

		if(!Adr_Comp2(LINK2.Adr1,EMPTY_BL)){
			printf("|                                                                          |\n");
			printf("|                               Branch one                                 |\n");
		}

		for(int i = 0;i<0;i++){
			printf("|                            Module %i\t found                             |\n",setup2[i]);
			LINK2 = Modules(setup2[i],LINK2);
		}
		Connect_Segments();
		setup_JSON(setup,setup2,4,0);
		usleep(1000000);
	/*Loading Trains*/
		printf("|                                                                          |\n");
		printf("|                             Loading trains                               |\n");
		printf("|                                                                          |\n");

		init_trains();

	printf("|                                                                          |\n");
	printf("----------------------------------------------------------------------------\n\n");
	printf("                              STARTUP COMPLETE\n\n\n");

	//Set all Switches and Signals to known positions
	do_once_Magic();
	//procces(blocks2[2][0],1);
	//procces(blocks2[2][1],1);
	//procces(blocks2[2][2],1);
	//procces(blocks2[2][3],1);

	delay(5);

	/*COM test*/
		//COM_Recv();

		/*
		struct COM_t{
			char Adr;
			char Length;
			char Opcode;
			char Data[32];
		};


		struct COM_t C;
		C.Adr = 1;
		C.Length = 4;
		C.Opcode = 14;
		memset(C.Data,0,32);
		C.Data[0] = 89;
		C.Data[1] = 74;
		C.Data[2] = 62;
		C.Data[3] = 98;

		COM_Send(C);
		delay(1100);
		C.Adr = 0xFF;
		COM_Send(C);
		delay(1100);
		C.Adr = 4;
		COM_Send(C);
		delay(1100);
		C.Adr = 8;

		//Read blocks from address 8
		COM_Send(C);
		delay(1100);
		C.Adr = 2;
		C.Opcode = 6;
		C.Length = 0;
		COM_Send(C);
		char COM_data[20];
		memset(COM_data,0,20);
		COM_Recv(COM_data);
		printf("Data recieved:\n");
		printf("Address: 0x%02x\n",COM_data[0]);
		printf("Opcode:  0x%01x\n",(COM_data[1] & 0b1111));
		char length;
		printf("Length:  0x%01x\n",length = (COM_data[1] >> 4));
		for(int i = 0;i<length;i++){
			printf(BYTE_TO_BINARY_PATTERN,BYTE_TO_BINARY(COM_data[i+2]));
		}
		printf("\n");
	  digitalWrite(0,LOW);/**/
	startup = 1;


	/*Pathfinding test

		struct Sw_PATH2 * (Route)[MAX_ROUTE] = {NULL};
		//memset(Route,NULL,MAX_ROUTE);

		//train_set_path(trains[4],A,B);
		pathFinding2(Units[4]->B[23],stations[1]->Blocks[0],Route);*/


	//Done with setup when there is at least one client
	if(connected_clients == 0){
		printf("                   Waiting until for a client connects\n");
	}
	while(connected_clients == 0){
		usleep(1000000);
	}
	initialise = 0;

	usleep(400000);

	pthread_t tid[10];
	//throw_switch(Switch[5][2][1]);
	//throw_switch(Switch[6][3][1]);

	/*Timers Test code*/
		/*printf("Create clear thread\n");*/
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
	/*Time test code*/
		/*
		clock_t t;
		t = clock();
		usleep(1000000);
		t = clock() - t;
		printf ("It took me %d clicks (%f seconds).\n",t,((float)t)/CLOCKS_PER_SEC);
		struct timeval  tv1, tv2;
		gettimeofday(&tv1, NULL);
		usleep(1000000);
		gettimeofday(&tv2, NULL);

		printf ("Total time = %f seconds\n",
	         (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
	         (double) (tv2.tv_sec - tv1.tv_sec));*/

	printf("Test octal: 023 = %i\n",023);

	//usleep(5000000);


	printf("Creating Threads\n");
	pthread_create(&tid[0], NULL, do_Magic, NULL);
	pthread_create(&tid[1], NULL, STOP_FUNC, NULL);
	pthread_create(&tid[2], NULL, clear_timers, NULL);

	pthread_create(&tid[3], NULL, TRAIN_SIMA, NULL);
	usleep(500000);
	pthread_create(&tid[4], NULL, TRAIN_SIMB, NULL);
  //pthread_create(&tid[5], NULL, TRAIN_SIMC, NULL);
  //pthread_create(&tid[6], NULL, TRAIN_SIMD, NULL);


	//COM_Recv();

	//Web_Emergency_Stop(ACTIVATE);
	/*
	usleep(22000000);

	pthread_mutex_lock(&mutex_lockB);
	//throw_switch(Switch[4][6][1]);
	//throw_switch(Switch[4][6][2]);
	pthread_mutex_unlock(&mutex_lockB);
	*/
	pthread_join(tid[0],NULL);
	printf("Magic JOINED\n");
	pthread_join(tid[1],NULL);
	printf("STOP JOINED\n");
	pthread_join(tid[2],NULL);
	printf("Timer JOINED\n");
	pthread_join(tid[3],NULL);
	printf("SimA JOINED\n");
	pthread_join(tid[4],NULL);
	printf("SimB JOINED\n");
	pthread_join(thread_UART,NULL);
	pthread_join(thread_web_server,NULL);
	//procces(C_Adr(6,2,1),1);

  //----- CLOSE THE UART -----
	close(uart0_filestream);

	printf("STOPPED");
	//pthread_exit(NULL);
}
