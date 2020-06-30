#include <stdio.h>

#include "logger.h"
#include "mem.h"
#include "config.h"
#include "config/RollingConfig.h"

RollingConfig::RollingConfig(char * filename){
  strcpy(this->filename, filename);
  this->parsed = false;
}

RollingConfig::RollingConfig(const char * filename){
  strcpy(this->filename, filename);
  this->parsed = false;
}

RollingConfig::~RollingConfig(){
  _free(this->P_Cat);
  _free(this->C_Cat);
  _free(this->Engines);
  _free(this->Cars);
  _free(this->Trains);
}


int RollingConfig::read(){
  FILE * fp = fopen(filename, "rb");

  if(!fp){
    loggerf(ERROR, "Failed to open file");
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

  uint8_t ** buf_ptr = (uint8_t **)&buffer;

  *buf_ptr += 1;

  this->header = read_s_train_header_conf(buf_ptr);

  if (header[0] != TRAIN_CONF_VERSION) {
    loggerf(ERROR, "Not correct version");
    return -1;
    // this->header.IO_Nodes = 0;
    // loggerf(WARNING, "Please re-save to update", this->header.module);
  }

  this->P_Cat = (struct cat_conf *)_calloc(this->header.P_Catagories, struct cat_conf);
  this->C_Cat = (struct cat_conf *)_calloc(this->header.C_Catagories, struct cat_conf);
  this->Engines = (struct engines_conf *)_calloc(this->header.Engines, struct engines_conf);
  this->Cars = (struct cars_conf *)_calloc(this->header.Cars, struct cars_conf);
  this->Trains = (struct trains_conf *)_calloc(this->header.Trains, struct trains_conf);
  
  for(int i = 0; i < this->header.P_Catagories; i++){
    this->P_Cat[i]  = read_cat_conf(buf_ptr);
  }
  
  for(int i = 0; i < this->header.C_Catagories; i++){
    this->C_Cat[i]  = read_cat_conf(buf_ptr);
  }
  
  for(int i = 0; i < this->header.Engines; i++){
    this->Engines[i]  = read_engines_conf(buf_ptr);
  }
  
  for(int i = 0; i < this->header.Cars; i++){
    this->Cars[i]  = read_cars_conf(buf_ptr);
  }
  
  for(int i = 0; i < this->header.Trains; i++){
    this->Trains[i]  = read_trains_conf(buf_ptr);
  }

  _free(header);
  _free(buffer_start);

  this->parsed = true;

  return 1;
}

int RollingConfig::calc_size(){
  int size = 1; //header
  size += sizeof(struct s_train_header_conf) + 1;
  int subsize = 0;

  //Catagories
  for(int i = 0; i < this->header.P_Catagories; i++){
    size += sizeof(struct s_cat_conf) + 1;
    size += this->P_Cat[i].name_len + 1;
  }
  for(int i = 0; i < this->header.C_Catagories; i++){
    size += sizeof(struct s_cat_conf) + 1;
    size += this->C_Cat[i].name_len + 1;
  }

  //Engines
  for(int i = 0; i < this->header.Engines; i++){
    subsize = sizeof(struct s_engine_conf) + 1;
    subsize += this->Engines[i].name_len + this->Engines[i].img_path_len + 2;
    subsize += this->Engines[i].icon_path_len + 1;
    subsize += this->Engines[i].config_steps * sizeof(struct engine_speed_steps) + 1;

    loggerf(INFO, "Engines %i bytes\n", subsize);

    size += subsize;
  }

  loggerf(INFO, "Size Engines: %i\n", size);

  //Cars
  for(int i = 0; i < this->header.Cars; i++){
    subsize = sizeof(struct s_car_conf) + this->Cars[i].name_len + 2;
    subsize += this->Cars[i].icon_path_len + 2;

    loggerf(INFO, "Car %i bytes\n", subsize);

    size += subsize;
  }

  loggerf(INFO, "Size Cars: %i\n", size);

  //Trains
  for(int i = 0; i < this->header.Trains; i++){
    subsize = sizeof(struct s_train_conf) + 1;
    subsize += this->Trains[i].name_len + 1;
    subsize += sizeof(struct train_comp_ws) * this->Trains[i].nr_stock + 1;

    loggerf(INFO, "Train %i bytes\n", subsize);

    size += subsize;
  }

  return size;
}

void RollingConfig::write(){
  loggerf(DEBUG, "write_train_from_conf");
  int size = this->calc_size();

  loggerf(INFO, "Writing %i bytes", size);

  char * data = (char *)_calloc(size, char);

  data[0] = TRAIN_CONF_VERSION;

  char * p = &data[1];
  //Copy header
  memcpy(p, &this->header, sizeof(struct s_train_header_conf));

  p += sizeof(struct s_train_header_conf) + 1;

  //Copy Catagories
  for(int i = 0; i < this->header.P_Catagories; i++){
    memcpy(p, &this->P_Cat[i], sizeof(struct s_cat_conf));
    p += sizeof(struct s_cat_conf) + 1;

    memcpy(p, this->P_Cat[i].name, this->P_Cat[i].name_len);
    p += this->P_Cat[i].name_len + 1;
  }
  for(int i = 0; i < this->header.C_Catagories; i++){
    memcpy(p, &this->C_Cat[i], sizeof(struct s_cat_conf));
    p += sizeof(struct s_cat_conf) + 1;

    memcpy(p, this->C_Cat[i].name, this->C_Cat[i].name_len);
    p += this->C_Cat[i].name_len + 1;
  }

  //Copy Engine
  for(int i = 0; i < this->header.Engines; i++){
    memcpy(p, &this->Engines[i], sizeof(struct s_engine_conf));
    p += sizeof(struct s_engine_conf) + 1;

    memcpy(p, this->Engines[i].name, this->Engines[i].name_len);
    p += this->Engines[i].name_len + 1;

    memcpy(p, this->Engines[i].img_path, this->Engines[i].img_path_len);
    p += this->Engines[i].img_path_len + 1;

    memcpy(p, this->Engines[i].icon_path, this->Engines[i].icon_path_len);
    p += this->Engines[i].icon_path_len + 1;

    memcpy(p, this->Engines[i].speed_steps, this->Engines[i].config_steps * sizeof(struct engine_speed_steps));
    p += this->Engines[i].config_steps * sizeof(struct engine_speed_steps) + 1;
  }

  //Copy Cars
  for(int i = 0; i < this->header.Cars; i++){
    memcpy(p, &this->Cars[i], sizeof(struct s_car_conf));
    p += sizeof(struct s_car_conf) + 1;

    memcpy(p, this->Cars[i].name, this->Cars[i].name_len);
    p += this->Cars[i].name_len + 1;

    memcpy(p, this->Cars[i].icon_path, this->Cars[i].icon_path_len);
    p += this->Cars[i].icon_path_len + 1;
  }

  //Copy trains
  for(int i =0; i < this->header.Trains; i++){
    memcpy(p, &this->Trains[i], sizeof(struct s_train_conf));
    p += sizeof(struct s_train_conf) + 1;

    memcpy(p, this->Trains[i].name, this->Trains[i].name_len);
    p += this->Trains[i].name_len + 1;

    memcpy(p, this->Trains[i].composition, sizeof(struct train_comp_ws) * this->Trains[i].nr_stock);
    p += sizeof(struct train_comp_ws) * this->Trains[i].nr_stock + 1;
  }

  //Print output
  // print_hex(data, size);

  FILE * fp = fopen(filename, "wb");

  fwrite(data, size, 1, fp);

  fclose(fp);

  _free(data);
}


void print_Cars(struct cars_conf car){
  char debug[200];

  sprintf(debug, "%i\t%x\t%i\t%i\t%-20s\t%-20s",
                car.nr,
                car.type & 0x0f,
                car.max_speed,
                car.length,
                car.name,
                car.icon_path);

  printf( "%s\n", debug);
}

void print_Engines(struct engines_conf engine){
  char debug[200];

  sprintf(debug, "%i\t%x\t%x\t%i\t%-20s\t%-20s\t%-20s",
                engine.DCC_ID,
                engine.type >> 4,
                engine.type & 0x0f,
                engine.length,
                engine.name,
                engine.img_path,
                engine.icon_path);

  printf( "%s\n", debug);
}

void print_Trains(struct trains_conf train){
  char debug[220];
  char * debugptr = debug;

  debugptr += sprintf(debugptr, "%-20s\t%i\t\t",
                train.name,
                train.nr_stock);

  for(int i = 0; i < train.nr_stock; i++){
    debugptr += sprintf(debugptr, "%i:%i\t", train.composition[i].type, train.composition[i].id);
  }

  printf( "%s\n", debug);
}

void print_Catagories(struct RollingConfig * config){
  uint8_t max_cats = config->header.P_Catagories;
  if (config->header.C_Catagories > max_cats)
    max_cats = config->header.C_Catagories;

  printf("\tPerson\t\t\t\tCargo\n");
  for(int i = 0; i < max_cats; i++){
    if(i < config->header.P_Catagories)
      printf("%3i\t%-20s\t", i, config->P_Cat[i].name);
    else
      printf("   \t                                        ");

    if(i < config->header.C_Catagories)
      printf("%3i\t%-20s\n", i + 0x80, config->C_Cat[i].name);
    else
      printf("\n");
  }
}

void RollingConfig::print(char ** cmds, uint8_t cmd_len){
  printf( "Cars:    %i\n", this->header.Cars);
  printf( "Engines: %i\n", this->header.Engines);
  printf( "Trains:  %i\n", this->header.Trains);

  printf("\nCatagories\n");
  print_Catagories(this);
  printf("\n\n");

  printf( "Cars\n");
  printf( "id\tNr\tType\tSpeed\tLength\tName\t\t\tIcon_path\n");
  for(int i = 0; i < this->header.Cars; i++){
    printf("%i\t", i);
    print_Cars(this->Cars[i]);
  }
  
  printf( "Engines\n");
  printf( "id\tDCC\tSteps\tType\tLength\tName\t\t\tImg_path\t\tIcon_path\n");
  for(int i = 0; i < this->header.Engines; i++){
    printf("%i\t", i);
    print_Engines(this->Engines[i]);
  }

  printf( "Trains\n");
  printf( "id\tName\t\t\tRolling Stock\t...\n");
  for(int i = 0; i < this->header.Trains; i++){
    printf("%i\t", i);
    print_Trains(this->Trains[i]);
  }
}