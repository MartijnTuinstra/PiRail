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
#include "./../lib/switch.h"

#include "./../lib/trains.h"

#include "./../lib/logger.h"

#include "./../lib/pathfinding.h"
#include "./../lib/com.h"
#include "./../lib/Z21.h"

#define MAX_TIMERS 10

#define ROUND(nr)  (int)(nr+0.5)

pthread_t train_timer_thread[MAX_TIMERS];
int        train_timer_state[MAX_TIMERS];

Trains ** trains;
int trains_len = 0;
Engines ** engines;
int engines_len = 0;
Cars ** cars;
int cars_len = 0;
struct train_composition ** trains_comp;
int trains_comp_len = 0;

void init_trains(){
	loggerf(INFO, "Initializing cars/engines/trains");
	alloc_trains();
	train_read_confs();
}

int find_free_index(void ** list, int * length){
	if(!list){
		logger("LIST DOESNT EXIST",CRITICAL);
		return -1;
	}
	for(int i = 0;i<*length;i++){
		if(!list[i]){
			return i;
		}
	}

	list = (void **)realloc(list,*length+10);
	*length += 10;
	logger("EXPANDING LIST",WARNING);
	return find_free_index(list, length);
}

void alloc_trains(){
	trains = (Trains **)calloc(10,sizeof(Trains *));
	trains_len = 10;
	engines = (Engines **)calloc(10,sizeof(Engines *));
	engines_len = 10;
	cars = (Cars **)calloc(10,sizeof(Cars *));
	cars_len = 10;
	trains_comp = (struct train_composition **)calloc(10,sizeof(struct train_composition *));
	trains_comp_len = 10;
}

void free_trains(){
	logger("Clearing trains memory",INFO);

	for(int i = 0;i<trains_len;i++){
		if(trains[i]){
			trains[i]->name = free0(trains[i]->name);
			trains[i]->composition = free0(trains[i]->composition);
			trains[i] = free0(trains[i]);
		}
	}

	for(int i = 0;i<engines_len;i++){
		if(engines[i]){
			engines[i]->name = free0(engines[i]->name);
			engines[i]->img_path = free0(engines[i]->img_path);
			engines[i]->icon_path = free0(engines[i]->icon_path);
			engines[i] = free0(engines[i]);
		}
	}

	for(int i = 0;i<cars_len;i++){
		if(cars[i]){
			cars[i]->name = free0(cars[i]->name);
			cars[i]->img_path = free0(cars[i]->img_path);
			cars[i]->icon_path = free0(cars[i]->icon_path);
			cars[i] = free0(cars[i]);
		}
	}

	for(int i = 0;i<trains_comp_len;i++){
		if(trains_comp[i]){
			trains_comp[i]->name = free0(trains_comp[i]->name);
			trains_comp[i]->composition = free0(trains_comp[i]->composition);
			trains_comp[i] = free0(trains_comp[i]);
		}
	}

	trains = free0(trains);
	engines = free0(engines);
	cars = free0(cars);
	trains_comp = free0(trains_comp);

	trains_len = 0;
	engines_len	= 0;
	cars_len = 0;
	trains_comp_len = 0;
}

int create_train(char * name, int nr_stock, struct train_comp_ws * comps){
	Trains * Z = (Trains *)malloc(sizeof(Trains));

	Z->nr_stock = nr_stock;
	Z->composition = (struct train_comp *)malloc(nr_stock * sizeof(struct train_comp));

	Z->max_spd = 0xFFFF;
	Z->length = 0;

	for(int i = 0;i<nr_stock;i++){
		loggerf(DEBUG, "create_train: stock %c %i", comps[i].type, comps[i].ID);
		Z->composition[i].type = comps[i].type;
		Z->composition[i].id = comps[i].ID;
		if(comps[i].type == 'E' || comps[i].type == 'e'){
			if(comps[i].ID < engines_len && engines[comps[i].ID]){
				Z->length += engines[comps[i].ID]->length;
				if(Z->max_spd > engines[comps[i].ID]->max_spd){
					Z->max_spd = engines[comps[i].ID]->max_spd;
				}

				Z->composition[i].p = engines[comps[i].ID];
				loggerf(DEBUG, "Engine (%i) found", comps[i].ID);
			}
			else{
				loggerf(ERROR, "Engine (%i) doesn't exist", comps[i].ID);
			}
		}
		else{ //Car
			if(comps[i].ID < cars_len && cars[comps[i].ID]){
				Z->length += cars[comps[i].ID]->length;
				if(Z->max_spd > cars[comps[i].ID]->max_spd){
					Z->max_spd = cars[comps[i].ID]->max_spd;
				}

				loggerf(DEBUG, "Car (%i) found", comps[i].ID);
				Z->composition[i].p = cars[comps[i].ID];
			}
			else{
				loggerf(ERROR, "Car (%i) doesn't exist", comps[i].ID);
			}
		}
	}

	Z->name = (char *)malloc(strlen(name)+2);
	strcpy(Z->name,name);

	int index = find_free_index((void **)trains,&trains_len);

	trains[index] = Z;

	loggerf(INFO, "Train created at %i",index);
}

int create_train_from_comp(){}

int create_engine(char * name,int DCC,char * img, char * icon, int max, char type, int length){
	//DCC cant be used twice
	for(int i = 0;i<engines_len;i++){
		if(engines[i] && engines[i]->DCC_ID == DCC){
			loggerf(WARNING,"create_engine: found duplicate: %s",engines[i]->name);
		}
	}
	Engines * Z = (Engines *)malloc(sizeof(Engines));

	Z->name = (char *)malloc(strlen(name)+2);
	Z->img_path = (char *)malloc(strlen(img)+2);
	Z->icon_path = (char *)malloc(strlen(icon)+2);

	strcpy(Z->name,name);
	strcpy(Z->img_path,img);
	strcpy(Z->icon_path,icon);

	Z->DCC_ID = DCC;

	Z->length = length;
	Z->max_spd = max;
	Z->cur_spd = 0;

	Z->type = type;
	_Bool dir = FALSE;

	int index = find_free_index((void **)engines,&engines_len);

	engines[index] = Z;

	loggerf(INFO, "Engine \"%s\" created at %i\t%s, %s", name, index, img, icon);
}

int create_car(char * name,int nr,char * img, char * icon, char type, int length){
	Cars * Z = (Cars *)malloc(sizeof(Cars));

	Z->name = (char *)malloc(strlen(name)+2);
	Z->img_path = (char *)malloc(strlen(img)+2);
	Z->icon_path = (char *)malloc(strlen(icon)+2);

	Z->nr = nr;
	Z->length = length;

	strcpy(Z->name,name);
	strcpy(Z->img_path,img);
	strcpy(Z->icon_path,icon);

	Z->type = type;
	_Bool dir = FALSE;

	int index = find_free_index((void **)cars,&cars_len);

	cars[index] = Z;

	loggerf(INFO, "Car \"%s\" created at %i",name,index);
}

int train_read_confs(){
	char * header = (char *)malloc(2);

	memset(header,2,0);

	FILE *f;
	f = fopen(ENGINES_CONF,"rb");

	if(!f){
		loggerf(CRITICAL, "ENGINES COMPS CONFIG FILE NOT FOUND");
		raise(SIGTERM);
		return 0;
	}

	fread(header, 2, 2, f);

	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 2, SEEK_SET);  //same as rewind(f);

	if(header[0] == CONF_VERSION){
		// Compatible Read further
		int engines_nr = header[1];
		if(engines_nr > 0){
			char *buffer = malloc(fsize - 1);
			fread(buffer, fsize, 1, f);

			long index = 0;

			for(int i = 0;i < engines_nr;i++){
				struct engine_conf * engine = (void *)&buffer[index];
				
				if(engine->check != 0){
					loggerf(ERROR,"Enignes config file wrong format in engine number %i",i+1);
					break;
				}

				char * name = (char *)malloc(engine->name_len+2);
				char * img = (char *)malloc(engine->img_path_len+2);
				char * icon = (char *)malloc(engine->icon_path_len+2);

				index += sizeof(struct engine_conf);

				memset(name,0,engine->name_len+2);
				memset(img,0,engine->img_path_len+2);
				memset(icon,0,engine->icon_path_len+2);

				strncpy(name,&buffer[index],engine->name_len);
				index += engine->name_len;

				strncpy(img,&buffer[index],engine->img_path_len);
				index += engine->img_path_len;

				strncpy(icon,&buffer[index],engine->icon_path_len);
				index += engine->icon_path_len;

				if(buffer[index] != 0){
					loggerf(ERROR, "%s, %s, %s", name, img, icon);
					loggerf(ERROR, "Engines config file wrong format / padding after engine number %i",i+1);
					break;
				}
				create_engine(name, engine->DCC_ID, img, icon, engine->max_spd, engine->type, engine->length);

				free(name);
				free(img);
				free(icon);
				index += 1;
			}

			free(buffer);

			// struct engine_conf;
		}
	}
	else{
		loggerf(ERROR,"ENGINES_CONF has wrong format (%i) and is not compatible",header[0]);
		return 0;
	}

	fclose(f);

	memset(header,2,0);

	f = fopen(CARS_CONF,"rb");

	if(!f){
		loggerf(CRITICAL, "CARS COMPS CONFIG FILE NOT FOUND");
		raise(SIGTERM);
		return 0;
	}

	fread(header, 2, 2, f);

	fseek(f, 0, SEEK_END);
	fsize = ftell(f);
	fseek(f, 2, SEEK_SET);  //same as rewind(f);

	if(header[0] == CONF_VERSION){
		// Compatible Read further
		int cars_nr = header[1];
		if(cars_nr > 0){
			char *buffer = malloc(fsize - 1);
			fread(buffer, fsize, 1, f);

			long index = 0;

			for(int i = 0;i < cars_nr;i++){
				struct car_conf * car = (void *)&buffer[index];
				
				if(car->check != 0){
					loggerf(ERROR,"Cars config file wrong format in car number %i",i+1);
					break;
				}

				char * name = (char *)malloc(car->name_len+2);
				char * img = (char *)malloc(car->img_path_len+2);
				char * icon = (char *)malloc(car->icon_path_len+2);

				index += sizeof(struct car_conf);

				memset(name,0,car->name_len+2);
				memset(img,0,car->img_path_len+2);
				memset(icon,0,car->icon_path_len+2);

				strncpy(name,&buffer[index],car->name_len);

				index += car->name_len;
				strncpy(img,&buffer[index],car->img_path_len);

				index += car->img_path_len;
				strncpy(icon,&buffer[index],car->icon_path_len);

				index += car->icon_path_len;

				if(buffer[index] != 0){
					loggerf(ERROR,"Cars config file wrong format / padding after car number %i",i+1);
					break;
				}
				create_car(name, car->nr, img, icon, car->type, car->length);

				free(name);
				free(img);
				free(icon);
				index += 1;
			}

			free(buffer);

			// struct engine_conf;
		}
	}
	else{
		loggerf(ERROR,"CARS_CONF has wrong format (%i) and is not compatible",header[0]);
		return 0;
	}

	fclose(f);

	memset(header,2,0);

	f = fopen(TRAIN_COMPS_CONF,"r");

	if(!f){
		loggerf(CRITICAL, "TRAINS COMPS CONFIG FILE NOT FOUND");
		raise(SIGTERM);
		return 0;
	}

	fread(header, 2, 2, f);

	fseek(f, 0, SEEK_END);
	fsize = ftell(f);
	fseek(f, 2, SEEK_SET);  //same as rewind(f);

	if(header[0] == CONF_VERSION){
		// Compatible Read further
		int trains_nr = header[1];
		if(trains_nr > 0){
			char *buffer = malloc(fsize - 1);
			fread(buffer, fsize, 1, f);

			long index = 0;

			for(int i = 0;i < trains_nr;i++){
				struct train_comp_conf * train = (void *)&buffer[index];
				
				if(train->check != 0){
					loggerf(ERROR,"Trains config file wrong format in train number %i",i+1);
					break;
				}

				char * name = (char *)malloc(train->name_len+2);
				memset(name,0,train->name_len+2);

				index += sizeof(struct train_comp_conf);

				strncpy(name,&buffer[index],train->name_len);

				index += train->name_len;

				struct train_comp_ws * comp = (struct train_comp_ws *)calloc(train->nr_stock,sizeof(struct train_comp_ws));

				memcpy(comp,&buffer[index],train->nr_stock*sizeof(struct train_comp_ws));

				index += train->nr_stock * sizeof(struct train_comp_ws);

				if(buffer[index] != 0){
					loggerf(ERROR,"Trains config file wrong format / padding after train number %i",i+1);
					break;
				}
				create_train(name, train->nr_stock, comp);

				index += 1;
			}

			free(buffer);

			// struct engine_conf;
		}
	}
	else{
		loggerf(ERROR,"TRAIN_COMPS_CONF has wrong format (%i) and is not compatible",header[0]);
		return 0;
	}

	fclose(f);

	_SYS_change(STATE_TRAIN_LOADED, 1);
	return 1;
}

int link_train(char link,char train, char type){
	//Link = follow ID
	//train = tID
	if(type > 0){
		// Create train from engine
		return;
	}

	for(int i = 0; i < trains[train]->nr_engines; i++){
		if(engines[trains[train]->engines[i]]->use){
			return 0;
		}
	}

	if(train_link[link] == NULL){
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


struct train *trains2[MAX_TRAINS] = {};
struct train *DCC_train[9999] = {};
struct train *train_link[MAX_TRAINS];
int iTrain = 0; //Counter for trains in library
int bTrain = 0; //Counter for trains on layout

int add_train2(int DCC,int speed,char name[],char type){
	printf("Add train\n");
	struct train *Z = (struct train*)malloc(sizeof(struct train));
	printf("Add train\n");

	struct adr Route[20] = {{0,0,0,0}};

	Z->DCC_ID = DCC;
	Z->ID = iTrain;
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
	trains2[iTrain++] = Z;
	if(DCC < 9999){
		DCC_train[DCC] = Z;
	}
	return (iTrain - 1);
}

int create_train2(int DCC,int speed,char name[],char type){
	printf("Create train, %i trains in library\n",iTrain);
	for(int i = 0;i<iTrain;i++){
		printf("Train[%i]\n",i);
		if(trains2[i]->DCC_ID == DCC){
			printf("Address already in use");
			return -1;
		}
	}
	printf("Add train\n");

	int value = add_train2(DCC,speed,name,type);

	printf("Open file\n");
	FILE * f;
	f = fopen("./trains/trainlist_raw.txt","a");
	printf("write to file\n");
	fprintf(f,"%d\t%s\t%i\t%c\t%i\t000000000000000000000000000\r\n",value,name,DCC,type,speed);
	fclose(f);

	//Return train ID
	return value;
}

void init_trains2(){
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

			printf("line %02i:\t%s\t\t%s(#%i)\t\t%s\t\t%s\n",line_nr,L1,L2,atoi(L2),L3,L4);
			if(line_nr != 0){
				if(atoi(L2) < 10000){
					add_train2(atoi(L2),atoi(L4),L1,L3[0]);
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
		sprintf(buf,"%s(%02i) #%04i  %s",buf,i,trains2[i]->DCC_ID,trains2[i]->name);
		printf("%s",buf);
		for(int j = strlen(buf);j<69;j++)
			printf(" ");
		printf("|\n");
	}

	printf("|                                                                          |\n");
}

int link_train2(char link,int train){
	loggerf(ERROR, "Depricated");
	//Link = follow ID
	//train = tID
	if(train_link[link] == NULL && trains2[train]->use == 0){
		printf("link is empty %i\n",train_link[link]);
		train_link[link] = trains2[train];
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

void unlink_train2(char link){
	loggerf(ERROR, "Depricated");
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
			if(train_timer_state[id] > 2){
				goto END;
			}
			usleep(time_step);
			if(train_timer_state[id] > 2){
				goto END;
			}
		}
	}else{
		for(int i = 0;i<=(-step_dif);i++){
			T->cur_speed--;
			T->cur_speed--;
			printf("--");
			usleep(time_step);
			if(train_timer_state[id] > 2){
				goto END;
			}
			usleep(time_step);
			if(train_timer_state[id] > 2){
				goto END;
			}
		}
	}
	T->cur_speed = des_s;
	train_timer_state[id] = 2;
	T->timer = 0;
	T->timer_id = 0;
	printf("%i done\tCurrent speed %i\n",id,T->cur_speed);

	END:{}
}

void *clear_train_timers(){
	while(_SYS->_STATE & STATE_RUN){
		for(int i = 0;i<MAX_TIMERS;i++){
			if(train_timer_state[i] == 2){
				pthread_join(train_timer_thread[i], NULL);
				train_timer_state[i] = 0;
				printf("Reset time %i\n",i);
			}
		}
		usleep(10000);
	}
}
void train_speed(block * B, Trains * T, char speed){
	loggerf(ERROR, "TODO: implement train_speed");
	return;
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
			if(train_timer_state[i] == 0){
				train_timer_state[i] = 1;
				T->timer_id = i;
				T->timer = 2;

				thread_data->thread_id = i;
				thread_data->T = T;
				thread_data->B = B;
				thread_data->Flag = 1;
				thread_data->speed = des_s;
				//printf("Create train signal timer, %ius sleep, fire %i times\n",thread_data->time,thread_data->r);
				pthread_create(&train_timer_thread[i], NULL, train_timer, (void *) thread_data);
				break;
			}
			i++;
			if(i==MAX_TIMERS){
				COM_set_train_speed(T,des_s);
			}
		}
	}
}

void train_set_speed(Trains *T, char speed){
	loggerf(ERROR, "TODO: implement train_set_speed");
	return;
	if(!T){ return;printf("Empty T\n");}
	T->cur_speed = speed;
	Z21_GET_LOCO_INFO(T->DCC_ID);
}

void train_set_dir(Trains *T, char dir){
	loggerf(ERROR, "TODO: implement train_set_dir");
	return;
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

void train_set_route(Trains *T, struct Station * S){
	loggerf(ERROR, "TODO: implement train_set_route");
	return;
	if(pathFinding(T->Cur_Block,S->Blocks[0],T->Route,&T->Sw_len)){
		T->halt = 0;
		T->Destination = S->Blocks[0];
	}
}

void train_stop(Trains * T){
	loggerf(ERROR, "TODO: implement train_stop");
	printf("KILL TRAIN:\t#%i\t%s\n",T->DCC_ID,T->name);
}

void train_signal(block * B, Trains * T, int type){
	loggerf("TODO: reimplement train_signal")
	return;
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
			if(train_timer_state[i] == 0){
				train_timer_state[i] = 1;
				T->timer = 1;
				T->timer_id = i;
				thread_data->thread_id = i;
				thread_data->T = T;
				thread_data->B = B;
				thread_data->Flag = 2;
				thread_data->speed = des_s;
				//printf("Create train signal timer, %ius sleep, fire %i times\n",thread_data->time,thread_data->r);
				pthread_create(&train_timer_thread[i], NULL, train_timer, (void *) thread_data);
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
