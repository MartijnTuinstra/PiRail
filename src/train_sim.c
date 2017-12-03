
void *TRAIN_SIMA(){
	struct Seg *B = blocks[4][7][2];
	struct Seg *N = blocks[4][7][2];
	struct Seg *A = blocks[0][0][0];
	int i = 0;
	pthread_mutex_lock(&mutex_lockA);

	while(!stop){
		printf("Train Sim Step (id:%i)\t",pthread_self());
		N = Next(B->Adr,1+i);
		if(i > 0){
			A = Next(B->Adr,i);
		}
		//printf(" %i:%i:%i\n",N->Adr.M,N->Adr.B,N->Adr.S);
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
	struct Seg *B2 = blocks[7][4][5];
	struct Seg *N2 = blocks[7][4][5];
	struct Seg *A2[3] = {blocks[0][0][0],blocks[0][0][0],blocks[0][0][0]};
	int i2 = 0;
	pthread_mutex_lock(&mutex_lockA);

	while(!stop){
		//printf("Train Sim Step (id:%i)\t",pthread_self());
		N2 = Next(B2->Adr,1+i2);
		if(i2 > 0){
			A2[i2] = Next(B2->Adr,i2);
		}
		//printf(" %i:%i:%i\n",N2->Adr.M,N2->Adr.B,N2->Adr.S);
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

void *TRAIN_SIMC(){
	struct Seg *B2 = blocks[7][3][5];
	struct Seg *N2 = blocks[7][3][5];
	struct Seg *A2[3] = {blocks[0][0][0],blocks[0][0][0],blocks[0][0][0]};
	int i2 = 0;
	pthread_mutex_lock(&mutex_lockA);

	while(!stop){
		//printf("Train Sim Step (id:%i)\t",pthread_self());
		N2 = Next(B2->Adr,1+i2);
		if(i2 > 0){
			A2[i2] = Next(B2->Adr,i2);
		}
		//printf(" %i:%i:%i\n",N2->Adr.M,N2->Adr.B,N2->Adr.S);
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

void *TRAIN_SIMD(){
	struct Seg *B2 = blocks[7][2][5];
	struct Seg *N2 = blocks[7][2][5];
	struct Seg *A2[3] = {blocks[0][0][0],blocks[0][0][0],blocks[0][0][0]};
	int i2 = 0;
	pthread_mutex_lock(&mutex_lockA);

	while(!stop){
		//printf("Train Sim Step (id:%i)\t",pthread_self());
		N2 = Next(B2->Adr,1+i2);
		if(i2 > 0){
			A2[i2] = Next(B2->Adr,i2);
		}
		//printf(" %i:%i:%i\n",N2->Adr.M,N2->Adr.B,N2->Adr.S);
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
