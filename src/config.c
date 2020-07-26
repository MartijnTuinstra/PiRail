#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "logger.h"
#include "mem.h"

uint8_t read_byte_conf(uint8_t ** p){
  uint8_t byte = **p;
  *p += 1;
  return byte;
}


void print_hex(char * data, int size){
  // if(read_level() >= TRACE){
    printf("print_hex:\n");
    for(int i = 0; i < size; i++){
      printf("%02x ", data[i]);
      if((i % 16) == 15)
        printf("\n");
    }
    printf("\n");
  // }
}

int check_Spacing(uint8_t ** p){
  if(**p != 0){
    loggerf(CRITICAL, "Format missing sepperator, got %x", **p);
    return 0;
  }
  *p += 1;

  return 1;
}

struct node_conf read_s_node_conf(uint8_t ** p){
  loggerf(DEBUG, "Reading Node");
  struct node_conf n;
  memcpy(&n, *p, sizeof(struct s_node_conf));

  *p += sizeof(struct s_node_conf);
  
  check_Spacing(p);

  n.data = (uint8_t *)_calloc((n.size+1)/2, uint8_t);
  memcpy(n.data, *p, (n.size+1)/2);

  *p += (n.size+1)/2;
  
  check_Spacing(p);

  return n;
}

struct s_unit_conf read_s_unit_conf(uint8_t ** p){
  loggerf(DEBUG, "Reading Unit");
  struct s_unit_conf s;
  memcpy(&s, *p, sizeof(struct s_unit_conf));

  *p += sizeof(struct s_unit_conf);
  
  check_Spacing(p);

  return s;
}

struct s_block_conf read_s_block_conf(uint8_t ** p){
  loggerf(DEBUG, "Reading Block");
  struct s_block_conf s;
  memcpy(&s, *p, sizeof(struct s_block_conf));

  *p += sizeof(struct s_block_conf);

  check_Spacing(p);

  return s;
}

struct switch_conf read_s_switch_conf(uint8_t ** p){
  loggerf(DEBUG, "Reading Switch");
  struct switch_conf s;

  memcpy(&s, *p, sizeof(struct s_switch_conf));

  *p += sizeof(struct s_switch_conf);

  if(!check_Spacing(p))
    return s;

  s.IO_Ports = (struct s_IO_port_conf *)_calloc(s.IO & 0x0f, struct s_IO_port_conf);

  for(int i = 0; i < (s.IO & 0x0f); i++){
    memcpy(&s.IO_Ports[i], *p, 2);
    *p += sizeof(struct s_IO_port_conf);
  }

  check_Spacing(p);

  return s;
}

struct ms_switch_conf read_s_ms_switch_conf(uint8_t ** p){
  loggerf(DEBUG, "Reading MSSwitch");
  struct ms_switch_conf s;

  memcpy(&s, *p, sizeof(struct s_ms_switch_conf));

  *p += sizeof(struct s_ms_switch_conf);

  if(!check_Spacing(p))
    return s;

  s.states = (struct s_ms_switch_state_conf *)_calloc(s.nr_states, struct s_ms_switch_state_conf);

  for(int i = 0; i < s.nr_states; i++){
    memcpy(&s.states[i], *p, sizeof(struct s_ms_switch_state_conf));
    *p += sizeof(struct s_ms_switch_state_conf);
  }

  if(!check_Spacing(p))
    return s;

  s.IO_Ports = (struct s_IO_port_conf *)_calloc(s.IO, struct s_IO_port_conf);

  for(int i = 0; i < s.IO; i++){
    memcpy(&s.IO_Ports[i], *p, sizeof(struct s_IO_port_conf));
    *p += sizeof(struct s_IO_port_conf);
  }

  check_Spacing(p);

  return s;
}

struct station_conf read_s_station_conf(uint8_t ** p){
  loggerf(DEBUG, "Reading Station");
  struct station_conf s;

  memcpy(&s, *p, sizeof(struct s_station_conf));

  *p += sizeof(struct s_station_conf);

  s.name = (char *)_calloc(s.name_len, char);
  s.blocks = (uint8_t *)_calloc(s.nr_blocks, uint8_t);

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
  loggerf(DEBUG, "Reading Signal");
  struct signal_conf s;

  memcpy(&s, *p, sizeof(struct s_signal_conf));

  print_hex((char *)*p, sizeof(struct s_signal_conf));

  *p += sizeof(struct s_signal_conf);

  s.output = (struct s_IO_port_conf *)_calloc(s.output_len, struct s_IO_port_conf);
  s.stating = (struct s_IO_signal_event_conf *)_calloc(s.output_len, struct s_IO_signal_event_conf);
  s.Switches = (struct s_Signal_DependentSwitch *)_calloc(s.Switch_len, struct s_Signal_DependentSwitch);

  if(!check_Spacing(p))
    return s;

  memcpy(s.output, *p, s.output_len * sizeof(struct s_IO_port_conf));
  *p += s.output_len * sizeof(struct s_IO_port_conf);

  if(!check_Spacing(p))
    return s;

  memcpy(s.stating, *p, s.output_len * sizeof(struct s_IO_signal_event_conf));
  *p += s.output_len * sizeof(struct s_IO_signal_event_conf);

  if(!check_Spacing(p))
    return s;

  memcpy(s.Switches, *p, s.Switch_len * sizeof(struct s_Signal_DependentSwitch));
  *p += s.Switch_len * sizeof(struct s_Signal_DependentSwitch);

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

  c.name = (char *)_calloc(c.name_len+1, char);
  memcpy(c.name, *p, sizeof(char) * c.name_len);
  *p += c.name_len;
  if(!check_Spacing(p))
    return c;

  c.icon_path = (char *)_calloc(c.icon_path_len+1, char);
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

  e.name = (char *)_calloc(e.name_len+1, char);
  memcpy(e.name, *p, sizeof(char) * e.name_len);
  *p += e.name_len;
  if(!check_Spacing(p))
    return e;

  e.img_path = (char *)_calloc(e.img_path_len+1, char);
  memcpy(e.img_path, *p, sizeof(char) * e.img_path_len);
  *p += e.img_path_len;
  if(!check_Spacing(p))
    return e;

  e.icon_path = (char *)_calloc(e.icon_path_len+1, char);
  memcpy(e.icon_path, *p, sizeof(char) * e.icon_path_len);
  *p += e.icon_path_len;
  if(!check_Spacing(p))
    return e;

  e.speed_steps = (struct engine_speed_steps *)_calloc(e.config_steps, struct engine_speed_steps);
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

  t.name = (char *)_calloc(t.name_len+1, char);
  memcpy(t.name, *p, sizeof(char) * t.name_len);
  *p += t.name_len;
  if(!check_Spacing(p))
    return t;

  t.composition = (struct train_comp_ws *)_calloc(t.nr_stock+1, struct train_comp_ws);
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

  c.name = (char *)_calloc(c.name_len+1, char);
  memcpy(c.name, *p, sizeof(char) * c.name_len);
  *p += c.name_len;

  check_Spacing(p);

  return c;
}