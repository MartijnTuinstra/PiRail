#ifndef H_train
	#include "./train.h"
#endif

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
	Z->control = 0; //0 = User (Manual), 1 = Semi-Automatic, 2 = Computer (Automatic)
	Z->use = 0;
	Z->dir = 0;
	Z->halt = FALSE;
	//return Z;
	//printf("Add train %i (#%i)\n",iTrain,Z->DCC_ID);
	trains[iTrain++] = Z;
	if(DCC < 9999){
		DCC_train[DCC] = Z;
	}
	return (iTrain - 1);
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
	printf("Add train\n");

	int value = add_train(DCC,speed,name,type);

	printf("Open file\n");
	FILE * f;
	f = fopen("./trains/trainlist_raw.txt","a");
	printf("write to file\n");
	fprintf(f,"%d\t%s\t%i\t%c\t%i\t000000000000000000000000000\r\n",value,name,DCC,type,speed);
	fclose(f);

	//Return train ID
	return value;
}

void init_trains(){
	FILE *f;
	f = fopen("./trains/trainlist_raw.txt","r");
	char line[256] = "";
	int line_nr = 0;
	int nr_trains = 0;

	while (fgets(line, sizeof(line), f)) {
		/* note that fgets don't strip the terminating \n, checking its
		   presence would allow to handle lines longer that sizeof(line) */
		char *D1 = strchr(line, '\t');
		char *D2 = strchr(&line[(D1-line)+1], '\t');
		char *D3 = strchr(&line[(D2-line)+1], '\t');
		char *D4 = strchr(&line[(D3-line)+1], '\t');
		char *D5 = strchr(&line[(D4-line)+1], '\t');
		char *D6 = strchr(&line[(D5-line)+1], '\r');
		if (D1 && D2 && D3 && D4 && D5 && D6){ /* deal with error: / not present" */;
			int start = 0;
			int tab1 = D1-line;
			int tab2 = D2-line;
			int tab3 = D3-line;
			int tab4 = D4-line;
			int tab5 = D5-line;
			int end = D6-line;

			char L1[21],L2[7],L3[5],L4[5],L5[30];

			memset(L1,0,21);
			memset(L2,0,7);
			memset(L3,0,5);
			memset(L4,0,5);
			memset(L5,0,30);

			for(int i = tab1;i<=end;i++){
				if(i < tab2){
				 L1[(i-tab1)] = line[i];
			 }else if(i > tab2 && i < tab3){
				 L2[(i-tab2-1)] = line[i];
			 }else if(i > tab3 && i < tab4){
				 L3[(i-tab3-1)] = line[i];
			 }else if(i > tab4 && i < tab5){
				 L4[(i-tab4-1)] = line[i];
			 }else if(i > tab5 && i < end){
				 L5[(i-tab5-1)] = line[i];
				}
				if(i == tab2){
				 L1[(i-tab1)] = 0;
			 }else if(i == tab3){
				 L2[(i-tab2-1)] = 0;
			 }else if(i == tab4){
				 L3[(i-tab3-1)] = 0;
			 }else if(i == tab5){
				 L4[(i-tab4-1)] = 0;
				}else if(i == end){
				 L5[(i-tab5-1)] = 0;
				}
			}

			//printf("line %02i:\t%s\t\t%s(#%i)\t\t%s\t\t%s\n",line_nr,L1,L2,atoi(L2),L3,L4);
			if(line_nr != 0){
				if(atoi(L2) < 10000){
					add_train(atoi(L2),atoi(L4),L1,L3[0]);
					nr_trains++;
				}
			}
		}else{
			printf("Corrupt trainlist file!!!!!!!\n");
		}
		line_nr++;
	}
	fclose(f);

	printf("|                           %i\ttrains found                               |\n",nr_trains);
	printf("|                                                                          |\n");

	for(int i = 0;i<nr_trains;i++){
		char buf[40] = "|                    ";
		sprintf(buf,"%s(%02i) #%04i  %s",buf,i,trains[i]->DCC_ID,trains[i]->name);
		printf("%s",buf);
		for(int j = strlen(buf);j<69;j++)
			printf(" ");
		printf("|\n");
	}

	printf("|                                                                          |\n");
}

int link_train(char link,int train){
	if(train_link[link] == NULL && trains[train]->use == 0){
		printf("link is empty %i\n",train_link[link]);
		train_link[link] = trains[train];
		if(train != 0 || train != 1){
			train_link[link]->use = 1;
		}else{
			printf("Duplicates allowed");
		}
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
		return 0;
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

void train_set_speed(struct train *T,char speed){
	if(!T){ return;printf("Empty T\n");}
	T->cur_speed = speed;
	Z21_GET_LOCO_INFO(T->DCC_ID);
}

void train_set_dir(struct train *T,char dir){
	if(!T){ return;printf("Empty T\n");}
	if(dir == 0){
		printf("Set dir to forward\n");
		T->dir = dir;
	}else{
		printf("Set dir to reverse\n");
		T->dir = dir;
	}
	Z21_GET_LOCO_INFO(T->DCC_ID);
}

void train_set_route(struct train *T,struct Station * S){
	if(pathFinding(T->Cur_Block,S->Blocks[0],T->Route,&T->Sw_len)){
		T->halt = 0;
		T->Destination = S->Blocks[0];
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
