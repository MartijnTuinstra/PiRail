#include <stdio.h>

#include "utils/logger.h"
#include "utils/mem.h"
#include "config/RollingConfig.h"

#include "config/RollingStructure.h"
#include "config/configReader.h"

#include "rollingstock/train.h"
#include "rollingstock/engine.h"
#include "rollingstock/car.h"

RollingConfig::RollingConfig(char * _filename){
  memset(this, 0, sizeof(RollingConfig));
  strcpy(filename, _filename);
  parsed = false;
}

RollingConfig::RollingConfig(const char * _filename){
  memset(this, 0, sizeof(RollingConfig));
  strcpy(filename, _filename);
  parsed = false;
}

// RollingConfig::RollingConfig(char * _filename, RollingConfig * oC){
//   memset(this, 0, sizeof(RollingConfig));
  
//   strcpy(filename, _filename);
//   parsed = true;

//   header = (struct configStruct_TrainHeader *)_calloc(1, struct configStruct_TrainHeader);

//   header->PersonCatagories = oC->header.P_Catagories;
//   header->CargoCatagories  = oC->header.C_Catagories;
//   header->Trains           = oC->header.Trains;
//   header->Engines          = oC->header.Engines;
//   header->Cars             = oC->header.Cars;

//   P_Cat = (struct configStruct_Category *)_calloc(header->PersonCatagories, struct configStruct_Category);
//   C_Cat = (struct configStruct_Category *)_calloc(header->CargoCatagories, struct configStruct_Category);

//   Trains  = (struct configStruct_Train  *)_calloc(header->Trains, struct configStruct_Train);
//   Engines = (struct configStruct_Engine *)_calloc(header->Trains, struct configStruct_Engine);
//   Cars    = (struct configStruct_Car    *)_calloc(header->Trains, struct configStruct_Car);

//   for(uint16_t i = 0; i < header->PersonCatagories; i++){
//     P_Cat[i].name_len = oC->P_Cat[i].name_len;
//     P_Cat[i].name = (char *)_calloc(P_Cat[i].name_len, char);
//     strcpy(P_Cat[i].name, oC->P_Cat[i].name);
//   }
//   for(uint16_t i = 0; i < header->CargoCatagories;  i++){
//     C_Cat[i].name_len = oC->C_Cat[i].name_len;
//     C_Cat[i].name = (char *)_calloc(C_Cat[i].name_len, char);
//     strcpy(C_Cat[i].name, oC->C_Cat[i].name);
//   }

//   for(uint16_t i = 0; i < header->Cars; i++){
//     Cars[i].nr            = oC->Cars[i].nr;
//     Cars[i].max_speed     = oC->Cars[i].max_speed;
//     Cars[i].length        = oC->Cars[i].length;
//     Cars[i].flags         = oC->Cars[i].flags;
//     Cars[i].type          = oC->Cars[i].type;
//     Cars[i].name_len      = oC->Cars[i].name_len;
//     Cars[i].icon_path_len = oC->Cars[i].icon_path_len;

//     memcpy(Cars[i].functions, oC->Cars[i].functions, sizeof(uint8_t));

//     Cars[i].name = (char *)_calloc(Cars[i].name_len, char);
//     Cars[i].icon_path = (char *)_calloc(Cars[i].icon_path_len, char);

//     strcpy(Cars[i].name, oC->Cars[i].name);
//     strcpy(Cars[i].icon_path, oC->Cars[i].icon_path);
//   }

//   for(uint16_t i = 0; i < header->Engines; i++){
//     Engines[i].DCC_ID = oC->Engines[i].DCC_ID;
//     Engines[i].length = oC->Engines[i].length;
//     Engines[i].type   = oC->Engines[i].type;

//     Engines[i].config_steps  = oC->Engines[i].config_steps;
//     Engines[i].name_len      = oC->Engines[i].name_len;
//     Engines[i].img_path_len  = oC->Engines[i].img_path_len;
//     Engines[i].icon_path_len = oC->Engines[i].icon_path_len;

//     memcpy(Engines[i].functions, oC->Engines[i].functions, sizeof(uint8_t));

//     Engines[i].name = (char *)_calloc(Engines[i].name_len, char);
//     Engines[i].img_path = (char *)_calloc(Engines[i].img_path_len, char);
//     Engines[i].icon_path = (char *)_calloc(Engines[i].icon_path_len, char);
//     Engines[i].speed_steps = (struct configStruct_EngineSpeedSteps *)_calloc(Engines[i].config_steps, struct configStruct_EngineSpeedSteps);

//     strcpy(Engines[i].name, oC->Engines[i].name);
//     strcpy(Engines[i].img_path, oC->Engines[i].img_path);
//     strcpy(Engines[i].icon_path, oC->Engines[i].icon_path);

//     memcpy(Engines[i].speed_steps, oC->Engines[i].speed_steps, Engines[i].config_steps * sizeof(struct configStruct_EngineSpeedSteps));
//   }

//   for(uint16_t i = 0; i < header->Trains; i++){
//     Trains[i].name_len = oC->Trains[i].name_len;
//     Trains[i].nr_stock = oC->Trains[i].nr_stock;
//     Trains[i].category = oC->Trains[i].category;

//     Trains[i].name = (char *)_calloc(Trains[i].name_len, char);
//     strcpy(Trains[i].name, oC->Trains[i].name);

//     Trains[i].composition = (struct configStruct_TrainComp *)_calloc(Trains[i].nr_stock, struct configStruct_TrainComp);
//     for(uint16_t j = 0; j < Trains[i].nr_stock; j++){
//       Trains[i].composition[j].type = oC->Trains[i].composition[j].type;
//       Trains[i].composition[j].id   = oC->Trains[i].composition[j].id;
//     }
//   }
// }

void RollingConfig::addTrain(Train * T){
  if(!T)
    return;

  if(!Trains)
    Trains = (struct configStruct_Train *)_calloc(header->Trains + 1, struct configStruct_Train);
  else
    Trains = (struct configStruct_Train *)_realloc(Trains, header->Trains + 1, struct configStruct_Train);

  struct configStruct_Train * cT = &Trains[header->Trains++];

  cT->name_len = strlen(T->name);
  cT->name = (char *)_calloc(cT->name_len, char);
  strcpy(cT->name, T->name);

  cT->nr_stock = T->nr_stock;
  cT->category = T->type;

  cT->composition = (struct configStruct_TrainComp *)_calloc(T->nr_stock, struct configStruct_TrainComp);

  for(int i = 0; i < T->nr_stock; i++){
    cT->composition[i].type = T->composition[i].type;
    cT->composition[i].id = T->composition[i].id;
  }
}

void RollingConfig::addEngine(Engine * E){
  if(!E)
    return;

  if(!Engines)
    Engines = (struct configStruct_Engine *)_calloc(header->Engines + 1, struct configStruct_Engine);
  else
    Engines = (struct configStruct_Engine *)_realloc(Engines, header->Engines + 1, struct configStruct_Engine);

  struct configStruct_Engine * cE = &Engines[header->Engines++];

  cE->DCC_ID = E->DCC_ID;
  cE->length = E->length;
  cE->type = E->type;
  cE->config_steps = E->steps_len;
  cE->name_len = strlen(E->name);
  cE->img_path_len = strlen(E->img_path);
  cE->icon_path_len = strlen(E->icon_path);
  for(uint8_t i = 0; i < 29; i++){
    cE->functions[i] = (E->function[i].button << 6) | (E->function[i].type & 0x3F);
  }
  
  cE->name = (char *)_calloc(strlen(E->name), char);
  strcpy(cE->name, E->name);
  cE->img_path = (char *)_calloc(strlen(E->img_path), char);
  strcpy(cE->img_path, E->img_path);
  cE->icon_path = (char *)_calloc(strlen(E->icon_path), char);
  strcpy(cE->icon_path, E->icon_path);

  cE->speed_steps = (struct configStruct_EngineSpeedSteps *)_calloc(E->steps_len, sizeof(struct configStruct_EngineSpeedSteps));
  memcpy(cE->speed_steps, E->steps, sizeof(struct configStruct_EngineSpeedSteps) * E->steps_len);
}
void RollingConfig::addCar(Car * C){
  if(!C)
    return;

  if(!Cars)
    Cars = (struct configStruct_Car *)_calloc(header->Cars + 1, struct configStruct_Car);
  else
    Cars = (struct configStruct_Car *)_realloc(Cars, header->Cars + 1, struct configStruct_Car);

  struct configStruct_Car * cC = &Cars[header->Cars++];

  cC->nr = C->nr;
  cC->max_speed = C->max_speed;
  cC->length = C->length;
  cC->type = C->type;
  cC->name_len = strlen(C->name);
  cC->icon_path_len = strlen(C->icon_path);
  for(uint8_t i = 0; i < 29; i++){
    cC->functions[i] = (C->function[i].button << 6) | (C->function[i].type & 0x3F);
  }

  cC->name = (char *)_calloc(strlen(C->name), char);
  strcpy(cC->name, C->name);
  cC->icon_path = (char *)_calloc(strlen(C->icon_path), char);
  strcpy(cC->icon_path, C->icon_path);
}

RollingConfig::~RollingConfig(){
  // _free(P_Cat);
  // _free(C_Cat);

  // for(uint8_t i = 0; i < header->Engines; i++){
  //   _free(Engines[i].name);
  //   _free(Engines[i].img_path);
  //   _free(Engines[i].icon_path);
  //   _free(Engines[i].speed_steps);
  // }
  // _free(Engines);

  // for(uint8_t i = 0; i < header->Cars; i++){
  //   _free(Cars[i].name);
  //   _free(Cars[i].icon_path);
  // }
  // _free(Cars);

  // for(uint8_t i = 0; i < header->Trains; i++){
  //   _free(Trains[i].name);
  //   _free(Trains[i].composition);
  // }
  // _free(Trains);

  _free(header);
  _free(buffer);
}


int RollingConfig::read(){
  if(parsed){
    loggerf(WARNING, "Allready parsed, aborting!");
    return -1;
  }

  loggerf(INFO, "Reading Rolling Stock configuration from %s", filename);
  FILE * fp = fopen(filename, "rb");

  if(!fp){
    loggerf(ERROR, "Failed to open file");
    return -1;
  }

  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  buffer_len = fsize + 10;
  buffer = (char *)_calloc(buffer_len, char);
  fread(buffer, fsize, 1, fp);

  uint8_t * base_buf_ptr = (uint8_t *)&buffer[0];
  uint8_t ** buf_ptr = &base_buf_ptr;

  uint8_t fileVersion;
  Config_read_uint8_t_uint8_t(&fileVersion, buf_ptr);

  if (fileVersion > CONFIG_ROLLINGSTRUCTURE_LU_MAX_VERSION) {
    loggerf(WARNING, "Rolling Config not correct version (%s)", filename);
    return -1;
  }

  header = (struct configStruct_TrainHeader *)_calloc(1, struct configStruct_TrainHeader);

  Config_read_TrainHeader(fileVersion, header, buf_ptr);

  loggerf(INFO, "RollingConfig: %i %i %i %i %i  (%x - %x)", header->PersonCatagories, header->CargoCatagories, header->Engines, header->Cars, header->Trains, base_buf_ptr, *buf_ptr);

  P_Cat   = (struct configStruct_Category *)_calloc(header->PersonCatagories, struct configStruct_Category);
  C_Cat   = (struct configStruct_Category *)_calloc(header->CargoCatagories, struct configStruct_Category);
  Engines = (struct configStruct_Engine *)_calloc(header->Engines, struct configStruct_Engine);
  Cars    = (struct configStruct_Car *)_calloc(header->Cars, struct configStruct_Car);
  Trains  = (struct configStruct_Train *)_calloc(header->Trains, struct configStruct_Train);
  
  for(int i = 0; i < header->PersonCatagories; i++){
    Config_read_Category(fileVersion, &P_Cat[i], buf_ptr);
  }
  
  for(int i = 0; i < header->CargoCatagories; i++){
    Config_read_Category(fileVersion, &C_Cat[i], buf_ptr);
  }
  
  for(int i = 0; i < header->Engines; i++){
    Config_read_Engine(fileVersion, &Engines[i], buf_ptr);
  }
  
  for(int i = 0; i < header->Cars; i++){
    Config_read_Car(fileVersion, &Cars[i], buf_ptr);
  }
  
  for(int i = 0; i < header->Trains; i++){
    Config_read_Train(fileVersion, &Trains[i], buf_ptr);
  }

  parsed = true;

  _free(buffer);
  fclose(fp);

  return 1;
}

int RollingConfig::calc_size(){
  int size = 1 + Config_write_size_TrainHeader(header);

  //Catagories
  for(int i = 0; i < header->PersonCatagories; i++){
    size += Config_write_size_Category(&P_Cat[i]);
  }
  for(int i = 0; i < header->CargoCatagories; i++){
    size += Config_write_size_Category(&C_Cat[i]);
  }

  //Engines
  for(int i = 0; i < header->Engines; i++){
    size += Config_write_size_Engine(&Engines[i]);
  }

  //Cars
  for(int i = 0; i < header->Cars; i++){
    size += Config_write_size_Car(&Cars[i]);
  }

  //Trains
  for(int i = 0; i < header->Trains; i++){
    size += Config_write_size_Train(&Trains[i]);
  }

  return size;
}

void RollingConfig::dump(){
  FILE * fp = fopen(filename, "wb");

  fwrite(buffer, buffer_len - 10, 1, fp);

  fclose(fp);
}

void RollingConfig::write(){
  int size = calc_size();

  loggerf(WARNING, "write_train_from_conf (%i bytes)", size);

  char * data = (char *)_calloc(size + 50, char);
  uint8_t * p = (uint8_t *)data;

  {
    uint8_t tmp = CONFIG_ROLLINGSTRUCTURE_LU_MAX_VERSION;
    Config_write_uint8_t(&tmp, &p);
  }

  //Copy header
  Config_write_TrainHeader(header, &p);

  //Copy Catagories
  for(int i = 0; i < header->PersonCatagories; i++){
    Config_write_Category(&P_Cat[i], &p);
  }
  for(int i = 0; i < header->CargoCatagories; i++){
    Config_write_Category(&C_Cat[i], &p);
  }

  //Copy Engine
  for(int i = 0; i < header->Engines; i++){
    Config_write_Engine(&Engines[i], &p);
  }

  //Copy Cars
  for(int i = 0; i < header->Cars; i++){
    Config_write_Car(&Cars[i], &p);
  }

  //Copy trains
  for(int i =0; i < header->Trains; i++){
    Config_write_Train(&Trains[i], &p);
  }

  FILE * fp = fopen(filename, "wb");

  fwrite(data, size, 1, fp);

  fclose(fp);

  _free(data);
}


void print_Cars(struct configStruct_Car car){
  char debug[200];

  sprintf(debug, "%i\t%x\t%x\t%i\t%i\t%-20s\t%-20s",
                car.nr,
                car.type & 0x0f,
                car.flags,
                car.max_speed,
                car.length,
                car.name,
                car.icon_path);

  printf( "%s\n", debug);
}

void print_Engines(struct configStruct_Engine engine){
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

void print_Trains(struct configStruct_Train train){
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

void print_Catagories(RollingConfig * config){
  uint8_t max_cats = config->header->PersonCatagories;
  if (config->header->CargoCatagories > max_cats)
    max_cats = config->header->CargoCatagories;

  printf("\tPerson\t\t\t\tCargo\n");
  for(int i = 0; i < max_cats; i++){
    if(i < config->header->PersonCatagories)
      printf("%3i\t%-20s\t", i, config->P_Cat[i].name);
    else
      printf("   \t                                        ");

    if(i < config->header->CargoCatagories)
      printf("%3i\t%-20s\n", i + 0x80, config->C_Cat[i].name);
    else
      printf("\n");
  }
}

void RollingConfig::print(char ** cmds, uint8_t cmd_len){
  uint16_t mask = 0;

  if(cmds == 0){
    if(cmd_len)
      return;
    
    mask = 0x1FF;
  }
  else{
    cmds = &cmds[1];
    cmd_len -= 1;
  }

  printf( "Cars:    %i\n", this->header->Cars);
  printf( "Engines: %i\n", this->header->Engines);
  printf( "Trains:  %i\n", this->header->Trains);

  printf("\nCatagories\n");
  print_Catagories(this);
  printf("\n\n");

  printf( "Cars\n");
  printf( "id\tNr\tType\tFlags\tSpeed\tLength\tName\t\t\tIcon_path\n");
  for(int i = 0; i < this->header->Cars; i++){
    printf("%i\t", i);
    print_Cars(this->Cars[i]);
  }
  
  printf( "Engines\n");
  printf( "id\tDCC\tSteps\tType\tLength\tName\t\t\tImg_path\t\tIcon_path\n");
  for(int i = 0; i < this->header->Engines; i++){
    printf("%i\t", i);
    print_Engines(this->Engines[i]);
  }

  printf( "Trains\n");
  printf( "id\tName\t\t\tRolling Stock\t...\n");
  for(int i = 0; i < this->header->Trains; i++){
    printf("%i\t", i);
    print_Trains(this->Trains[i]);
  }
}