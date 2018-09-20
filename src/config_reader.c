#include <stdint.h>
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
    else if(link.type == RAIL_LINK_M)
      sprintf(debug, "%s%c  \t", debug, 'M');
    else if(link.type == RAIL_LINK_m)
      sprintf(debug, "%s%c  \t", debug, 'm');
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
  char debug[200];

  sprintf(debug, "%i\t%i\t",
                block.id,
                block.type);
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

  sprintf(debug, "%i\t%i\t%i\n\t",
                Switch.det_block,
                Switch.nr_states,
                Switch.IO);
  for(int i = 0; i < Switch.nr_states; i++){
    sprintf(debug, "%s\t%2i:%2i:%2i\t%2i:%2i:%2i\t%i\t%x",
                debug,
                Switch.states[i].sideA.module, Switch.states[i].sideA.id, Switch.states[i].sideA.type,
                Switch.states[i].sideB.module, Switch.states[i].sideB.id, Switch.states[i].sideB.type,
                Switch.states[i].speed, Switch.states[i].output_sequence);
  }

  sprintf(debug, "%s\n\t", debug);

  for(int i = 0; i < Switch.IO; i++){
    sprintf(debug, "%s\t%i:%i", debug, Switch.IO_Ports[i].Node, Switch.IO_Ports[i].Adr);
  }

  printf( "%s\n", debug);
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

void print_config(struct config * config){
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
  printf( "id\ttype\tNext    \tPrev    \tMax_sp\tdir\tlen\tOneWay\tOut en\tIO_in\tIO_out\n");
  for(int i = 0; i < config->header.Blocks; i++){
    print_Block(config->Blocks[i]);
  }
  
  printf( "Switch\n");
  printf( "id\tblock\tApp       \tStr       \tDiv       \tIO\tSpeed\n");
  for(int i = 0; i < config->header.Switches; i++){
    print_Switch(config->Switches[i]);
  }

  printf( "MSSwitch\n");
  printf( "id\tblock\tSideA     \tSideB     \tSpeed\tSequence\t...\n");
  for(int i = 0; i < config->header.MSSwitches; i++){
    print_MSSwitch(config->MSSwitches[i]);
  }

  printf( "Station\n");
  printf( "type\tName\t\tblocks\n");
  for(int i = 0; i < config->header.Stations; i++){
    print_Stations(config->Stations[i]);
  }
}

void read_config(struct config * config, FILE * fp){


  char * header = _calloc(2, char);

  fread(header, 1, 1, fp);

  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char * buffer = _calloc(fsize, char);
  char * buffer_start = &buffer[0];
  fread(buffer, fsize, 1, fp);

  print_hex(buffer, fsize);

  uint8_t ** buf_ptr = (uint8_t **)&buffer;

  *buf_ptr += 1;

  config->header = read_s_unit_conf(buf_ptr);

  if (header[0] != 1) {
    loggerf(WARNING, "Module %i not correct version", config->header.module);
    config->header.IO_Nodes = 0;
    loggerf(WARNING, "Please re-save to update", config->header.module);
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

  config->Stations = _calloc(config->header.Stations, struct station_conf);

  for(int i = 0; i < config->header.Stations; i++){
    config->Stations[i]  = read_s_station_conf(buf_ptr);
  }

  printf( "buf_ptr %x\n", (unsigned int)*buf_ptr);

  _free(header);
  _free(buffer_start);
}

void modify_Node(struct config * config, char cmd){
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

void modify_Block(struct config * config, char cmd){
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
    if(sscanf(_cmd, "%i:%i:%i", &tmp, &tmp1, &tmp2) > 2){
      config->Blocks[id].next.module = tmp;
      config->Blocks[id].next.id = tmp1;
      config->Blocks[id].next.type = tmp2;
    }

    printf("Block Link Prev (%2i:%2i:%2x)  | ", config->Blocks[id].prev.module, config->Blocks[id].prev.id, config->Blocks[id].prev.type);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i:%i:%i", &tmp, &tmp1, &tmp2) > 2){
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
    if(sscanf(_cmd, "%i:%i", &tmp, &tmp1) > 0){
      config->Blocks[id].IO_In.Node = tmp;
      config->Blocks[id].IO_In.Adr = tmp1;
    }

    if(config->Blocks[id].fl & 0x8){
      printf("Block IO Out    (%2i:%2i)       | ", config->Blocks[id].IO_Out.Node, config->Blocks[id].IO_Out.Adr);
      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%i:%i", &tmp, &tmp1) > 0){
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

void modify_Switch(struct config * config, char cmd){
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
    if(sscanf(_cmd, "%i:%i:%i", &tmp, &tmp1, &tmp2) > 2){
      config->Switches[id].App.module = tmp;
      config->Switches[id].App.id = tmp1;
      config->Switches[id].App.type = tmp2;
    }

    printf("Switch Link Str (%2i:%2i:%2x)  | ", config->Switches[id].Str.module, config->Switches[id].Str.id, config->Switches[id].Str.type);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i:%i:%i", &tmp, &tmp1, &tmp2) > 2){
      config->Switches[id].Str.module = tmp;
      config->Switches[id].Str.id = tmp1;
      config->Switches[id].Str.type = tmp2;
    }

    printf("Switch Link Div (%2i:%2i:%2x)  | ", config->Switches[id].Div.module, config->Switches[id].Div.id, config->Switches[id].Div.type);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i:%i:%i", &tmp, &tmp1, &tmp2) > 2){
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
      printf("Switch IO%2i  (%2i:%2i)        | ", i, config->Blocks[id].IO_In.Node, config->Blocks[id].IO_In.Adr);
      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%i:%i", &tmp, &tmp1) > 0){
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

void modify_MSSwitch(struct config * config, char cmd){
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
      if(sscanf(_cmd, "%i:%i:%i", &tmp, &tmp1, &tmp2) > 2){
        config->MSSwitches[id].states[i].sideA.module = tmp;
        config->MSSwitches[id].states[i].sideA.id = tmp1;
        config->MSSwitches[id].states[i].sideA.type = tmp2;
      }
      
      printf(" - Link B (%2i:%2i:%2x)         | ",
                config->MSSwitches[id].states[i].sideB.module,
                config->MSSwitches[id].states[i].sideB.id,
                config->MSSwitches[id].states[i].sideB.type);

      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%i:%i:%i", &tmp, &tmp1, &tmp2) > 2){
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
      if(sscanf(_cmd, "%i:%i:%i", &tmp, &tmp1, &tmp2) > 1){
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

void modify_Station(struct config * config, char cmd){
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

int main(){
  init_logger("log_config.txt");
  set_level(MEMORY);

  char filename[40] = "configs/units/";
  int file;

  printf("Open module: ");

  scanf("%i", &file);

  sprintf(filename, "%s%i.bin", filename, file);

  FILE * fp = fopen(filename,"rb");

  struct config config;

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
    read_config(&config, fp);
    if(config.header.module != file){
      loggerf(CRITICAL, "INVALID MODULE ID");
      return 0;
    }
    fclose(fp);
  }
  print_config(&config);

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
      else if(strcmp(cmd, "St") == 0){
        modify_Station(&config, cmd1);
      }
      else if(strcmp(cmd, "N") == 0){
        modify_Node(&config, cmd1);
      }
    }
    else if(strcmp(cmd, "s") == 0){
      int size = calc_write_size(&config);
      printf("save %i bytes\n", size);
      write_from_conf(&config, filename);
    }
    else if(strcmp(cmd, "p") == 0){
      print_config(&config);
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

  //free station
  for(int i = 0; i < config.header.Stations; i++){
    _free(config.Stations[i].name);
    _free(config.Stations[i].blocks);
  }

  _free(config.Nodes);
  _free(config.Blocks);
  _free(config.Switches);
  _free(config.MSSwitches);
  _free(config.Stations);

  exit_logger();

  printf("Done\n");
  return 1;
}