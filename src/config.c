#include <stdint.h>
#include "config.h"
#include "logger.h"
#include "mem.h"

uint8_t read_byte_conf(uint8_t ** p){
  uint8_t byte = **p;
  *p += 1;
  return byte;
}

int calc_write_size(struct config * config){
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


  //Stations
  for(int i = 0; i <  config->header.Stations; i++){
    size += sizeof(struct s_station_conf) + 1;
    size += config->Stations[i].name_len + 1;
    size += config->Stations[i].nr_blocks + 1;
  }

  return size;
}

void print_hex(char * data, int size){
  printf("print_hex:\n");
  for(int i = 0; i < size; i++){
    printf("%02x ", data[i]);
    if((i % 16) == 15)
      printf("\n");
  }
  printf("\n");
}

void write_from_conf(struct config * config, char * filename){
  printf("write_from_conf\n");
  int size = calc_write_size(config);

  char * data = calloc(size, 1);

  data[0] = 0x01;

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
      memcpy(p, &config->Switches[i].IO_Ports[j], 2);
      p += 2;
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

  //Copy Stations
  for(int i = 0; i < config->header.Stations; i++){
    memcpy(p, &config->Stations[i], sizeof(struct s_station_conf));

    p += sizeof(struct s_station_conf) + 1;

    memcpy(p, config->Stations[i].blocks, config->Stations[i].nr_blocks);
    p += config->Stations[i].nr_blocks + 1;

    memcpy(p, config->Stations[i].name, config->Stations[i].name_len);
    p += config->Stations[i].name_len + 1;
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
    *p += 2;
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
    *p += 2;
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
