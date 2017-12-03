
void *TRAIN_SIMA(){
	struct Seg *B = blocks2[8][5];
	struct Seg *N = blocks2[8][5];
	struct Seg *A = blocks2[0][0];
	int i = 0;

	B->blocked = 1;
	B->change  = 1;


	while(!stop){
		//printf("Train Sim Step (id:%i)\n",pthread_self());
		while(1){
			if(train_link[1] && train_link[1]->halt == 0){
				break;
			}
			usleep(100);
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

void *TRAIN_SIMB(){
	struct Seg *B = blocks2[4][23];
	struct Seg *N = blocks2[4][23];
	struct Seg *A = blocks2[0][0];
	int i = 0;

	B->blocked = 1;
	B->change  = 1;

	while(blocks2[4][23]->train == 0){}

	while(!train_link[blocks2[4][23]->train]){}

	train_link[blocks2[4][23]->train]->halt = 1;

	while(train_link[blocks2[4][23]->train]->halt == 1){}

	while(!stop){
		//printf("Train Sim Step (id:%i)\n",pthread_self());
		while(1){
			if(train_link[2] && train_link[2]->halt == 0){
				break;
			}
			usleep(100);
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

	while(!stop){
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

	while(!stop){
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
