#include <stdint.h>
#include <stdio.h>
#include "logger.h"
#include "config.h"
#include "mem.h"

void print_link(char * debug, struct s_link_conf link){
  if(link.type == RAIL_LINK_C){
    sprintf(debug, "%sC %2i:%2i  \t", debug, link.module, link.id);
  }
  else if(link.type == RAIL_LINK_E){
    sprintf(debug, "%sE        \t", debug);
  }
  else{
    sprintf(debug, "%s%2i:%2i:", debug, link.module, link.id);
    if(link.type == RAIL_LINK_R)
      sprintf(debug, "%s%c  \t", debug, 'R');
    else if(link.type == RAIL_LINK_S)
      sprintf(debug, "%s%c  \t", debug, 'S');
    else if(link.type == RAIL_LINK_s)
      sprintf(debug, "%s%c  \t", debug, 's');
    else if(link.type == RAIL_LINK_MA)
      sprintf(debug, "%s%s  \t", debug, "MB");
    else if(link.type == RAIL_LINK_MB)
      sprintf(debug, "%s%s  \t", debug, "MB");
    else if(link.type == RAIL_LINK_ma)
      sprintf(debug, "%s%s  \t", debug, "ma");
    else if(link.type == RAIL_LINK_mb)
      sprintf(debug, "%s%s  \t", debug, "mb");
    else if(link.type == RAIL_LINK_TT)
      sprintf(debug, "%s%c%c \t", debug, 'T', 'T');
    else
      sprintf(debug, "%s%i  \t", debug, link.type);
  }
}

void print_Node(struct s_node_conf node){
  printf("%i\t%i\n",
                node.Node,
                node.size);
}

void print_Block(struct s_block_conf block){
  const char * rail_types_string[4] = {
    "MAIN",
    "STATION",
    "SWITCHBLOCK",
    "TURNTABLE"
  };

  char debug[200];

  sprintf(debug, "%i\t%11s\t",
                block.id,
                rail_types_string[block.type]);
  print_link(debug, block.next);
  print_link(debug, block.prev);
  sprintf(debug, "%s%i\t%i\t%i\t%i\t%i\t%i:%i\t%i:%i",
                debug,
                block.speed,
                (block.fl & 0x6) >> 1,
                block.length,
                block.fl & 0x1,
                (block.fl & 0x8) >> 3,
                block.IO_In.Node, block.IO_In.Adr,
                block.IO_Out.Node, block.IO_Out.Adr);

  printf( "%s\n", debug);
}

void print_Switch(struct switch_conf Switch){
  char debug[200];

  sprintf(debug, "%i\t%i\t",
                Switch.id,
                Switch.det_block);
  print_link(debug, Switch.App);
  print_link(debug, Switch.Str);
  print_link(debug, Switch.Div);
  sprintf(debug, "%s%x\t%i %i",
                debug,
                Switch.IO,
                Switch.speed_Str, Switch.speed_Div);

  for(int j = 0; j < (Switch.IO & 0x0f); j++){
    sprintf(debug, "%s\t%i:%i", debug, Switch.IO_Ports[j].Node, Switch.IO_Ports[j].Adr);
  }

  printf( "%s\n", debug);
}

void print_MSSwitch(struct ms_switch_conf Switch){
  char debug[400];

  sprintf(debug, "%i\t%i\t%i\t%2i -> [%2i:%2i",
                Switch.id,
                Switch.det_block,
                Switch.nr_states,
                Switch.IO,
                Switch.IO_Ports[0].Node, Switch.IO_Ports[0].Adr);

  for(int i = 1; i < Switch.IO; i++){
    sprintf(debug, "%s, %2i:%2i", debug, Switch.IO_Ports[i].Node, Switch.IO_Ports[i].Adr);
  }
  sprintf(debug, "%s]\n", debug);

  for(int i = 0; i < Switch.nr_states; i++){
    sprintf(debug, "%s\t\t\t%2i >\t", debug, i);
    print_link(debug, Switch.states[i].sideA);
    print_link(debug, Switch.states[i].sideB);
    sprintf(debug, "%s%i\t%x\n", debug, Switch.states[i].speed, Switch.states[i].output_sequence);
  }

  printf( "%s", debug);
}

void print_Signals(struct signal_conf signal){
  char debug[400];

  sprintf(debug, "%i\t%i\t%i", signal.id, signal.blockId, signal.output_len);
  for(int i = 0;i < signal.output_len; i++){
    sprintf(debug, "%s\n\t\t\t%02i:%02i - ", debug, signal.output[i].Node, signal.output[i].Adr);
    for(int j = 0; j < 8; j++){
      sprintf(debug, "%s%i ", debug, signal.stating[i].event[j]);
    }
  }

  printf("%s\n", debug);
}

void print_Stations(struct station_conf stations){
  char debug[200];

  sprintf(debug, "%i\t%9s\t",
                stations.type,
                stations.name);

  for(int j = 0; j < stations.nr_blocks; j++){
    sprintf(debug, "%s%i ", debug, stations.blocks[j]);
  }

  printf( "%s\n", debug);
}

void print_Layout(struct module_config * config){
  printf("Length: %i\n", config->Layout_length);
  printf("Data:\n%s\n\n", config->Layout);
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
  char debug[200];

  sprintf(debug, "%-20s\t%i\t\t",
                train.name,
                train.nr_stock);

  for(int i = 0; i < train.nr_stock; i++){
    sprintf(debug, "%s%i:%i\t", debug, train.composition[i].type, train.composition[i].id);
  }

  printf( "%s\n", debug);
}

void print_Catagories(struct train_config * config){
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

void print_module_config(struct module_config * config){
  printf( "Modules:     %i\n", config->header.module);
  printf( "Connections: %i\n", config->header.connections);
  printf( "IO_Nodes:    %i\n", config->header.IO_Nodes);
  printf( "Blocks:      %i\n", config->header.Blocks);
  printf( "Switches:    %i\n", config->header.Switches);
  printf( "MSSwitches:  %i\n", config->header.MSSwitches);
  printf( "Signals:     %i\n", config->header.Signals);
  printf( "Stations:    %i\n", config->header.Stations);
  
  printf( "IO Nodes\n");
  printf( "id\tSize\n");
  for(int i = 0; i < config->header.IO_Nodes; i++){
    print_Node(config->Nodes[i]);
  }

  printf( "Block\n");
  printf( "id\ttype\t\tNext    \tPrev    \tMax_sp\tdir\tlen\tOneWay\tOut en\tIO_in\tIO_out\n");
  for(int i = 0; i < config->header.Blocks; i++){
    print_Block(config->Blocks[i]);
  }
  
  printf( "Switch\n");
  printf( "id\tblock\tApp       \tStr       \tDiv       \tIO\tSpeed\n");
  for(int i = 0; i < config->header.Switches; i++){
    print_Switch(config->Switches[i]);
  }

  printf( "MSSwitch\n");
  printf( "id\tblock\tstates\tIO\tSideA     \tSideB     \tSpeed\tSequence\t...\n");
  for(int i = 0; i < config->header.MSSwitches; i++){
    print_MSSwitch(config->MSSwitches[i]);
  }

  printf( "Signals\n");
  printf( "id\tBlockID\tOutput Length\t\tOutput states\n");
  for(int i = 0; i < config->header.Signals; i++){
    print_Signals(config->Signals[i]);
  }

  printf( "Station\n");
  printf( "type\tName\t\tblocks\n");
  for(int i = 0; i < config->header.Stations; i++){
    print_Stations(config->Stations[i]);
  }
}

void print_train_config(struct train_config * config){
  printf( "Cars:    %i\n", config->header.Cars);
  printf( "Engines: %i\n", config->header.Engines);
  printf( "Trains:  %i\n", config->header.Trains);

  printf("\nCatagories\n");
  print_Catagories(config);
  printf("\n\n");

  printf( "Cars\n");
  printf( "id\tNr\tType\tSpeed\tLength\tName\t\t\tIcon_path\n");
  for(int i = 0; i < config->header.Cars; i++){
    printf("%i\t", i);
    print_Cars(config->Cars[i]);
  }
  
  printf( "Engines\n");
  printf( "id\tDCC\tSteps\tType\tLength\tName\t\t\tImg_path\t\tIcon_path\n");
  for(int i = 0; i < config->header.Engines; i++){
    printf("%i\t", i);
    print_Engines(config->Engines[i]);
  }

  printf( "Trains\n");
  printf( "id\tName\t\t\tRolling Stock\t...\n");
  for(int i = 0; i < config->header.Trains; i++){
    printf("%i\t", i);
    print_Trains(config->Trains[i]);
  }
}

int read_module_config(struct module_config * config, FILE * fp){


  char * header = _calloc(2, char);

  fread(header, 1, 1, fp);

  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char * buffer = _calloc(fsize + 10, char);
  char * buffer_start = &buffer[0];
  fread(buffer, fsize, 1, fp);

  uint8_t ** buf_ptr = (uint8_t **)&buffer;

  *buf_ptr += 1;

  config->header = read_s_unit_conf(buf_ptr);

  if (header[0] != MODULE_CONF_VERSION) {
    loggerf(WARNING, "Module %i not correct version", config->header.module);
    return -1;
  }

  config->Nodes = _calloc(config->header.IO_Nodes, struct s_node_conf);
  
  for(int i = 0; i < config->header.IO_Nodes; i++){
    config->Nodes[i]  = read_s_node_conf(buf_ptr);
  }

  config->Blocks = _calloc(config->header.Blocks, struct s_block_conf);

  
  for(int i = 0; i < config->header.Blocks; i++){
    config->Blocks[i]  = read_s_block_conf(buf_ptr);
  }

  config->Switches = _calloc(config->header.Switches, struct switch_conf);

  for(int i = 0; i < config->header.Switches; i++){
    config->Switches[i]  = read_s_switch_conf(buf_ptr);
  }

  config->MSSwitches = _calloc(config->header.MSSwitches, struct ms_switch_conf);

  for(int i = 0; i < config->header.MSSwitches; i++){
    config->MSSwitches[i]  = read_s_ms_switch_conf(buf_ptr);
  }

  config->Signals = _calloc(config->header.Signals, struct signal_conf);

  for(int i = 0; i < config->header.Signals; i++){
    config->Signals[i]  = read_s_signal_conf(buf_ptr);
  }

  config->Stations = _calloc(config->header.Stations, struct station_conf);

  for(int i = 0; i < config->header.Stations; i++){
    config->Stations[i]  = read_s_station_conf(buf_ptr);
  }

  //Layout
  memcpy(&config->Layout_length, *buf_ptr, sizeof(uint16_t));
  *buf_ptr += sizeof(uint16_t) + 1;

  config->Layout = _calloc(config->Layout_length + 1, uint8_t);
  memcpy(config->Layout, *buf_ptr, config->Layout_length);

  _free(header);
  _free(buffer_start);

  return 1;
}

int read_train_config(struct train_config * config, FILE * fp){
  char * header = _calloc(2, char);

  fread(header, 1, 1, fp);

  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char * buffer = _calloc(fsize, char);
  char * buffer_start = &buffer[0];
  fread(buffer, fsize, 1, fp);

  uint8_t ** buf_ptr = (uint8_t **)&buffer;

  *buf_ptr += 1;

  config->header = read_s_train_header_conf(buf_ptr);

  if (header[0] != TRAIN_CONF_VERSION) {
    loggerf(ERROR, "Not correct version");
    return -1;
    // config->header.IO_Nodes = 0;
    // loggerf(WARNING, "Please re-save to update", config->header.module);
  }

  config->P_Cat = _calloc(config->header.P_Catagories, struct cat_conf);
  config->C_Cat = _calloc(config->header.C_Catagories, struct cat_conf);
  config->Engines = _calloc(config->header.Engines, struct engines_conf);
  config->Cars = _calloc(config->header.Cars, struct cars_conf);
  config->Trains = _calloc(config->header.Trains, struct trains_conf);
  
  for(int i = 0; i < config->header.P_Catagories; i++){
    config->P_Cat[i]  = read_cat_conf(buf_ptr);
  }
  
  for(int i = 0; i < config->header.C_Catagories; i++){
    config->C_Cat[i]  = read_cat_conf(buf_ptr);
  }
  
  for(int i = 0; i < config->header.Engines; i++){
    config->Engines[i]  = read_engines_conf(buf_ptr);
  }
  
  for(int i = 0; i < config->header.Cars; i++){
    config->Cars[i]  = read_cars_conf(buf_ptr);
  }
  
  for(int i = 0; i < config->header.Trains; i++){
    config->Trains[i]  = read_trains_conf(buf_ptr);
  }

  _free(header);
  _free(buffer_start);

  return 1;
}

void modify_Node(struct module_config * config, char cmd){
  int id;
  char _cmd[20];
  int tmp;
  if(cmd == 'e'){
    printf("Node ID: ");
    fgets(_cmd, 20, stdin);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;
    printf("Editing Node %i\n", id);
    // print_Block(config->Blocks[id]);
  }
  else if(cmd == 'a'){
    printf("Node ID: (%i)\n", config->header.IO_Nodes);
    fgets(_cmd, 20, stdin); // Clear stdin

    if(config->header.IO_Nodes == 0){
      printf("Calloc");
      config->Nodes = _calloc(1, struct s_node_conf);
    }
    else{
      printf("Realloc");
      config->Nodes = _realloc(config->Nodes, config->header.IO_Nodes+1, struct s_node_conf);
    }
    memset(&config->Nodes[config->header.IO_Nodes], 0, sizeof(struct s_node_conf));
    config->Nodes[config->header.IO_Nodes].Node = config->header.IO_Nodes;
    id = config->header.IO_Nodes++;
  }
  else if(cmd == 'r'){
    printf("Remove Node ID: ");
    fgets(_cmd, 20, stdin);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;

    if(id == (config->header.IO_Nodes - 1) && id >= 0){
      memset(&config->Nodes[config->header.IO_Nodes - 1], 0, sizeof(struct s_node_conf));
      config->Nodes = _realloc(config->Nodes, --config->header.IO_Nodes, struct s_node_conf);
    }
    else{
      printf("Only last block can be removed\n");
    }
  }


  if(cmd == 'e' || cmd == 'a'){
    printf("Node Size      (%i)         | ", config->Nodes[id].size);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->Nodes[id].size = tmp;

    printf("New:      \t");
    print_Node(config->Nodes[id]);
    // struct s_block_conf tmp;
    // int dir, oneway, Out_en;
    // scanf("%i\t%i\t%2i:%2i:%c\t\t%2i:%2i:%c\t\t%i\t%i\t%i\t%i\t%i\t%i:%i\t%i:%i",
    //   &tmp.id, &tmp.type,
    //   &tmp.next.module, &tmp.next.id, &tmp.next.type,
    //   &tmp.prev.module, &tmp.prev.id, &tmp.prev.type,
    //   &tmp.speed,
    //   &dir,
    //   &tmp.length,
    //   &oneway,
    //   &Out_en,
    //   &tmp.IO_In.Node, &tmp.IO_In.Adr,
    //   &tmp.IO_Out.Node, &tmp.IO_Out.Adr);

  }
}

void modify_Block(struct module_config * config, char cmd){
  int id;
  char _cmd[20];
  int tmp, tmp1, tmp2;
  char tmpc;
  if(cmd == 'e'){
    printf("Block ID: ");
    fgets(_cmd, 20, stdin);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;
    printf("Editing block %i\n", id);
    // print_Block(config->Blocks[id]);
  }
  else if(cmd == 'a'){
    printf("Block ID: (%i)\n", config->header.Blocks);
    fgets(_cmd, 20, stdin); // Clear stdin

    if(config->header.Blocks == 0){
      printf("Calloc");
      config->Blocks = _calloc(1, struct s_block_conf);
    }
    else{
      printf("Realloc");
      config->Blocks = _realloc(config->Blocks, config->header.Blocks+1, struct s_block_conf);
    }
    memset(&config->Blocks[config->header.Blocks], 0, sizeof(struct s_block_conf));
    config->Blocks[config->header.Blocks].id = config->header.Blocks;
    id = config->header.Blocks++;
  }
  else if(cmd == 'r'){
    printf("Remove Block ID: ");
    fgets(_cmd, 20, stdin);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;

    if(id == (config->header.Blocks - 1) && id >= 0){
      memset(&config->Blocks[config->header.Blocks - 1], 0, sizeof(struct s_block_conf));
      config->Blocks = _realloc(config->Blocks, --config->header.Blocks, struct s_block_conf);
    }
    else{
      printf("Only last block can be removed\n");
    }
  }


  if(cmd == 'e' || cmd == 'a'){
    printf("Block Type      (%i)         | ", config->Blocks[id].type);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->Blocks[id].type = tmp;

    printf("Block Link Next (%2i:%2i:%2x)  | ", config->Blocks[id].next.module, config->Blocks[id].next.id, config->Blocks[id].next.type);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i%*c%i%*c%i", &tmp, &tmp1, &tmp2) > 2){
      config->Blocks[id].next.module = tmp;
      config->Blocks[id].next.id = tmp1;
      config->Blocks[id].next.type = tmp2;
    }

    printf("Block Link Prev (%2i:%2i:%2x)  | ", config->Blocks[id].prev.module, config->Blocks[id].prev.id, config->Blocks[id].prev.type);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i%*c%i%*c%i", &tmp, &tmp1, &tmp2) > 2){
      config->Blocks[id].prev.module = tmp;
      config->Blocks[id].prev.id = tmp1;
      config->Blocks[id].prev.type = tmp2;
    }

    printf("Block Speed     (%3i)       | ", config->Blocks[id].speed);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->Blocks[id].speed = tmp;

    printf("Block Length    (%3i)       | ", config->Blocks[id].length);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->Blocks[id].length = tmp;

    printf("Block Oneway    (");
    if(config->Blocks[id].fl & 1)
      printf("Y)         | ");
    else
      printf("N)         | ");
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%c", &tmpc) > 0){
      if(tmpc == 'Y' || tmpc == 'y')
        config->Blocks[id].fl |= 0x1;
      else
        config->Blocks[id].fl &= ~0x1;
    }

    printf("Block Direction (%i)         | ", (config->Blocks[id].fl & 0x6) >> 1);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->Blocks[id].fl = ((tmp & 0x3) << 1) + (~0x6 & config->Blocks[id].fl);

    printf("Block IO out en (");
    if(config->Blocks[id].fl & 8)
      printf("Y)         | ");
    else
      printf("N)         | ");
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%c", &tmpc) > 0){
      if(tmpc == 'Y' || tmpc == 'y')
        config->Blocks[id].fl |= 0x8;
      else
        config->Blocks[id].fl &= ~0x8;
    }

    printf("Block IO In     (%2i:%2i)      | ", config->Blocks[id].IO_In.Node, config->Blocks[id].IO_In.Adr);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i%*c%i", &tmp, &tmp1) > 0){
      config->Blocks[id].IO_In.Node = tmp;
      config->Blocks[id].IO_In.Adr = tmp1;
    }

    if(config->Blocks[id].fl & 0x8){
      printf("Block IO Out    (%2i:%2i)       | ", config->Blocks[id].IO_Out.Node, config->Blocks[id].IO_Out.Adr);
      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%i%*c%i", &tmp, &tmp1) > 0){
        config->Blocks[id].IO_Out.Node = tmp;
        config->Blocks[id].IO_Out.Adr = tmp1;
      }
    }
    printf("New:      \t");
    print_Block(config->Blocks[id]);
    // struct s_block_conf tmp;
    // int dir, oneway, Out_en;
    // scanf("%i\t%i\t%2i:%2i:%c\t\t%2i:%2i:%c\t\t%i\t%i\t%i\t%i\t%i\t%i:%i\t%i:%i",
    //   &tmp.id, &tmp.type,
    //   &tmp.next.module, &tmp.next.id, &tmp.next.type,
    //   &tmp.prev.module, &tmp.prev.id, &tmp.prev.type,
    //   &tmp.speed,
    //   &dir,
    //   &tmp.length,
    //   &oneway,
    //   &Out_en,
    //   &tmp.IO_In.Node, &tmp.IO_In.Adr,
    //   &tmp.IO_Out.Node, &tmp.IO_Out.Adr);

  }
}

void modify_Switch(struct module_config * config, char cmd){
  int id;
  char _cmd[20];
  int tmp, tmp1, tmp2;
  if(cmd == 'e'){
    printf("Switch ID: ");
    fgets(_cmd, 20, stdin);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;
    printf("Editing Switch %i\n", id);
    // print_Block(config->Blocks[id]);
  }
  else if(cmd == 'a'){
    printf("Switch ID: (%i)\n", config->header.Switches);
    fgets(_cmd, 20, stdin); // Clear stdin

    if(config->header.Switches == 0){
      printf("Calloc");
      config->Switches = _calloc(1, struct switch_conf);
    }
    else{
      printf("Realloc");
      config->Switches = _realloc(config->Switches, config->header.Switches+1, struct switch_conf);
    }
    memset(&config->Switches[config->header.Switches], 0, sizeof(struct switch_conf));
    config->Switches[config->header.Switches].id = config->header.Switches;
    id = config->header.Switches++;
  }
  else if(cmd == 'r'){
    printf("Remove Switch ID: ");
    fgets(_cmd, 20, stdin);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;

    if(id == (config->header.Switches - 1) && id >= 0){
      memset(&config->Switches[config->header.Switches - 1], 0, sizeof(struct switch_conf));
      config->Switches = _realloc(config->Switches, --config->header.Switches, struct switch_conf);
    }
    else{
      printf("Only last block can be removed\n");
    }
  }


  if(cmd == 'e' || cmd == 'a'){
    printf("Switch Detblock (%i)         | ", config->Switches[id].det_block);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->Switches[id].det_block = tmp;

    printf("Switch Link App (%2i:%2i:%2x)  | ", config->Switches[id].App.module, config->Switches[id].App.id, config->Switches[id].App.type);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i%*c%i%*c%i", &tmp, &tmp1, &tmp2) > 2){
      config->Switches[id].App.module = tmp;
      config->Switches[id].App.id = tmp1;
      config->Switches[id].App.type = tmp2;
    }

    printf("Switch Link Str (%2i:%2i:%2x)  | ", config->Switches[id].Str.module, config->Switches[id].Str.id, config->Switches[id].Str.type);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i%*c%i%*c%i", &tmp, &tmp1, &tmp2) > 2){
      config->Switches[id].Str.module = tmp;
      config->Switches[id].Str.id = tmp1;
      config->Switches[id].Str.type = tmp2;
    }

    printf("Switch Link Div (%2i:%2i:%2x)  | ", config->Switches[id].Div.module, config->Switches[id].Div.id, config->Switches[id].Div.type);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i%*c%i%*c%i", &tmp, &tmp1, &tmp2) > 2){
      config->Switches[id].Div.module = tmp;
      config->Switches[id].Div.id = tmp1;
      config->Switches[id].Div.type = tmp2;
    }

    printf("Switch IO_Ports  (%2i)       | ", config->Switches[id].IO & 0x0f);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->Switches[id].IO = (tmp & 0x0f) + (config->Switches[id].IO & 0xf0);

    printf("Switch IO Type   (%i)        | ", config->Switches[id].IO >> 4);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->Switches[id].IO = (tmp & 0xf0) + (config->Switches[id].IO & 0x0f);;

    for(int i = 0; i < (config->Switches[id].IO & 0x0f); i++){
      printf("Switch IO%2i  (%2i:%2i)        | ", i, config->Switches[id].IO_Ports[i].Node, config->Switches[id].IO_Ports[i].Adr);
      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%i%*c%i", &tmp, &tmp1) > 0){
        config->Switches[id].IO_Ports[i].Node = tmp;
        config->Switches[id].IO_Ports[i].Adr = tmp1;
      }
    }
    printf("New:      \t");
    print_Switch(config->Switches[id]);
    // struct s_block_conf tmp;
    // int dir, oneway, Out_en;
    // scanf("%i\t%i\t%2i:%2i:%c\t\t%2i:%2i:%c\t\t%i\t%i\t%i\t%i\t%i\t%i:%i\t%i:%i",
    //   &tmp.id, &tmp.type,
    //   &tmp.next.module, &tmp.next.id, &tmp.next.type,
    //   &tmp.prev.module, &tmp.prev.id, &tmp.prev.type,
    //   &tmp.speed,
    //   &dir,
    //   &tmp.length,
    //   &oneway,
    //   &Out_en,
    //   &tmp.IO_In.Node, &tmp.IO_In.Adr,
    //   &tmp.IO_Out.Node, &tmp.IO_Out.Adr);

  }
}

void modify_MSSwitch(struct module_config * config, char cmd){
  int id;
  char _cmd[20];
  int tmp, tmp1, tmp2;
  if(cmd == 'e'){
    printf("MSSwitch ID: ");
    fgets(_cmd, 20, stdin);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1 || id >= config->header.MSSwitches || id < 0){
      printf("Invalid\n");
      return;
    }
    printf("Editing MSSwitch %i\n", id);
    // print_Block(config->Blocks[id]);
  }
  else if(cmd == 'a'){
    printf("MSSwitch ID: (%i)\n", config->header.MSSwitches);
    fgets(_cmd, 20, stdin); // Clear stdin

    if(config->header.MSSwitches == 0){
      printf("Calloc");
      config->MSSwitches = _calloc(1, struct ms_switch_conf);
    }
    else{
      printf("Realloc");
      config->MSSwitches = _realloc(config->MSSwitches, config->header.MSSwitches+1, struct ms_switch_conf);
    }
    memset(&config->MSSwitches[config->header.MSSwitches], 0, sizeof(struct ms_switch_conf));
    id = config->header.MSSwitches++;

    //Set child pointers
    config->MSSwitches[id].nr_states = 1;
    config->MSSwitches[id].states = _calloc(1, struct s_ms_switch_state_conf);
    config->MSSwitches[id].IO = 1;
    config->MSSwitches[id].IO_Ports = _calloc(1, struct s_IO_port_conf);
  }
  else if(cmd == 'r'){
    printf("Remove Switch ID: ");
    fgets(_cmd, 20, stdin);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;

    if(id == (config->header.MSSwitches - 1) && id >= 0){
      memset(&config->MSSwitches[config->header.MSSwitches - 1], 0, sizeof(struct ms_switch_conf));
      config->MSSwitches = _realloc(config->MSSwitches, --config->header.MSSwitches, struct ms_switch_conf);
    }
    else{
      printf("Only last block can be removed\n");
    }
  }


  if(cmd == 'e' || cmd == 'a'){
    printf("MSSwitch Detblock (%i)         | ", config->MSSwitches[id].det_block);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->MSSwitches[id].det_block = tmp;

    printf("MSSwitch Nr States (%2i)      | ", config->MSSwitches[id].nr_states);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      config->MSSwitches[id].nr_states = tmp;
      config->MSSwitches[id].states = _realloc(config->MSSwitches[id].states, tmp, struct s_ms_switch_state_conf);
    }

    printf("MSSwitch nr IO Ports  (%2i)    | ", config->MSSwitches[id].IO);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      if(tmp <= 16){
        config->MSSwitches[id].IO = tmp;
        config->MSSwitches[id].IO_Ports = _realloc(config->MSSwitches[id].IO_Ports, tmp, struct s_IO_port_conf);
      }
      else{
        printf("Invalid length\n");
      }
    }

    for(int i = 0; i < config->MSSwitches[id].nr_states; i++){
      printf("MSSwitch State %2i\n", i);
      
      printf(" - Link A (%2i:%2i:%2x)         | ",
                config->MSSwitches[id].states[i].sideA.module,
                config->MSSwitches[id].states[i].sideA.id,
                config->MSSwitches[id].states[i].sideA.type);

      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%i%*c%i%*c%i", &tmp, &tmp1, &tmp2) > 2){
        config->MSSwitches[id].states[i].sideA.module = tmp;
        config->MSSwitches[id].states[i].sideA.id = tmp1;
        config->MSSwitches[id].states[i].sideA.type = tmp2;
      }
      
      printf(" - Link B (%2i:%2i:%2x)         | ",
                config->MSSwitches[id].states[i].sideB.module,
                config->MSSwitches[id].states[i].sideB.id,
                config->MSSwitches[id].states[i].sideB.type);

      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%i%*c%i%*c%i", &tmp, &tmp1, &tmp2) > 2){
        config->MSSwitches[id].states[i].sideB.module = tmp;
        config->MSSwitches[id].states[i].sideB.id = tmp1;
        config->MSSwitches[id].states[i].sideB.type = tmp2;
      }
      
      printf(" - Speed (%3i)              | ",
                config->MSSwitches[id].states[i].speed);

      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%i", &tmp) > 0){
        config->MSSwitches[id].states[i].speed = tmp;
      }
      
      printf(" - State output (0x%4x)    | 0x",
                config->MSSwitches[id].states[i].output_sequence);

      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%x", &tmp) > 0){
        config->MSSwitches[id].states[i].output_sequence = tmp;
      }
    }

    for(int i = 0; i < config->MSSwitches[id].IO; i++){
      printf("MSSwitch IO %2i - Adr (%2i:%2i)         | ",
                i,
                config->MSSwitches[id].IO_Ports[i].Node,
                config->MSSwitches[id].IO_Ports[i].Adr);

      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%i%*c%i%*c%i", &tmp, &tmp1, &tmp2) > 1){
        config->MSSwitches[id].IO_Ports[i].Node = tmp;
        config->MSSwitches[id].IO_Ports[i].Adr = tmp1;
      }
    }
    printf("New:      \t");
    print_MSSwitch(config->MSSwitches[id]);
    // struct s_block_conf tmp;
    // int dir, oneway, Out_en;
    // scanf("%i\t%i\t%2i:%2i:%c\t\t%2i:%2i:%c\t\t%i\t%i\t%i\t%i\t%i\t%i:%i\t%i:%i",
    //   &tmp.id, &tmp.type,
    //   &tmp.next.module, &tmp.next.id, &tmp.next.type,
    //   &tmp.prev.module, &tmp.prev.id, &tmp.prev.type,
    //   &tmp.speed,
    //   &dir,
    //   &tmp.length,
    //   &oneway,
    //   &Out_en,
    //   &tmp.IO_In.Node, &tmp.IO_In.Adr,
    //   &tmp.IO_Out.Node, &tmp.IO_Out.Adr);

  }
}

void modify_Signal(struct module_config * config, char cmd){
  int id;
  char _cmd[20];
  int tmp, tmp1;//, tmp2;
  if(cmd == 'e'){
    printf("Signal ID: ");
    fgets(_cmd, 20, stdin);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;
    printf("Editing Signal %i\n", id);
    // print_Block(config->Blocks[id]);
  }
  else if(cmd == 'a'){
    printf("Signal ID: (%i)\n", config->header.Signals);
    fgets(_cmd, 20, stdin); // Clear stdin

    if(config->header.Signals == 0){
      printf("Calloc");
      config->Signals = _calloc(1, struct signal_conf);
    }
    else{
      printf("Realloc");
      config->Signals = _realloc(config->Signals, config->header.Signals+1, struct signal_conf);
    }
    memset(&config->Signals[config->header.Signals], 0, sizeof(struct signal_conf));
    id = config->header.Signals++;

    //Set child pointers
    config->Signals[id].id = id;
    config->Signals[id].output_len = 1;
    config->Signals[id].output = _calloc(1, struct s_IO_port_conf);
    config->Signals[id].stating = _calloc(1, struct s_IO_signal_event_conf);
  }
  else if(cmd == 'r'){
    printf("Remove Signal ID: ");
    fgets(_cmd, 20, stdin);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;

    if(id == (config->header.Signals - 1) && id >= 0){
      _free(config->Signals[id].output);
      _free(config->Signals[id].stating);

      memset(&config->Signals[config->header.Signals - 1], 0, sizeof(struct signal_conf));
      config->Signals = _realloc(config->Signals, --config->header.Signals, struct signal_conf);
    }
    else{
      printf("Only last block can be removed\n");
    }
  }


  if(cmd == 'e' || cmd == 'a'){
    printf("Signals Block id (%i)| ", config->Signals[id].blockId);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->Signals[id].blockId = tmp;
    
    
    if(config->Signals[id].side == 1) // NEXT
      printf("Signals Block side (N)| ");
    else
      printf("Signals Block side (P)| ");
    
    fgets(_cmd, 20, stdin);
    char tmp_char;
    if(sscanf(_cmd, "%c", &tmp_char) > 0){
      printf("Got %i\n", tmp_char);
      if(tmp_char == 'N')
        config->Signals[id].side = 1; // NEXT
      else if(tmp_char == 'P')
        config->Signals[id].side = 0; // PREV
    }
    
    printf("Signals Outputs (%i) | ", config->Signals[id].output_len);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      config->Signals[id].output = _realloc(config->Signals[id].output, tmp, struct s_IO_port_conf);
      config->Signals[id].stating = _realloc(config->Signals[id].stating, tmp, struct s_IO_signal_event_conf);
      config->Signals[id].output_len = tmp;
    }

    for(int i = 0; i < config->Signals[id].output_len; i++){
      printf(" - IO-Address: (%02i:%02i) | ", config->Signals[id].output[i].Node, config->Signals[id].output[i].Adr);
      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%i%*c%i", &tmp, &tmp1) > 0){
        config->Signals[id].output[i].Node = tmp;
        config->Signals[id].output[i].Adr = tmp1;
      }
      for(int j = 0; j < 8; j++){
        printf("   IO-Event %i: (%i)    | ", j, config->Signals[id].stating[i].event[j]);
        fgets(_cmd, 20, stdin);
        if(sscanf(_cmd, "%i", &tmp) > 0){
          config->Signals[id].stating[i].event[j] = tmp;
        }
      }
    }

    printf("New:      \t");
    print_Signals(config->Signals[id]);
    // struct s_block_conf tmp;
    // int dir, oneway, Out_en;
    // scanf("%i\t%i\t%2i:%2i:%c\t\t%2i:%2i:%c\t\t%i\t%i\t%i\t%i\t%i\t%i:%i\t%i:%i",
    //   &tmp.id, &tmp.type,
    //   &tmp.next.module, &tmp.next.id, &tmp.next.type,
    //   &tmp.prev.module, &tmp.prev.id, &tmp.prev.type,
    //   &tmp.speed,
    //   &dir,
    //   &tmp.length,
    //   &oneway,
    //   &Out_en,
    //   &tmp.IO_In.Node, &tmp.IO_In.Adr,
    //   &tmp.IO_Out.Node, &tmp.IO_Out.Adr);

  }
}

void modify_Station(struct module_config * config, char cmd){
  int id;
  char _cmd[20];
  int tmp;//, tmp1, tmp2;
  if(cmd == 'e'){
    printf("Station ID: ");
    fgets(_cmd, 20, stdin);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;
    printf("Editing Station %i\n", id);
    // print_Block(config->Blocks[id]);
  }
  else if(cmd == 'a'){
    printf("Station ID: (%i)\n", config->header.Stations);
    fgets(_cmd, 20, stdin); // Clear stdin

    if(config->header.Stations == 0){
      printf("Calloc");
      config->Stations = _calloc(1, struct station_conf);
    }
    else{
      printf("Realloc");
      config->Stations = _realloc(config->Stations, config->header.Stations+1, struct station_conf);
    }
    memset(&config->Stations[config->header.Stations], 0, sizeof(struct station_conf));
    id = config->header.Stations++;

    //Set child pointers
    config->Stations[id].name_len = 1;
    config->Stations[id].name = _calloc(1, char);
    config->Stations[id].nr_blocks = 1;
    config->Stations[id].blocks = _calloc(1, uint8_t);
  }
  else if(cmd == 'r'){
    printf("Remove Station ID: ");
    fgets(_cmd, 20, stdin);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;

    if(id == (config->header.Stations - 1) && id >= 0){
      memset(&config->Stations[config->header.Stations - 1], 0, sizeof(struct station_conf));
      config->Stations = _realloc(config->Stations, --config->header.Stations, struct station_conf);
    }
    else{
      printf("Only last block can be removed\n");
    }
  }


  if(cmd == 'e' || cmd == 'a'){
    printf("Station Type (%i)         | ", config->Stations[id].type);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->Stations[id].type = tmp;

    printf("Station Name (%s)\n\t\t\t | ", config->Stations[id].name);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%s", _cmd) > 0){
      tmp = strlen(_cmd);
      config->Stations[id].name = _realloc(config->Stations[id].name, tmp, char);
      memcpy(config->Stations[id].name, _cmd, tmp);
      config->Stations[id].name[tmp] = 0;
      config->Stations[id].name_len = tmp;
    }

    printf("Station blocks nr (%2i)   | ", config->Stations[id].nr_blocks);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      config->Stations[id].nr_blocks = tmp;
      config->Stations[id].blocks = _realloc(config->Stations[id].blocks, tmp, uint8_t);
    }

    for(int i = 0; i < config->Stations[id].nr_blocks; i++){
      printf("Station Block %2i (%2i)    | ", i, config->Stations[id].blocks[i]);
      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%i", &tmp) > 0){
        config->Stations[id].blocks[i] = tmp;
      }
    }

    printf("New:      \t");
    print_Stations(config->Stations[id]);
    // struct s_block_conf tmp;
    // int dir, oneway, Out_en;
    // scanf("%i\t%i\t%2i:%2i:%c\t\t%2i:%2i:%c\t\t%i\t%i\t%i\t%i\t%i\t%i:%i\t%i:%i",
    //   &tmp.id, &tmp.type,
    //   &tmp.next.module, &tmp.next.id, &tmp.next.type,
    //   &tmp.prev.module, &tmp.prev.id, &tmp.prev.type,
    //   &tmp.speed,
    //   &dir,
    //   &tmp.length,
    //   &oneway,
    //   &Out_en,
    //   &tmp.IO_In.Node, &tmp.IO_In.Adr,
    //   &tmp.IO_Out.Node, &tmp.IO_Out.Adr);

  }
}

void export_Layout(struct module_config * config){
  FILE * fp;
  /* open the file for writing*/
  fp = fopen ("Layout_export.txt","w");

  /* write 10 lines of text into the file stream*/
  fprintf(fp, "%s", config->Layout);

  /* close the file*/  
  fclose (fp);
}

void import_Layout(struct module_config * config){
  FILE * fp;
  char src[80];

  fgets(src, 2, stdin);
  printf("Location: ");
  fgets(src, 80, stdin);
  sscanf(src, "%s", src);
  /* open the file for reading*/
  fp = fopen (src,"r");

  if(!fp){
    loggerf(ERROR, "File not found '%s'", src);
    return;
  }

  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  config->Layout = _realloc(config->Layout, fsize, uint8_t);

  fread(config->Layout, fsize, 1, fp);
  // config->Layout[fsize] = 0;

  config->Layout_length = fsize;

  /* close the file*/  
  fclose (fp);
}

void modify_Car(struct train_config * config, char cmd){
  int id;
  char _cmd[80];
  int tmp;//, tmp1, tmp2;
  if(cmd == 'e'){
    printf("Car ID: ");
    fgets(_cmd, 80, stdin);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;
    printf("Editing Car %i\n", id);
  }
  else if(cmd == 'a'){
    printf("Car ID: (%i)\n", config->header.Cars);
    fgets(_cmd, 80, stdin); // Clear stdin

    if(config->header.Cars == 0){
      printf("Calloc");
      config->Cars = _calloc(1, struct cars_conf);
    }
    else{
      printf("Realloc");
      config->Cars = _realloc(config->Cars, config->header.Cars+1, struct cars_conf);
    }
    memset(&config->Cars[config->header.Cars], 0, sizeof(struct cars_conf));
    id = config->header.Cars++;

    //Set child pointers
    config->Cars[id].name_len = 1;
    config->Cars[id].name = _calloc(1, char);
    config->Cars[id].icon_path_len = 1;
    config->Cars[id].icon_path = _calloc(1, char);
  }
  else if(cmd == 'r'){
    printf("Remove Station ID: ");
    fgets(_cmd, 80, stdin);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;

    if(id == (config->header.Cars - 1) && id >= 0){
      _free(config->Cars[config->header.Cars - 1].name);
      _free(config->Cars[config->header.Cars - 1].icon_path);

      memset(&config->Cars[config->header.Cars - 1], 0, sizeof(struct cars_conf));
      config->Cars = _realloc(config->Cars, --config->header.Cars, struct cars_conf);
    }
    else{
      printf("Only last block can be removed\n");
    }
  }


  if(cmd == 'e' || cmd == 'a'){
    printf("Reference Number (%i)         | ", config->Cars[id].nr);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->Cars[id].nr = tmp;

    printf("Speedsteps  (%x)   | ", config->Cars[id].type >> 4);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      printf("(%i << 4) & (%i & 0x0f)\n", tmp & 0x0f, config->Cars[id].type);
      config->Cars[id].type = ((tmp & 0x0f) << 4) | (config->Cars[id].type & 0x0f);
    }

    printf("Type  (%x)   | ", config->Cars[id].type & 0x0f);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      config->Cars[id].type = (tmp & 0x0f) | (config->Cars[id].type & 0xf0);
    }

    printf("Maximum speed    (%i)         | ", config->Cars[id].max_speed);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->Cars[id].max_speed = tmp;

    printf("Name      (%s) | ", config->Cars[id].name);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%s", _cmd) > 0){
      tmp = strlen(_cmd);
      config->Cars[id].name = _realloc(config->Cars[id].name, tmp, char);
      memcpy(config->Cars[id].name, _cmd, tmp);
      config->Cars[id].name[tmp] = 0;
      config->Cars[id].name_len = tmp;
    }

    printf("Icon path (%s) | ", config->Cars[id].icon_path);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%s", _cmd) > 0){
      tmp = strlen(_cmd);
      config->Cars[id].icon_path = _realloc(config->Cars[id].icon_path, tmp, char);
      memcpy(config->Cars[id].icon_path, _cmd, tmp);
      config->Cars[id].icon_path[tmp] = 0;
      config->Cars[id].icon_path_len = tmp;
    }

    printf("Length  (%2i)   | ", config->Cars[id].length);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      config->Cars[id].length = tmp;
    }

  //   printf("New:      \t");
  //   print_Stations(config->Stations[id]);
  //   // struct s_block_conf tmp;
  //   // int dir, oneway, Out_en;
  //   // scanf("%i\t%i\t%2i:%2i:%c\t\t%2i:%2i:%c\t\t%i\t%i\t%i\t%i\t%i\t%i:%i\t%i:%i",
  //   //   &tmp.id, &tmp.type,
  //   //   &tmp.next.module, &tmp.next.id, &tmp.next.type,
  //   //   &tmp.prev.module, &tmp.prev.id, &tmp.prev.type,
  //   //   &tmp.speed,
  //   //   &dir,
  //   //   &tmp.length,
  //   //   &oneway,
  //   //   &Out_en,
  //   //   &tmp.IO_In.Node, &tmp.IO_In.Adr,
  //   //   &tmp.IO_Out.Node, &tmp.IO_Out.Adr);

  }
}

void modify_Engine(struct train_config * config, char cmd){
  int id;
  char _cmd[80];
  int tmp, tmp1;//, tmp2;
  if(cmd == 'e'){
    printf("Engine ID: ");
    fgets(_cmd, 80, stdin);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;
    printf("Editing Engine %i\n", id);
  }
  else if(cmd == 'a'){
    printf("Engine ID: (%i)\n", config->header.Engines);
    fgets(_cmd, 80, stdin); // Clear stdin

    if(config->header.Engines == 0){
      printf("Calloc");
      config->Engines = _calloc(1, struct engines_conf);
    }
    else{
      printf("Realloc");
      config->Engines = _realloc(config->Engines, config->header.Engines+1, struct engines_conf);
    }
    memset(&config->Engines[config->header.Engines], 0, sizeof(struct engines_conf));
    id = config->header.Engines++;

    //Set child pointers
    config->Engines[id].name_len = 1;
    config->Engines[id].name = _calloc(1, char);
    config->Engines[id].img_path_len = 1;
    config->Engines[id].img_path = _calloc(1, char);
    config->Engines[id].icon_path_len = 1;
    config->Engines[id].icon_path = _calloc(1, char);
    config->Engines[id].config_steps = 1;
    config->Engines[id].speed_steps = _calloc(1, struct engine_speed_steps);
  }
  else if(cmd == 'r'){
    printf("Remove Car ID: ");
    fgets(_cmd, 80, stdin);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;

    if(id == (config->header.Engines - 1) && id >= 0){
      _free(config->Engines[config->header.Engines - 1].name);
      _free(config->Engines[config->header.Engines - 1].img_path);
      _free(config->Engines[config->header.Engines - 1].icon_path);
      _free(config->Engines[config->header.Engines - 1].speed_steps);

      memset(&config->Engines[config->header.Engines - 1], 0, sizeof(struct engines_conf));
      config->Engines = _realloc(config->Engines, --config->header.Engines, struct engines_conf);
    }
    else{
      printf("Only last engine can be removed\n");
    }
  }


  if(cmd == 'e' || cmd == 'a'){
    printf("DCC Address (%i)         | ", config->Engines[id].DCC_ID);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->Engines[id].DCC_ID = tmp;

    printf("Speedsteps  (%x)   | ", config->Engines[id].type >> 4);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      printf("(%i << 4) & (%i & 0x0f)\n", tmp & 0x0f, config->Engines[id].type);
      config->Engines[id].type = ((tmp & 0x0f) << 4) | (config->Engines[id].type & 0x0f);
    }

    printf("Type  (%x)   | ", config->Engines[id].type & 0x0f);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      config->Engines[id].type = (tmp & 0x0f) | (config->Engines[id].type & 0xf0);
    }

    printf("Length  (%2i)   | ", config->Engines[id].length);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      config->Engines[id].length = tmp;
    }

    printf("Calibrated Steps  (%2i)   | ", config->Engines[id].config_steps);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      config->Engines[id].config_steps = tmp;
      _realloc(config->Engines[id].speed_steps, tmp, sizeof(struct engine_speed_steps));
    }

    for(int i = 0; i < config->Engines[id].config_steps; i++){
      printf(" - Step %i   (%03i, %03i) | ", i, config->Engines[id].speed_steps[i].step, config->Engines[id].speed_steps[i].speed);
      fgets(_cmd, 80, stdin);
      if(sscanf(_cmd, "%i %i", &tmp, &tmp1) > 1){
        config->Engines[id].speed_steps[i].step = tmp;
        config->Engines[id].speed_steps[i].speed = tmp1;
      }
    }

    printf("Name      (%s) | ", config->Engines[id].name);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%s", _cmd) > 0){
      tmp = strlen(_cmd);
      config->Engines[id].name = _realloc(config->Engines[id].name, tmp, char);
      memcpy(config->Engines[id].name, _cmd, tmp);
      config->Engines[id].name[tmp] = 0;
      config->Engines[id].name_len = tmp;
    }

    printf("Image path (%s) | ", config->Engines[id].img_path);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%s", _cmd) > 0){
      tmp = strlen(_cmd);
      config->Engines[id].img_path = _realloc(config->Engines[id].img_path, tmp, char);
      memcpy(config->Engines[id].img_path, _cmd, tmp);
      config->Engines[id].img_path[tmp] = 0;
      config->Engines[id].img_path_len = tmp;
    }

    printf("Icon path (%s) | ", config->Engines[id].icon_path);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%s", _cmd) > 0){
      tmp = strlen(_cmd);
      config->Engines[id].icon_path = _realloc(config->Engines[id].icon_path, tmp, char);
      memcpy(config->Engines[id].icon_path, _cmd, tmp);
      config->Engines[id].icon_path[tmp] = 0;
      config->Engines[id].icon_path_len = tmp;
    }

  //   printf("New:      \t");
  //   print_Stations(config->Stations[id]);
  //   // struct s_block_conf tmp;
  //   // int dir, oneway, Out_en;
  //   // scanf("%i\t%i\t%2i:%2i:%c\t\t%2i:%2i:%c\t\t%i\t%i\t%i\t%i\t%i\t%i:%i\t%i:%i",
  //   //   &tmp.id, &tmp.type,
  //   //   &tmp.next.module, &tmp.next.id, &tmp.next.type,
  //   //   &tmp.prev.module, &tmp.prev.id, &tmp.prev.type,
  //   //   &tmp.speed,
  //   //   &dir,
  //   //   &tmp.length,
  //   //   &oneway,
  //   //   &Out_en,
  //   //   &tmp.IO_In.Node, &tmp.IO_In.Adr,
  //   //   &tmp.IO_Out.Node, &tmp.IO_Out.Adr);

  }
}

void modify_Train(struct train_config * config, char cmd){
  int id;
  char _cmd[20];
  int tmp, tmp1;//, tmp2;
  if(cmd == 'e'){
    printf("Train ID: ");
    fgets(_cmd, 20, stdin);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;
    printf("Editing Train %i\n", id);
  }
  else if(cmd == 'a'){
    printf("Train ID: (%i)\n", config->header.Trains);
    fgets(_cmd, 20, stdin); // Clear stdin

    if(config->header.Trains == 0){
      printf("Calloc");
      config->Trains = _calloc(1, struct trains_conf);
    }
    else{
      printf("Realloc");
      config->Trains = _realloc(config->Trains, config->header.Trains+1, struct trains_conf);
    }
    memset(&config->Trains[config->header.Trains], 0, sizeof(struct trains_conf));
    id = config->header.Trains++;

    //Set child pointers
    config->Trains[id].name_len = 1;
    config->Trains[id].name = _calloc(1, char);
    config->Trains[id].nr_stock = 1;
    config->Trains[id].composition = _calloc(1, sizeof(struct train_comp_ws));
  }
  else if(cmd == 'r'){
    printf("Remove Train ID: ");
    fgets(_cmd, 20, stdin);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;

    if(id == (config->header.Trains - 1) && id >= 0){
      _free(config->Trains[config->header.Trains - 1].name);

      memset(&config->Engines[config->header.Trains - 1], 0, sizeof(struct trains_conf));
      config->Trains = _realloc(config->Trains, --config->header.Trains, struct trains_conf);
    }
    else{
      printf("Only last engine can be removed\n");
    }
  }


  if(cmd == 'e' || cmd == 'a'){
    printf("Nr stock (%i)         | ", config->Trains[id].nr_stock);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      config->Trains[id].nr_stock = tmp;
      config->Trains[id].composition = _realloc(config->Trains[id].composition, tmp, struct train_comp_ws);
    }

    printf("Name      (%s) | ", config->Trains[id].name);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%s", _cmd) > 0){
      tmp = strlen(_cmd);
      config->Trains[id].name = _realloc(config->Trains[id].name, tmp, char);
      memcpy(config->Trains[id].name, _cmd, tmp);
      config->Trains[id].name[tmp] = 0;
      config->Trains[id].name_len = tmp;
    }

    printf("Catagory  (%i) | ", config->Trains[id].catagory);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->Trains[id].catagory = tmp;

    for(int i = 0; i < config->Trains[id].nr_stock; i++){
      printf(" - Step %i   (%02i, %04i) | ", i, config->Trains[id].composition[i].type, config->Trains[id].composition[i].id);
      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%i %i", &tmp, &tmp1) > 1){
        config->Trains[id].composition[i].type = tmp;
        config->Trains[id].composition[i].id = tmp1;
      }
    }

  //   printf("New:      \t");
  //   print_Stations(config->Stations[id]);
  //   // struct s_block_conf tmp;
  //   // int dir, oneway, Out_en;
  //   // scanf("%i\t%i\t%2i:%2i:%c\t\t%2i:%2i:%c\t\t%i\t%i\t%i\t%i\t%i\t%i:%i\t%i:%i",
  //   //   &tmp.id, &tmp.type,
  //   //   &tmp.next.module, &tmp.next.id, &tmp.next.type,
  //   //   &tmp.prev.module, &tmp.prev.id, &tmp.prev.type,
  //   //   &tmp.speed,
  //   //   &dir,
  //   //   &tmp.length,
  //   //   &oneway,
  //   //   &Out_en,
  //   //   &tmp.IO_In.Node, &tmp.IO_In.Adr,
  //   //   &tmp.IO_Out.Node, &tmp.IO_Out.Adr);

  }
}

void modify_Catagory(struct train_config * config, char type, char cmd){
  int id;
  char _cmd[20];
  int tmp;//, tmp2;

  uint8_t * _header;
  struct cat_conf ** _cat;
  if(type == 'P'){
    _header = &config->header.P_Catagories;
    _cat = &config->P_Cat;
  }
  else{
    _header = &config->header.C_Catagories;
    _cat = &config->C_Cat;
  }



  if(cmd == 'e'){
    printf("Catagory ID: ");
    fgets(_cmd, 20, stdin);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;
    printf("Editing Catagory %i\n", id);

    if (type == 'C' && id > 0x80)
      id -= 0x80;
  }
  else if(cmd == 'a'){
    printf("Catagory ID: (%i)\n", *_header);

    fgets(_cmd, 20, stdin); // Clear stdin

    if(*_header == 0){
      printf("Calloc");
      *_cat = _calloc(1, struct cat_conf);
    }
    else{
      printf("Realloc");
      *_cat = _realloc(*_cat, (*_header)+1, struct cat_conf);
    }
    memset(&(*_cat)[*_header], 0, sizeof(struct cat_conf));
    id = *_header;
    *_header += 1;

    //Set child pointers
    (*_cat)[id].name_len = 1;
    (*_cat)[id].name = _calloc(1, char);
  }
  else if(cmd == 'r'){
    printf("Remove Train ID: ");
    fgets(_cmd, 20, stdin);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;

    if(id == (*_header - 1) && id >= 0){
      _free((*_cat)[*_header - 1].name);

      memset(&(*_cat)[*_header - 1], 0, sizeof(struct cat_conf));
      *_cat = _realloc(*_cat, --(*_header), struct cat_conf);
    }
    else{
      printf("Only last engine can be removed\n");
    }
  }


  if(cmd == 'e' || cmd == 'a'){
    printf("Name      (%s) | ", (*_cat)[id].name);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%s", _cmd) > 0){
      tmp = strlen(_cmd);
      (*_cat)[id].name = _realloc((*_cat)[id].name, tmp, char);
      memcpy((*_cat)[id].name, _cmd, tmp);
      (*_cat)[id].name_len = tmp;
    }
  }
}

int edit_module(){
  char filename[40] = "configs/units/";
  int file;

  printf("Open module: ");

  scanf("%i", &file);

  sprintf(filename, "%s%i.bin", filename, file);

  FILE * fp = fopen(filename,"rb");

  struct module_config config;

  if(file <= 0 || file > 254){
    loggerf(ERROR, "Only module numbers between 1-254 supportede");

    if(file == 0){
      printf("Open Test Module? ");
      char c;
      scanf("%c", &c);
      scanf("%c", &c);
      printf("%c",c);
      if(c != 'y')
        return -1;
    }
    else
      return -1;
  }

  if(!fp){
    loggerf(ERROR, "Failed to open file");
    loggerf(INFO,  "Creating New File");
    
    int connections;

    printf("How many sides does the module connect? ");
    scanf("%i", &connections);

    config.header.module = file;
    config.header.connections = connections;
    config.header.Blocks = 0;
    config.header.Switches = 0;
    config.header.MSSwitches = 0;
    config.header.Signals = 0;
    config.header.Stations = 0;
  }
  else{
    if(read_module_config(&config, fp) == -1){
      return 0;
    };
    if(config.header.module != file){
      loggerf(CRITICAL, "INVALID MODULE ID");
      return 0;
    }
    fclose(fp);
  }
  print_module_config(&config);

  char cmd[300];

  while (1){
    printf("Command: ");
    scanf("%s", cmd);
    if(strcmp(cmd, "q") == 0){
      break;
    }
    else if(strcmp(cmd, "e") == 0 || strcmp(cmd, "a") == 0 || strcmp(cmd, "r") == 0){
      char cmd1 = cmd[0];
      if(cmd1 == 'e')
        printf("Edit");
      else if(cmd1 == 'a')
        printf("Add");
      else if(cmd1 == 'r')
        printf("Remove");
      printf(" (B/Sw/MSSw/Sig/St)? ");
      scanf("%s", cmd);
      if(strcmp(cmd, "B") == 0){
        modify_Block(&config, cmd1);
      }
      else if(strcmp(cmd, "Sw") == 0){
        modify_Switch(&config, cmd1);
      }
      else if(strcmp(cmd, "MSSw") == 0){
        modify_MSSwitch(&config, cmd1);
      }
      else if(strcmp(cmd, "Sig") == 0){
        modify_Signal(&config, cmd1);
      }
      else if(strcmp(cmd, "St") == 0){
        modify_Station(&config, cmd1);
      }
      else if(strcmp(cmd, "N") == 0){
        modify_Node(&config, cmd1);
      }
    }
    else if(strcmp(cmd, "ex") == 0 || strcmp(cmd, "Ex") == 0){
      export_Layout(&config);
    }
    else if(strcmp(cmd, "im") == 0 || strcmp(cmd, "Im") == 0){
      import_Layout(&config);
    }
    else if(strcmp(cmd,  "pL") == 0 || strcmp(cmd, "pl") == 0){
      print_Layout(&config);
    }
    else if(strcmp(cmd, "s") == 0){
      write_module_from_conf(&config, filename);
    }
    else if(strcmp(cmd, "p") == 0){
      print_module_config(&config);
    }
    else
      printf("Not a command\n");
  }

  //free switches
  for(int i = 0; i < config.header.Switches; i++){
    _free(config.Switches[i].IO_Ports);
  }

  //free msswitches
  for(int i = 0; i < config.header.MSSwitches; i++){
    _free(config.MSSwitches[i].states);
    _free(config.MSSwitches[i].IO_Ports);
  }

  //free signals
  for(int i = 0; i < config.header.Signals; i++){
    _free(config.Signals[i].output);
    _free(config.Signals[i].stating);
  }

  //free station
  for(int i = 0; i < config.header.Stations; i++){
    _free(config.Stations[i].name);
    _free(config.Stations[i].blocks);
  }

  _free(config.Nodes);
  _free(config.Blocks);
  _free(config.Switches);
  _free(config.MSSwitches);
  _free(config.Signals);
  _free(config.Stations);

  exit_logger();

  printf("Done\n");
  return 1;
}

struct train_config * global_config;

int edit_rolling_stock(){
  printf("Reading file\n");

  FILE * fp = fopen(TRAIN_CONF_PATH,"rb");

  struct train_config config;

  if(!fp){
    loggerf(ERROR, "Failed to open file");
    printf("No File");
    return -1;
  }
  else{
    read_train_config(&config, fp);
    fclose(fp);
  }
  print_train_config(&config);
  global_config = &config;

  char cmd[300];

  while (1){
    printf("Command: ");
    scanf("%s", cmd);
    if(strcmp(cmd, "q") == 0){
      break;
    }
    else if(strcmp(cmd, "e") == 0 || strcmp(cmd, "a") == 0 || strcmp(cmd, "r") == 0){
      char cmd1 = cmd[0];
      if(cmd1 == 'e')
        printf("Edit");
      else if(cmd1 == 'a')
        printf("Add");
      else if(cmd1 == 'r')
        printf("Remove");

      printf(" (C(ar)/E(ngine)/T(rain)/(P/C)_Cat)? ");
      scanf("%s", cmd);
      if(strcmp(cmd, "C") == 0){
        modify_Car(&config, cmd1);
      }
      else if(strcmp(cmd, "E") == 0){
        modify_Engine(&config, cmd1);
      }
      else if(strcmp(cmd, "T") == 0){
        modify_Train(&config, cmd1);
      }
      else if(strcmp(cmd, "P_Cat") == 0){
        modify_Catagory(&config, 'P', cmd1);
      }
      else if(strcmp(cmd, "C_Cat") == 0){
        modify_Catagory(&config, 'C', cmd1);
      }
    }
    else if(strcmp(cmd, "s") == 0){
      write_train_from_conf(&config, TRAIN_CONF_PATH);
    }
    else if(strcmp(cmd, "p") == 0){
      print_train_config(&config);
    }
    else
      printf("Not a command\n");
  }

  _free(config.Cars);
  _free(config.Engines);
  _free(config.Trains);

  exit_logger();

  printf("Done\n");
  return 1;
}

int main(){
  init_logger("log_config.txt");
  set_level(MEMORY);

  printf("Edit module or rolling stock? ");

  char cmd[40] = "";

  fgets(cmd, 20, stdin);
  sscanf(cmd, "%s", cmd);

  if(strcmp(cmd, "module") == 0 || strcmp(cmd, "Module") == 0){
    edit_module();
  }
  else if(strcmp(cmd, "Train") == 0 || strcmp(cmd, "Car") == 0 || strcmp(cmd, "Engine") == 0 || strcmp(cmd, "rolling stock") == 0){
    edit_rolling_stock();
  }
  else{
    printf("Unknown command");
  }

  printf("Done");

  
}
