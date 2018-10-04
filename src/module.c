#include "system.h"
#include "mem.h"
#include "logger.h"
#include "config.h"

#include "train.h"

#include "module.h"

#include "algorithm.h"

#include "websocket_msg.h"
#include "websocket_control.h"

int unit_len;
Unit ** Units;

struct rail_link EMPTY_BL(){
  struct rail_link A;
  A.module = 0;
  A.id = 0;
  A.type = 'e';
  return A;
}

void setup_JSON(int arr[], int arr2[], int size, int size2){
  char setup_data[100];

  setup_data[0] = 2;
  int setup_data_l = 2 + size + size2;

  int i;

  for(i = 2; (i-2) < size; i++){
    setup_data[i] = arr[i-2];
  }

  if(size2 != 0){
    setup_data[1] = size;

    for(; (i-2 - size) < size2; i++){
      setup_data[i] = arr2[i-2-size];
    }
  }

  printf("setup_data %x: %s\n", setup_data_l, setup_data);
}

void Create_Unit(int module, uint8_t Nodes, char points){
  Unit * Z = _calloc(1, Unit);

  if(module < unit_len){
    Units[module] = Z;
  }
  else{
    loggerf(CRITICAL, "NEED TO EXPAND UNITS");
    return;
  }

  Z->module = module;

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
}

// void Unit_expand_IO(_Bool type, Unit * U){
//   loggerf(ERROR, "Not supported");
//   return;
// }

void join(struct rail_link Adr, struct rail_link link){
  printf("LINK %c%i:%i => %c%i:%i\t",Adr.type,Adr.module,Adr.id,link.type,link.module,link.id);
  if(Adr.type == RAIL_LINK_R && Units[Adr.module]->B[Adr.id]){
    Units[Adr.module]->B[Adr.id]->prev = link;
    printf("D\n");
  }else if(Adr.type == RAIL_LINK_S){
    Units[Adr.module]->Sw[Adr.id]->app = link;
  }else if(Adr.type == RAIL_LINK_s){
    if(Units[Adr.module]->Sw[Adr.id]->div.p == 0){
      Units[Adr.module]->Sw[Adr.id]->div = link;
    }else{
      Units[Adr.module]->Sw[Adr.id]->str = link;
    }
  }
}

void init_modules(){
  Units = _calloc(30, Unit *);
  unit_len = 30;
}

void clear_Modules(){

  for(int i = 0;i<unit_len;i++){
    if(Units[i]){
      printf("Clearing module %i\n",i);
      //Clear unit connections array
      _free(Units[i]->connection);

      //Clear IO
      for(int j = 0; j < Units[i]->IO_Nodes; j++){
        for(int k =0; k < Units[i]->Node[j].io_ports; k++){
          _free(Units[i]->Node[j].io[k]);
        }
        _free(Units[i]->Node[j].io);
      }
      _free(Units[i]->Node);

      //clear Segments
      loggerf(DEBUG,"Clear segments (%i)",Units[i]->block_len);
      for(int j = 0;j<=Units[i]->block_len;j++){
        if(!Units[i]->B[j])
          continue;
        printf("- Block %i\n",j);
        _free(Units[i]->B[j]->Sw);
        _free(Units[i]->B[j]->MSSw);

        _free(Units[i]->B[j]);
        Units[i]->B[j] = NULL;
      }
      _free(Units[i]->B);

      //clear Switches
      if(Units[i]->Sw){
        loggerf(DEBUG,"Clear switches (%i)",Units[i]->switch_len);
        for(int j = 0; j <= Units[i]->switch_len; j++){
          if(!Units[i]->Sw[j])
            continue;
          printf("- Switch %i\n",j);
          _free(Units[i]->Sw[j]->feedback);
          _free(Units[i]->Sw[j]->IO);
          _free(Units[i]->Sw[j]->IO_states);
          _free(Units[i]->Sw[j]->links);
          _free(Units[i]->Sw[j]->preferences);

          _free(Units[i]->Sw[j]);
          Units[i]->Sw[j] = NULL;
        }
        _free(Units[i]->Sw);
      }
      //clear Mods
      for(int j = 0;j<=Units[i]->msswitch_len;j++){
        if(Units[i]->MSSw[j]){
          printf("- Mod %i\n",j);
          _free(Units[i]->MSSw[j]);
          Units[i]->MSSw[j] = NULL;
        }
      }
      _free(Units[i]->MSSw);

      //clear Signals
      if(Units[i]->Sig){
        for(int j = 0;j<=Units[i]->signal_len;j++){
          if(!Units[i]->Sig[j])
            continue;
          printf("- Signal %i\n",j);
          _free(Units[i]->Sig[j]);
          Units[i]->Sig[j] = NULL;
        }
      }

      _free(Units[i]->Sig);
      //clear Stations
      if(Units[i]->St){
        for(int j = 0;j<=Units[i]->station_len;j++){
          if(!Units[i]->St[j])
            continue;
          printf("- Station %i\n",j);

          for(int k = 0; k <= Units[i]->St[j]->switches_len; k++){
            if(Units[i]->St[j]->switch_link[k])
              _free(Units[i]->St[j]->switch_link[k]);
          }
          _free(Units[i]->St[j]->switch_link);

          _free(Units[i]->St[j]->name);
          _free(Units[i]->St[j]->blocks);

          _free(Units[i]->St[j]);
          Units[i]->St[j] = NULL;
        }
        _free(Units[i]->St);
      }

      printf("- Unit %i\n",i);
      _free(Units[i]);
      Units[i] = 0;
      printf("\t Cleared!\n");
    }
  }
  _free(Units);
  _free(stations);
}

void Modules(int m){
  loggerf(CRITICAL, "DEPRICATED (%i)", m);
  return;
}

// Binary structured files
// #define MAXBUFLEN 1000000
// struct test_struct {
//  char a;
//  char b;
//  char c;
//  char d;
//  char e;
//  char f;
// };

// char source[MAXBUFLEN + 1];
// memset(source,0,MAXBUFLEN+1);

// FILE *fp = fopen("test_bin.bin", "r");
// if (fp != NULL) {
//     size_t newLen = fread(source, sizeof(char), MAXBUFLEN, fp);
//     if ( ferror( fp ) != 0 ) {
//         fputs("Error reading file", stderr);
//     }

//     fclose(fp);
// }

//  struct test_struct * test = &source[0];

void free_modules(){
  logger("FREE_MODULES IMPLEMENT",CRITICAL);
  clear_Modules();
}

void LoadModuleFromConfig(int M){
  char filename[40] = "configs/units/";

  sprintf(filename, "%s%i.bin", filename, M);

  FILE * fp = fopen(filename,"rb");

  struct module_config * config = calloc(1, sizeof(struct module_config));

  char * header = calloc(2, sizeof(char));

  fread(header, 1, 1, fp);
  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char * buffer = calloc(fsize, sizeof(char));
  char * buf_start = &buffer[0];
  fread(buffer, fsize, 1, fp);

  print_hex(buffer, fsize);

  uint8_t ** buf_ptr = (uint8_t **)&buffer;

  *buf_ptr += 1;

  config->header = read_s_unit_conf(buf_ptr);

  if (header[0] != MODULE_CONF_VERSION) {
    loggerf(ERROR, "Module %i not correct version, please update using the config_reader", config->header.module);
    return;
  }


  Create_Unit(M, config->header.IO_Nodes, config->header.connections);

  for(int i = 0; i < config->header.IO_Nodes; i++){
    struct s_node_conf node = read_s_node_conf(buf_ptr);
    Add_IO_Node(Units[M], node.Node, node.size);
  }

  
  for(int i = 0; i < config->header.Blocks; i++){
    struct s_block_conf block = read_s_block_conf(buf_ptr);
    create_block(M, block);
  }

  for(int i = 0; i < config->header.Switches; i++){
    struct switch_conf s = read_s_switch_conf(buf_ptr);
    struct switch_connect connect;

    connect.module = M;
    connect.id = s.id;
    connect.app.module = s.App.module; connect.app.id = s.App.id; connect.app.type = s.App.type;
    connect.str.module = s.Str.module; connect.str.id = s.Str.id; connect.str.type = s.Str.type;
    connect.div.module = s.Div.module; connect.div.id = s.Div.id; connect.div.type = s.Div.type;

    Node_adr * Adrs = _calloc(s.IO & 0x0f, sizeof(Node_adr));

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

  config->MSSwitches = calloc(config->header.MSSwitches, sizeof(struct ms_switch_conf));

  for(int i = 0; i < config->header.MSSwitches; i++){
    config->MSSwitches[i]  = read_s_ms_switch_conf(buf_ptr);
  }

  config->Stations = calloc(config->header.Stations, sizeof(struct station_conf));

  for(int i = 0; i < config->header.Stations; i++){
    struct station_conf st = read_s_station_conf(buf_ptr);

    _free(st.name);
    _free(st.blocks);
  }

  free(header);
  free(buf_start);
  fclose(fp);
}

void JoinModules(){
  if((_SYS->_STATE & STATE_Modules_Loaded) == 0){
    //No track loaded
    return;
  }
  printf("Ready to join modules\n");

  struct ConnectList List;
  List.length = 0;
  List.list_index = 8;
  List.R_L = _calloc(8, struct rail_link *);


  int i = 0;
  int x = 0;
  int max_j = init_connect_Algor(&List);
  int cur_j = max_j;
  int prev_j = max_j;
  while((_SYS->_STATE & STATE_Modules_Coupled) == 0){
    cur_j = connect_Algor(&List);
    if(i > 30){
      printf(" (%02i/%02i)\n",cur_j,max_j);
      i = 0;
      x++;
    }
    if(prev_j == cur_j){
      printf(".");
    }else{
      printf("+");

      char data[20];
      data[0] = 0x82;
      data[1] = cur_j;
      data[2] = max_j;
      int k = 3;
      for(int j = 0;j<unit_len;j++){
        if(Units[j]){
          data[k++] = j;
        }
      }
      ws_send_all(data,k,0x10);
    }
    i++;
    usleep(10000);
    prev_j = cur_j;

    if(i == 15){
    if(x == 1){
      Units[20]->B[5]->blocked = 1;
      Units[22]->B[0]->blocked = 1;
      printf("\n1\n");
    }else if(x == 2){
      Units[21]->B[3]->blocked = 1;
      Units[23]->B[0]->blocked = 1;

      Units[20]->B[5]->blocked = 0;
      Units[22]->B[0]->blocked = 0;
      printf("\n2\n");
    }else if(x == 3){
      Units[20]->B[0]->blocked = 1;
      Units[23]->B[1]->blocked = 1;

      Units[21]->B[3]->blocked = 0;
      Units[23]->B[0]->blocked = 0;
      printf("\n3\n");
    }else if(x == 4){
      Units[21]->B[0]->blocked = 1;
      Units[22]->B[1]->blocked = 1;

      Units[20]->B[0]->blocked = 0;
      Units[23]->B[1]->blocked = 0;
      printf("\n4\n");
    }else if(x == 5){
      Units[21]->B[0]->blocked = 0;
      Units[22]->B[1]->blocked = 0;
      printf("\n41\n");
    }else if(x == 6){
      printf("\nend\n");
    }
    else if(x == 10){
      // _SYS_change(STATE_Modules_Coupled,1);
    }
    }
    //IF ALL JOINED
    //BREAK
  }
  
  Units[21]->B[0]->blocked = 0;
  Units[22]->B[1]->blocked = 0;

  for(int i = 0;i<List.length;i++){
    if(List.R_L[i]->type == 'S'){
      if(((Switch *)List.R_L[i]->p)->app.type == RAIL_LINK_C){
        ((Switch *)List.R_L[i]->p)->app.type = 0;
      }else if(((Switch *)List.R_L[i]->p)->str.type == RAIL_LINK_C){
        ((Switch *)List.R_L[i]->p)->str.type = 0;
      }else if(((Switch *)List.R_L[i]->p)->div.type == RAIL_LINK_C){
        ((Switch *)List.R_L[i]->p)->div.type = 0;
      }
    }else if(((Block *)List.R_L[i]->p)){
      if(((Block *)List.R_L[i]->p)->next.type == RAIL_LINK_C){
        ((Block *)List.R_L[i]->p)->next.type = 0;
      }else if(((Block *)List.R_L[i]->p)->prev.type == RAIL_LINK_C){
        ((Block *)List.R_L[i]->p)->prev.type = 0;
      }
    }

    _free(List.R_L[i]);
  }
  _free(List.R_L);

  WS_Track_Layout();

}

void ConnectModulePoints(Block * A,Block * B){
  uint8_t anchor_A,rail_A,anchor_B,rail_B;
  if(A->next.type == RAIL_LINK_C){
    anchor_A = A->next.module;
    rail_A   = A->next.id;
  }else{
    anchor_A = A->next.module;
    rail_A   = A->next.id;
  }

  printf("%i, %i, %i, %i\n", anchor_A, rail_A, anchor_B, rail_B);
}
