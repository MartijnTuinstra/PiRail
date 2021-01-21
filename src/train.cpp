#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

#include "system.h"
#include "utils/mem.h"
#include "utils/logger.h"

#include "train.h"
#include "config/RollingConfig.h"
#include "switchboard/switch.h"

#include "Z21_msg.h"

#include "websocket/server.h"
#include "websocket/stc.h"

#include "scheduler/scheduler.h"

// #include "pathfinding.h"

// #include "./../lib/pathfinding.h"
// #include "./../lib/com.h"
// #include "./../lib/Z21.h"

#define MAX_TIMERS 10

#define ROUND(nr)  (int)(nr+0.5)

struct cat_conf * train_P_cat;
int train_P_cat_len = 0;
struct cat_conf * train_C_cat;
int train_C_cat_len = 0;

struct SchedulerEvent * railtraincontinue_event;

// int load_rolling_Configs(const char * filename){
//   loggerf(INFO, "Initializing cars/engines/trains");

//   // Allocation Basic Space
//   trains = (Train **)_calloc(10, Train *);
//   trains_len = 10;
//   engines = (Engine **)_calloc(10, Engine *);
//   engines_len = 10;
//   cars = (Car **)_calloc(10, Car *);
//   cars_len = 10;
//   trains_comp = (struct train_composition **)_calloc(10, struct train_composition *);
//   trains_comp_len = 10;
//   train_link = (RailTrain **)_calloc(10,RailTrain *);
//   train_link_len = 10;
  
//   railtraincontinue_event = scheduler->addEvent("RailTrain_continue", {2, 0});
//   railtraincontinue_event->function = &RailTrain_ContinueCheck;
//   scheduler->enableEvent(railtraincontinue_event);

//   // read_rolling_Configs();

//   auto config = RollingConfig(filename);

//   config.read();

//   train_P_cat = (struct cat_conf *)_calloc(config.header.P_Catagories, struct cat_conf);
//   train_P_cat_len = config.header.P_Catagories;
//   memcpy(train_P_cat, config.P_Cat, sizeof(struct cat_conf) * train_P_cat_len);

//   train_C_cat = (struct cat_conf *)_calloc(config.header.C_Catagories, struct cat_conf);
//   train_C_cat_len = config.header.C_Catagories;
//   memcpy(train_C_cat, config.C_Cat, sizeof(struct cat_conf) * train_C_cat_len);

//   loggerf(INFO, "Reading Engines");
//   for(int i = 0; i < config.header.Engines; i++){
//     auto E = new Engine(config.Engines[i]);
//     auto index = find_free_index(engines, engines_len);

//     engines[index] = E;
//     DCC_train[E->DCC_ID] = E;
//   }
  
//   loggerf(INFO, "Reading Cars");
//   for(int i = 0; i < config.header.Cars; i++){
//     auto C = new Car(config.Cars[i]);
//     int index = find_free_index(cars, cars_len);

//     cars[index] = C;
//   }
  
//   loggerf(INFO, "Reading Trains");
//   for(int i = 0; i < config.header.Trains; i++){
//     auto T = new Train(config.Trains[i]);

//     int index = find_free_index(trains, trains_len);

//     trains[index] = T;
//     T->id = index;
//   }

//   SYS->trains_loaded = 1;

//   return 1;
// }
/*
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
*/
/*
void write_rolling_Configs(){
  auto config = RollingConfig(TRAIN_CONF_PATH);

  config.header.P_Catagories = train_P_cat_len;
  config.P_Cat = (struct cat_conf *)_calloc(config.header.P_Catagories, struct cat_conf);
  memcpy(config.P_Cat, train_P_cat, sizeof(struct cat_conf) * train_P_cat_len);

  config.header.C_Catagories = train_C_cat_len;
  config.C_Cat = (struct cat_conf *)_calloc(config.header.C_Catagories, struct cat_conf);
  memcpy(config.C_Cat, train_C_cat, sizeof(struct cat_conf) * train_C_cat_len);

  for(uint8_t i = 0; i < trains_len; i++){
    config.addTrain(trains[i]);
  }
  for(uint8_t i = 0; i < engines_len; i++){
    config.addEngine(engines[i]);
  }
  for(uint8_t i = 0; i < cars_len; i++){
    config.addCar(cars[i]);
  }

  config.write();
}*/
/*
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
*/
/*
void unload_rolling_Configs(){
  scheduler->removeEvent(railtraincontinue_event);

  log("Clearing trains memory",INFO);

  if(trains){
    for(int i = 0;i<trains_len;i++){
      if(!trains[i])
        continue;

      delete trains[i];
    }
    trains = (Train **)_free(trains);
  }

  if(engines){
    for(int i = 0;i<engines_len;i++){
      if(!engines[i])
        continue;

      delete engines[i];
    }
    engines = (Engine **)_free(engines);
  }

  if(cars){
    for(int i = 0;i<cars_len;i++){
      if(!cars[i])
        continue;

      delete cars[i];
    }
    cars = (Car **)_free(cars);
  }

  if(trains_comp){
    for(int i = 0;i<trains_comp_len;i++){
      if(trains_comp[i]){
        trains_comp[i]->name = (char *)_free(trains_comp[i]->name);
        trains_comp[i]->composition = (struct train_comp *)_free(trains_comp[i]->composition);
        trains_comp[i] = (struct train_composition *)_free(trains_comp[i]);
      }
    }
    trains_comp = (train_composition **)_free(trains_comp);
  }

  if(train_link){
    for(int i = 0; i < train_link_len; i++){
      if(!train_link[i])
        continue;

      scheduler->removeEvent(train_link[i]->speed_event);

      _free(train_link[i]);
      train_link[i] = 0;
    }

    train_link = (RailTrain **)_free(train_link);
  }

  if(train_P_cat){
    for(int i = 0; i < train_P_cat_len; i++){
      _free(train_P_cat[i].name);
    }
    train_P_cat = (cat_conf *)_free(train_P_cat);
  }


  if(train_C_cat){
    for(int i = 0; i < train_C_cat_len; i++){
      _free(train_C_cat[i].name);
    }
    train_C_cat = (cat_conf *)_free(train_C_cat);
  }


  trains_len = 0;
  engines_len = 0;
  cars_len = 0;
  trains_comp_len = 0;

  if(SYS)
    SYS->trains_loaded = 0;
}
*/

// Speed

// void Train_Set_Speed(RailTrain * T){

// }


