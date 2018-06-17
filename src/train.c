#define _BSD_SOURCE
// #define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "system.h"

#include "train.h"
#include "switch.h"

#include "logger.h"

// #include "./../lib/pathfinding.h"
// #include "./../lib/com.h"
// #include "./../lib/Z21.h"

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
Trains ** train_link;
int train_link_len;

Trains * DCC_train[9999];

void init_trains(){
  loggerf(INFO, "Initializing cars/engines/trains");
  alloc_trains();
  train_read_confs();
}

void alloc_trains(){
  trains = _calloc(10, Trains *);
  trains_len = 10;
  engines = _calloc(10, Engines *);
  engines_len = 10;
  cars = _calloc(10, Cars *);
  cars_len = 10;
  trains_comp = _calloc(10, struct train_composition *);
  trains_comp_len = 10;
  train_link = _calloc(10,Trains *);
  train_link_len = 10;
}

void free_trains(){
  logger("Clearing trains memory",INFO);

  for(int i = 0;i<trains_len;i++){
    if(trains[i]){
      trains[i]->name = _free(trains[i]->name);
      trains[i]->composition = _free(trains[i]->composition);
      trains[i] = _free(trains[i]);
    }
  }

  for(int i = 0;i<engines_len;i++){
    if(engines[i]){
      engines[i]->name = _free(engines[i]->name);
      engines[i]->img_path = _free(engines[i]->img_path);
      engines[i]->icon_path = _free(engines[i]->icon_path);
      engines[i] = _free(engines[i]);
    }
  }

  for(int i = 0;i<cars_len;i++){
    if(cars[i]){
      cars[i]->name = _free(cars[i]->name);
      cars[i]->img_path = _free(cars[i]->img_path);
      cars[i]->icon_path = _free(cars[i]->icon_path);
      cars[i] = _free(cars[i]);
    }
  }

  for(int i = 0;i<trains_comp_len;i++){
    if(trains_comp[i]){
      trains_comp[i]->name = _free(trains_comp[i]->name);
      trains_comp[i]->composition = _free(trains_comp[i]->composition);
      trains_comp[i] = _free(trains_comp[i]);
    }
  }

  trains = _free(trains);
  engines = _free(engines);
  cars = _free(cars);
  trains_comp = _free(trains_comp);

  trains_len = 0;
  engines_len = 0;
  cars_len = 0;
  trains_comp_len = 0;
}

int create_train(char * name, int nr_stock, struct train_comp_ws * comps){
  Trains * Z = _calloc(1, Trains);

  Z->nr_stock = nr_stock;
  Z->composition = _calloc(nr_stock, struct train_comp);

  Z->max_speed = 0xFFFF;
  Z->length = 0;

  for(int i = 0;i<nr_stock;i++){
    loggerf(DEBUG, "create_train: stock %c %i", comps[i].type, comps[i].ID);
    Z->composition[i].type = comps[i].type;
    Z->composition[i].id = comps[i].ID;
    if(comps[i].type == 'E' || comps[i].type == 'e'){
      if(comps[i].ID < engines_len && engines[comps[i].ID]){
        Z->length += engines[comps[i].ID]->length;
        if(Z->max_speed > engines[comps[i].ID]->max_speed){
          Z->max_speed = engines[comps[i].ID]->max_speed;
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
        if(Z->max_speed > cars[comps[i].ID]->max_speed){
          Z->max_speed = cars[comps[i].ID]->max_speed;
        }

        loggerf(DEBUG, "Car (%i) found", comps[i].ID);
        Z->composition[i].p = cars[comps[i].ID];
      }
      else{
        loggerf(ERROR, "Car (%i) doesn't exist", comps[i].ID);
      }
    }
  }

  Z->name = _calloc(strlen(name)+2, char);
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
  Engines * Z = _calloc(1, Engines);

  Z->name = _calloc(strlen(name)+2, char);
  Z->img_path = _calloc(strlen(img)+2, char);
  Z->icon_path = _calloc(strlen(icon)+2, char);

  strcpy(Z->name,name);
  strcpy(Z->img_path,img);
  strcpy(Z->icon_path,icon);

  Z->DCC_ID = DCC;

  Z->length = length;
  Z->max_speed = max;
  Z->cur_speed_step = 0;

  Z->type = type;
  _Bool dir = FALSE;

  int index = find_free_index((void **)engines,&engines_len);

  engines[index] = Z;

  loggerf(INFO, "Engine \"%s\" created at %i\t%s, %s", name, index, img, icon);
}

int create_car(char * name,int nr,char * img, char * icon, char type, int length){
  Cars * Z = _calloc(1, Cars);

  Z->name = _calloc(strlen(name)+2, char);
  Z->img_path = _calloc(strlen(img)+2, char);
  Z->icon_path = _calloc(strlen(icon)+2, char);

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
  char * header = _calloc(2, char);

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
      char *buffer = _calloc(fsize - 1, char);
      fread(buffer, fsize, 1, f);

      long index = 0;

      for(int i = 0;i < engines_nr;i++){
        struct engine_conf * engine = (void *)&buffer[index];
        
        if(engine->check != 0){
          loggerf(ERROR,"Enignes config file wrong format in engine number %i",i+1);
          break;
        }

        char * name = _calloc(engine->name_len+2, char);
        char * img = _calloc(engine->img_path_len+2, char);
        char * icon = _calloc(engine->icon_path_len+2, char);

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

        _free(name);
        _free(img);
        _free(icon);
        index += 1;
      }

      _free(buffer);

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
      char *buffer = _calloc(fsize - 1, char);
      fread(buffer, fsize, 1, f);

      long index = 0;

      for(int i = 0;i < cars_nr;i++){
        struct car_conf * car = (void *)&buffer[index];
        
        if(car->check != 0){
          loggerf(ERROR,"Cars config file wrong format in car number %i",i+1);
          break;
        }

        char * name = _calloc(car->name_len+2, char);
        char * img = _calloc(car->img_path_len+2, char);
        char * icon = _calloc(car->icon_path_len+2, char);

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

        _free(name);
        _free(img);
        _free(icon);
        index += 1;
      }

      _free(buffer);

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
      char *buffer = _calloc(fsize - 1, char);
      fread(buffer, fsize, 1, f);

      long index = 0;

      for(int i = 0;i < trains_nr;i++){
        struct train_comp_conf * train = (void *)&buffer[index];
        
        if(train->check != 0){
          loggerf(ERROR,"Trains config file wrong format in train number %i",i+1);
          break;
        }

        char * name = _calloc(train->name_len+2, char);
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

        _free(name);

        index += 1;
      }

      _free(buffer);

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

int link_train(int fid, int tid, char type){
  //Link = follow ID
  //train = tID

  // If it is only a engine -> make it a train
  if(type > 0){
    // Create train from engine
    loggerf(CRITICAL, "CREATE TRAIN FROM ENGINE, NOT IMPLEMENTED");
    return 0;
  }

  //Check if all engines are available
  for(int i = 0; i < trains[tid]->nr_engines; i++){
    if(trains[tid]->engines[i]->use){
      return 0;
    }
  }

  //Link train
  if(train_link[fid] == NULL){
    printf("link is empty %i\n",train_link[fid]);
    train_link[fid] = trains[tid];

    printf("Set to %i\n",train_link[fid]);
  }else{
    return 0;
  }

  //Lock all engines
  for(int i = 0; i < trains[tid]->nr_engines; i++){
    trains[tid]->engines[i]->use = 1;
  }
  return 1;
}

int unlink_train(int fid){
  //Unlock all engines
  for(int i = 0; i < train_link[fid]->nr_engines; i++){
    train_link[fid]->engines[i]->use = 0;
  }
  //Reset link
  train_link[fid] = NULL;
}

void train_speed(Block * B, Trains * T, char speed){
  loggerf(ERROR, "TODO: implement train_speed");
}

void train_set_speed(Trains *T, char speed){
  loggerf(ERROR, "TODO: implement train_set_speed");
}

void train_set_dir(Trains *T, char dir){
  loggerf(ERROR, "TODO: implement train_set_dir");
}

void train_set_route(Trains *T, Station * S){
  loggerf(ERROR, "TODO: implement train_set_route");
}

void train_stop(Trains * T){
  loggerf(ERROR, "TODO: implement train_stop");
}

void train_signal(Block * B, Trains * T, int type){
  loggerf(ERROR, "TODO: implement train_signal");
}

void train_block_timer();