#include <stdint.h>
#include "config.h"
#include "logger.h"
#include "mem.h"

uint8_t read_byte_conf(uint8_t ** p){
  uint8_t byte = **p;
  *p += 1;
  return byte;
}

int calc_write_module_size(struct module_config * config){
  int size = 1; //header
  size += sizeof(struct s_unit_conf) + 1;

  //Nodes
  size += (sizeof(struct s_node_conf) + 1) * config->header.IO_Nodes;

  //Blocks
  size += (sizeof(struct s_block_conf) + 1) * config->header.Blocks;

  //Switches
  for(int i = 0; i < config->header.Switches; i++){
    size += sizeof(struct s_switch_conf) + 1;
    size += sizeof(struct s_IO_port_conf) * (config->Switches[i].IO & 0xf) + 1;
  }

  //MSSwitches
  for(int i = 0; i < config->header.MSSwitches; i++){
    size += sizeof(struct s_ms_switch_conf) + 1;
    size += sizeof(struct s_ms_switch_state_conf) * config->MSSwitches[i].nr_states + 1;
    size += 2 * config->MSSwitches[i].IO + 1;
  }


  //Signals
  for(int i = 0; i <  config->header.Signals; i++){
    size += sizeof(struct s_signal_conf) + 1;
    size += config->Signals[i].output_len * sizeof(struct s_IO_port_conf) + 1;
    size += config->Signals[i].output_len * sizeof(struct s_IO_signal_event_conf) + 1;
  }

  //Stations
  for(int i = 0; i <  config->header.Stations; i++){
    size += sizeof(struct s_station_conf) + 1;
    size += config->Stations[i].name_len + 1;
    size += config->Stations[i].nr_blocks + 1;
  }

  //Layout

  size += 2 + config->Layout_length;

  return size;
}

int calc_write_train_size(struct train_config * config){
  int size = 1; //header
  size += sizeof(struct s_train_header_conf) + 1;
  int subsize = 0;

  //Catagories
  for(int i = 0; i < config->header.P_Catagories; i++){
    size += sizeof(struct s_cat_conf) + 1;
    size += config->P_Cat[i].name_len + 1;
  }
  for(int i = 0; i < config->header.C_Catagories; i++){
    size += sizeof(struct s_cat_conf) + 1;
    size += config->C_Cat[i].name_len + 1;
  }

  //Engines
  for(int i = 0; i < config->header.Engines; i++){
    subsize = sizeof(struct s_engine_conf) + 1;
    subsize += config->Engines[i].name_len + config->Engines[i].img_path_len + 2;
    subsize += config->Engines[i].icon_path_len + 1;
    subsize += config->Engines[i].config_steps * sizeof(struct engine_speed_steps) + 1;

    loggerf(INFO, "Engines %i bytes\n", subsize);

    size += subsize;
  }

  loggerf(INFO, "Size Engines: %i\n", size);

  //Cars
  for(int i = 0; i < config->header.Cars; i++){
    subsize = sizeof(struct s_car_conf) + config->Cars[i].name_len + 2;
    subsize += config->Cars[i].img_path_len + config->Cars[i].icon_path_len + 2;

    loggerf(INFO, "Car %i bytes\n", subsize);

    size += subsize;
  }

  loggerf(INFO, "Size Cars: %i\n", size);

  //Trains
  for(int i = 0; i < config->header.Trains; i++){
    subsize = sizeof(struct s_train_conf) + 1;
    subsize += config->Trains[i].name_len + 1;
    subsize += sizeof(struct train_comp_ws) * config->Trains[i].nr_stock + 1;

    loggerf(INFO, "Train %i bytes\n", subsize);

    size += subsize;
  }

  return size;
}

void print_hex(char * data, int size){
  if(read_level() >= TRACE){
    printf("print_hex:\n");
    for(int i = 0; i < size; i++){
      printf("%02x ", data[i]);
      if((i % 16) == 15)
        printf("\n");
    }
    printf("\n");
  }
}

void write_module_from_conf(struct module_config * config, char * filename){
  printf("write_from_conf\n");
  int size = calc_write_module_size(config);

  loggerf(INFO, "Writing %i bytes", size);

  char * data = _calloc(size, 1);

  data[0] = MODULE_CONF_VERSION;

  char * p = &data[1];
  //Copy header
  memcpy(p, &config->header, sizeof(struct s_unit_conf));

  p += sizeof(struct s_unit_conf) + 1;

  //Copy Nodes
  for(int i = 0; i < config->header.IO_Nodes; i++){
    memcpy(p, &config->Nodes[i], sizeof(struct s_node_conf));

    p += sizeof(struct s_node_conf) + 1;
  }

  //Copy blocks
  for(int i = 0; i < config->header.Blocks; i++){
    memcpy(p, &config->Blocks[i], sizeof(struct s_block_conf));

    p += sizeof(struct s_block_conf) + 1;
  }

  //Copy Switches
  for(int i = 0; i < config->header.Switches; i++){
    memcpy(p, &config->Switches[i], sizeof(struct s_switch_conf));

    p += sizeof(struct s_switch_conf) + 1;

    for(int j = 0; j < (config->Switches[i].IO & 0x0f); j++){
      memcpy(p, &config->Switches[i].IO_Ports[j], sizeof(struct s_IO_port_conf));
      p += sizeof(struct s_IO_port_conf);
    }

    p += 1;
  }

  //Copy MMSwitches
  for(int i = 0; i < config->header.MSSwitches; i++){
    memcpy(p, &config->MSSwitches[i], sizeof(struct s_ms_switch_conf));

    p += sizeof(struct s_ms_switch_conf) + 1;

    for(int j = 0; j < config->MSSwitches[i].nr_states; j++){
      memcpy(p, &config->MSSwitches[i].states[j], sizeof(struct s_ms_switch_state_conf));
      p += sizeof(struct s_ms_switch_state_conf);
    }

    p += 1;

    for(int j = 0; j < config->MSSwitches[i].IO; j++){
      memcpy(p, &config->MSSwitches[i].IO_Ports[j], 2);
      p += 2;
    }

    p += 1;
  }

  //Copy Signals
  for(int i = 0; i < config->header.Signals; i++){
    memcpy(p, &config->Signals[i], sizeof(struct s_signal_conf));

    p += sizeof(struct s_signal_conf) + 1;

    memcpy(p, config->Signals[i].output, config->Signals[i].output_len * sizeof(struct s_IO_port_conf));
    p += config->Signals[i].output_len * sizeof(struct s_IO_port_conf) + 1;

    memcpy(p, config->Signals[i].stating, config->Signals[i].output_len * sizeof(struct s_IO_signal_event_conf));
    p += config->Signals[i].output_len * sizeof(struct s_IO_signal_event_conf) + 1;
  }

  //Copy Stations
  for(int i = 0; i < config->header.Stations; i++){
    memcpy(p, &config->Stations[i], sizeof(struct s_station_conf));

    p += sizeof(struct s_station_conf) + 1;

    memcpy(p, config->Stations[i].blocks, config->Stations[i].nr_blocks);
    p += config->Stations[i].nr_blocks + 1;

    memcpy(p, config->Stations[i].name, config->Stations[i].name_len);
    p += config->Stations[i].name_len + 1;
  }

  //Copy Layout
  memcpy(p, &config->Layout_length, sizeof(uint16_t));
  p += sizeof(uint16_t);

  printf("Writing data: \n%s", config->Layout);
  memcpy(p, config->Layout, config->Layout_length);
  p += sizeof(uint16_t);

  //Print output
  print_hex(data, size);

  FILE * fp = fopen(filename, "wb");

  fwrite(data, size, 1, fp);

  fclose(fp);

  free(data);
}

void write_train_from_conf(struct train_config * config, char * filename){
  printf("write_from_conf\n");
  int size = calc_write_train_size(config);

  loggerf(INFO, "Writing %i bytes", size);

  char * data = _calloc(size, 1);

  data[0] = TRAIN_CONF_VERSION;

  char * p = &data[1];
  //Copy header
  memcpy(p, &config->header, sizeof(struct s_train_header_conf));

  p += sizeof(struct s_train_header_conf) + 1;

  //Copy Catagories
  for(int i = 0; i < config->header.P_Catagories; i++){
    memcpy(p, &config->P_Cat[i], sizeof(struct s_cat_conf));
    p += sizeof(struct s_cat_conf) + 1;

    memcpy(p, config->P_Cat[i].name, config->P_Cat[i].name_len);
    p += config->P_Cat[i].name_len + 1;
  }
  for(int i = 0; i < config->header.C_Catagories; i++){
    memcpy(p, &config->C_Cat[i], sizeof(struct s_cat_conf));
    p += sizeof(struct s_cat_conf) + 1;

    memcpy(p, config->C_Cat[i].name, config->C_Cat[i].name_len);
    p += config->C_Cat[i].name_len + 1;
  }

  //Copy Engine
  for(int i = 0; i < config->header.Engines; i++){
    memcpy(p, &config->Engines[i], sizeof(struct s_engine_conf));
    p += sizeof(struct s_engine_conf) + 1;

    memcpy(p, config->Engines[i].name, config->Engines[i].name_len);
    p += config->Engines[i].name_len + 1;

    memcpy(p, config->Engines[i].img_path, config->Engines[i].img_path_len);
    p += config->Engines[i].img_path_len + 1;

    memcpy(p, config->Engines[i].icon_path, config->Engines[i].icon_path_len);
    p += config->Engines[i].icon_path_len + 1;

    memcpy(p, config->Engines[i].speed_steps, config->Engines[i].config_steps * sizeof(struct engine_speed_steps));
    p += config->Engines[i].config_steps * sizeof(struct engine_speed_steps) + 1;
  }

  //Copy Cars
  for(int i = 0; i < config->header.Cars; i++){
    memcpy(p, &config->Cars[i], sizeof(struct s_car_conf));
    p += sizeof(struct s_car_conf) + 1;

    memcpy(p, config->Cars[i].name, config->Cars[i].name_len);
    p += config->Cars[i].name_len + 1;

    memcpy(p, config->Cars[i].img_path, config->Cars[i].img_path_len);
    p += config->Cars[i].img_path_len + 1;

    memcpy(p, config->Cars[i].icon_path, config->Cars[i].icon_path_len);
    p += config->Cars[i].icon_path_len + 1;
  }

  //Copy trains
  for(int i =0; i < config->header.Trains; i++){
    memcpy(p, &config->Trains[i], sizeof(struct s_train_conf));
    p += sizeof(struct s_train_conf) + 1;

    memcpy(p, config->Trains[i].name, config->Trains[i].name_len);
    p += config->Trains[i].name_len + 1;

    memcpy(p, config->Trains[i].composition, sizeof(struct train_comp_ws) * config->Trains[i].nr_stock);
    p += sizeof(struct train_comp_ws) * config->Trains[i].nr_stock + 1;
  }

  //Print output
  print_hex(data, size);

  FILE * fp = fopen(filename, "wb");

  fwrite(data, size, 1, fp);

  fclose(fp);

  free(data);
}

int check_Spacing(uint8_t ** p){
  if(**p != 0){
    loggerf(CRITICAL, "Format missing sepperator, got %x", **p);
    return 0;
  }
  *p += 1;

  return 1;
}

struct s_node_conf read_s_node_conf(uint8_t ** p){
  struct s_node_conf n;
  memcpy(&n, *p, sizeof(struct s_node_conf));

  *p += sizeof(struct s_node_conf);
  
  check_Spacing(p);

  return n;
}

struct s_unit_conf read_s_unit_conf(uint8_t ** p){
  struct s_unit_conf s;
  memcpy(&s, *p, sizeof(struct s_unit_conf));

  *p += sizeof(struct s_unit_conf);
  
  check_Spacing(p);

  return s;
}

struct s_block_conf read_s_block_conf(uint8_t ** p){
  struct s_block_conf s;
  memcpy(&s, *p, sizeof(struct s_block_conf));

  *p += sizeof(struct s_block_conf);

  check_Spacing(p);

  return s;
}

struct switch_conf read_s_switch_conf(uint8_t ** p){
  struct switch_conf s;

  memcpy(&s, *p, sizeof(struct s_switch_conf));

  *p += sizeof(struct s_switch_conf);

  if(!check_Spacing(p))
    return s;

  s.IO_Ports = _calloc(s.IO & 0x0f, struct s_IO_port_conf);

  for(int i = 0; i < (s.IO & 0x0f); i++){
    memcpy(&s.IO_Ports[i], *p, 2);
    *p += sizeof(struct s_IO_port_conf);
  }

  check_Spacing(p);

  return s;
}

struct ms_switch_conf read_s_ms_switch_conf(uint8_t ** p){
  struct ms_switch_conf s;

  print_hex((char *)*p, 16);

  memcpy(&s, *p, sizeof(struct s_ms_switch_conf));

  *p += sizeof(struct s_ms_switch_conf);

  if(!check_Spacing(p))
    return s;

  s.states = _calloc(s.nr_states, struct s_ms_switch_state_conf);

  for(int i = 0; i < s.nr_states; i++){
    memcpy(&s.states[i], *p, sizeof(struct s_ms_switch_state_conf));
    *p += sizeof(struct s_ms_switch_state_conf);
  }

  if(!check_Spacing(p))
    return s;

  s.IO_Ports = _calloc(s.IO, struct s_IO_port_conf);

  for(int i = 0; i < s.IO; i++){
    memcpy(&s.IO_Ports[i], *p, 2);
    *p += sizeof(struct s_IO_port_conf);
  }

  check_Spacing(p);

  return s;
}

struct station_conf read_s_station_conf(uint8_t ** p){
  struct station_conf s;

  memcpy(&s, *p, sizeof(struct s_station_conf));

  *p += sizeof(struct s_station_conf);

  s.name = _calloc(s.name_len, char);
  s.blocks = _calloc(s.nr_blocks, uint8_t);

  if(!check_Spacing(p))
    return s;

  memcpy(s.blocks, *p, s.nr_blocks);
  *p += s.nr_blocks;

  if(!check_Spacing(p))
    return s;

  memcpy(s.name, *p, s.name_len);
  *p += s.name_len;

  check_Spacing(p);

  return s;
}

struct signal_conf read_s_signal_conf(uint8_t ** p){
  struct signal_conf s;

  memcpy(&s, *p, sizeof(struct s_signal_conf));

  *p += sizeof(struct s_signal_conf);

  s.output = _calloc(s.output_len, struct s_IO_port_conf);
  s.stating = _calloc(s.output_len, struct s_IO_signal_event_conf);

  if(!check_Spacing(p))
    return s;

  memcpy(s.output, *p, s.output_len * sizeof(struct s_IO_port_conf));
  *p += s.output_len * sizeof(struct s_IO_port_conf);

  if(!check_Spacing(p))
    return s;

  memcpy(s.stating, *p, s.output_len * sizeof(struct s_IO_signal_event_conf));
  *p += s.output_len * sizeof(struct s_IO_signal_event_conf);

  check_Spacing(p);

  return s;
}


struct s_train_header_conf read_s_train_header_conf(uint8_t ** p){
  struct s_train_header_conf s;
  memcpy(&s, *p, sizeof(struct s_train_header_conf));

  *p += sizeof(struct s_train_header_conf);
  
  check_Spacing(p);

  return s;
}


struct cars_conf read_cars_conf(uint8_t ** p){
  struct cars_conf c;

  memcpy(&c, *p, sizeof(struct s_car_conf));

  *p += sizeof(struct s_car_conf);

  if(!check_Spacing(p))
    return c;

  c.name = _calloc(c.name_len+1, char);
  memcpy(c.name, *p, sizeof(char) * c.name_len);
  *p += c.name_len;
  if(!check_Spacing(p))
    return c;

  c.img_path = _calloc(c.img_path_len+1, char);
  memcpy(c.img_path, *p, sizeof(char) * c.img_path_len);
  *p += c.img_path_len;
  if(!check_Spacing(p))
    return c;

  c.icon_path = _calloc(c.icon_path_len+1, char);
  memcpy(c.icon_path, *p, sizeof(char) * c.icon_path_len);
  *p += c.icon_path_len;

  check_Spacing(p);

  return c;
}

struct engines_conf read_engines_conf(uint8_t ** p){
  struct engines_conf e;

  memcpy(&e, *p, sizeof(struct s_engine_conf));

  *p += sizeof(struct s_engine_conf);

  if(!check_Spacing(p))
    return e;

  e.name = _calloc(e.name_len+1, char);
  memcpy(e.name, *p, sizeof(char) * e.name_len);
  *p += e.name_len;
  if(!check_Spacing(p))
    return e;

  e.img_path = _calloc(e.img_path_len+1, char);
  memcpy(e.img_path, *p, sizeof(char) * e.img_path_len);
  *p += e.img_path_len;
  if(!check_Spacing(p))
    return e;

  e.icon_path = _calloc(e.icon_path_len+1, char);
  memcpy(e.icon_path, *p, sizeof(char) * e.icon_path_len);
  *p += e.icon_path_len;
  if(!check_Spacing(p))
    return e;

  e.speed_steps = _calloc(e.config_steps, sizeof(struct engine_speed_steps));
  memcpy(e.speed_steps, *p, sizeof(struct engine_speed_steps) * e.config_steps);
  *p += e.config_steps * sizeof(struct engine_speed_steps);

  check_Spacing(p);

  return e;
}

struct trains_conf read_trains_conf(uint8_t ** p){
  struct trains_conf t;

  memcpy(&t, *p, sizeof(struct s_train_conf));

  *p += sizeof(struct s_train_conf);

  if(!check_Spacing(p))
    return t;

  t.name = _calloc(t.name_len+1, char);
  memcpy(t.name, *p, sizeof(char) * t.name_len);
  *p += t.name_len;
  if(!check_Spacing(p))
    return t;

  t.composition = _calloc(t.nr_stock+1, sizeof(struct train_comp_ws));
  memcpy(t.composition, *p, sizeof(struct train_comp_ws) * t.nr_stock);
  *p += sizeof(struct train_comp_ws) * t.nr_stock;

  check_Spacing(p);

  return t;
}

struct cat_conf read_cat_conf(uint8_t ** p){
  struct cat_conf c;

  memcpy(&c, *p, sizeof(struct s_cat_conf));

  *p += sizeof(struct s_cat_conf);

  if(!check_Spacing(p))
    return c;

  c.name = _calloc(c.name_len+1, char);
  memcpy(c.name, *p, sizeof(char) * c.name_len);
  *p += c.name_len;

  check_Spacing(p);

  return c;
}