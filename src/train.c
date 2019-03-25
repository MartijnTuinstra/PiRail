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

struct cat_conf * train_P_cat;
int train_P_cat_len = 0;
struct cat_conf * train_C_cat;
int train_C_cat_len = 0;

Engines * DCC_train[9999];

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
    if(!trains[i])
      continue;

    clear_train(&trains[i]);
  }

  for(int i = 0;i<engines_len;i++){
    if(!engines[i])
      continue;

    clear_engine(&engines[i]);
  }

  for(int i = 0;i<cars_len;i++){
    if(!cars[i])
      continue;

    clear_car(&cars[i]);
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

  for(int i = 0; i < train_P_cat_len; i++){
    _free(train_P_cat[i].name);
  }
  for(int i = 0; i < train_C_cat_len; i++){
    _free(train_C_cat[i].name);
  }

  _free(train_P_cat);
  _free(train_C_cat);

  trains_len = 0;
  engines_len = 0;
  cars_len = 0;
  trains_comp_len = 0;
}

void create_train(char * name, int nr_stock, struct train_comp_ws * comps, uint8_t catagory, uint8_t save){
  Trains * Z = _calloc(1, Trains);

  Z->nr_stock = nr_stock;
  Z->composition = _calloc(nr_stock, struct train_comp);

  Z->max_speed = 0xFFFF;
  Z->length = 0;
  Z->type = catagory;
  Z->save = save;

  Z->engines = _calloc(1, Engines *);
  Z->nr_engines = 0;

  for(int i = 0;i<nr_stock;i++){
    Z->composition[i].type = comps[i].type;
    Z->composition[i].id = comps[i].id;

    if(comps[i].type == 0){
      loggerf(DEBUG, "Add engine %i", comps[i].id);
      Engines * E = engines[comps[i].id];
      //Engine
      if(comps[i].id >= engines_len || E == 0){
        loggerf(ERROR, "Engine (%i) doesn't exist", comps[i].id);
        continue;
      }

      Z->length += E->length;
      if(Z->max_speed > E->max_speed && E->max_speed != 0){
        Z->max_speed = E->max_speed;
      }

      Z->composition[i].p = E;

      int index = find_free_index(Z->engines, Z->nr_engines);

      Z->engines[index] = E;

      loggerf(TRACE, "Train engine index: %d", index);
    }
    else{
      loggerf(DEBUG, "Add car %i", comps[i].id);
      //Car
      if(comps[i].id >= cars_len || cars[comps[i].id] == 0){
        loggerf(ERROR, "Car (%i) doesn't exist", comps[i].id);
        continue;
      }
      
      Z->length += cars[comps[i].id]->length;
      if(Z->max_speed > cars[comps[i].id]->max_speed && cars[comps[i].id]->max_speed != 0){
        Z->max_speed = cars[comps[i].id]->max_speed;
      }

      Z->composition[i].p = cars[comps[i].id];
    }
  }

  Z->name = name;
  _free(comps);

  int index = find_free_index(trains, trains_len);

  trains[index] = Z;
  Z->id = index;

  loggerf(INFO, "Train created at %i",index);
}

void clear_train(Trains ** T){
  _free((*T)->name);
  _free((*T)->engines);
  _free((*T)->composition);
  _free((*T));
  *T = 0;
}

void create_train_from_comp(){
  loggerf(CRITICAL, "NOT IMPLEMENTED");
}

void create_engine(char * name,int DCC,char * img, char * icon, char type, int length, int steps_len, struct engine_speed_steps * steps){
  //DCC cant be used twice
  for(int i = 0;i<engines_len;i++){
    if(engines[i] && engines[i]->DCC_ID == DCC){
      loggerf(WARNING,"create_engine: found duplicate: %s",engines[i]->name);
    }
  }
  Engines * Z = _calloc(1, Engines);

  Z->name = name;
  Z->img_path = img;
  Z->icon_path = icon;

  Z->DCC_ID = DCC;
  DCC_train[DCC] = Z;

  Z->length = length;
  //TODO add to arguments
  Z->speed_step_type = TRAIN_128_FAHR_STUFEN;

  Z->type = type;

  Z->steps_len = steps_len;
  Z->steps = steps;

  for(int i = 0; i < steps_len; i++){
    if(Z->max_speed < steps[i].speed){
      Z->max_speed = steps[i].speed;
    }
    printf("  %d, %d", steps[i].step, steps[i].speed);
  }
  printf("\n");

  int index = find_free_index(engines, engines_len);

  engines[index] = Z;
  Z->id = index;

  loggerf(DEBUG, "Engine \"%s\" created", name);
}

void clear_engine(Engines ** E){
  _free((*E)->name);
  _free((*E)->img_path);
  _free((*E)->icon_path);
  _free((*E)->steps);
  _free((*E)->funcs);
  _free((*E));
  *E = 0;
}

void create_car(char * name,int nr,char * img, char * icon, char type, uint16_t length, uint16_t speed){
  Cars * Z = _calloc(1, Cars);

  Z->name = name;
  Z->img_path = img;
  Z->icon_path = icon;

  Z->nr = nr;
  Z->length = length;
  Z->max_speed = speed;

  Z->type = type;

  int index = find_free_index(cars, cars_len);

  cars[index] = Z;

  loggerf(INFO, "Car \"%s\" created",name);
}

void clear_car(Cars ** C){
  _free((*C)->name);
  _free((*C)->img_path);
  _free((*C)->icon_path);
  _free((*C)->funcs);
  *C = _free((*C));
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

  //print_hex(buffer, fsize);

  uint8_t ** buf_ptr = (uint8_t **)&buffer;

  *buf_ptr += 1;

  struct s_train_header_conf h = read_s_train_header_conf(buf_ptr);

  if (header[0] != TRAIN_CONF_VERSION) {
    loggerf(ERROR, "Not correct version");
    return -1;
  }


  train_P_cat = _calloc(h.P_Catagories, struct cat_conf);
  train_P_cat_len = h.P_Catagories;
  train_C_cat = _calloc(h.C_Catagories, struct cat_conf);
  train_C_cat_len = h.C_Catagories;

  for(int i = 0; i < h.P_Catagories; i++){
    train_P_cat[i] = read_cat_conf(buf_ptr);
  }
  
  for(int i = 0; i < h.C_Catagories; i++){
    train_C_cat[i] = read_cat_conf(buf_ptr);
  }
  
  for(int i = 0; i < h.Engines; i++){
    struct engines_conf e = read_engines_conf(buf_ptr);

    create_engine_from_conf(e);
  }
  
  for(int i = 0; i < h.Cars; i++){
    struct cars_conf c = read_cars_conf(buf_ptr);

    create_car_from_conf(c);
  }
  
  for(int i = 0; i < h.Trains; i++){
    struct trains_conf t = read_trains_conf(buf_ptr);

    create_train_from_conf(t);
  }

  _free(header);
  _free(buffer_start);
  fclose(fp);

  _SYS_change(STATE_TRAIN_LOADED, 1);
  return 1;
}

void train_write_confs(){
  //Calculate size
  int size = 1; //header
  size += sizeof(struct s_train_header_conf) + 1;
  int subsize = 0;

  int tr = 0;
  int en = 0;
  int ca = 0;
  //Catagories
  for(int i = 0; i < train_P_cat_len; i++){
    size += sizeof(struct s_cat_conf) + 1;
    size += train_P_cat[i].name_len + 1;
  }
  for(int i = 0; i < train_C_cat_len; i++){
    size += sizeof(struct s_cat_conf) + 1;
    size += train_C_cat[i].name_len + 1;
  }

  //Engines
  for(int i = 0; i < engines_len; i++){
    if(!engines[i])
      continue;

    subsize = sizeof(struct s_engine_conf) + 1;
    subsize += strlen(engines[i]->name) + strlen(engines[i]->img_path) + 2;
    subsize += strlen(engines[i]->icon_path) + 1;
    subsize += engines[i]->steps_len * sizeof(struct engine_speed_steps) + 1;

    en++;

    size += subsize;
  }

  //Cars
  for(int i = 0; i < cars_len; i++){
    if(!cars[i])
      continue;

    subsize = sizeof(struct s_car_conf) + strlen(cars[i]->name) + 2;
    subsize += strlen(cars[i]->img_path) + strlen(cars[i]->icon_path) + 2;

    ca++;

    size += subsize;
  }

  //Trains
  for(int i = 0; i < trains_len; i++){
    if(!trains[i] || !trains[i]->save)
      continue;

    subsize = sizeof(struct s_train_conf) + 1;
    subsize += strlen(trains[i]->name) + 1;
    subsize += sizeof(struct train_comp_ws) * trains[i]->nr_stock + 1;

    tr++;

    size += subsize;
  }


  //Write contents

  loggerf(INFO, "Writing %i bytes", size);

  char * data = _calloc(size, 1);

  data[0] = TRAIN_CONF_VERSION;

  char * p = &data[1];

  //Copy header
  struct s_train_header_conf header;
  header.Trains = tr;
  header.Engines = en;
  header.Cars = ca;
  header.P_Catagories = train_P_cat_len;
  header.C_Catagories = train_C_cat_len;

  memcpy(p, &header, sizeof(struct s_train_header_conf));

  p += sizeof(struct s_train_header_conf) + 1;

  //Copy Catagories
  for(int i = 0; i < train_P_cat_len; i++){
    memcpy(p, &train_P_cat[i], sizeof(struct s_cat_conf));
    p += sizeof(struct s_cat_conf) + 1;

    memcpy(p, train_P_cat[i].name, train_P_cat[i].name_len);
    p += train_P_cat[i].name_len + 1;
  }
  for(int i = 0; i < train_C_cat_len; i++){
    memcpy(p, &train_C_cat[i], sizeof(struct s_cat_conf));
    p += sizeof(struct s_cat_conf) + 1;

    memcpy(p, train_C_cat[i].name, train_C_cat[i].name_len);
    p += train_C_cat[i].name_len + 1;
  }

  //Copy Engine
  for(int i = 0; i < engines_len; i++){
    if(!engines[i])
      continue;

    struct s_engine_conf e;
    e.DCC_ID = engines[i]->DCC_ID;
    e.length = engines[i]->length;
    e.type = engines[i]->type;
    e.config_steps = engines[i]->steps_len;
    e.name_len = strlen(engines[i]->name);
    e.img_path_len = strlen(engines[i]->img_path);
    e.icon_path_len = strlen(engines[i]->icon_path);

    memcpy(p, &e, sizeof(struct s_engine_conf));
    p += sizeof(struct s_engine_conf) + 1;

    memcpy(p, engines[i]->name, e.name_len);
    p += e.name_len + 1;

    memcpy(p, engines[i]->img_path, e.img_path_len);
    p += e.img_path_len + 1;

    memcpy(p, engines[i]->icon_path, e.icon_path_len);
    p += e.icon_path_len + 1;

    memcpy(p, engines[i]->steps, e.config_steps * sizeof(struct engine_speed_steps));
    p += e.config_steps * sizeof(struct engine_speed_steps) + 1;
  }

  // Copy Cars
  for(int i = 0; i < cars_len; i++){
    if(!cars[i])
      continue;

    struct s_car_conf c;
    c.nr = cars[i]->nr;
    c.length = cars[i]->length;
    c.type = cars[i]->type;
    c.name_len = strlen(cars[i]->name);
    c.img_path_len = strlen(cars[i]->img_path);
    c.icon_path_len = strlen(cars[i]->icon_path);

    memcpy(p, &c, sizeof(struct s_car_conf));
    p += sizeof(struct s_car_conf) + 1;

    memcpy(p, cars[i]->name, c.name_len);
    p += c.name_len + 1;

    memcpy(p, cars[i]->img_path, c.img_path_len);
    p += c.img_path_len + 1;

    memcpy(p, cars[i]->icon_path, c.icon_path_len);
    p += c.icon_path_len + 1;
  }

  //Copy trains
  for(int i =0; i < trains_len; i++){
    if(!trains[i] || !trains[i]->save)
      continue;

    struct s_train_conf t;
    t.name_len = strlen(trains[i]->name);
    t.nr_stock = trains[i]->nr_stock;

    memcpy(p, &t, sizeof(struct s_train_conf));
    p += sizeof(struct s_train_conf) + 1;

    memcpy(p, trains[i]->name, t.name_len);
    p += t.name_len + 1;

    for(int j=0; j<t.nr_stock; j++){
      memcpy(p, &trains[i]->composition[j], sizeof(struct train_comp_ws));
      p += sizeof(struct train_comp_ws);
    }
    p++;
  }

  //Print output
  // print_hex(data, size);

  FILE * fp = fopen(TRAIN_CONF_PATH, "wb");

  fwrite(data, size, 1, fp);

  fclose(fp);

  free(data);
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

// Speed

void engine_calc_speed(Engines * E){
  struct engine_speed_steps left;
  struct engine_speed_steps right;

  left.speed = 0; left.step = 0;
  right.speed = 0; right.step = 0;

  for(int i = 0; i < E->steps_len; i++){
    if(E->steps[i].speed < E->cur_speed){
      left.speed = E->steps[i].speed;
      left.step = E->steps[i].step;
    }

    if(E->steps[i].speed >= E->cur_speed && right.step == 0){
      right.speed = E->steps[i].speed;
      right.step = E->steps[i].step;
    }
  }

  float ratio = ((float)(E->cur_speed - left.speed)) / ((float)(right.speed - left.speed));
  uint8_t step = ratio * ((float)(right.step - left.step)) + left.step;

  E->speed = step;
}

void engine_calc_real_speed(Engines * E){
  struct engine_speed_steps * left;
  struct engine_speed_steps * right;

  struct engine_speed_steps temp;
  temp.step = 0;
  temp.speed = 0;

  left = &temp;
  right = 0;

  for(int i = 0; i < E->steps_len; i++){
    if(E->steps[i].step < E->speed){
      left = &E->steps[i];
    }

    if(E->steps[i].step >= E->speed && right == 0){
      right = &E->steps[i];
    }
  }

  float ratio = ((float)(E->speed - left->step)) / ((float)(right->step - left->step));
  uint8_t speed = ratio * ((float)(right->speed - left->speed)) + left->speed;

  E->cur_speed = speed;
}

void train_set_speed(Trains * T, uint16_t speed){
  T->cur_speed = speed;

  for(int i = 0; i < T->nr_engines; i++){
    T->engines[i]->cur_speed = speed;
    engine_calc_speed(T->engines[i]);
  }
}

void train_calc_speed(Trains * T){
  for(int i = 0; i < T->nr_engines; i++){
    T->engines[i]->cur_speed = T->cur_speed;
    engine_calc_speed(T->engines[i]);
  }
}