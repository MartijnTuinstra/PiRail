#define _BSD_SOURCE
// #define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "system.h"
#include "mem.h"

#include "train.h"
#include "switch.h"

#include "logger.h"
#include "config.h"

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
  train_link[0] = (Trains *)1;
}

void free_trains(){
  logger("Clearing trains memory",INFO);

  for(int i = 0;i<trains_len;i++){
    if(trains[i]){
      trains[i]->name = _free(trains[i]->name);
      trains[i]->engines = _free(trains[i]->engines);
      trains[i]->composition = _free(trains[i]->composition);
      trains[i] = _free(trains[i]);
    }
  }

  for(int i = 0;i<engines_len;i++){
    if(engines[i]){
      engines[i]->name = _free(engines[i]->name);
      engines[i]->img_path = _free(engines[i]->img_path);
      engines[i]->icon_path = _free(engines[i]->icon_path);
      engines[i]->funcs = _free(engines[i]->funcs);
      engines[i] = _free(engines[i]);
    }
  }

  for(int i = 0;i<cars_len;i++){
    if(cars[i]){
      cars[i]->name = _free(cars[i]->name);
      cars[i]->img_path = _free(cars[i]->img_path);
      cars[i]->icon_path = _free(cars[i]->icon_path);
      cars[i]->funcs = _free(cars[i]->funcs);
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
  train_link = _free(train_link);

  trains_len = 0;
  engines_len = 0;
  cars_len = 0;
  trains_comp_len = 0;
}

void create_train(char * name, int nr_stock, struct train_comp_ws * comps){
  Trains * Z = _calloc(1, Trains);

  Z->nr_stock = nr_stock;
  Z->composition = _calloc(nr_stock, struct train_comp);

  Z->max_speed = 0xFFFF;
  Z->length = 0;

  for(int i = 0;i<nr_stock;i++){
    Z->composition[i].type = comps[i].type;
    Z->composition[i].id = comps[i].id;

    if(comps[i].type == 0){
      //Engine
      if(comps[i].id >= engines_len || engines[comps[i].id] == 0){
        loggerf(ERROR, "Engine (%i) doesn't exist", comps[i].id);
        continue;
      }

      Z->length += engines[comps[i].id]->length;
      if(Z->max_speed > engines[comps[i].id]->max_speed){
        Z->max_speed = engines[comps[i].id]->max_speed;
      }

      Z->composition[i].p = engines[comps[i].id];
    }
    else{
      //Car
      if(comps[i].id >= cars_len || cars[comps[i].id] == 0){
        loggerf(ERROR, "Car (%i) doesn't exist", comps[i].id);
        continue;
      }
      
      Z->length += cars[comps[i].id]->length;
      if(Z->max_speed > cars[comps[i].id]->max_speed){
        Z->max_speed = cars[comps[i].id]->max_speed;
      }

      Z->composition[i].p = cars[comps[i].id];
    }
  }

  Z->name = _calloc(strlen(name)+2, char);
  strcpy(Z->name,name);

  int index = find_free_index(trains, trains_len);

  trains[index] = Z;

  loggerf(INFO, "Train created at %i",index);
}

void create_train_from_comp(){
  loggerf(CRITICAL, "NOT IMPLEMENTED");
}

void create_engine(char * name,int DCC,char * img, char * icon, int max, char type, int length){
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

  int index = find_free_index(engines, engines_len);

  engines[index] = Z;

  loggerf(INFO, "Engine \"%s\" created at %i\t%s, %s", name, index, img, icon);
}

void create_car(char * name,int nr,char * img, char * icon, char type, int length){
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

  int index = find_free_index(cars, cars_len);

  cars[index] = Z;

  loggerf(INFO, "Car \"%s\" created at %i",name,index);
}

int train_read_confs(){
  FILE * fp = fopen("configs/stock.bin", "rb");

  char * header = _calloc(2, char);

  fread(header, 1, 1, fp);

  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char * buffer = _calloc(fsize, char);
  char * buffer_start = &buffer[0];
  fread(buffer, fsize, 1, fp);

  print_hex(buffer, fsize);

  uint8_t ** buf_ptr = (uint8_t **)&buffer;

  *buf_ptr += 1;

  struct s_train_header_conf h = read_s_train_header_conf(buf_ptr);

  if (header[0] != TRAIN_CONF_VERSION) {
    loggerf(ERROR, "Not correct version");
    return -1;
  }
  
  for(int i = 0; i < h.Engines; i++){
    struct engines_conf e = read_engines_conf(buf_ptr);

    create_engine(e.name, e.DCC_ID, e.img_path, e.icon_path, e.type, e.type, e.length);
  }
  
  for(int i = 0; i < h.Cars; i++){
    struct cars_conf c = read_cars_conf(buf_ptr);

    create_car(c.name, c.nr, c.img_path, c.icon_path, c.type, c.length);
  }
  
  for(int i = 0; i < h.Trains; i++){
    struct trains_conf t = read_trains_conf(buf_ptr);

    create_train(t.name, t.nr_stock, t.composition);
  }

  _free(header);
  _free(buffer_start);
  fclose(fp);

  _SYS_change(STATE_TRAIN_LOADED, 1);
  return 1;
}

int link_train(int fid, int tid, char type){
  //Link = follow ID / train_link id
  //train = tID

  // If it is only a engine -> make it a train
  if(type){
    // Create train from engine
    loggerf(CRITICAL, "CREATE TRAIN FROM ENGINE, NOT IMPLEMENTED");
    return 0;
  }

  //Check if all engines are available
  for(int i = 0; i < trains[tid]->nr_engines; i++){
    if(trains[tid]->engines[i]->use){
      return 2; //Engine not available
    }
  }

  //Link train
  if(train_link[fid] == NULL){
    train_link[fid] = trains[tid];
  }else{
    printf("Train_link[%i] not empty (%x)\n", fid, (unsigned int)train_link[fid]);
    return 3; //Train allready occupied
  }

  //Lock all engines
  for(int i = 0; i < trains[tid]->nr_engines; i++){
    trains[tid]->engines[i]->use = 1;
  }
  return 1;
}

void unlink_train(int fid){
  //Unlock all engines
  for(int i = 0; i < train_link[fid]->nr_engines; i++){
    train_link[fid]->engines[i]->use = 0;
  }
  //Reset link
  train_link[fid] = NULL;
}

void train_speed(Block * B, Trains * T, char speed){
  loggerf(ERROR, "TODO: implement train_speed (%x, %x, %x)", (uint32_t)B, (uint32_t)T, speed);
}

void train_set_speed(Trains *T, char speed){
  loggerf(ERROR, "TODO: implement train_set_speed (%x, %i)", (uint32_t)T, speed);
}

void train_set_dir(Trains *T, char dir){
  loggerf(ERROR, "TODO: implement train_set_dir (%x, %i)", (uint32_t)T, dir);
}

void train_set_route(Trains *T, Station * S){
  loggerf(ERROR, "TODO: implement train_set_route (%x, %x)", (uint32_t)T, (uint32_t)S);
}

void train_stop(Trains * T){
  loggerf(ERROR, "TODO: implement train_stop (%x)", (uint32_t *)T);
}

void train_signal(Block * B, Trains * T, int type){
  loggerf(ERROR, "TODO: implement train_signal (%x, %x, %i)", (uint32_t)B, (uint32_t)T, type);
}

void train_block_timer();