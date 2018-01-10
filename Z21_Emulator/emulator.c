#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "Z21_Server.h"

#include "Z21_Codes.h"

struct Loc {
	uint16_t adr;
	uint8_t  fahrstufen;
	uint8_t  speed;
	uint8_t  f[4];
};

struct Loc * Engines[16383];

struct Loc * create_Loc(uint16_t adr){
	struct Loc * Z = (struct Loc *)malloc(sizeof(struct Loc));
	Z->adr = adr;
	Z->fahrstufen = 4;
	Z->speed = 0;
	memset(Z->f,0,4);
	return Z;
}

#include "Z21_Server.c"

char stop = 0;

void main(){
	pthread_t thread_web_server;
	pthread_create(&thread_web_server, NULL, server, NULL);

	while(stop == 0){
		printf(".");
		sleep(10);
	}
    printf("Done");
    return;
}
