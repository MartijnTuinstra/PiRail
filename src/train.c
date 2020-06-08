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

#include "Z21_msg.h"
#include "scheduler.h"

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
  Trains * Z = (Trains *)_calloc(1, Trains);

  Z->nr_stock = nr_stock;
  Z->composition = (struct train_comp *)_calloc(nr_stock, struct train_comp);

  Z->max_speed = 0xFFFF;
  Z->length = 0;
  Z->type = catagory;
  Z->save = save;

  Z->engines = (Engines **)_calloc(1, Engines *);
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

void Create_Engine(char * name,int DCC,char * img, char * icon, char type, int length, int steps_len, struct engine_speed_steps * steps, uint8_t functions[28]){
  //DCC cant be used twice
  for(int i = 0;i<engines_len;i++){
    if(engines[i] && engines[i]->DCC_ID == DCC){
      loggerf(WARNING,"create_engine: found duplicate: %s",engines[i]->name);
    }
  }
  Engines * Z = (Engines *)_calloc(1, Engines);

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


  // Copy each speed step
  for(uint8_t i = 0; i < steps_len; i++){
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
  Cars * Z = (Cars *)_calloc(1, Cars);

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
  _free((*C));
  *C = 0;
}

RailTrain * new_railTrain(){
  RailTrain * T = (RailTrain *)_calloc(1, RailTrain);
  uint16_t id = find_free_index(train_link, train_link_len);
  T->link_id = id;

  char name[64];
  sprintf(name, "Railtrain_%i_SpeedEvent", id);
  T->speed_event = scheduler->addEvent(name, {0, 0});
  T->speed_event_data = (struct TrainSpeedEventData *)_calloc(1, struct TrainSpeedEventData);
  T->speed_event->function = (void (*)(void *))train_speed_event_tick;
  T->speed_event->function_args = (void *)T->speed_event_data;

  train_link[id] = T;
  return T;
}

int load_rolling_Configs(){
  loggerf(INFO, "Initializing cars/engines/trains");

  // Allocation Basic Space
  trains = (Trains **)_calloc(10, Trains *);
  trains_len = 10;
  engines = (Engines **)_calloc(10, Engines *);
  engines_len = 10;
  cars = (Cars **)_calloc(10, Cars *);
  cars_len = 10;
  trains_comp = (struct train_composition **)_calloc(10, struct train_composition *);
  trains_comp_len = 10;
  train_link = (RailTrain **)_calloc(10,RailTrain *);
  train_link_len = 10;

  read_rolling_Configs();

  SYS->trains_loaded = 1;

  return 1;
}

int read_rolling_Configs(){
  FILE * fp = fopen("configs/stock.bin", "rb");

  if(!fp){
    loggerf(ERROR, "Failed to open config file");
    return -1;
  }

  char * header = (char *)_calloc(2, char);

  fread(header, 1, 1, fp);

  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char * buffer = (char *)_calloc(fsize, char);
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


  train_P_cat = (struct cat_conf *)_calloc(h.P_Catagories, struct cat_conf);
  train_P_cat_len = h.P_Catagories;
  train_C_cat = (struct cat_conf *)_calloc(h.C_Catagories, struct cat_conf);
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

  char * data = (char *)_calloc(size, 1);

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
      trains_comp[i]->name = (char *)_free(trains_comp[i]->name);
      trains_comp[i]->composition = (struct train_comp *)_free(trains_comp[i]->composition);
      trains_comp[i] = (struct train_composition *)_free(trains_comp[i]);
    }
  }

  trains = (Trains **)_free(trains);
  engines = (Engines **)_free(engines);
  cars = (Cars **)_free(cars);
  trains_comp = (train_composition **)_free(trains_comp);

  for(int i = 0; i < train_link_len; i++){
    if(!train_link[i])
      continue;

    scheduler->removeEvent(train_link[i]->speed_event);

    _free(train_link[i]);
    train_link[i] = 0;
  }

  train_link = (RailTrain **)_free(train_link);

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
    engines[tid]->RT = RT;
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
      trains[tid]->engines[i]->RT = RT;
    }
  }

  return 1;
}

void unlink_train(int id){
  //TODO implement RailTrain type
  //Unlock all engines
  if(train_link[id]->type == TRAIN_ENGINE_TYPE){
    Engines * E = (Engines *)train_link[id]->p;
    E->use = 0;
    E->RT = 0;
  }
  else{
    Trains * T = (Trains *)train_link[id]->p;
    for(int i = 0; i < T->nr_engines; i++){
      T->engines[i]->use = 0;
      T->engines[i]->RT = 0;
    }
  }
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
    train_speed_event_create(T, target_speed, T->B->length*2);
  }
  else if(type == GRADUAL_FAST_SPEED){
    train_speed_event_create(T, target_speed, T->B->length);
  }
}

void train_speed_event_create(RailTrain * T, uint16_t targetSpeed, uint16_t distance){
  loggerf(INFO, "train_speed_event_create %i", T->changing_speed);

  if (T->speed == targetSpeed){
    return;
  }

  T->target_speed = targetSpeed;
  T->target_distance = distance;

  if (T->changing_speed == RAILTRAIN_SPEED_T_INIT ||
      T->changing_speed == RAILTRAIN_SPEED_T_DONE){

    T->changing_speed = RAILTRAIN_SPEED_T_CHANGING;
  }
  else if(T->changing_speed == RAILTRAIN_SPEED_T_CHANGING){
    T->changing_speed = RAILTRAIN_SPEED_T_UPDATE;
  }
  else{
    return;
  }

  train_speed_event_init(T);
}

void train_speed_event_calc(struct TrainSpeedEventData * data){
  // v = v0 + at;
  // x = v0*t + 0.5at^2;
  // a = 0.5/x * (v^2 - v0^2);
  // t = sqrt((x - v0) / (0.5a))
  float start_speed = data->T->speed * 1.0;

  loggerf(INFO, "train_speed_timer_calc %i %i %f", data->T->target_distance, data->T->target_speed, start_speed);

  float real_distance = 160.0 * 0.00001 * (data->T->target_distance - 5); // km
  data->acceleration = (1 / (2 * real_distance));
  data->acceleration *= (data->T->target_speed - start_speed) * (data->T->target_speed + start_speed);
  // data->acceleration == km/h/h

  loggerf(DEBUG, "Train_speed_timer_run (data->acceleration at %f km/h^2)", data->acceleration);
  if (data->acceleration > 64800.0){ // 5 m/s^2
    loggerf(INFO, "data->acceleration to large, reduced to 5.0m/s^2)");
    data->acceleration = 64800.0;
  }
  else if (data->acceleration < -129600.0){
    loggerf(INFO, "Deccell to large, reduced to 10.0m/s^2)");
    data->acceleration = -129600.0;
  }

  if (data->acceleration == 0 || data->acceleration == 0.0){
    loggerf(INFO, "No speed difference");
    data->T->changing_speed = RAILTRAIN_SPEED_T_DONE;
    return;
  }

  loggerf(INFO, "train_speed  sqrt((2 * (%f - %f)) / (%f))", real_distance, start_speed, data->acceleration);

  data->time = sqrt(2 * (data->acceleration) * real_distance + data->T->speed * data->T->speed) - data->T->speed;
  data->time /= data->acceleration;
  data->time *= 3600; // convert to seconds

  if(data->time < 0.0){
    data->time *= -1.0;
  }
  
  data->steps = (uint16_t)(data->time / 0.5);
  data->stepTime = ((data->time / data->steps) * 1000000L); // convert to usec

  data->startSpeed = data->T->speed;

  loggerf(INFO, "train_speed time %f", data->time);
}

void train_speed_event_init(RailTrain * T){
  loggerf(INFO, "train_speed_event_init (%i -> %ikm/h, %icm)\n", T->speed, T->target_speed, T->target_distance);

  T->speed_event_data->T = T;

  train_speed_event_calc(T->speed_event_data);

  T->speed_event->interval.tv_sec = T->speed_event_data->stepTime / 1000000L;
  T->speed_event->interval.tv_nsec = (T->speed_event_data->stepTime % 1000000UL) * 1000;
  scheduler->enableEvent(T->speed_event);
  T->speed_event_data->stepCounter = 0;

  if(T->changing_speed == RAILTRAIN_SPEED_T_DONE)
    return;
}

void train_speed_event_tick(struct TrainSpeedEventData * data){
  data->stepCounter++;

  loggerf(INFO, "train_speed_event_tick %i a:%f, s:(%i/%i), t:%i", data->T->speed, data->acceleration, data->stepCounter, data->steps, data->stepTime);

  RailTrain * T = data->T;

  if(T->changing_speed == RAILTRAIN_SPEED_T_DONE){
    scheduler->disableEvent(T->speed_event);
    return;
  }

  T->speed = (data->startSpeed + data->acceleration * ((data->stepTime * data->stepCounter) / 1000000.0 / 3600.0)) + 0.01; // hours

  loggerf(INFO, "train_speed_timer_run %i %f", T->speed, data->acceleration);

  if(T->type == TRAIN_ENGINE_TYPE){
    engine_set_speed((Engines *)T->p, T->speed);
    Z21_Set_Loco_Drive_Engine((Engines *)T->p);
  }
  else{
    train_set_speed((Trains *)T->p, T->speed);
    Z21_Set_Loco_Drive_Train((Trains *)T->p);
  }
  WS_stc_UpdateTrain(T);

  if (data->stepCounter >= data->steps){
    T->changing_speed = RAILTRAIN_SPEED_T_DONE;
    scheduler->disableEvent(T->speed_event);
  }

  return;
}


void train_set_route(RailTrain * T, Block * dest){
  // struct pathfindingstep path = pathfinding(T->B, dest);

  // if(path.found){
  //   T->route = 1;
  //   T->instructions = path.instructions;
  // }
}