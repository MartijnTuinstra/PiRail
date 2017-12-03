int add_train(int DCC,int speed,char name[],char type){
	struct train *Z = (struct train*)malloc(sizeof(struct train));

	struct adr Route[20] = {{0,0,0,0}};

	Z->DCC_ID = DCC;
	Z->type = type;
	strcpy(Z->name,name);
	Z->max_speed = speed;
	Z->accelerate_speed = 60;
	Z->break_speed = 50;
	Z->cur_speed = 100;
	Z->control = 0; //0 = User, 1 = Computer
	Z->use = 0;
	Z->halt = FALSE;

	for(int i = 0;i<MAX_ROUTE;i++){
		Z->Route[i] = C_AdrT(0,0,0,'e');
	}
	//return Z;
	//printf("Add train %i\n",iTrain);
	trains[iTrain++] = Z;
	return (iTrain - 1);
}

void req_train(char ID, struct adr Adr){
	char data[40] = "";
	sprintf(data, "[%i,\"%i:%i:%i\"]", ID,Adr.M,Adr.B,Adr.S);
	new_message(11,data);
	printf("\n\new train Requested %i\n\n\n",ID);
}

int create_train(int DCC,int speed,char name[],char type){
	printf("Create train, %i trains in library\n",iTrain);
	for(int i = 0;i<iTrain;i++){
		printf("Train[%i]\n",i);
		if(trains[i]->DCC_ID == DCC){
			printf("Address already in use");
			return -1;
		}
	}
	printf("Open file\n");
	FILE * f;
	f = fopen("./trains/trainlist_raw.txt","a");
	printf("write to file\n");
	fprintf(f,"%s\t%i\t%c\t%i\r\n",name,DCC,type,speed);
	fclose(f);
	printf("Add train\n");
	return add_train(DCC,speed,name,type);
}

void init_trains(){
	FILE *f;
	f = fopen("./trains/trainlist_raw.txt","r");
	char line[256] = "";
	int line_nr = 0;

	while (fgets(line, sizeof(line), f)) {
		/* note that fgets don't strip the terminating \n, checking its
		   presence would allow to handle lines longer that sizeof(line) */
		char *D1 = strchr(line, '\t');
		char *D2 = strchr(&line[(D1-line)+1], '\t');
		char *D3 = strchr(&line[(D2-line)+1], '\t');
		char *D4 = strchr(&line[(D2-line)+1], '\r');
		if (D1 != NULL && D2 != NULL && D3 != NULL && D4 != NULL){ /* deal with error: / not present" */;
			int start = 0;
			int tab1 = D1-line;
			int tab2 = D2-line;
			int tab3 = D3-line;
			int end = D4-line;

			char L1[21],L2[5],L3[5],L4[5];

			memset(L1,0,21);
			memset(L2,0,5);
			memset(L3,0,5);
			memset(L4,0,5);

			for(int i = (start);i<=end;i++){
				if(i < tab1){
				 L1[(i-start)] = line[i];
				}else if(i > tab1 && i < tab2){
				 L2[(i-tab1-1)] = line[i];
				}else if(i > tab2 && i < tab3){
				 L3[(i-tab2-1)] = line[i];
				}else if(i > tab3 && i < end){
				 L4[(i-tab3-1)] = line[i];
				}
				if(i == tab1){
				 L1[(i-start)] = 0;
				}else if(i == tab2){
				 L2[(i-tab1-1)] = 0;
				}else if(i == tab3){
				 L3[(i-tab2-1)] = 0;
				}else if(i == end){
				 L4[(i-tab3-1)] = 0;
				}
			}

			//printf("line %i:\t%s\t\t%s\t\t%s\t\t%s\n",line_nr,L1,L2,L3,L4);
			if(line_nr != 0){
				add_train(atoi(L2),atoi(L4),L1,L3[0]);
			}
		}else{
			printf("Corrupt trainlist file!!!!!!!\n");
		}
		line_nr++;
	}
	fclose(f);

	int nr_trains = 0;
	//printf("Name\t\t\tDCC\tMax Speed\n");
	for(int q = 0;q<MAX_TRAINS;q++){
		if(trains[q] != NULL){
			nr_trains++;
			//printf("%s\t",trains[q]->name);
			if(strlen(trains[q]->name) < 12){
				//printf("\t");
			}
			if(strlen(trains[q]->name) < 8){
				//printf("\t");
			}
			//printf("#%i\t%i\n",trains[q]->DCC_ID,trains[q]->max_speed);
		}
	}

	printf("|                           %i\ttrains found                               |\n",nr_trains);
}

int link_train(char link,int train){
	if(train_link[link] == NULL && trains[train]->use == 0){
		printf("link is empty %i\n",train_link[link]);
		train_link[link] = trains[train];
		train_link[link]->use = 1;
		printf("Set to %i\n",train_link[link]);
		return 1;
	}else{
		return 0;
	}
}

void unlink_train(char link){
	train_link[link]->use = 0;
	train_link[link] = NULL;
}

void setup_JSON(int arr[], int arr2[], int size, int size2){
	char buf[100];
	FILE *fr;

	sprintf(buf, "[[");

	for(int i = 0;i<size-1;i++){
		sprintf(buf,"%s%i,",buf,arr[i]);
	}

	sprintf(buf,"%s%i],[",buf,arr[size-1]);

	int s = 0;
	for(int i = 0;i<size2;i++){
		if(s == 0){
			s = 1;
		}else{
			sprintf(buf,"%s,",buf);
		}
		sprintf(buf,"%s%i",buf,arr2[i]);
	}
	strcat(buf,"]]");

	fr = fopen("setup.json","w");
	fprintf(fr,"%s",buf);
	fclose(fr);

	strcat(setup_data,"{\"Setup\" : ");
	strcat(setup_data,buf);
	strcat(setup_data,"}");
}

struct train_timer_th_data{
   int  thread_id;
	 int  Flag;
	 int speed;
	 struct train * T;
	 struct Seg * B;
};

void *train_timer(void *threadArg){
	struct train_timer_th_data *my_data;
	my_data = (struct train_timer_th_data *) threadArg;
	int id = my_data->thread_id;
	struct train * T = my_data->T;
	struct Seg * B = my_data->B;
	int des_s = my_data->speed;

	free(threadArg);

	//int d = 100;
	//Real distance
	float d = (float)B->length/100 * TRACK_SCALE;

	int cur_s = T->cur_speed;
	int max_s = T->max_speed;

	float speed_step = (float)max_s/128;

	if(des_s/speed_step > 128){
		des_s = max_s;
	}

	if(ROUND((float)cur_s/speed_step) == ROUND((float)des_s/speed_step)){
		return;
	}

	float total_time = (((float)d)/(cur_s))*3.6;

	//pMPH=(pDistance/ScaleMile)*(3600/pTime);
	printf("Train name: %s\t",T->name);
	int step_dif = ROUND((float)des_s/speed_step) - ROUND((float)cur_s/speed_step);
	printf("Length: %f\tstep: %i->%i, d%i\t",d,ROUND((float)cur_s/speed_step),ROUND((float)des_s/speed_step),step_dif);
	int time_step = (int)(((float)total_time/step_dif)*1000000);
	if(time_step < 0){
		time_step = -1*time_step;
	}
	printf("Time %f\tTime step: %i\n",total_time,time_step);

	printf("Max_speed: %i\tCur_speed: %i\tDes_speed: %i\n",max_s,cur_s,des_s);

	if(step_dif>0){
		for(int i = 0;i<=step_dif;i++){
			T->cur_speed++;
			T->cur_speed++;
			printf("++");
			usleep(time_step);
			if(timers[id] > 2){
				goto END;
			}
			usleep(time_step);
			if(timers[id] > 2){
				goto END;
			}
		}
	}else{
		for(int i = 0;i<=(-step_dif);i++){
			T->cur_speed--;
			T->cur_speed--;
			printf("--");
			usleep(time_step);
			if(timers[id] > 2){
				goto END;
			}
			usleep(time_step);
			if(timers[id] > 2){
				goto END;
			}
		}
	}
	T->cur_speed = des_s;
	timers[id] = 2;
	T->timer = 0;
	T->timer_id = 0;
	printf("%i done\tCurrent speed %i\n",id,T->cur_speed);

	END:{}
}

void train_speed(struct Seg * B,struct train * T,char speed){
	if(T == NULL){
		return;
	}
	printf("\t#%i\t%s\n",T->DCC_ID,T->name);

	if(T->timer == 2){
		printf("TIMER ALLREADY STARTED\n");
	}else{

		int des_s = speed;

		struct train_timer_th_data * thread_data = (struct train_timer_th_data*)malloc(sizeof(struct train_timer_th_data));

		int i = 0;
		while(1){
			if(timers[i] == 0){
				timers[i] = 1;
				T->timer_id = i;
				T->timer = 2;

				thread_data->thread_id = i;
				thread_data->T = T;
				thread_data->B = B;
				thread_data->Flag = 1;
				thread_data->speed = des_s;
				//printf("Create train signal timer, %ius sleep, fire %i times\n",thread_data->time,thread_data->r);
				pthread_create(&timer_thread[i], NULL, train_timer, (void *) thread_data);
				break;
			}
			i++;
			if(i==MAX_TIMERS){
				COM_set_train_speed(T,des_s);
			}
		}
	}
}

void train_stop(struct train * T){
	printf("KILL TRAIN:\t#%i\t%s\n",T->DCC_ID,T->name);
}

void train_signal(struct Seg * B,struct train * T,int type){
	if(T == NULL){
		return;
	}
	printf("\t#%i\t%s\n",T->DCC_ID,T->name);

	if(T->timer == 1){
		printf("TIMER ALLREADY STARTED\n");
	}else{

		int des_s;
		if(type == AMBER){
			des_s = 40;
		}else{
			des_s = 0;
		}

		struct train_timer_th_data * thread_data = (struct train_timer_th_data*)malloc(sizeof(struct train_timer_th_data));

		int i = 0;
		while(1){
			if(timers[i] == 0){
				timers[i] = 1;
				T->timer = 1;
				T->timer_id = i;
				thread_data->thread_id = i;
				thread_data->T = T;
				thread_data->B = B;
				thread_data->Flag = 2;
				thread_data->speed = des_s;
				//printf("Create train signal timer, %ius sleep, fire %i times\n",thread_data->time,thread_data->r);
				pthread_create(&timer_thread[i], NULL, train_timer, (void *) thread_data);
				break;
			}
			i++;
			if(i==MAX_TIMERS){
				COM_set_train_speed(T,des_s);
			}
		}
	}
}

void train_block_timer();
