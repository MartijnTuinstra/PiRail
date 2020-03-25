#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "logger.h"
#include "config.h"
#include "mem.h"
#include "rail.h"

void print_link(char ** debug, struct s_link_conf link){
  if(link.type == RAIL_LINK_C){
    *debug += sprintf(*debug, "C %2i:%2i  \t", link.module, link.id);
  }
  else if(link.type == RAIL_LINK_E){
    *debug += sprintf(*debug, "E        \t");
  }
  else{
    *debug += sprintf(*debug, "%2i:%2i:", link.module, link.id);
    if(link.type == RAIL_LINK_R)
      *debug += sprintf(*debug, "%c  \t", 'R');
    else if(link.type == RAIL_LINK_S)
      *debug += sprintf(*debug, "%c  \t", 'S');
    else if(link.type == RAIL_LINK_s)
      *debug += sprintf(*debug, "%c  \t", 's');
    else if(link.type == RAIL_LINK_MA)
      *debug += sprintf(*debug, "%s  \t", "MB");
    else if(link.type == RAIL_LINK_MB)
      *debug += sprintf(*debug, "%s  \t", "MB");
    else if(link.type == RAIL_LINK_ma)
      *debug += sprintf(*debug, "%s  \t", "ma");
    else if(link.type == RAIL_LINK_mb)
      *debug += sprintf(*debug, "%s  \t", "mb");
    else if(link.type == RAIL_LINK_TT)
      *debug += sprintf(*debug, "%c%c \t", 'T', 'T');
    else
      *debug += sprintf(*debug, "%i  \t", link.type);
  }
}

void print_Node(struct node_conf node){
  char debug[300];
  char * debugptr = debug;
  const char hexset[16] = "0123456789ABCDEF";

  debugptr += sprintf(debugptr, "%i\t%i\t", node.Node, node.size);

  for(int j = 0; j < node.size; j++){
    debugptr[0] = hexset[(node.data[j/2] >> (4 * (j%2))) & 0xF];
    debugptr++;
  }

  debugptr[0] = '\n';
  debugptr[1] = 0;

  printf("%s", debug);
}

void print_Block(struct s_block_conf block){
  const char * rail_types_string[4] = {
    "MAIN",
    "STATION",
    "NOSTOP",
    "TURNTABLE"
  };

  char debug[300];
  char * debugptr = debug;

  debugptr += sprintf(debugptr, "%i\t%11s\t",
                block.id,
                rail_types_string[block.type]);
  print_link(&debugptr, block.next);
  print_link(&debugptr, block.prev);
  debugptr += sprintf(debugptr, "%i\t%i\t%i\t%i\t%i\t%i:%i\t%i:%i",
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
  char debug[300];
  char * debugptr = debug;

  debugptr += sprintf(debugptr, "%i\t%i\t",
                Switch.id,
                Switch.det_block);
  print_link(&debugptr, Switch.App);
  print_link(&debugptr, Switch.Str);
  print_link(&debugptr, Switch.Div);
  debugptr += sprintf(debugptr, "%x\t%i %i",
                Switch.IO,
                Switch.speed_Str, Switch.speed_Div);

  for(int j = 0; j < (Switch.IO & 0x0f); j++){
    debugptr += sprintf(debugptr, "\t%i:%i", Switch.IO_Ports[j].Node, Switch.IO_Ports[j].Adr);
  }

  printf( "%s\n", debug);
}

void print_MSSwitch(struct ms_switch_conf Switch){
  char debug[1000];
  char * debugptr = debug;

  debugptr += sprintf(debugptr, "%i\t%i\t%i\t%2i -> [%2i:%2i",
                Switch.id,
                Switch.det_block,
                Switch.nr_states,
                Switch.IO,
                Switch.IO_Ports[0].Node, Switch.IO_Ports[0].Adr);

  for(int i = 1; i < Switch.IO; i++){
    debugptr += sprintf(debugptr, ", %2i:%2i", Switch.IO_Ports[i].Node, Switch.IO_Ports[i].Adr);
  }
  debugptr += sprintf(debugptr, "]\n");

  for(int i = 0; i < Switch.nr_states; i++){
    debugptr += sprintf(debugptr, "\t\t\t%2i >\t", i);
    print_link(&debugptr, Switch.states[i].sideA);
    print_link(&debugptr, Switch.states[i].sideB);
    debugptr += sprintf(debugptr, "%i\t%x\n", Switch.states[i].speed, Switch.states[i].output_sequence);
  }

  printf( "%s", debug);
}

void print_Signals(struct signal_conf signal){
  char debug[400];
  char * debugptr = debug;

  debugptr += sprintf(debugptr, "%i\t%i\t%i", signal.id, signal.blockId, signal.output_len);
  for(int i = 0;i < signal.output_len; i++){
    debugptr += sprintf(debugptr, "\n\t\t\t%02i:%02i - ", signal.output[i].Node, signal.output[i].Adr);
    for(int j = 0; j < 8; j++){
      debugptr += sprintf(debugptr, "%i ", signal.stating[i].event[j]);
    }
  }

  printf("%s\n", debug);
}

void print_Stations(struct station_conf stations){
  char debug[250];
  char * debugptr = debug;

  debugptr += sprintf(debugptr, "%i\t%9s\t",
                stations.type,
                stations.name);

  for(int j = 0; j < stations.nr_blocks; j++){
    debugptr += sprintf(debugptr, "%i ", stations.blocks[j]);
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

void print_module_config(struct module_config * config, char ** cmds, uint8_t cmd_len){
  if(!cmds)
    return;
  
  cmds = &cmds[1];
  cmd_len -= 1;

  uint16_t mask = 0;

  for(uint8_t i = 0; i < cmd_len;){
    if(strcmp(cmds[i], "-h") == 0){
      // Help
      printf("Arguments for Print config: \n");
      printf("\t-H\tHeaders\n");
      printf("\t-n\tNodes\n");
      printf("\t-b\tBlocks\n");
      printf("\t-p\tSwitches/Points\n");
      printf("\t-m\tMSSwitches\n");
      printf("\t-s\tSignals\n");
      printf("\t-t\tStations\n");
      printf("\t-A\tAll\n");
      return;
    }
    else if(strcmp(cmds[i], "-H") == 0){
      mask |= 1;
    }
    else if(strcmp(cmds[i], "-n") == 0){
      mask |= 2;
    }
    else if(strcmp(cmds[i], "-b") == 0){
      mask |= 8;
    }
    else if(strcmp(cmds[i], "-p") == 0){
      mask |= 16;
    }
    else if(strcmp(cmds[i], "-m") == 0){
      mask |= 32;
    }
    else if(strcmp(cmds[i], "-s") == 0){
      mask |= 64;
    }
    else if(strcmp(cmds[i], "-t") == 0){
      mask |= 128;
    }
    else if(strcmp(cmds[i], "-A") == 0){
      mask |= 255;
    }
    i++;
  }

  if(mask == 0){
    // Help
    printf("Arguments for Print config: \n");
    printf("\t-h\tHeaders\n");
    printf("\t-n\tNodes\n");
    printf("\t-b\tBlocks\n");
    printf("\t-p\tSwitches/Points\n");
    printf("\t-m\tMSSwitches\n");
    printf("\t-s\tSignals\n");
    printf("\t-t\tStations\n");
    printf("\t-A\tAll");
    return;
  }

  printf("\n");
  if(mask & 1){
    printf( "Modules:     %i\n", config->header.module);
    printf( "Connections: %i\n", config->header.connections);
    printf( "IO_Nodes:    %i\n", config->header.IO_Nodes);
    printf( "Blocks:      %i\n", config->header.Blocks);
    printf( "Switches:    %i\n", config->header.Switches);
    printf( "MSSwitches:  %i\n", config->header.MSSwitches);
    printf( "Signals:     %i\n", config->header.Signals);
    printf( "Stations:    %i\n", config->header.Stations);
  }
  
  if(mask & 2){
    printf( "IO Nodes\n");
    printf( "id\tSize\n");
    for(int i = 0; i < config->header.IO_Nodes; i++){
      print_Node(config->Nodes[i]);
    }
  }

  if(mask & 8){
    printf( "Block\n");
    printf( "id\ttype\t\tNext    \tPrev    \tMax_sp\tdir\tlen\tOneWay\tOut en\tIO_in\tIO_out\n");
    for(int i = 0; i < config->header.Blocks; i++){
      print_Block(config->Blocks[i]);
    }
  }
  
  if(mask & 16){
    printf( "Switch\n");
    printf( "id\tblock\tApp       \tStr       \tDiv       \tIO\tSpeed\n");
    for(int i = 0; i < config->header.Switches; i++){
      print_Switch(config->Switches[i]);
    }
  }

  if(mask & 32){
    printf( "MSSwitch\n");
    printf( "id\tblock\tstates\tIO\tSideA     \tSideB     \tSpeed\tSequence\t...\n");
    for(int i = 0; i < config->header.MSSwitches; i++){
      print_MSSwitch(config->MSSwitches[i]);
    }
  }

  if(mask & 64){
    printf( "Signals\n");
    printf( "id\tBlockID\tOutput Length\t\tOutput states\n");
    for(int i = 0; i < config->header.Signals; i++){
      print_Signals(config->Signals[i]);
    }
  }

  if(mask & 128){
    printf( "Station\n");
    printf( "type\tName\t\tblocks\n");
    for(int i = 0; i < config->header.Stations; i++){
      print_Stations(config->Stations[i]);
    }
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

  config->Nodes = _calloc(config->header.IO_Nodes, struct node_conf);
  
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

void modify_Node(struct module_config * config, char ** cmds, uint8_t cmd_len){
  if(!cmds)
    return;

  int id;
  char mode = cmds[0][0];
  if(cmds[0][0] == 'e'){
    if(cmd_len > 2){
      id = atoi(cmds[2]);
      printf("Editing Node %i\n", id);
    }
    else{
      printf("No ID supplied\n");
      return;
    }
  }
  else if(cmds[0][0] == 'a'){
    printf("Node ID: (%i)\n", config->header.IO_Nodes);

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
  else if(cmds[0][0] == 'r'){
    if(cmd_len > 2){
      id = atoi(cmds[2]);
      printf("Removing Node %i\n", id);
    }
    else{
      printf("No ID supplied\n");
      return;
    }

    if(id == (config->header.IO_Nodes - 1) && id >= 0){
      memset(&config->Nodes[config->header.IO_Nodes - 1], 0, sizeof(struct s_node_conf));
      config->Nodes = _realloc(config->Nodes, --config->header.IO_Nodes, struct s_node_conf);
    }
    else{
      printf("Only last Node can be removed\n");
    }
  }


  if((mode == 'e' && cmd_len <= 3) || (mode == 'a' && cmd_len <= 2)){
    int tmp;
    char _cmd[20];
    printf("Node Size      (%i)         | ", config->Nodes[id].size);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->Nodes[id].size = tmp;

    printf("New:      \t");
    print_Node(config->Nodes[id]);
  }
  else if(mode == 'e'){
    printf("Argment mode\n");
    cmds = &cmds[3];
    cmd_len -= 3;

    for(uint8_t i = 0; i < cmd_len;){
      printf("%s\t", cmds[i]);
      if(strcmp(cmds[i], "-h") == 0){
        // Help
        printf("Arguments for Node: \n");
        printf("\t-p              Number of IO\n");
        printf("\t-t [io] [type]  Set io type\n");
        return;
      }
      else if(strcmp(cmds[i], "-p") == 0){
        printf("set size");
        config->Nodes[id].size = atoi(cmds[i+1]);
        config->Nodes[id].data = _realloc(config->Nodes[id].data, (config->Nodes[id].size+1)/2, char);

        i++;
      }
      else if(strcmp(cmds[i], "-t") == 0){
        uint8_t port = atoi(cmds[i+1]);
        printf("%i => %s\n", port, cmds[i+2]);
        config->Nodes[id].data[port/2] &= ~(0xF << (4 * (port % 2)));
        config->Nodes[id].data[port/2] |= (atoi(cmds[i+2]) & 0xF) << (4 * (port % 2));
        i+= 2;
      }
      i++;
    }
    printf("\n");
  }
}

void modify_Block(struct module_config * config, char ** cmds, uint8_t cmd_len){
  if(!cmds)
    return;
  
  int id;
  char mode = cmds[0][0];
  if(mode == 'e'){
    if(cmd_len > 2){
      id = atoi(cmds[2]);
      printf("Editing block %i\n", id);
    }
    else{
      printf("No ID supplied\n");
      return;
    }
  }
  else if(mode == 'a'){
    printf("Block ID: (%i)\n", config->header.Blocks);

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
  else if(mode == 'r'){
    if(cmd_len > 2){
      id = atoi(cmds[2]);
      printf("Removing block %i\n", id);
    }
    else{
      printf("No ID supplied\n");
      return;
    }

    if(id == (config->header.Blocks - 1) && id >= 0){
      memset(&config->Blocks[config->header.Blocks - 1], 0, sizeof(struct s_block_conf));
      config->Blocks = _realloc(config->Blocks, --config->header.Blocks, struct s_block_conf);
    }
    else{
      printf("Only last block can be removed\n");
    }
  }


  if((mode == 'e' && cmd_len <= 3) || (mode == 'a' && cmd_len <= 2)){
    char _cmd[20];
    int tmp, tmp1, tmp2;
    char tmpc;
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

    printf("Block Oneway    (%c)         | ",(config->Blocks[id].fl & 1)?'Y':'N');

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
  }
  else if(mode == 'e'){
    printf("Argment mode\n");
    cmds = &cmds[3];
    cmd_len -= 3;

    for(uint8_t i = 0; i < cmd_len;){
      printf("%s\t", cmds[i]);
      if(strcmp(cmds[i], "-h") == 0){
        // Help
        printf("Arguments for Block: \n");
        printf("\t-t\tType\n");
        printf("\t-N\tNextLink\n");
        printf("\t-P\tPrevLink\n");
        printf("\t-s\tSpeed\n");
        printf("\t-l\tLength\n");
        printf("\t--OW\tOneWay\n");
        printf("\t-d\tDirection\n");
        // printf("\t--IOIn\tIO Input Address\n");
        // printf("\t--IOOut\tIO Output Address\n");
        return;
      }
      else if(strcmp(cmds[i], "-t") == 0){
        config->Blocks[id].type = atoi(cmds[i+1]);
        i++;
      }
      else if(strcmp(cmds[i], "-N") == 0 || strcmp(cmds[i], "-P") == 0){
        struct s_link_conf * link;
        char _type[10];
        uint8_t _m, _id;

        if(cmds[i][1] == 'N')
          link = &config->Blocks[id].next;
        else
          link = &config->Blocks[id].prev;

        if(sscanf(cmds[i+1], "%hhu:%hhu:%s", &_m, &_id, _type) > 0){
          link->module = _m;
          link->id = _id;

          if(_type[0] == 'E')
            link->type = RAIL_LINK_E;
          else if(_type[0] == 'C')
            link->type = RAIL_LINK_C;
          else
            link->type = atoi(_type);
        }
        i++;
      }
      else if(strcmp(cmds[i], "-s") == 0){
        config->Blocks[id].speed = atoi(cmds[i+1]);
        i++;
      }
      else if(strcmp(cmds[i], "-l") == 0){
        config->Blocks[id].length = atoi(cmds[i+1]);
        i++;
      }
      else if(strcmp(cmds[i], "--OW") == 0){
        if(cmds[i+1][0] == 'Y' || cmds[i+1][0] == 'y')
          config->Blocks[id].fl |= 0x1;
        else
          config->Blocks[id].fl &= ~0x1;
        i++;
      }
      else if(strcmp(cmds[i], "-d") == 0){
        config->Blocks[id].fl &= ~0b1110;
        config->Blocks[id].fl |= (atoi(cmds[i+1]) & 0x3) << 1;
        i++;
      }
      i++;
    }
    printf("\n");
  }
  else{
    printf("Mode not supported");
  }
}

void modify_Switch(struct module_config * config, char cmd){
  int id;
  char _cmd[20];
  int tmp, tmp1, tmp2;
  if(cmd == 'e'){
    printf("Switch ID: ");
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;
    printf("Editing Switch %i\n", id);
    // print_Block(config->Blocks[id]);
  }
  else if(cmd == 'a'){
    printf("Switch ID: (%i)\n", config->header.Switches);

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
    if(sscanf(_cmd, "%i", &id) < 1 || id >= config->header.MSSwitches || id < 0){
      printf("Invalid\n");
      return;
    }
    printf("Editing MSSwitch %i\n", id);
    // print_Block(config->Blocks[id]);
  }
  else if(cmd == 'a'){
    printf("MSSwitch ID: (%i)\n", config->header.MSSwitches);

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
    if(sscanf(_cmd, "%i", &id) < 1)
      return;
    printf("Editing Signal %i\n", id);
    // print_Block(config->Blocks[id]);
  }
  else if(cmd == 'a'){
    printf("Signal ID: (%i)\n", config->header.Signals);

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
    if(sscanf(_cmd, "%i", &id) < 1)
      return;
    printf("Editing Station %i\n", id);
    // print_Block(config->Blocks[id]);
  }
  else if(cmd == 'a'){
    printf("Station ID: (%i)\n", config->header.Stations);

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

void export_Layout(struct module_config * config, char * location){
  FILE * fp;
  /* open the file for writing*/
  if(location)
    fp = fopen (location,"w");
  else
    fp = fopen ("Layout_export.txt","w");

  if(!fp){
    printf("Failed to export layout\n");
    return;
  }

  /* write 10 lines of text into the file stream*/
  fprintf(fp, "%s", config->Layout);

  /* close the file*/  
  fclose (fp);
}

void import_Layout(struct module_config * config, char * location){
  FILE * fp;

  if(!location){
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
  }
  else{
    fp = fopen (location,"r");
    if(!fp){
      loggerf(ERROR, "File not found '%s'", location);
      return;
    }
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
  const char filepath[20] = "configs/units/";
  char filename[40];
  int file;

  printf("Open module: ");

  scanf("%i", &file);

  sprintf(filename, "%s%i.bin", filepath, file);

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
  print_module_config(&config, 0, 0);

  char cmd[300];
  char ** cmds = _calloc(100, char *);
  uint8_t cmds_len = 0;

  while (1){
    memset(cmds, 0, 100);
    cmds_len = 0;
    printf("> ");
    fgets(cmd,300,stdin);

    // Split command into arguments
    cmds[0] = strtok(cmd, " ");

    while( cmds[cmds_len] != NULL ) {
      cmds[++cmds_len] = strtok(NULL, " ");
    }

    // Remove \n character from last command/argument
    cmds[cmds_len-1][strlen(cmds[cmds_len-1])-1] = 0;

    // If no command
    if(!cmds[0])
      continue;

    if(cmd[0] == 'q'){
      break;
    }
    else if(cmd[0] == 'h'){
      printf("Help:\n");
      printf("\ta [Item]            \tAdd an Item with ID\n");
      printf("\te [Item] [ID] {args}\tEdit an Item with ID\n");
      printf("\td [Item] [ID] {args}\tRemove an Item with ID\n");
      printf("\tp                   \tPrint configuration\n");
      printf("\tim [path]           \tImport web layout from path\n");
      printf("\tex {path}           \tExport web layout to path, defaults to Layout_export.txt\n");
      printf("\tpl                  \tPrint current web layout\n");
      printf("\ts                   \tSave configuration\n");
    }
    else if(strcmp(cmds[0], "e") == 0 || strcmp(cmds[0], "a") == 0 || strcmp(cmds[0], "r") == 0){
      if(cmds_len < 2){
        printf("Command too short\n");
        continue;
      }

      if(strcmp(cmds[1], "B") == 0){
        modify_Block(&config, cmds, cmds_len);
      }
      else if(strcmp(cmds[1], "Sw") == 0){
        modify_Switch(&config, cmds[0][0]);
      }
      else if(strcmp(cmds[1], "MSSw") == 0){
        modify_MSSwitch(&config, cmds[0][0]);
      }
      else if(strcmp(cmds[1], "Sig") == 0){
        modify_Signal(&config, cmds[0][0]);
      }
      else if(strcmp(cmds[1], "St") == 0){
        modify_Station(&config, cmds[0][0]);
      }
      else if(strcmp(cmds[1], "N") == 0){
        modify_Node(&config, cmds, cmds_len);
      }
    }
    else if(strcmp(cmds[0], "ex") == 0 || strcmp(cmds[0], "Ex") == 0){
      export_Layout(&config, cmds[1]);
    }
    else if(strcmp(cmds[0], "im") == 0 || strcmp(cmds[0], "Im") == 0){
      import_Layout(&config, cmds[1]);
    }
    else if(strcmp(cmds[0],  "pL") == 0 || strcmp(cmds[0], "pl") == 0){
      print_Layout(&config);
    }
    else if(strcmp(cmds[0], "s") == 0){
      write_module_from_conf(&config, filename);
    }
    else if(strcmp(cmds[0], "p") == 0){
      print_module_config(&config, cmds, cmds_len);
    }
  //   else
  //     printf("Not a command\n");
  }

  int exit = 1;


  if(cmds_len > 1 && strcmp(cmds[1], "-r") == 0){
    exit = -1;
  }

  _free(cmds);

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

  return exit;
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
  set_level(TRACE);

  printf("Edit module or rolling stock? ");
  char cmd[40] = "";
  int val;

  restart:

  memset(cmd,0,40);

  fgets(cmd, 20, stdin);
  sscanf(cmd, "%s", cmd);

  if(strcmp(cmd, "module") == 0 || strcmp(cmd, "Module") == 0){
    val = edit_module();
  }
  else if(strcmp(cmd, "Train") == 0 || strcmp(cmd, "Car") == 0 || strcmp(cmd, "Engine") == 0 || strcmp(cmd, "rolling stock") == 0){
    edit_rolling_stock();
  }
  else{
    printf("Unknown command");
  }

  if(val == -1){
    printf("Restart!!\n");
    goto restart;
  }

  printf("Done");

  
  loggerf(INFO, "STOPPED");
  exit_logger(); //Close logger

  print_allocs();
}
