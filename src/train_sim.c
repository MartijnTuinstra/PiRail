#define _BSD_SOURCE
// #define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include <pthread.h>

#include "./../lib/system.h"

#include "./../lib/rail.h"
#include "./../lib/trains.h"

#include "./../lib/modules.h"

pthread_mutex_t mutex_lockA;

#define delayA 5000000
#define delayB 5000000


void *TRAIN_SIMA(){
	struct Seg *B = Units[4]->B[27];
	struct Seg *N = Units[4]->B[27];
	struct Seg *A = 0;
	int i = 0;

	B->blocked = 1;
	B->change  = 1;

	while(!train_link[Units[4]->B[27]->train]){}


	while(_SYS->_STATE & STATE_RUN){
		printf("Train Sim Step (id:%i)\n",pthread_self());

		pthread_mutex_lock(&mutex_lockA);

		N = Next2(B,1+i);
		if(i > 0){
			A = Next2(B,i);
		}
		if(!N){
			while(1){
				usleep(100000);
			}
		}
		printf(" %i:%i\n",N->Module,N->id);
		N->change = 1;
		N->blocked = 1;
		pthread_mutex_unlock(&mutex_lockA);
		usleep(delayA/2);
		pthread_mutex_lock(&mutex_lockA);
		if(i>0){
			A->change = 1;
			A->blocked = 0;
		}else{
			B->change = 1;
			B->blocked = 0;
		}
		pthread_mutex_unlock(&mutex_lockA);
		usleep(delayA/2);
		pthread_mutex_lock(&mutex_lockA);
		if(N->type == 'T'){
			i++;
		}else{
			B = N;
			i = 0;
		}
		pthread_mutex_unlock(&mutex_lockA);
	}
}

void *TRAIN_SIMB(){
	struct Seg *B = Units[4]->B[23];
	struct Seg *N = Units[4]->B[23];
	struct Seg *A = 0;
	int i = 0;

	B->blocked = 1;
	B->change  = 1;

	while(B->train == 0){}

	while(!train_link[B->train]){}

	train_link[B->train]->halt = 1;

	while(train_link[B->train]->halt == 1){}

	while(_SYS->_STATE & STATE_RUN){
		//printf("Train Sim Step (id:%i)\n",pthread_self());
		while(1){
			if(train_link[2] && train_link[2]->halt == 0){
				break;
			}
			usleep(1000);
		}

		pthread_mutex_lock(&mutex_lockA);

		N = Next2(B,1+i);
		if(i > 0){
			A = Next2(B,i);
		}
		if(!N){
			while(1){
				usleep(100000);
			}
		}
		//printf(" %i:%i:%i\n",N->Adr.M,N->Adr.B,N->Adr.S);
		N->change = 1;
		N->blocked = 1;
		pthread_mutex_unlock(&mutex_lockA);
		usleep(delayA/2);
		pthread_mutex_lock(&mutex_lockA);
		if(i>0){
			A->change = 1;
			A->blocked = 0;
		}else{
			B->change = 1;
			B->blocked = 0;
		}
		pthread_mutex_unlock(&mutex_lockA);
		usleep(delayA/2);
		pthread_mutex_lock(&mutex_lockA);
		if(N->type == 'T'){
			i++;
		}else{
			B = N;
			i = 0;
		}
		pthread_mutex_unlock(&mutex_lockA);
	}
}
/*
void *TRAIN_SIMC(){
	struct Seg *B2 = blocks2[7][5];
	struct Seg *N2 = blocks2[7][5];
	struct Seg *A2[3] = {blocks2[0][0]};
	int i2 = 0;
	pthread_mutex_lock(&mutex_lockA);

	while(_SYS->_STATE & STATE_RUN){
		//printf("Train Sim Step (id:%i)\t",pthread_self());
		N2 = Next2(B2,1+i2);
		if(i2 > 0){
			A2[i2] = Next2(B2,i2);
		}
		if(!N2){
			while(1){
				usleep(100000);
			}
		}
		//printf(" %i:%i:%i\n",N2->Adr.M,N2->Adr.B,N2->Adr.S);
		N2->change = 1;
		N2->blocked = 1;
		pthread_mutex_unlock(&mutex_lockA);
		usleep(delayB/2);
		pthread_mutex_lock(&mutex_lockA);
		if(i2>0){
			A2[i2]->change = 1;
			A2[i2]->blocked = 0;
		}else{
			B2->change = 1;
			B2->blocked = 0;
		}
		pthread_mutex_unlock(&mutex_lockA);
		usleep(delayB/2);
		pthread_mutex_lock(&mutex_lockA);
		if(N2->Adr.S == 0){
			i2++;
		}else{
			B2 = N2;
			i2 = 0;
		}
	}
}

void *TRAIN_SIMD(){
	struct Seg *B2 = blocks2[7][2];
	struct Seg *N2 = blocks2[7][2];
	struct Seg *A2[3] = {blocks2[0][0]};
	int i2 = 0;
	pthread_mutex_lock(&mutex_lockA);

	while(_SYS->_STATE & STATE_RUN){
		//printf("Train Sim Step (id:%i)\t",pthread_self());
		N2 = Next2(B2,1+i2);
		if(i2 > 0){
			A2[i2] = Next2(B2,i2);
		}
		if(!N2){
			while(1){
				usleep(100000);
			}
		}
		//printf(" %i:%i:%i\n",N2->Adr.M,N2->Adr.B,N2->Adr.S);
		N2->change = 1;
		N2->blocked = 1;
		pthread_mutex_unlock(&mutex_lockA);
		usleep(delayB/2);
		pthread_mutex_lock(&mutex_lockA);
		if(i2>0){
			A2[i2]->change = 1;
			A2[i2]->blocked = 0;
		}else{
			B2->change = 1;
			B2->blocked = 0;
		}
		pthread_mutex_unlock(&mutex_lockA);
		usleep(delayB/2);
		pthread_mutex_lock(&mutex_lockA);
		if(N2->Adr.S == 0){
			i2++;
		}else{
			B2 = N2;
			i2 = 0;
		}
	}
}
*/
