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

#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/signals.h"
#include "IO.h"

#include "switchboard/station.h"

void read_module_Config(uint16_t M){
  char filename[40] = "";

  sprintf(filename, "%s%i.bin", ModuleConfigBasePath, M);

  loggerf(DEBUG, "Loading %s", filename);

  FILE * fp = fopen(filename,"rb");

  if(!fp){
    loggerf(ERROR, "Unable to open %s (errno %i)", filename, errno);
    return;
  }

  struct module_config * config = (struct module_config *)_calloc(1, struct module_config);

  char * header = (char *)_calloc(2, char);

  fread(header, 1, 1, fp);
  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char * buffer = (char *)_calloc(fsize, char);
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
  Units[M]->raw = (char *)_calloc(fsize+5, char);
  loggerf(INFO, "Reading Module %i (%d b)", M, fsize);
  memcpy(Units[M]->raw, raw, fsize-2);

  loggerf(DEBUG, "  Module nodes");

  for(int i = 0; i < config->header.IO_Nodes; i++){
    struct node_conf node = read_s_node_conf(buf_ptr);
    loggerf(CRITICAL, "TODO IMPLEMENT node data");
    Add_IO_Node(Units[M], node);
  }

  loggerf(DEBUG, "  Module Block");

  
  for(int i = 0; i < config->header.Blocks; i++){
    struct s_block_conf block = read_s_block_conf(buf_ptr);
    new Block(M, block);
  }

  loggerf(DEBUG, "  Module Switch");

  for(int i = 0; i < config->header.Switches; i++){
    struct switch_conf s = read_s_switch_conf(buf_ptr);
    struct s_switch_connect connect;

    connect.module = M;
    connect.id = s.id;
    connect.app.module = s.App.module; connect.app.id = s.App.id; connect.app.type = (enum link_types)s.App.type;
    connect.str.module = s.Str.module; connect.str.id = s.Str.id; connect.str.type = (enum link_types)s.Str.type;
    connect.div.module = s.Div.module; connect.div.id = s.Div.id; connect.div.type = (enum link_types)s.Div.type;

    Node_adr * Adrs = (Node_adr *)_calloc(s.IO & 0x0f, Node_adr);

    for(int i = 0; i < (s.IO & 0x0f); i++){
      Adrs[i].Node = s.IO_Ports[i].Node;
      Adrs[i].io = s.IO_Ports[i].Adr;
    }
    uint8_t * States = (uint8_t *)_calloc(2, uint8_t);
    States[0] = 1 + (0 << 1); //State 0 - Address 0 hight, address 1 low
    States[1] = 0 + (1 << 1); //State 1 - Address 1 hight, address 0 low

    new Switch(connect, s.det_block, s.IO & 0x0f, Adrs, States);

    _free(s.IO_Ports);
    _free(Adrs);
  }

  loggerf(DEBUG, "  Module MSSwitch");

  for(int i = 0; i < config->header.MSSwitches; i++){
    struct ms_switch_conf s = read_s_ms_switch_conf(buf_ptr);

    create_msswitch_from_conf(M, s);
  }

  loggerf(DEBUG, "  Module Signals");
  
  for(int i = 0; i < config->header.Signals; i++){
    struct signal_conf sig = read_s_signal_conf(buf_ptr);

    create_signal_from_conf(M, sig);

    _free(sig.stating);
    _free(sig.output);
  }

  for(int i = 0; i < config->header.Stations; i++){
    struct station_conf st = read_s_station_conf(buf_ptr);

    new Station(M, i, st.name, st.name_len, (enum Station_types)st.type, st.nr_blocks, st.blocks);

    _free(st.name);
    _free(st.blocks);
  }

  //Layout
  memcpy(&config->Layout_length, *buf_ptr, sizeof(uint16_t));
  Units[M]->Layout_length = config->Layout_length;
  *buf_ptr += sizeof(uint16_t) + 1;

  config->Layout = (char *)_calloc(config->Layout_length + 1, char);
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

  Units = (Unit **)_calloc(30, Unit *);
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