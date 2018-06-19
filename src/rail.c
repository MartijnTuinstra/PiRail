#include "rail.h"
#include "system.h"
#include "module.h"
#include "switch.h"
#include "logger.h"

Station ** stations;
int stations_len;

int dircmp(Block *A, Block *B){
  if((A->dir == 2 && (B->dir == 1 || B->dir == 0)) || ((A->dir == 1 || A->dir == 0) && B->dir == 2)){
    return 1;
  }else if(A->dir == B->dir){
    return 1;
  }else if(((A->dir == 0 || A->dir == 2) && B->dir == 0b101) || (A->dir == 1 && B->dir == 0b100)){
    return 1;
  }else if(((B->dir == 0 || B->dir == 2) && A->dir == 0b101) || (B->dir == 1 && A->dir == 0b100)){
    return 1;
  }{
    return 0;
  }
}

int block_cmp(Block *A, Block *B){
  if(A && !B){
    //Compare with empty block
    if(A->module == 0 && A->id == 0 && A->type == 'e'){
      return 1;
    }
    else{
      loggerf(ERROR, "SOMETHING WENT WRONG");
    }

  }else{
    if(A->module == B->module && A->id == B->id && A->type == B->type){
      return 1;
    }else{
      return 0;
    }
  }
}


void init_rail(){

}

void Create_Segment(int IO_Adr, struct block_connect connect ,char max_speed, char dir,char len){
  Block * p = _calloc(1, Block);

  p->module = connect.module;
  p->id = connect.id;
  p->type = connect.type;

  Unit * U = Units[p->module];

  p->next.type = connect.next.type;
  p->next.module = connect.next.module;
  p->next.id = connect.next.id;

  p->prev.type = connect.prev.type;
  p->prev.module = connect.prev.module;
  p->prev.id = connect.prev.id;

  p->max_speed = max_speed;
  p->dir = dir;
  p->length = len;

  p->blocked = 0;
  p->state = PROCEED;

  while(IO_Adr >= (U->input_regs * 8)){
    Unit_expand_IO(0, U); //Expand input
  }
  p->ioadr = IO_Adr;

  gpio_link gpio;
  gpio.type = gpio_RAIL;
  gpio.p = p;
  if(Units[p->module]->input_link[IO_Adr].type != gpio_NC){
    loggerf(WARNING, "Overwriting gpio link %i", IO_Adr);
  }
  Units[p->module]->input_link[IO_Adr] = gpio;


  // If block array is to small
  if(Units[p->module]->block_len <= p->id){
    loggerf(INFO, "Expand block len %i", Units[p->module]->block_len+8); 
    Units[p->module]->B = _realloc(Units[p->module]->B, (Units[p->module]->block_len + 8), Block *);

    int i = Units[p->module]->block_len;
    for(0; i < Units[p->module]->block_len+8; i++){
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

void Create_Station(int module, int id, char * name, char name_len, enum Station_types type, int len, Block ** blocks){
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
    for(0; i < Units[Z->module]->station_len+8; i++){
      Units[Z->module]->St[i] = 0;
    }
    Units[Z->module]->station_len += 8;
  }

  for(int i = 0; i < len; i++){
    Z->blocks[find_free_index((void **)Z->blocks, &Z->blocks_len)] = blocks[i];
  }
  
  Z->uid = find_free_index((void **)stations, &stations_len);

  stations[Z->uid] = Z;
}

void * rail_link_pointer(struct rail_link link){
  if(link.type == 'R'){
    return Units[link.module]->B[link.id];
  }
  else if(link.type == 'S' || link.type == 's'){
    return Units[link.module]->Sw[link.id];
  }
  else if(link.type == 'M' || link.type == 'm'){
    return Units[link.module]->MSSw[link.id];
  }
  return 0;
}

void Connect_Rail_links(){
  // add pointer to the rail_link
  for(int m = 0; m<unit_len; m++){
    if(!Units[m]){
      continue;
    }

    Unit * tU = Units[m];

    for(int i = 0; i<tU->block_len; i++){
      if(!tU->B[i]){
        continue;
      }

      Block * tB = tU->B[i];

      tB->next.p = rail_link_pointer(tB->next);
      tB->prev.p = rail_link_pointer(tB->prev);
    }

    for(int i = 0; i<tU->switch_len; i++){
      if(!tU->Sw[i]){
        continue;
      }

      Switch * tSw = tU->Sw[i];

      tSw->app.p = rail_link_pointer(tSw->app);
      tSw->str.p = rail_link_pointer(tSw->str);
      tSw->div.p = rail_link_pointer(tSw->div);
    }

    for(int i = 0; i<tU->msswitch_len; i++){
      if(!tU->MSSw[i]){
        continue;
      }

      MSSwitch * tMSSw = tU->MSSw[i];

      for(int s = 0; s < tMSSw->state_len; s++){
        tMSSw->sideA[s].p = rail_link_pointer(tMSSw->sideA[s]);
        tMSSw->sideB[s].p = rail_link_pointer(tMSSw->sideB[s]);
      }
    }
  }
}

Block * Next(Block * B, int dir, int level){
  // Find next (detection) block in direction dir. Could be used recurse for x-levels
  // dir: 0 next, 1 prev
  if(!B){
    loggerf(ERROR, "Empty block");
  }

  level--;

  struct rail_link next;

  if((dir == 0 && B->dir == 1) || (dir == 1 && B->dir == 0)
      || (dir == 1 && B->dir == 2)){
    // If next + reversed direction
    // Or prev + normal direction
    // or prev + switching direction (normal)
    next = B->prev;
  }
  else if((dir == 0 && B->dir == 0) || (dir == 1 && B->dir == 1)
      || (dir == 0 && B->dir == 2)){
    // If next + normal direction
    // or prev + reversed direction
    // or next + switching direction (normal)
    next = B->next;
  }

  if(B->dir == 2){
    dir = !dir;
  }

  // printf("Next     : dir:%i\t%i:%i => %i:%i:%c\t%i\n", dir, B->module, B->id, next.module, next.id, next.type, level);

  if(!next.p && next.type != 'e'){
    loggerf(ERROR, "NO POINTERS");
    return 0;
  }

  if(next.type == 'R'){
    if(level <= 0){
      return (Block *)next.p;
    }
    else{
      return Next((Block *)next.p, dir, level);
    }
  }
  else if(next.type == 'S' || next.type == 's'){
    if(level <= 0 && !block_cmp( ((Switch *)next.p)->Detection, B)){
      // printf("Detection block\n");
      return ((Switch *)next.p)->Detection;
    }
    else{
      return Next_Switch_Block((Switch *)next.p, next.type, dir, level);
    }
  }
  else if(next.type == 'M' || next.type == 'm'){
    if(level <= 0 && !block_cmp( ((MSSwitch *)next.p)->Detection, B)){
      // printf("Detection block\n");
      return ((MSSwitch *)next.p)->Detection;
    }
    else{
      return Next_MSSwitch_Block((MSSwitch *)next.p, next.type, dir, level);
    }
  }
  else if(next.type == 'e'){
    return 0;
  }

  loggerf(ERROR, "FIX Next");
  return 0;
}

Block * Next_Switch_Block(Switch * S, char type, int dir, int level){
  struct rail_link next;

  if(type == 's'){
    next = S->app;
  }
  else{
    if(S->state == 0){
      next = S->str;
    }
    else{
      next = S->div;
    }
  }

  // printf("Next   Sw: dir:%i\t%i:%i => %i:%i:%c\t%i\n", dir, S->module, S->id, next.module, next.id, next.type, level);

  if(!next.p){
    loggerf(ERROR, "NO POINTERS");
    return 0;
  }

  if(next.type == 'R'){
    if(!block_cmp(S->Detection, next.p)){
      level--;
    }
    if(level <= 0){
      return (Block *)next.p;
    }
    else{
      return Next((Block *)next.p, dir, level);
    }
  }
  else if(next.type == 'S' || next.type == 's'){
    return Next_Switch_Block((Switch *)next.p, next.type, dir, level);
  }
  else if(next.type == 'M' || next.type == 'm'){
    return Next_MSSwitch_Block((MSSwitch *)next.p, next.type, dir, level);
  }
  else if(next.type == 'e'){
    return 0;
  }

  return 0;
}

Block * Next_MSSwitch_Block(MSSwitch * S, char type, int dir, int level){
  struct rail_link next;

  if(type == 'M'){
    next = S->sideB[S->state];
  }
  else{
    next = S->sideA[S->state];
  }

  // printf("Next MSSw: dir:%i\t%i:%i => %i:%i:%c\t%i\n", dir, S->module, S->id, next.module, next.id, next.type, level);

  if(!next.p){
    loggerf(ERROR, "NO POINTERS");
    return 0;
  }

  if(next.type == 'R'){
    if(!block_cmp(S->Detection, next.p)){
      level--;
    }
    if(level <= 0){
      return (Block *)next.p;
    }
    else{
      return Next((Block *)next.p, dir, level);
    }
  }
  else if(next.type == 'S' || next.type == 's'){
    return Next_Switch_Block((Switch *)next.p, next.type, dir, level);
  }
  else if(next.type == 'M' || next.type == 'm'){
    return Next_MSSwitch_Block((MSSwitch *)next.p, next.type, dir, level);
  }
  else if(next.type == 'e'){
    return 0;
  }

  return 0;
}

struct rail_link Next_link(Block * B){
  struct rail_link link;
  int dir = B->dir;
  if(dir == 0 || dir == 2 || dir == 0b101){
    link = B->next;
  }else{
    link = B->prev;
  }
  return link;
  loggerf(ERROR, "FIX Next_link");
}

struct rail_link Prev_link(Block * B){
  struct rail_link link;
  int dir = B->dir;
  if(dir == 0 || dir == 2 || dir == 0b101){
    link = B->prev;
  }else{
    link = B->next;
  }
  return link;
  loggerf(ERROR, "FIX Prev_link");
}
