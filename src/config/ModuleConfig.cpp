#include <stdio.h>

#include "logger.h"
#include "mem.h"
#include "config.h"
#include "config/ModuleConfig.h"

ModuleConfig::ModuleConfig(char * filename){
  memset(this, 0, sizeof(ModuleConfig));

  strcpy(this->filename, filename);
  this->parsed = false;
}

ModuleConfig::~ModuleConfig(){
  loggerf(DEBUG, "Destructor ModuleConfig %s", this->filename);

  for(int i = 0; i < this->header.IO_Nodes; i++){
    _free(this->Nodes[i].data);
  }

  loggerf(TRACE, "  Module Block");

  
  for(int i = 0; i < this->header.Blocks; i++){}

  loggerf(TRACE, "  Module Switch");

  for(int i = 0; i < this->header.Switches; i++){
    _free(this->Switches[i].IO_Ports);
  }

  loggerf(TRACE, "  Module MSSwitch");

  for(int i = 0; i < this->header.MSSwitches; i++){
    _free(this->MSSwitches[i].states);
    _free(this->MSSwitches[i].IO_Ports);
  }

  loggerf(TRACE, "  Module Signals");
  
  for(int i = 0; i < this->header.Signals; i++){
    if(this->Signals[i].stating)
      _free(this->Signals[i].stating);
    if(this->Signals[i].output)
      _free(this->Signals[i].output);
  }

  loggerf(TRACE, "  Module Stations");

  for(int i = 0; i < this->header.Stations; i++){
    _free(this->Stations[i].name);
    _free(this->Stations[i].blocks);
  }

  _free(this->Nodes);
  _free(this->Blocks);
  _free(this->Switches);
  _free(this->MSSwitches);
  _free(this->Stations);
  _free(this->Signals);
  _free(this->Layout);
}

void ModuleConfig::newModule(uint8_t file, uint8_t connections){
  this->header.module = file;
  this->header.connections = connections;
  this->header.IO_Nodes = 0;
  this->header.Blocks = 0;
  this->header.Switches = 0;
  this->header.MSSwitches = 0;
  this->header.Signals = 0;
  this->header.Stations = 0;

  this->Nodes = 0;
  this->Blocks = 0;
  this->Switches = 0;
  this->MSSwitches = 0;
  this->Signals = 0;
  this->Stations = 0;
}

int ModuleConfig::read(){
  if(this->parsed){
    loggerf(WARNING, "Allready parsed, aborting!");
    return -1;
  }

  loggerf(INFO, "Reading Module Config from %s", filename);
  FILE * fp = fopen(filename, "rb");

  if(!fp){
    loggerf(ERROR, "Failed to open file '%s'", filename);
    return -1;
  }

  char * header = (char *)_calloc(2, char);

  fread(header, 1, 1, fp);

  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char * buffer = (char *)_calloc(fsize + 10, char);
  char * buffer_start = &buffer[0];
  fread(buffer, fsize, 1, fp);

  uint8_t ** buf_ptr = (uint8_t **)&buffer;

  *buf_ptr += 1;

  this->header = read_s_unit_conf(buf_ptr);

  if (header[0] != MODULE_CONF_VERSION) {
    loggerf(WARNING, "Module %i not correct version", this->header.module);
    return -1;
  }

  loggerf(DEBUG, "Module start reading %d, %d, %d, %d, %d, %d", this->header.IO_Nodes, this->header.Blocks, this->header.Switches, this->header.MSSwitches, this->header.Signals, this->header.Stations);

  this->Nodes = (struct node_conf *)_calloc(this->header.IO_Nodes, struct node_conf);
  this->Blocks = (struct s_block_conf *)_calloc(this->header.Blocks, struct s_block_conf);
  this->Switches = (struct switch_conf *)_calloc(this->header.Switches, struct switch_conf);
  this->MSSwitches = (struct ms_switch_conf *)_calloc(this->header.MSSwitches, struct ms_switch_conf);
  this->Signals = (struct signal_conf *)_calloc(this->header.Signals, struct signal_conf);
  this->Stations = (struct station_conf *)_calloc(this->header.Stations, struct station_conf);
  
  for(int i = 0; i < this->header.IO_Nodes; i++){
    this->Nodes[i]  = read_s_node_conf(buf_ptr);
  }

  for(int i = 0; i < this->header.Blocks; i++){
    this->Blocks[i]  = read_s_block_conf(buf_ptr);
  }

  for(int i = 0; i < this->header.Switches; i++){
    this->Switches[i]  = read_s_switch_conf(buf_ptr);
  }

  for(int i = 0; i < this->header.MSSwitches; i++){
    this->MSSwitches[i]  = read_s_ms_switch_conf(buf_ptr);
  }

  for(int i = 0; i < this->header.Signals; i++){
    this->Signals[i]  = read_s_signal_conf(buf_ptr);
  }

  for(int i = 0; i < this->header.Stations; i++){
    this->Stations[i]  = read_s_station_conf(buf_ptr);
  }

  //Layout
  memcpy(&this->Layout_length, *buf_ptr, sizeof(uint16_t));
  *buf_ptr += sizeof(uint16_t) + 1;

  this->Layout = (char *)_calloc(this->Layout_length + 1, char);
  memcpy(this->Layout, *buf_ptr, this->Layout_length);

  _free(header);
  _free(buffer_start);

  this->parsed = true;

  return 1;
}

int ModuleConfig::calc_size(){
  int size = 1; //header
  size += sizeof(struct s_unit_conf) + 1;

  //Nodes
  size += (sizeof(struct s_node_conf) + 2) * this->header.IO_Nodes;
  for(int i = 0; i < this->header.IO_Nodes; i++){
    size += (this->Nodes[i].size + 1) / 2;
  }


  //Blocks
  size += (sizeof(struct s_block_conf) + 1) * this->header.Blocks;

  //Switches
  for(int i = 0; i < this->header.Switches; i++){
    size += sizeof(struct s_switch_conf) + 1;
    size += sizeof(struct s_IO_port_conf) * (this->Switches[i].IO & 0xf) + 1;
  }

  //MSSwitches
  for(int i = 0; i < this->header.MSSwitches; i++){
    size += sizeof(struct s_ms_switch_conf) + 1;
    size += sizeof(struct s_ms_switch_state_conf) * this->MSSwitches[i].nr_states + 1;
    size += 2 * this->MSSwitches[i].IO + 1;
  }


  //Signals
  for(int i = 0; i <  this->header.Signals; i++){
    size += sizeof(struct s_signal_conf) + 1;
    size += this->Signals[i].output_len * sizeof(struct s_IO_port_conf) + 1;
    size += this->Signals[i].output_len * sizeof(struct s_IO_signal_event_conf) + 1;
    size += this->Signals[i].Switch_len * sizeof(struct s_Signal_DependentSwitch) + 1;
  }

  //Stations
  for(int i = 0; i <  this->header.Stations; i++){
    size += sizeof(struct s_station_conf) + 1;
    size += this->Stations[i].name_len + 1;
    size += this->Stations[i].nr_blocks + 1;
  }

  //Layout

  size += 3 + this->Layout_length+3;

  return size;
}

void ModuleConfig::write(){
  loggerf(DEBUG, "write_module_from_conf");
  int size = this->calc_size();

  loggerf(INFO, "Writing %i bytes", size);

  char * data = (char *)_calloc(size + 50, char);

  data[0] = MODULE_CONF_VERSION;

  char * p = &data[1];
  //Copy header
  memcpy(p, &this->header, sizeof(struct s_unit_conf));

  p += sizeof(struct s_unit_conf) + 1;

  //Copy Nodes
  for(int i = 0; i < this->header.IO_Nodes; i++){
    memcpy(p, &this->Nodes[i], sizeof(struct s_node_conf));

    p += sizeof(struct s_node_conf) + 1;
    memcpy(p, this->Nodes[i].data, (this->Nodes[i].size+1)/2);
    p += (this->Nodes[i].size+1)/2 + 1;
  }

  //Copy blocks
  for(int i = 0; i < this->header.Blocks; i++){
    memcpy(p, &this->Blocks[i], sizeof(struct s_block_conf));

    p += sizeof(struct s_block_conf) + 1;
  }

  //Copy Switches
  for(int i = 0; i < this->header.Switches; i++){
    memcpy(p, &this->Switches[i], sizeof(struct s_switch_conf));

    p += sizeof(struct s_switch_conf) + 1;

    for(int j = 0; j < (this->Switches[i].IO & 0x0f); j++){
      memcpy(p, &this->Switches[i].IO_Ports[j], sizeof(struct s_IO_port_conf));
      p += sizeof(struct s_IO_port_conf);
    }

    p += 1;
  }

  //Copy MMSwitches
  for(int i = 0; i < this->header.MSSwitches; i++){
    memcpy(p, &this->MSSwitches[i], sizeof(struct s_ms_switch_conf));

    p += sizeof(struct s_ms_switch_conf) + 1;

    for(int j = 0; j < this->MSSwitches[i].nr_states; j++){
      memcpy(p, &this->MSSwitches[i].states[j], sizeof(struct s_ms_switch_state_conf));
      p += sizeof(struct s_ms_switch_state_conf);
    }

    p += 1;

    for(int j = 0; j < this->MSSwitches[i].IO; j++){
      memcpy(p, &this->MSSwitches[i].IO_Ports[j], sizeof(struct s_IO_port_conf));
      p += sizeof(struct s_IO_port_conf);
    }

    p += 1;
  }

  //Copy Signals
  for(int i = 0; i < this->header.Signals; i++){
    memcpy(p, &this->Signals[i], sizeof(struct s_signal_conf));

    p += sizeof(struct s_signal_conf) + 1;

    memcpy(p, this->Signals[i].output, this->Signals[i].output_len * sizeof(struct s_IO_port_conf));
    p += this->Signals[i].output_len * sizeof(struct s_IO_port_conf) + 1;

    memcpy(p, this->Signals[i].stating, this->Signals[i].output_len * sizeof(struct s_IO_signal_event_conf));
    p += this->Signals[i].output_len * sizeof(struct s_IO_signal_event_conf) + 1;

    memcpy(p, this->Signals[i].Switches, this->Signals[i].Switch_len * sizeof(struct s_Signal_DependentSwitch));
    p += this->Signals[i].Switch_len * sizeof(struct s_Signal_DependentSwitch) + 1;
  }

  //Copy Stations
  for(int i = 0; i < this->header.Stations; i++){
    memcpy(p, &this->Stations[i], sizeof(struct s_station_conf));

    p += sizeof(struct s_station_conf) + 1;

    memcpy(p, this->Stations[i].blocks, this->Stations[i].nr_blocks);
    p += this->Stations[i].nr_blocks + 1;

    memcpy(p, this->Stations[i].name, this->Stations[i].name_len);
    p += this->Stations[i].name_len + 1;
  }

  //Copy Layout
  memcpy(p, &this->Layout_length, sizeof(uint16_t));
  p += sizeof(uint16_t) + 1;

  memcpy(p, this->Layout, this->Layout_length);
  p += this->Layout_length + 1;

  //Print output
  // print_hex(data, size);

  FILE * fp = fopen(this->filename, "wb");

  fwrite(data, size, 1, fp);

  fclose(fp);

  _free(data);
}


void print_link(char ** debug, struct s_link_conf link){
  const char * typestring[7] = {
    "R", "S", "s", "MA", "MB", "MAi", "MBi"
  };
  if(link.type == RAIL_LINK_C){
    *debug += sprintf(*debug, "C %2i:%2i  \t", link.module, link.id);
  }
  else if(link.type == RAIL_LINK_E){
    *debug += sprintf(*debug, "E        \t");
  }
  else{
    *debug += sprintf(*debug, "%2i:%2i:", link.module, link.id);
    if(link.type <= RAIL_LINK_MB_inside)
      *debug += sprintf(*debug, "%3s\t", typestring[link.type]);
    else if(link.type == RAIL_LINK_TT)
      *debug += sprintf(*debug, "%3s\t", "TT");
    else
      *debug += sprintf(*debug, "%2x \t", link.type);
  }
}

void print_Node(struct node_conf node){
  char debug[300];
  char * debugptr = debug;
  const char hexset[17] = "0123456789ABCDEF";

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
  const char * rail_types_string[5] = {
    "MAIN",
    "STATION",
    "NOSTOP",
    "TURNTABLE",
    "CROSSOVER"
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

  const char * typestring[3] = {"Crossing", "Turntable", "Traverse Table"};

  debugptr += sprintf(debugptr, "%i\t%i\t%i\t%2i -> [",
                Switch.id,
                Switch.det_block,
                Switch.nr_states,
                Switch.IO);
  
  if(Switch.IO)
    debugptr += sprintf(debugptr, "%2i:%2i", Switch.IO_Ports[0].Node, Switch.IO_Ports[0].Adr);

  for(int i = 1; i < Switch.IO; i++){
    debugptr += sprintf(debugptr, ", %2i:%2i", Switch.IO_Ports[i].Node, Switch.IO_Ports[i].Adr);
  }
  debugptr += sprintf(debugptr, "]\t%i - %s\n", Switch.type, typestring[Switch.type]);

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

  debugptr += sprintf(debugptr, "%i\t", signal.id);
  print_link(&debugptr, signal.Block);
  debugptr += sprintf(debugptr, "\t%i", signal.output_len);

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

  if(stations.parent != 0xFFFF){
    debugptr += sprintf(debugptr, "%i", stations.parent);
  }

  debugptr += sprintf(debugptr, "\t%i\t%9s\t",
                stations.type,
                stations.name);

  for(int j = 0; j < stations.nr_blocks; j++){
    debugptr += sprintf(debugptr, "%i ", stations.blocks[j]);
  }

  printf( "%s\n", debug);
}

void print_Layout(struct ModuleConfig * config){
  printf("Length: %i\n", config->Layout_length);
  printf("Data:\n%s\n\n", config->Layout);
}


void ModuleConfig::print(char ** cmds, uint8_t cmd_len){
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
    printf( "Modules:     %i\n", this->header.module);
    printf( "Connections: %i\n", this->header.connections);
    printf( "IO_Nodes:    %i\n", this->header.IO_Nodes);
    printf( "Blocks:      %i\n", this->header.Blocks);
    printf( "Switches:    %i\n", this->header.Switches);
    printf( "MSSwitches:  %i\n", this->header.MSSwitches);
    printf( "Signals:     %i\n", this->header.Signals);
    printf( "Stations:    %i\n", this->header.Stations);
  }
  
  if(mask & 2){
    printf( "IO Nodes\n");
    printf( "id\tSize\n");
    for(int i = 0; i < this->header.IO_Nodes; i++){
      print_Node(this->Nodes[i]);
    }
  }

  if(mask & 8){
    printf( "Block\n");
    printf( "id\ttype\t\tNext    \tPrev    \tMax_sp\tdir\tlen\tOneWay\tOut en\tIO_in\tIO_out\n");
    for(int i = 0; i < this->header.Blocks; i++){
      print_Block(this->Blocks[i]);
    }
  }
  
  if(mask & 16){
    printf( "Switch\n");
    printf( "id\tblock\tApp       \tStr       \tDiv       \tIO\tSpeed\n");
    for(int i = 0; i < this->header.Switches; i++){
      print_Switch(this->Switches[i]);
    }
  }

  if(mask & 32){
    printf( "MSSwitch\n");
    printf( "id\tblock\tstates\tIO\tSideA     \tSideB     \tSpeed\tSequence\t...\n");
    for(int i = 0; i < this->header.MSSwitches; i++){
      print_MSSwitch(this->MSSwitches[i]);
    }
  }

  if(mask & 64){
    printf( "Signals\n");
    printf( "id\tBlockID\tOutput Length\t\tOutput states\n");
    for(int i = 0; i < this->header.Signals; i++){
      print_Signals(this->Signals[i]);
    }
  }

  if(mask & 128){
    printf( "Station\n");
    printf( "id\tparent\ttype\tName\t\tblocks\n");
    for(int i = 0; i < this->header.Stations; i++){
      printf("%i\t", i);
      print_Stations(this->Stations[i]);
    }
  }
}