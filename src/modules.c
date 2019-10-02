#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#include "modules.h"
#include "system.h"
#include "logger.h"
#include "mem.h"
#include "config.h"

#include "rail.h"
#include "switch.h"
#include "signals.h"
#include "IO.h"

int unit_len;
Unit ** Units;

void Create_Unit(uint16_t M, uint8_t Nodes, char points){
  Unit * Z = _calloc(1, Unit);

  if(M < unit_len){
    Units[M] = Z;
  }
  else{
    loggerf(CRITICAL, "NEED TO EXPAND UNITS");
    return;
  }

  Z->module = M;

  Z->connections_len = points;
  Z->connection = _calloc(points, Unit *);

  Z->IO_Nodes = Nodes;
  Z->Node = _calloc(Z->IO_Nodes, IO_Node);

  Z->block_len = 8;
  Z->B = _calloc(Z->block_len, Block);

  Z->switch_len = 8;
  Z->Sw = _calloc(Z->switch_len, Switch);

  Z->msswitch_len = 8;
  Z->MSSw = _calloc(Z->switch_len, MSSwitch);

  Z->signal_len = 8;
  Z->Sig = _calloc(Z->signal_len, Signal);

  Z->station_len = 8;
  Z->St = _calloc(Z->station_len, Station);
}

void * Clear_Unit(Unit * U){
  printf("Clearing module %i\n", U->module);

  //Clear unit connections array
  _free(U->connection);

  //Clear IO
  for(int j = 0; j < U->IO_Nodes; j++){
    for(int k =0; k < U->Node[j].io_ports; k++){
      _free(U->Node[j].io[k]);
    }
    _free(U->Node[j].io);
  }
  _free(U->Node);

  //clear Segments
  loggerf(DEBUG,"Clear segments (%i)",U->block_len);
  for(int j = 0; j < U->block_len; j++){
    if(!U->B[j])
      continue;

    printf("- Block %i\n",j);
    U->B[j] = Clear_Block(U->B[j]);;
  }
  _free(U->B);

  //clear Switches
  if(U->Sw){
    loggerf(DEBUG,"Clear switches (%i)",U->switch_len);
    for(int j = 0; j <= U->switch_len; j++){
      if(!U->Sw[j])
        continue;
      printf("- Switch %i\n",j);

      U->Sw[j] = Clear_Switch(U->Sw[j]);
    }
    _free(U->Sw);
  }
  //clear Mods
  for(int j = 0;j<=U->msswitch_len;j++){
    if(U->MSSw[j]){
      printf("- Mod %i\n",j);
      U->MSSw[j] = Clear_MSSwitch(U->MSSw[j]);
    }
  }
  _free(U->MSSw);

  //clear Signals
  if(U->Sig){
    for(int j = 0;j<=U->signal_len;j++){
      if(!U->Sig[j])
        continue;
      printf("- Signal %i\n",j);
      U->Sig[j] = Clear_Signal(U->Sig[j]);
    }
  }

  _free(U->Sig);
  //clear Stations
  if(U->St){
    for(int j = 0;j<=U->station_len;j++){
      if(!U->St[j])
        continue;
      printf("- Station %i\n",j);
      U->St[j] = Clear_Station(U->St[j]);
    }
    _free(U->St);
  }

  _free(U->raw);
  _free(U->Layout);

  printf("- Unit %i\n", U->module);
  _free(U);
  U = 0;
  printf("\t Cleared!\n");
}

void read_module_Config(uint16_t M){
  char filename[40] = "";

  sprintf(filename, "%s%i.bin", ModuleConfigBasePath, M);

  loggerf(DEBUG, "Loading %s", filename);

  FILE * fp = fopen(filename,"rb");

  if(!fp){
    loggerf(ERROR, "Unable to open %s (errno %i)", filename, errno);
    return;
  }

  struct module_config * config = _calloc(1, struct module_config);

  char * header = _calloc(2, char);

  fread(header, 1, 1, fp);
  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char * buffer = _calloc(fsize, char);
  char * buf_start = &buffer[0];
  fread(buffer, fsize, 1, fp);

  // print_hex(buffer, fsize);

  uint8_t ** buf_ptr = (uint8_t **)&buffer;

  char * raw = &buffer[2];

  *buf_ptr += 1;

  config->header = read_s_unit_conf(buf_ptr);

  if (header[0] != MODULE_CONF_VERSION) {
    loggerf(ERROR, "Module %i not correct version, please update using the config_reader", config->header.module);
    return;
  }


  Create_Unit(M, config->header.IO_Nodes, config->header.connections);

  //Raw copy
  Units[M]->raw_length = fsize-2;
  Units[M]->raw = _calloc(fsize+5, char);
  loggerf(INFO, "Reading Module %i (%d b)", M, fsize);
  memcpy(Units[M]->raw, raw, fsize-2);

  for(int i = 0; i < config->header.IO_Nodes; i++){
    struct s_node_conf node = read_s_node_conf(buf_ptr);
    Add_IO_Node(Units[M], node.Node, node.size);
  }

  
  for(int i = 0; i < config->header.Blocks; i++){
    struct s_block_conf block = read_s_block_conf(buf_ptr);
    Create_Block(M, block);
  }

  for(int i = 0; i < config->header.Switches; i++){
    struct switch_conf s = read_s_switch_conf(buf_ptr);
    struct s_switch_connect connect;

    connect.module = M;
    connect.id = s.id;
    connect.app.module = s.App.module; connect.app.id = s.App.id; connect.app.type = s.App.type;
    connect.str.module = s.Str.module; connect.str.id = s.Str.id; connect.str.type = s.Str.type;
    connect.div.module = s.Div.module; connect.div.id = s.Div.id; connect.div.type = s.Div.type;

    Node_adr * Adrs = _calloc(s.IO & 0x0f, Node_adr);

    for(int i = 0; i < (s.IO & 0x0f); i++){
      Adrs[i].Node = s.IO_Ports[i].Node;
      Adrs[i].io = s.IO_Ports[i].Adr;
    }
    uint8_t * States = _calloc(2, _Bool *);
    States[0] = 1 + (0 << 1); //State 0 - Address 0 hight, address 1 low
    States[1] = 0 + (1 << 1); //State 1 - Address 1 hight, address 0 low

    Create_Switch(connect, s.det_block, s.IO & 0x0f, Adrs, States);

    _free(s.IO_Ports);
    _free(Adrs);
  }

  for(int i = 0; i < config->header.MSSwitches; i++){
    struct ms_switch_conf s = read_s_ms_switch_conf(buf_ptr);

    create_msswitch_from_conf(M, s);
  }
  
  for(int i = 0; i < config->header.Signals; i++){
    struct signal_conf sig = read_s_signal_conf(buf_ptr);

    create_signal_from_conf(M, sig);

    _free(sig.stating);
    _free(sig.output);
  }

  for(int i = 0; i < config->header.Stations; i++){
    struct station_conf st = read_s_station_conf(buf_ptr);

    Create_Station(M, i, st.name, st.name_len, (enum Station_types)st.type, st.nr_blocks, st.blocks);

    _free(st.name);
    _free(st.blocks);
  }

  //Layout
  memcpy(&config->Layout_length, *buf_ptr, sizeof(uint16_t));
  Units[M]->Layout_length = config->Layout_length;
  *buf_ptr += sizeof(uint16_t) + 1;

  config->Layout = _calloc(config->Layout_length + 1, uint8_t);
  memcpy(config->Layout, *buf_ptr, config->Layout_length);
  Units[M]->Layout = config->Layout;

  _free(header);
  _free(config);
  _free(buf_start);
  fclose(fp);
}

void write_module_Config(uint16_t M){
  loggerf(ERROR, "IMPLEMENT");
}

void load_module_Configs(){
  DIR *d;

  struct dirent *dir;

  d = opendir(ModuleConfigBasePath);

  char type[5] = "";
  int moduleID;
  uint8_t moduleID_list[255];
  memset(moduleID_list, 0, 255);

  Units = _calloc(30, Unit *);
  unit_len = 30;

  if (d)
  {
    while ((dir = readdir(d)) != NULL)
    {
      if(sscanf(dir->d_name, "%i.%s", &moduleID, type) > 1 && strcmp(type, "bin") == 0){
        int i = 0;
        uint8_t tmp = 0;
        uint8_t tmp2 = 0;
        // Add to list and sort
        for(; i<255; i++){
          if(moduleID_list[i] == 0){
            moduleID_list[i] = moduleID;
            break;
          }
          else if(moduleID_list[i] > moduleID){
            tmp = moduleID_list[i];
            moduleID_list[i] = moduleID;
            break;
          }
        }
        i++;
        if(tmp != 0){
          for(; i<255; i++){
            tmp2 = moduleID_list[i];
            moduleID_list[i] = tmp;
            tmp = tmp2;
            if(tmp == 0){
              break;
            }
          }
        }
      }
    }
    closedir(d);
  }
  
  for(uint8_t i = 0; i<255; i++){
    if(moduleID_list[i] == 0){
      break;
    }
    read_module_Config((uint16_t)moduleID_list[i]);
    // WS_Track_LayoutDataOnly((int)moduleID_list[i], 0);
  }

  SYS->modules_loaded = 1;

  return;
}

void unload_module_Configs(){
  SYS->modules_loaded = 0;

  for(int i = 0;i<unit_len;i++){
    if(Units[i]){
      Units[i] = Clear_Unit(Units[i]);
    }
  }
  _free(Units);
  _free(stations);
}
