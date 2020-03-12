#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

#include "system.h"
#include "mem.h"

#include "train.h"
#include "switch.h"

#include "logger.h"
#include "config.h"

#include "websocket.h"

// #include "pathfinding.h"

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
RailTrain ** train_link;
int train_link_len;

struct cat_conf * train_P_cat;
int train_P_cat_len = 0;
struct cat_conf * train_C_cat;
int train_C_cat_len = 0;

Engines * DCC_train[9999];


void Create_Train(char * name, int nr_stock, struct train_comp_ws * comps, uint8_t catagory, uint8_t save){
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

  loggerf(DEBUG, "Train created at %i",index);
}

void Clear_Train(Trains ** T){
  _free((*T)->name);
  _free((*T)->engines);
  _free((*T)->composition);
  _free((*T));
  *T = 0;
}

void Create_Engine(char * name,int DCC,char * img, char * icon, char type, int length, int steps_len, struct engine_speed_steps * steps){
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
  }

  int index = find_free_index(engines, engines_len);

  engines[index] = Z;
  Z->id = index;

  loggerf(DEBUG, "Engine \"%s\" created", name);
}

void Clear_Engine(Engines ** E){
  _free((*E)->name);
  _free((*E)->img_path);
  _free((*E)->icon_path);
  _free((*E)->steps);
  _free((*E)->funcs);
  _free((*E));
  *E = 0;
}

void Create_Car(char * name,int nr, char * icon, char type, uint16_t length, uint16_t speed){
  Cars * Z = _calloc(1, Cars);

  Z->name = name;
  Z->icon_path = icon;

  Z->nr = nr;
  Z->length = length;
  Z->max_speed = speed;

  Z->type = type;

  int index = find_free_index(cars, cars_len);

  cars[index] = Z;

  loggerf(DEBUG, "Car \"%s\" created",name);
}

void Clear_Car(Cars ** C){
  _free((*C)->name);
  _free((*C)->icon_path);
  _free((*C)->funcs);
  *C = _free((*C));
}

RailTrain * new_railTrain(){
  RailTrain * T = _calloc(1, RailTrain);
  uint16_t id = find_free_index(train_link, train_link_len);
  T->link_id = id;
  train_link[id] = T;
  return T;
}

int load_rolling_Configs(){
  loggerf(INFO, "Initializing cars/engines/trains");

  // Allocation Basic Space
  trains = _calloc(10, Trains *);
  trains_len = 10;
  engines = _calloc(10, Engines *);
  engines_len = 10;
  cars = _calloc(10, Cars *);
  cars_len = 10;
  trains_comp = _calloc(10, struct train_composition *);
  trains_comp_len = 10;
  train_link = _calloc(10,RailTrain *);
  train_link_len = 10;

  read_rolling_Configs();

  SYS->trains_loaded = 1;

  return 1;
}

int read_rolling_Configs(){
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
  return 1;
}

void write_rolling_Configs(){
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
    subsize += strlen(cars[i]->icon_path) + 2;

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
    c.icon_path_len = strlen(cars[i]->icon_path);

    memcpy(p, &c, sizeof(struct s_car_conf));
    p += sizeof(struct s_car_conf) + 1;

    memcpy(p, cars[i]->name, c.name_len);
    p += c.name_len + 1;

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

void unload_rolling_Configs(){
  logger("Clearing trains memory",INFO);

  for(int i = 0;i<trains_len;i++){
    if(!trains[i])
      continue;

    Clear_Train(&trains[i]);
  }

  for(int i = 0;i<engines_len;i++){
    if(!engines[i])
      continue;

    Clear_Engine(&engines[i]);
  }

  for(int i = 0;i<cars_len;i++){
    if(!cars[i])
      continue;

    Clear_Car(&cars[i]);
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

  for(int i = 0; i < train_link_len; i++){
    if(!train_link[i])
      continue;

    _free(train_link[i]);
    train_link[i] = 0;
  }

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

  SYS->trains_loaded = 0;
}


int link_train(int fid, int tid, char type){
  //Link = follow ID / train_link id
  //train = tID
  RailTrain * RT = train_link[fid];

  if(!RT){
    loggerf(ERROR, "No RailTrain found");
    return 1;
  }

  // If it is only a engine -> make it a train
  if(type){
    if(engines[tid]->use){
      loggerf(ERROR, "Engine allready used");
      _free(RT);
      return 3;
    }

    // Create train from engine
    RT->type = TRAIN_ENGINE_TYPE;
    RT->p = engines[tid];
    RT->max_speed = engines[tid]->max_speed;

    //Lock engines
    engines[tid]->use = 1;
  }
  else{
    for(int  i = 0; i < trains[tid]->nr_engines; i++){
      if(trains[tid]->engines[i]->use){
        loggerf(ERROR, "Engine of Train allready used");
        _free(RT);
        return 3;
      }
    }

    // Crate Rail Train
    RT->type = TRAIN_TRAIN_TYPE;
    RT->p = trains[tid];
    RT->max_speed = trains[tid]->max_speed;

    //Lock all engines    
    for(int i = 0; i < trains[tid]->nr_engines; i++){
      trains[tid]->engines[i]->use = 1;
    }
  }

  return 1;
}

void unlink_train(int id){
  //TODO implement RailTrain type
  //Unlock all engines
  //for(int i = 0; i < train_link[fid]->nr_engines; i++){
  //  train_link[fid]->engines[i]->use = 0;
  //}
  //Reset link
  _free(train_link[id]);
  train_link[id] = NULL;
}

// Speed

// void Train_Set_Speed(RailTrain * T){

// }

void engine_set_speed(Engines * E, uint16_t speed){
  if(!E){
    loggerf(ERROR, "No Engine");
    return;
  }
  // Convert from speed to steps
  struct engine_speed_steps left;
  struct engine_speed_steps right;

  E->cur_speed = speed;

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

void engine_read_speed(Engines * E){
  if(!E){
    loggerf(ERROR, "No Engine");
    return;
  }
  // Convert from steps to speed
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
  if(!T){
    loggerf(ERROR, "No Train");
    return;
  }
  T->cur_speed = speed;

  for(int i = 0; i < T->nr_engines; i++){
    engine_set_speed(T->engines[i], speed);
  }
}

void train_calc_speed(Trains * T){
  if(!T){
    loggerf(ERROR, "No Train");
    return;
  }
  for(int i = 0; i < T->nr_engines; i++){
    engine_set_speed(T->engines[i], T->cur_speed);
  }
}

void train_change_speed(RailTrain * T, uint16_t target_speed, uint8_t type){
  if(!T || !T->p){
    loggerf(ERROR, "No Train");
    return;
  }
  loggerf(INFO, "train_change_speed %i -> %i", T->link_id, target_speed);
  //T->target_speed = target_speed;

  if(type == IMMEDIATE_SPEED){
    // if(T->type == TRAIN_TRAIN_TYPE){
      T->changing_speed = RAILTRAIN_SPEED_T_DONE;
      T->speed = target_speed;
      WS_stc_UpdateTrain(T);
      // train_set_speed(T->p, target_speed);
    // }
    // else{
      // T->changing_speed = RAILTRAIN_SPEED_T_DONE;
      // T->speed = target_speed;
      // engine_set_speed(T->p, target_speed);
    // }
  }
  else if(type == GRADUAL_SLOW_SPEED){
    train_speed_timer_create(T, target_speed, T->B->length*2);
  }
  else if(type == GRADUAL_FAST_SPEED){
    train_speed_timer_create(T, target_speed, T->B->length);
  }
}



void train_speed_timer_create(RailTrain * T, uint16_t target, uint16_t length){
  if(!T){
    loggerf(ERROR, "No Train");
    return;
  }

  const char * speed_String[9] = {
    "INIT",
    "CHANGING",
    "UPDATE",
    "DONE",
    "FAIL"
  };

  loggerf(INFO, "train_speed_timer_create %s", speed_String[T->changing_speed]);

  if (T->changing_speed == RAILTRAIN_SPEED_T_DONE ||
      T->changing_speed == RAILTRAIN_SPEED_T_FAIL){
    pthread_join(T->speed_thread, NULL);
  }

  if (T->speed == target){
    return;
  }

  if (T->changing_speed == RAILTRAIN_SPEED_T_INIT ||
      T->changing_speed == RAILTRAIN_SPEED_T_DONE){

    T->target_speed = target;
    T->target_distance = length;
    T->changing_speed = RAILTRAIN_SPEED_T_CHANGING;
    if(pthread_create(&T->speed_thread, NULL, train_speed_timer_run, (void *)T) != 0)
      loggerf(ERROR, "Error while creating train_speed_timer");
  }
  else if(T->changing_speed == RAILTRAIN_SPEED_T_CHANGING && T->target_speed != target){
    T->target_speed = target;
    T->target_distance = length;
    T->changing_speed = RAILTRAIN_SPEED_T_UPDATE;
  }
}

void train_speed_timer_calc(float * accel, float * time, RailTrain * T){
  // v = v0 + at;
  // x = v0*t + 0.5at^2;
  // a = 0.5/x * (v^2 - v0^2);
  // t = sqrt((x - v0) / (0.5a))
  float start_speed = T->speed * 1.0;

  loggerf(INFO, "train_speed_timer_calc %i %i %f", T->target_distance, T->target_speed, start_speed);

  float real_distance = 160.0 * 0.00001 * (T->target_distance - 5); // km
  *accel = (1 / (2 * real_distance));
  *accel *= (T->target_speed - start_speed) * (T->target_speed + start_speed);
  // accel == km/h/h

  loggerf(DEBUG, "Train_speed_timer_run (accel at %f km/h^2)", *accel);
  if (*accel > 64800.0){ // 5 m/s^2
    loggerf(INFO, "Accel to large, reduced to 5.0m/s^2)");
    *accel = 64800.0;
  }
  else if (*accel < -129600.0){
    loggerf(INFO, "Deccell to large, reduced to 10.0m/s^2)");
    *accel = -129600.0;
  }

  if (*accel == 0 || *accel == 0.0){
    loggerf(INFO, "No speed difference");
    T->changing_speed = RAILTRAIN_SPEED_T_DONE;
    return;
  }

  loggerf(INFO, "train_speed  sqrt((2 * (%f - %f)) / (%f))", real_distance, start_speed, *accel);

  *time = sqrt(2 * (*accel) * real_distance + T->speed * T->speed) - T->speed;
  *time /= *accel;
  *time *= 3600; // convert to seconds

  if(*time < 0.0){
    *time *= -1.0;
  }

  loggerf(INFO, "train_speed time %f", *time);
}

void * train_speed_timer_run(void * args){
  RailTrain * T = ((RailTrain *)args);
  loggerf(INFO, "Train_speed_timer_run (%i -> %ikm/h, %icm)\n", T->speed, T->target_speed, T->target_distance);

  float acceleration = 0;
  float time = 0;

  train_speed_timer_calc(&acceleration, &time, T);

  if(T->changing_speed == RAILTRAIN_SPEED_T_DONE){
    loggerf(INFO, "return");
    return NULL;
  }

  uint16_t steps = (uint16_t)(time / 0.5);
  uint32_t steptime = ((time / steps) * 1000000); // convert to usec

  uint16_t start_speed = T->speed;

  loggerf(INFO, "train_speed_timer_run start %i a:%f, s:%i, t:%i", T->speed, acceleration, steps, steptime);

  for(uint16_t i = 0; i <= steps; i++){
    usleep(steptime);
    if(T->changing_speed == RAILTRAIN_SPEED_T_DONE){
      loggerf(INFO, "return");
      return NULL;
    }

    T->speed = (start_speed + acceleration * ((steptime * i) / 1000000.0 / 3600.0)) + 0.01; // hours

    loggerf(INFO, "train_speed_timer_run %i %f", T->speed, acceleration);

    WS_stc_UpdateTrain(T);

    if(T->changing_speed == RAILTRAIN_SPEED_T_DONE){
      loggerf(INFO, "return");
      return NULL;
    }
    else if(T->changing_speed == RAILTRAIN_SPEED_T_UPDATE){
      loggerf(INFO, "train_speed_timer_run UPDATE");
      train_speed_timer_calc(&acceleration, &time, T);
      if(T->changing_speed == RAILTRAIN_SPEED_T_DONE)
        return NULL;
      steps = (uint16_t)(time / 0.5);
      steptime = ((time / steps) * 1000000); // convert to usec
      T->changing_speed = RAILTRAIN_SPEED_T_CHANGING;

      start_speed = T->speed;
      i = 0;
    }
  }
  T->changing_speed = RAILTRAIN_SPEED_T_DONE;
  return NULL;
}

void train_set_route(RailTrain * T, Block * dest){
  // struct pathfindingstep path = pathfinding(T->B, dest);

  // if(path.found){
  //   T->route = 1;
  //   T->instructions = path.instructions;
  // }
}