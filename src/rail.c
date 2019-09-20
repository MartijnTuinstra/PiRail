#include "rail.h"
#include "system.h"
#include "mem.h"
#include "modules.h"
#include "switch.h"
#include "logger.h"
#include "IO.h"

#include <signal.h>

Station ** stations;
int stations_len;


void Create_Block(uint8_t module, struct s_block_conf block){
  Block * p = _calloc(1, Block);

  p->module = module;
  p->id = block.id;
  p->type = block.type;

  //Unit * U = Units[p->module]; Never used

  p->next.module = block.next.module;
  p->next.id = block.next.id;
  p->next.type = block.next.type;
  p->prev.module = block.prev.module;
  p->prev.id = block.prev.id;
  p->prev.type = block.prev.type;

  p->max_speed = block.speed;
  p->dir = (block.fl & 0x6) >> 1;
  p->length = block.length;

  Algor_init_Blocks(&p->Alg, p);

  p->IOchanged = 1;
  p->algorchanged = 1;

  p->state = PROCEED;

  struct s_node_adr in;
  in.Node = block.IO_In.Node;
  in.io = block.IO_In.Adr;

  Init_IO(Units[p->module], in, IO_Input);

  if(block.fl & 0x8){
    struct s_node_adr out;
    out.Node = block.IO_Out.Node;
    out.io = block.IO_Out.Adr;

    Init_IO(Units[p->module], out, IO_Output);
  }

  // If block array is to small
  if(Units[p->module]->block_len <= p->id){
    loggerf(TRACE, "Expand block len %i", Units[p->module]->block_len+8);
    Units[p->module]->B = _realloc(Units[p->module]->B, (Units[p->module]->block_len + 8), Block *);

    int i = Units[p->module]->block_len;
    for(; i < Units[p->module]->block_len+8; i++){
      Units[p->module]->B[i] = 0;
    }
    Units[p->module]->block_len += 8;
  }

  // If id is already in use
  if(Units[p->module]->B[p->id]){
    loggerf(ERROR, "Duplicate segment %i", p->id);
    free(Units[p->module]->B[p->id]);
  }
  Units[p->module]->B[p->id] = p;
}

void * Clear_Block(Block * B){
  Algor_free_Blocks(&B->Alg);

  _free(B->Sw);
  _free(B->MSSw);

  _free(B);

  return 0;
}

void Create_Station(int module, int id, char * name, char name_len, enum Station_types type, int len, uint8_t * blocks){
  Station * Z = _calloc(1, Station);
  Z->module = module;
  Z->id = id;
  Z->type = type;

  Z->name = _calloc(name_len+1, char);
  strncpy(Z->name, name, name_len);

  // If block array is to small
  if(Units[Z->module]->station_len <= Z->id){
    loggerf(INFO, "Expand station len %i", Units[Z->module]->station_len+8);
    Units[Z->module]->St = _realloc(Units[Z->module]->St, (Units[Z->module]->station_len + 8), Station *);

    int i = Units[Z->module]->station_len;
    for(; i < Units[Z->module]->station_len+8; i++){
      Units[Z->module]->St[i] = 0;
    }
    Units[Z->module]->station_len += 8;
  }

  Units[Z->module]->St[Z->id] = Z;

  Z->blocks_len = len;
  Z->blocks = _calloc(Z->blocks_len, void *);

  for(int i = 0; i < len; i++){
    Z->blocks[i] = U_B(module, blocks[i]);
  }

  if(!stations){
    stations = _calloc(1, void *);
    stations_len = 1;
  }

  Z->uid = find_free_index(stations, stations_len);

  stations[Z->uid] = Z;
}

void * Clear_Station(Station * St){
  if(St->switch_link){
    for(int k = 0; k <= St->switches_len; k++){
      if(St->switch_link[k])
        _free(St->switch_link[k]);
    }
    _free(St->switch_link);
  }

  _free(St->name);
  _free(St->blocks);

  _free(St);

  return NULL;
}