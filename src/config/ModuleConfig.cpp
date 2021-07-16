
#include <stdio.h>
#include "config/LayoutStructure.h"
#include "config/configReader.h"

#include "utils/logger.h"
#include "utils/mem.h"
#include "config/ModuleConfig.h"
#include "switchboard/links.h"


// void hexdump(void * data, int length){
//   printf("HEXDUMP (%x) %i:\n", (unsigned int)data, length);
//   char text[2000];
//   char * ptr = text;

//   for(int i = 0; i < length; i++){
//     ptr += sprintf(ptr, "%02x ", ((uint8_t *)data)[i]);
//     if((i % 16) == 15)
//       ptr += sprintf(ptr, "\n");
//   }

//   // f(INFO, file, line, (const char *)text);
//   printf("%s", text);
// }


ModuleConfig::ModuleConfig(char * _filename){
  memset(this, 0, sizeof(ModuleConfig));

  strcpy(filename, _filename);
  parsed = false;
}

// ModuleConfig::ModuleConfig(char * _filename, ModuleConfig * oC){
//   memset(this, 0, sizeof(ModuleConfig));

//   strcpy(filename, _filename);
//   parsed = true;

//   header = (struct configStruct_Unit *)_calloc(1, struct configStruct_Unit);
//   header->Module     = oC->header.module;
//   header->Connections = oC->header.connections;
//   header->IO_Nodes   = oC->header.IO_Nodes;
//   header->Blocks     = oC->header.Blocks;
//   header->Switches   = oC->header.Switches;
//   header->MSSwitches = oC->header.MSSwitches;
//   header->Signals    = oC->header.Signals;
//   header->Stations   = oC->header.Stations;

//   loggerf(DEBUG, "Module convert %d, %d, %d, %d, %d, %d", header->IO_Nodes, header->Blocks, header->Switches, header->MSSwitches, header->Signals, header->Stations);

//   Nodes      = (struct configStruct_Node *)_calloc(header->IO_Nodes, struct configStruct_Node);
//   Blocks     = (struct configStruct_Block *)_calloc(header->Blocks, struct configStruct_Block);
//   Switches   = (struct configStruct_Switch *)_calloc(header->Switches, struct configStruct_Switch);
//   MSSwitches = (struct configStruct_MSSwitch *)_calloc(header->MSSwitches, struct configStruct_MSSwitch);
//   Signals    = (struct configStruct_Signal *)_calloc(header->Signals, struct configStruct_Signal);
//   Stations   = (struct configStruct_Station *)_calloc(header->Stations, struct configStruct_Station);
  
//   for(int i = 0; i < header->IO_Nodes; i++){
//     // Config_read_Node(fileVersion, &Nodes[i], buf_ptr);
//     Nodes[i].Node = oC->Nodes[i].Node;
//     Nodes[i].size = oC->Nodes[i].size;

//     Nodes[i].data = (uint8_t *)_calloc(Nodes[i].size, uint8_t);
//     memcpy(Nodes[i].data, oC->Nodes[i].data, Nodes[i].size * sizeof(uint8_t));
//   }

//   for(int i = 0; i < header->Blocks; i++){
//     // Config_read_Block(fileVersion, &Blocks[i], buf_ptr);
//     Blocks[i].id     = oC->Blocks[i].id;
//     Blocks[i].type   = oC->Blocks[i].type;
//     Blocks[i].next.module   = oC->Blocks[i].next.module;
//     Blocks[i].next.id       = oC->Blocks[i].next.id;
//     Blocks[i].next.type     = oC->Blocks[i].next.type;
//     Blocks[i].prev.module   = oC->Blocks[i].prev.module;
//     Blocks[i].prev.id       = oC->Blocks[i].prev.id;
//     Blocks[i].prev.type     = oC->Blocks[i].prev.type;
//     Blocks[i].IO_In.Node    = oC->Blocks[i].IO_In.Node;
//     Blocks[i].IO_In.Port    = oC->Blocks[i].IO_In.Adr;
//     Blocks[i].IO_Out.Node   = oC->Blocks[i].IO_Out.Node;
//     Blocks[i].IO_Out.Port   = oC->Blocks[i].IO_Out.Adr;
//     Blocks[i].speed  = oC->Blocks[i].speed;
//     Blocks[i].length = oC->Blocks[i].length;
//     Blocks[i].fl     = oC->Blocks[i].fl;
//   }

//   for(int i = 0; i < header->Switches; i++){
//     // Config_read_Switch(fileVersion, &Switches[i], buf_ptr);
//     Switches[i].id        = oC->Switches[i].id;
//     Switches[i].det_block = oC->Switches[i].det_block;

//     Switches[i].App.module   = oC->Switches[i].App.module;
//     Switches[i].App.id       = oC->Switches[i].App.id;
//     Switches[i].App.type     = oC->Switches[i].App.type;
//     Switches[i].Str.module   = oC->Switches[i].Str.module;
//     Switches[i].Str.id       = oC->Switches[i].Str.id;
//     Switches[i].Str.type     = oC->Switches[i].Str.type;
//     Switches[i].Div.module   = oC->Switches[i].Div.module;
//     Switches[i].Div.id       = oC->Switches[i].Div.id;
//     Switches[i].Div.type     = oC->Switches[i].Div.type;

//     Switches[i].IO_length    = oC->Switches[i].IO_len;
//     Switches[i].IO_type      = oC->Switches[i].IO_type;

//     Switches[i].speed_Str    = oC->Switches[i].speed_Str;
//     Switches[i].speed_Div    = oC->Switches[i].speed_Div;
//     Switches[i].feedback_len = oC->Switches[i].feedback_len;

//     Switches[i].IO_Ports = (struct configStruct_IOport *)_calloc(Switches[i].IO_length, struct configStruct_IOport);
//     for(unsigned int j = 0; j < Switches[i].IO_length; j++){
//       Switches[i].IO_Ports[j].Node    = oC->Switches[i].IO_Ports[j].Node;
//       Switches[i].IO_Ports[j].Port    = oC->Switches[i].IO_Ports[j].Adr;
//     }

//     Switches[i].IO_Event = (uint8_t *)_calloc(Switches[i].IO_length * 2, uint8_t);
//     memcpy(Switches[i].IO_Event, oC->Switches[i].IO_events, Switches[i].IO_length * 2 * sizeof(uint8_t));

//     Switches[i].FB_Ports = (configStruct_IOport *)_calloc(Switches[i].feedback_len, struct configStruct_IOport);
//     for(unsigned int j = 0; j < Switches[i].feedback_len; j++){
//       Switches[i].FB_Ports[j].Node    = oC->Switches[i].FB_Ports[j].Node;
//       Switches[i].FB_Ports[j].Port    = oC->Switches[i].FB_Ports[j].Adr;
//     }

//     Switches[i].FB_Event = (uint8_t *)_calloc(Switches[i].feedback_len * 2, uint8_t);
//     memcpy(Switches[i].FB_Event, oC->Switches[i].FB_events, Switches[i].feedback_len * 2 * sizeof(uint8_t));
//   }

//   for(int i = 0; i < header->MSSwitches; i++){
//     MSSwitches[i].id        = oC->MSSwitches[i].id;
//     MSSwitches[i].det_block = oC->MSSwitches[i].det_block;
//     MSSwitches[i].type      = oC->MSSwitches[i].type;
//     MSSwitches[i].nr_states = oC->MSSwitches[i].nr_states;
//     MSSwitches[i].IO        = oC->MSSwitches[i].IO;

//     MSSwitches[i].states = (struct configStruct_MSSwitchState *)_calloc(MSSwitches[i].nr_states, struct configStruct_MSSwitchState);
//     for(unsigned int j = 0; j < MSSwitches[i].nr_states; j++){
        
//         MSSwitches[i].states[j].sideA.module = oC->MSSwitches[i].states[j].sideA.module;
//         MSSwitches[i].states[j].sideA.id     = oC->MSSwitches[i].states[j].sideA.id;
//         MSSwitches[i].states[j].sideA.type   = oC->MSSwitches[i].states[j].sideA.type;
//         MSSwitches[i].states[j].sideB.module = oC->MSSwitches[i].states[j].sideB.module;
//         MSSwitches[i].states[j].sideB.id     = oC->MSSwitches[i].states[j].sideB.id;
//         MSSwitches[i].states[j].sideB.type   = oC->MSSwitches[i].states[j].sideB.type;

//         MSSwitches[i].states[j].speed           = oC->MSSwitches[i].states[j].speed;
//         MSSwitches[i].states[j].dir             = oC->MSSwitches[i].states[j].dir;
//         MSSwitches[i].states[j].output_sequence = oC->MSSwitches[i].states[j].output_sequence;
//     }

//     MSSwitches[i].IO_Ports = (struct configStruct_IOport *)_calloc(MSSwitches[i].IO, struct configStruct_IOport);
//     for(unsigned int j = 0; j < MSSwitches[i].IO; j++){
//         MSSwitches[i].IO_Ports[j].Node = oC->MSSwitches[i].IO_Ports[j].Node;
//         MSSwitches[i].IO_Ports[j].Port = oC->MSSwitches[i].IO_Ports[j].Adr;
//     }
//   }

//   for(int i = 0; i < header->Signals; i++){
//     Signals[i].direction  = oC->Signals[i].direction;
//     Signals[i].id         = oC->Signals[i].id;
//     Signals[i].block.module = oC->Signals[i].Block.module;
//     Signals[i].block.id     = oC->Signals[i].Block.id;
//     Signals[i].block.type   = oC->Signals[i].Block.type;
//     Signals[i].output_len = oC->Signals[i].output_len;
//     Signals[i].Switch_len = oC->Signals[i].Switch_len;

//     Signals[i].output = (struct configStruct_IOport *)_calloc(Signals[i].output_len, struct configStruct_IOport);
//     for(uint8_t j = 0; j < Signals[i].output_len; j++){
//         Signals[i].output[j].Node = oC->Signals[i].output[j].Node;
//         Signals[i].output[j].Port = oC->Signals[i].output[j].Adr;
//     }

//     Signals[i].stating = (struct configStruct_SignalEvent *)_calloc(Signals[i].output_len, struct configStruct_SignalEvent);
//     for(uint8_t j = 0; j < Signals[i].output_len; j++){
//       memcpy(Signals[i].stating[j].event, oC->Signals[i].stating[j].event, 8 * sizeof(uint8_t));
//     }

//     Signals[i].Switches = (struct configStruct_SignalDependentSwitch *)_calloc(Signals[i].Switch_len, struct configStruct_SignalDependentSwitch);
//     for(uint8_t j = 0; j < Signals[i].Switch_len; j++){
//       Signals[i].Switches[j].type  = oC->Signals[i].Switches[j].type;
//       Signals[i].Switches[j].Sw    = oC->Signals[i].Switches[j].Sw;
//       Signals[i].Switches[j].state = oC->Signals[i].Switches[j].state;
//     }
//   }

//   for(int i = 0; i < header->Stations; i++){
//     Stations[i].type      = oC->Stations[i].type;
//     Stations[i].nr_blocks = oC->Stations[i].nr_blocks;
//     Stations[i].name_len  = oC->Stations[i].name_len;
//     Stations[i].reserved  = oC->Stations[i].reserved;
//     Stations[i].parent    = oC->Stations[i].parent;

//     Stations[i].blocks = (uint8_t *)_calloc(Stations[i].nr_blocks, uint8_t);
//     memcpy(Stations[i].blocks, oC->Stations[i].blocks, Stations[i].nr_blocks * 1 * sizeof(uint8_t));

//     Stations[i].name = (char *)_calloc(Stations[i].name_len, char);
//     memcpy(Stations[i].name, oC->Stations[i].name, Stations[i].name_len * 1 * sizeof(char));
//   }

//   //Layout
//   Layout = (struct configStruct_WebLayout *)_calloc(1, struct configStruct_WebLayout);

//   Layout->LayoutLength = oC->Layout_length;

//   Layout->Layout = (char *)_calloc(Layout->LayoutLength + 1, char);
//   memcpy(Layout->Layout, oC->Layout, Layout->LayoutLength);
//   // Config_read_WebLayout(fileVersion, &Layout, buf_ptr);

// }

ModuleConfig::~ModuleConfig(){
  loggerf(DEBUG, "Destructor ModuleConfig %s", this->filename);

  for(int i = 0; i < header->IO_Nodes; i++){
    _free(Nodes[i].config);
  }

  loggerf(TRACE, "  Module Block");

  
  for(int i = 0; i < header->Blocks; i++){}

  loggerf(TRACE, "  Module Switch");

  for(int i = 0; i < header->Switches; i++){
    _free(Switches[i].IO_Ports);
    _free(Switches[i].IO_Event);

    _free(Switches[i].FB_Ports);
    _free(Switches[i].FB_Event);
  }

  loggerf(TRACE, "  Module MSSwitch");

  for(int i = 0; i < header->MSSwitches; i++){
    _free(MSSwitches[i].states);
    _free(MSSwitches[i].IO_Ports);
  }

  loggerf(TRACE, "  Module Signals");
  
  for(int i = 0; i < header->Signals; i++){
    _free(Signals[i].stating);
    _free(Signals[i].output);
    _free(Signals[i].Switches);
  }

  loggerf(TRACE, "  Module Stations");

  for(int i = 0; i < header->Stations; i++){
    _free(Stations[i].name);
    _free(Stations[i].blocks);
  }

  _free(Nodes);
  _free(Blocks);
  _free(Switches);
  _free(MSSwitches);
  _free(Stations);
  _free(Signals);
  _free(buffer);

  _free(Layout->Layout);
  _free(Layout);

  _free(header);
}

void ModuleConfig::newModule(uint8_t file, uint8_t connections){
  header = (struct configStruct_Unit *)_calloc(1, struct configStruct_Unit);
  header->Module = file;
  header->Connections = connections;
  header->IO_Nodes = 0;
  header->Blocks = 0;
  header->Switches = 0;
  header->MSSwitches = 0;
  header->Signals = 0;
  header->Stations = 0;

  Layout = (struct configStruct_WebLayout *)_calloc(1, struct configStruct_WebLayout);
  Layout->LayoutLength = 0;
  // Layout->Layout = (char *)_calloc(Layout->LayoutLength + 1, char);

  Nodes = 0;
  Blocks = 0;
  Switches = 0;
  MSSwitches = 0;
  Signals = 0;
  Stations = 0;
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

  if (fileVersion > CONFIG_LAYOUTSTRUCTURE_LU_MAX_VERSION) {
    loggerf(WARNING, "Module Config not correct version (%s)", filename);
    return -1;
  }

  header = (struct configStruct_Unit *)_calloc(1, struct configStruct_Unit);

  Config_read_Unit(fileVersion, header, buf_ptr);

  loggerf(INFO, "Module %i start reading %d, %d, %d, %d, %d, %d", header->Module, header->IO_Nodes, header->Blocks, header->Switches, header->MSSwitches, header->Signals, header->Stations);

  Nodes      = (struct configStruct_Node *)_calloc(header->IO_Nodes, struct configStruct_Node);
  Blocks     = (struct configStruct_Block *)_calloc(header->Blocks, struct configStruct_Block);
  Switches   = (struct configStruct_Switch *)_calloc(header->Switches, struct configStruct_Switch);
  MSSwitches = (struct configStruct_MSSwitch *)_calloc(header->MSSwitches, struct configStruct_MSSwitch);
  Signals    = (struct configStruct_Signal *)_calloc(header->Signals, struct configStruct_Signal);
  Stations   = (struct configStruct_Station *)_calloc(header->Stations, struct configStruct_Station);
  
  for(int i = 0; i < header->IO_Nodes; i++){
    Config_read_Node(fileVersion, &Nodes[i], buf_ptr);
  }

  for(int i = 0; i < header->Blocks; i++){
    Config_read_Block(fileVersion, &Blocks[i], buf_ptr);
  }

  for(int i = 0; i < header->Switches; i++){
    Config_read_Switch(fileVersion, &Switches[i], buf_ptr);
  }

  for(int i = 0; i < header->MSSwitches; i++){
    Config_read_MSSwitch(fileVersion, &MSSwitches[i], buf_ptr);
  }

  for(int i = 0; i < header->Signals; i++){
    Config_read_Signal(fileVersion, &Signals[i], buf_ptr);
  }

  for(int i = 0; i < header->Stations; i++){
    Config_read_Station(fileVersion, &Stations[i], buf_ptr);
  }

  //Layout
  Layout = (struct configStruct_WebLayout *)_calloc(1, struct configStruct_WebLayout);
  Config_read_WebLayout(fileVersion, Layout, buf_ptr);

  parsed = true;
  
  _free(buffer);
  fclose(fp);

  return 1;
}

int ModuleConfig::calc_size(){
  int size = 1 + Config_write_size_Unit(header); //header

  //Nodes
  for(int i = 0; i < header->IO_Nodes; i++){
    size += Config_write_size_Node(&Nodes[i]);
  }

  //Blocks
  size += Config_write_size_Block(Blocks) * header->Blocks;

  //Switches
  for(int i = 0; i < header->Switches; i++){
    size += Config_write_size_Switch(&Switches[i]);
  }

  //MSSwitches
  for(int i = 0; i < header->MSSwitches; i++){
    size += Config_write_size_MSSwitch(&MSSwitches[i]);
  }


  //Signals
  for(int i = 0; i <  header->Signals; i++){
    size += Config_write_size_Signal(&Signals[i]);
  }

  //Stations
  for(int i = 0; i <  header->Stations; i++){
    size += Config_write_size_Station(&Stations[i]);
  }

  //Layout
  size += Config_write_size_WebLayout(Layout);

  return size;
}

void ModuleConfig::dump(){
  FILE * fp = fopen(filename, "wb");

  fwrite(buffer, buffer_len - 10, 1, fp);

  fclose(fp);
}

void ModuleConfig::write(){
  int size = calc_size();

  loggerf(DEBUG, "write_module_from_conf (%i bytes)", size);

  char * data = (char *)_calloc(size + 50, char);
  uint8_t * p = (uint8_t *)data;

  {
    uint8_t tmp = CONFIG_LAYOUTSTRUCTURE_LU_MAX_VERSION;
    Config_write_uint8_t(&tmp, &p);
  }

  //Copy header
  // memcpy(p, &this->header, sizeof(struct s_unit_conf));
  Config_write_Unit(header, &p);

  //Copy Nodes
  for(int i = 0; i < header->IO_Nodes; i++){
    Config_write_Node(&Nodes[i], &p);
  }

  //Copy blocks
  for(int i = 0; i < header->Blocks; i++){
    Config_write_Block(&Blocks[i], &p);
  }

  //Copy Switches
  for(int i = 0; i < header->Switches; i++){
    Config_write_Switch(&Switches[i], &p);
  }

  //Copy MMSwitches
  for(int i = 0; i < header->MSSwitches; i++){
    Config_write_MSSwitch(&MSSwitches[i], &p);
  }

  //Copy Signals
  for(int i = 0; i < header->Signals; i++){
    Config_write_Signal(&Signals[i], &p);
  }

  //Copy Stations
  for(int i = 0; i < header->Stations; i++){
    Config_write_Station(&Stations[i], &p);
  }

  //Copy Layout
  Config_write_WebLayout(Layout, &p);

  //Print output
  // print_hex(data, size);

  FILE * fp = fopen(filename, "wb");

  fwrite(data, size, 1, fp);

  fclose(fp);

  _free(data);
}


void print_link(char ** debug, struct configStruct_RailLink link){
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

void print_Node(struct configStruct_Node node){
  char debug[3000];
  char * debugptr = &debug[0];
  // const char hexset[17] = "0123456789ABCDEF";

  debugptr += sprintf(debugptr, "%2i (%2i)\n\t\t00\t01\t02\t03\t04\t05\t06\t07\t08\t09\t0A\t0B\t0C\t0D\t0E\t0F\n\t00\t", node.Node, node.ports);
  // printf("%2i (%2i)\n\t\t01\t02\t03\t04\t05\t06\t07\t08\t09\t0A\t0B\t0C\t0D\t0E\t0F\n\t\t", node.Node, node.ports);

  for(int j = 0; j < node.ports; j++){
    debugptr += sprintf(debugptr, "%x %x %c\t", node.config[j].type, node.config[j].defaultState, node.config[j].inverted ? 'I' : ' ');
    // printf("%x %x %c\t", node.config[j].type, node.config[j].defaultState, node.config[j].inverted ? 'I' : ' ');

    if(j % 16 == 15){
      debugptr += sprintf(debugptr, "\n\t%2x\t", (j + 1) / 16);
      // printf("\n\t%2x\t", j / 16);
    }
  }

  printf("%s\n\n", debug);
}

void print_Block(struct configStruct_Block block){
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
                block.IOdetection.Node, block.IOdetection.Port,
                block.IOpolarity.Node, block.IOpolarity.Port);

  printf( "%s\n", debug);
}

void print_Switch(struct configStruct_Switch Switch){
  char debug[300];
  char * debugptr = debug;

  debugptr += sprintf(debugptr, "%i\t%i\t",
                Switch.id,
                Switch.det_block);
  print_link(&debugptr, Switch.App);
  print_link(&debugptr, Switch.Str);
  print_link(&debugptr, Switch.Div);
  debugptr += sprintf(debugptr, "%x\t%i %i",
                Switch.IO_length,
                Switch.speed_Str, Switch.speed_Div);

  for(int j = 0; j < Switch.IO_length; j++){
    debugptr += sprintf(debugptr, "\t%i:%i", Switch.IO_Ports[j].Node, Switch.IO_Ports[j].Port);
  }

  printf( "%s\n", debug);
}

void print_MSSwitch(struct configStruct_MSSwitch Switch){
  char debug[1000];
  char * debugptr = debug;

  const char * typestring[3] = {"Crossing", "Turntable", "Traverse Table"};

  debugptr += sprintf(debugptr, "%i\t%i\t%i\t%2i -> [",
                Switch.id,
                Switch.det_block,
                Switch.nr_states,
                Switch.IO);
  
  if(Switch.IO)
    debugptr += sprintf(debugptr, "%2i:%2i", Switch.IO_Ports[0].Node, Switch.IO_Ports[0].Port);

  for(int i = 1; i < Switch.IO; i++){
    debugptr += sprintf(debugptr, ", %2i:%2i", Switch.IO_Ports[i].Node, Switch.IO_Ports[i].Port);
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

void print_Signals(struct configStruct_Signal signal){
  char debug[400];
  char * debugptr = debug;

  debugptr += sprintf(debugptr, "%i\t", signal.id);
  print_link(&debugptr, signal.block);
  debugptr += sprintf(debugptr, "%i", signal.output_len);

  for(int i = 0;i < signal.output_len; i++){
    debugptr += sprintf(debugptr, "\n\t\t\t%02i:%02i - ", signal.output[i].Node, signal.output[i].Port);
    for(int j = 0; j < 8; j++){
      debugptr += sprintf(debugptr, "%i ", signal.stating[i].event[j]);
    }
  }

  printf("%s\n", debug);
}

void print_Stations(struct configStruct_Station stations){
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

void print_Layout(struct configStruct_WebLayout * config){
  printf("Length: %i\n", config->LayoutLength);
  printf("Data:\n%s\n\n", config->Layout);
}

void ModuleConfig::print(char ** cmds, uint8_t cmd_len){
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
    else if(strcmp(cmds[i], "-L") == 0){
      mask |= 256;
    }
    else if(strcmp(cmds[i], "-A") == 0){
      mask |= 0x1FF;
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
    printf( "Modules:     %i\n", header->Module);
    printf( "Connections: %i\n", header->Connections);
    printf( "IO_Nodes:    %i\n", header->IO_Nodes);
    printf( "Blocks:      %i\n", header->Blocks);
    printf( "Switches:    %i\n", header->Switches);
    printf( "MSSwitches:  %i\n", header->MSSwitches);
    printf( "Signals:     %i\n", header->Signals);
    printf( "Stations:    %i\n", header->Stations);
  }
  
  if(mask & 2){
    printf( "IO Nodes\n");
    printf( "id\tSize\n");
    for(int i = 0; i < header->IO_Nodes; i++){
      print_Node(Nodes[i]);
    }
  }

  if(mask & 8){
    printf( "Block\n");
    printf( "id\ttype\t\tNext    \tPrev    \tMax_sp\tdir\tlen\tOneWay\tOut en\tIO_in\tIO_out\n");
    for(int i = 0; i < header->Blocks; i++){
      print_Block(Blocks[i]);
    }
  }
  
  if(mask & 16){
    printf( "Switch\n");
    printf( "id\tblock\tApp       \tStr       \tDiv       \tIO\tSpeed\n");
    for(int i = 0; i < header->Switches; i++){
      print_Switch(Switches[i]);
    }
  }

  if(mask & 32){
    printf( "MSSwitch\n");
    printf( "id\tblock\tstates\tIO\tSideA     \tSideB     \tSpeed\tSequence\t...\n");
    for(int i = 0; i < header->MSSwitches; i++){
      print_MSSwitch(MSSwitches[i]);
    }
  }

  if(mask & 64){
    printf( "Signals\n");
    printf( "id\tBlockID\t\tOutput Length\tOutput states\n");
    for(int i = 0; i < header->Signals; i++){
      print_Signals(Signals[i]);
    }
  }

  if(mask & 128){
    printf( "Station\n");
    printf( "id\tparent\ttype\tName\t\tblocks\n");
    for(int i = 0; i < header->Stations; i++){
      printf("%i\t", i);
      print_Stations(Stations[i]);
    }
  }

  if(mask & 256){
    printf( "Layout\n");
    print_Layout(Layout);
  }
}
