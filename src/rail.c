#include "rail.h"
#include "system.h"
#include "mem.h"
#include "module.h"
#include "switch.h"
#include "logger.h"
#include "IO.h"
#include "algorithm.h"

#include <signal.h>

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
  return 0;
}


void init_rail(){

}

void create_block(uint8_t module, struct s_block_conf block){
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

  p->changed = IO_Changed | Block_Algor_Changed;

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

void * clear_Block(Block * B){
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

void * rail_link_pointer(struct rail_link link){
  if(link.type == RAIL_LINK_R){
    return Units[link.module]->B[link.id];
  }
  else if(link.type == RAIL_LINK_S || link.type == RAIL_LINK_s){
    return Units[link.module]->Sw[link.id];
  }
  else if(link.type == RAIL_LINK_M || link.type == RAIL_LINK_m){
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

    printf("LINKING UNIT %i\n", m);

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

Block * _Next(Block * B, int flags, int level){
  loggerf(TRACE, "Next(%8x, %2x, %2x)", B, flags, level);
  // Find next (detection) block in direction dir. Could be used recurse for x-levels
  int dir = flags & 0x0F;
  // dir: 0 next, 1 prev
  if(!B){
    return 0;
  }

  level--;

  if(B->type == SPECIAL){
    return Next_Special_Block(B, flags, level);
  }

  struct rail_link next;

  uint8_t pdir = (dir >> 1);

  if(pdir != 0b111){
    if((pdir ^ 0b100) == B->dir){
      dir ^= 0b1;
    }
    else if((pdir == 1 && B->dir == 2) || (pdir == 2 && B->dir ==1)){
      dir ^= 0b1;
    }
    else if(pdir == 1 && (B->dir == 0)){
      dir ^= 0b1;
    }
  }

  // printf("dir: %i:%i-%x\t%i\n", B->module, B->id, B->dir, pdir);
  //NEXT == 0
  //PREV == 1

  if(((dir & 1) == NEXT && (B->dir == 1 || B->dir == 4 || B->dir == 6)) ||
     ((dir & 1) == PREV && (B->dir == 0 || B->dir == 2 || B->dir == 5))){
    // If next + reversed direction / flipped normal / flipped switching
    // Or prev + normal direction / switching direction (normal) / flipped reversed direction
    // printf("Prev\n");
    next = B->prev;
  }
  else if(((dir & 1) == NEXT && (B->dir == 0 || B->dir == 2 || B->dir == 5)) ||
          ((dir & 1) == PREV && (B->dir == 1 || B->dir == 4 || B->dir == 6))){
    // If next + normal direction / switching direction (normal) / flipped reversed
    // or prev + reversed direction / flipped normal / flipped switching
    // printf("Next\n");
    next = B->next;
  }
  else
    loggerf(WARNING, "No dir found");

  dir = (dir & 1) + (B->dir << 1);

  flags = (flags & 0xF0) + (dir & 0x0F);

  // printf("Next     : dir:%i/%x\t%i:%i => %i:%i:%i\t%i\n", B->dir, dir, B->module, B->id, next.module, next.id, next.type, level);

  if(!next.p && next.type != RAIL_LINK_E){
    loggerf(ERROR, "NO POINTERS %i:%i", B->module, B->id);
    return 0;
  }

  if(next.type == RAIL_LINK_R){
    if(level <= 0){
      return (Block *)next.p;
    }
    else{
      return _Next((Block *)next.p, flags, level);
    }
  }
  else if(next.type == RAIL_LINK_S || next.type == RAIL_LINK_s){
    if(Next_check_Switch(B, next, flags)){
      if(level <= 0 && !block_cmp( ((Switch *)next.p)->Detection, B)){
        return ((Switch *)next.p)->Detection;
      }
      else{
        return Next_Switch_Block((Switch *)next.p, next.type, flags, level);
      }
    }
  }
  else if(next.type == RAIL_LINK_M || next.type == RAIL_LINK_m){
    if(Next_check_Switch(B, next, flags)){
      if(level <= 0 && !block_cmp( ((MSSwitch *)next.p)->Detection, B)){
        // printf("Detection block\n");
        return ((MSSwitch *)next.p)->Detection;
      }
      else{
        return Next_MSSwitch_Block((MSSwitch *)next.p, next.type, flags, level);
      }
    }
  }
  else if(next.type == RAIL_LINK_E && next.module == 0 && next.id == 0){
    return 0;
  }

  return 0;
}

int Next_check_Switch(void * p, struct rail_link link, int flags){
  loggerf(TRACE, "Next_check_Switch (%x, %x, %i)", (unsigned int)p, (unsigned int)&link, flags);
  if((flags & 0x80) == 0){
    //No SWITCH_CARE
    return 1;
  }
  else if(link.type == RAIL_LINK_S){
    return 1;
  }
  else if(link.type == RAIL_LINK_s){
    Switch * N = link.p;
    loggerf(TRACE, "check s (state: %i, str.p: %x, div.p: %x)", (N->state & 0x7F), (unsigned int)N->str.p, (unsigned int)N->div.p);
    if(((N->state & 0x7F) == 0 && N->str.p == p) || ((N->state & 0x7F) == 1 && N->div.p == p)){
      return 1;
    }
    // else
    //   printf("str: %i  %x==%x\tdiv: %i  %x==%x\t",N->state, N->str.p, p, N->state, N->div.p, p);
  }
  else if(link.type == RAIL_LINK_M){
    MSSwitch * N = link.p;
    if(N->sideB[N->state].p == p){
      return 1;
    }
  }
  else if(link.type == RAIL_LINK_m){
    MSSwitch * N = link.p;
    if(N->sideA[N->state].p == p){
      return 1;
    }
  }
  return 0;
}

int Next_check_Switch_Path(void * p, struct rail_link link, int flags){
  loggerf(TRACE, "Next_check_Switch_Path (%x, %x, %i)", (unsigned int)p, (unsigned int)&link, flags);
  if(!link.p){
    loggerf(ERROR, "Empty LINK {%i:%i - %i}", link.module, link.id, link.type);
    return 1;
  }

  if((flags & 0x80) == 0){
    //No SWITCH_CARE
    return 1;
  }
  else if(link.type == RAIL_LINK_S){
    Switch * Sw = link.p;
    if((Sw->state & 0x7F) == 0 && Sw->str.type != RAIL_LINK_R && Sw->str.type != 'D'){
      return Next_check_Switch_Path(Sw, Sw->str, flags);
    }
    else if((Sw->state & 0x7F) == 1 && Sw->div.type != RAIL_LINK_R && Sw->div.type != 'D'){
      return Next_check_Switch_Path(Sw, Sw->div, flags);
    }
  }
  else if(link.type == RAIL_LINK_s){
    Switch * N = link.p;
    loggerf(TRACE, "check s (state: %i, str.p: %x, div.p: %x)", (N->state & 0x7F), (unsigned int)N->str.p, (unsigned int)N->div.p);
    if((N->state & 0x7F) == 0 && N->str.p == p){
      return Next_check_Switch_Path(N, N->app, flags);
    }
    else if((N->state & 0x7F) == 1 && N->div.p == p){
      return Next_check_Switch_Path(N, N->app, flags);
    }
    loggerf(TRACE, "wrong State");
  }
  else if(link.type == RAIL_LINK_M){
    loggerf(WARNING, "IMPLEMENT");
    MSSwitch * N = link.p;
    if(N->sideB[N->state].p == p){
      return 1;
    }
  }
  else if(link.type == RAIL_LINK_m){
    loggerf(WARNING, "IMPLEMENT");
    MSSwitch * N = link.p;
    if(N->sideA[N->state].p == p){
      return 1;
    }
  }

  else if (link.type == RAIL_LINK_R || link.type == 'D'){
    return 1;
  }
  return 0;
}

int Next_check_Switch_Path_one_block(Block * B, void * p, struct rail_link link, int flags){
  loggerf(TRACE, "Next_check_Switch_Path_one_block (%x, %x, %i)", (unsigned int)p, (unsigned int)&link, flags);
  if(((link.type == RAIL_LINK_S || link.type == RAIL_LINK_s) && ((Switch *)link.p)->Detection != B) ||
     ((link.type == RAIL_LINK_M || link.type == RAIL_LINK_m) && ((MSSwitch *)link.p)->Detection != B)){
    return 1;
  }
  if((flags & 0x80) == 0){
    //No SWITCH_CARE
    return 1;
  }
  else if(link.type == RAIL_LINK_S){
    Switch * Sw = link.p;
    if((Sw->state & 0x7F) == 0){
      if(Sw->str.type != RAIL_LINK_R)
        return Next_check_Switch_Path_one_block(B, Sw, Sw->str, flags);
      else if (Sw->str.p != B)
        return 1;
    }
    else if((Sw->state & 0x7F) == 1){
      if(Sw->div.type != RAIL_LINK_R)
        return Next_check_Switch_Path_one_block(B, Sw, Sw->div, flags);
      else if (Sw->str.p != B)
        return 1;
    }
    loggerf(TRACE, "S wrong State %x", Sw->state & 0x7F);
  }
  else if(link.type == RAIL_LINK_s){
    Switch * N = link.p;
    loggerf(TRACE, "check s (state: %i, str.p: %x, div.p: %x)", (N->state & 0x7F), (unsigned int)N->str.p, (unsigned int)N->div.p);
    if((N->state & 0x7F) == 0 && N->str.p == p){
      return Next_check_Switch_Path_one_block(B, N, N->app, flags);
    }
    else if((N->state & 0x7F) == 1 && N->div.p == p){
      return Next_check_Switch_Path_one_block(B, N, N->app, flags);
    }
    loggerf(TRACE, "wrong State");
  }
  else if(link.type == RAIL_LINK_M){
    loggerf(WARNING, "IMPLEMENT");
    MSSwitch * N = link.p;
    if(N->sideB[N->state].p == p){
      return 1;
    }
  }
  else if(link.type == RAIL_LINK_m){
    loggerf(WARNING, "IMPLEMENT");
    MSSwitch * N = link.p;
    if(N->sideA[N->state].p == p){
      return 1;
    }
  }

  else if (link.type == RAIL_LINK_R){
    return 1;
  }
  return 0;
}

Block * Next_Switch_Block(Switch * S, char type, int flags, int level){
  struct rail_link next;

  // printf("%i:%i\t",S->module,S->id);

  if(type == RAIL_LINK_s){
    next = S->app;
  }
  else{
    if((S->state & 0x7F) == 0){
      next = S->str;
    }
    else{
      next = S->div;
    }
  }

  // printf("N%cL%i\t",next.type,level);

  if(!next.p){
    loggerf(ERROR, "NO POINTERS");
    return 0;
  }

  if(next.type == RAIL_LINK_R){
    if(!block_cmp(S->Detection, next.p)){
      level--;
    }
    if(level <= 0){
      // printf("RET BLOCK %i:%i\n",((Block *)next.p)->module, ((Block *)next.p)->id);
      return (Block *)next.p;
    }
    else{
      struct rail_link * nextnext = Next_link(next.p, flags);
      // printf("-%x==%x-\t",S,nextnext.p);
      if(nextnext->p == S){
        // printf("FLIP\n");
        if((flags & 0xf) == NEXT){
          flags = (flags & 0xf0) | PREV;
        }
        else
          flags = (flags & 0xf0) | NEXT;
      }
      // printf("NB %i:%i\t",((Block *)next.p)->module,((Block *)next.p)->id);
      return _Next((Block *)next.p, flags, level);
    }
  }
  else if(next.type == RAIL_LINK_S || next.type == RAIL_LINK_s){
    if(Next_check_Switch(S, next, flags)){
      Switch * N = next.p;
      if(N->Detection && !block_cmp(S->Detection, N->Detection)){
        level--;
        if(level == 0){
          // printf("RET DET\n");
          return N->Detection;
        }
      }

      return Next_Switch_Block(N, next.type, flags, level);
    }
  }
  else if(next.type == RAIL_LINK_M || next.type == RAIL_LINK_m){
    // printf("RET MSSw\n");
    MSSwitch * N = next.p;
    if(N->Detection && !block_cmp(S->Detection, N->Detection) && level == 1){
      return N->Detection;
    }
    else if(Next_check_Switch(S, next, flags)){
      return Next_MSSwitch_Block(N, next.type, flags, level);
    }
  }
  else if(next.type == RAIL_LINK_E){
    return 0;
  }
  // printf("RET END\n");
  return 0;
}

Block * Next_MSSwitch_Block(MSSwitch * S, char type, int flags, int level){
  struct rail_link next;

  if(flags & SWITCH_CARE){
    loggerf(CRITICAL, "Fix next_msswitch_block switch state care");
  }

  if(type == RAIL_LINK_M){
    next = S->sideB[S->state & 0x7F];
  }
  else{
    next = S->sideA[S->state & 0x7F];
  }

  if(!next.p){
    loggerf(ERROR, "NO POINTERS");
    return 0;
  }

  if(next.type == RAIL_LINK_R){
    if(!block_cmp(S->Detection, next.p)){
      level--;
    }
    if(level <= 0){
      return (Block *)next.p;
    }
    else{
      return _Next((Block *)next.p, flags, level);
    }
  }
  else if(next.type == RAIL_LINK_S || next.type == RAIL_LINK_s){
    if(Next_check_Switch(S, next, flags)){
      return Next_Switch_Block((Switch *)next.p, next.type, flags, level);
    }
  }
  else if(next.type == RAIL_LINK_M || next.type == RAIL_LINK_m){
    if(Next_check_Switch(S, next, flags)){
      return Next_MSSwitch_Block((MSSwitch *)next.p, next.type, flags, level);
    }
  }
  else if(next.type == RAIL_LINK_E){
    return 0;
  }

  return 0;
}

Block * Next_Special_Block(Block * Bl, int flags, int level){
  loggerf(DEBUG, "Next_Special_Block %i:%i", Bl->module, Bl->id);
  struct next_prev_Block {
    Block * prev;
    Block * next;
  };

  int dir = flags & 0x01;

  int pairs = 0;
  struct next_prev_Block * np_blocks = _calloc(Bl->switch_len + Bl->msswitch_len, struct next_prev_Block);
  level++;

  for(int i = 0; i < Bl->switch_len; i++){
    if(!Bl->Sw[i]){
      continue;
    }
    Switch * S = Bl->Sw[i];
    Block * A = Next_Switch_Block(S, RAIL_LINK_s, NEXT, level);
    Block * B = Next_Switch_Block(S, RAIL_LINK_S, NEXT, level);
    Block * _A = 0; //Mirror to other side
    Block * _B = 0; //Mirror to other side
    int prioA = 0;
    int prioB = 0;

    if(A && A->type != SPECIAL){
      if(A->blocked)
        prioA++;

      if(Next(A, NEXT, level) == Bl)
        _A = Next(A, NEXT, 2*level);
      else
        _A = Next(A, PREV, 2*level);
    }

    if(B && B->type != SPECIAL){
      if(B->blocked)
        prioB++;

      if(Next(B, NEXT, level) == Bl)
        _B = Next(B, NEXT, 2*level);
      else
        _B = Next(B, PREV, 2*level);
    }

    printf("\t\t%i:%i-%i-%i ", Bl->module, Bl->id, level,Bl->dir);
    if(A)
      printf("\t%2i:%2i-%i<>",A->module,A->id,A->dir);
    else
      printf("\t     <>");
    if(_A)
      printf("%2i:%2i-%i", _A->module,_A->id,_A->dir);
    else
      printf("     ");
    if(B)
      printf("\t%2i:%2i-%i",B->module,B->id,B->dir);
    else
      printf("\t     ");
    if(_B)
      printf("<>%2i:%2i-%i\n",_B->module,_B->id,_B->dir);
    else
      printf("<>\n");

    loggerf(ERROR, "B==_A, %i", B == _A);
    loggerf(ERROR, "A==_B, %i", B == _A);

    if(A && B && _A && B == _A){
      loggerf(ERROR, "A Is the same %i", i);
      if(A->dir == Bl->dir ||
        ((A->dir == 0 || A->dir == 2) && Bl->dir == 0b101) ||
        (A->dir == 1 && (Bl->dir == 0b100 || Bl->dir == 0b110))){
        if(Next(A, NEXT, level) == Bl){
          // A = Next_Switch_Block(S, RAIL_LINK_s, NEXT | SWITCH_CARE, level);
          // B = Next_Switch_Block(S, RAIL_LINK_S, NEXT | SWITCH_CARE, level);
          loggerf(DEBUG, "A N(%i)\t%x <p %i:%i n> %x\n", level, A, Bl->module, Bl->id, B);
          np_blocks[pairs].next = B;
          np_blocks[pairs].prev = A;
          pairs++;
        }
        else if(Next(A, PREV, level) == Bl){
          // A = Next_Switch_Block(S, RAIL_LINK_s, NEXT | SWITCH_CARE, level);
          // B = Next_Switch_Block(S, RAIL_LINK_S, NEXT | SWITCH_CARE, level);
          loggerf(DEBUG, "A P(%i)\t%x <p %i:%i n> %x\n", level, B, Bl->module, Bl->id, A);
          np_blocks[pairs].next = A;
          np_blocks[pairs].prev = B;
          pairs++;
        }
        else{
          loggerf(ERROR, "Some weird pair");
        }
      }
      else{
        if(Next(B, NEXT, level) == Bl){
          // A = Next_Switch_Block(S, RAIL_LINK_s, NEXT | SWITCH_CARE, level);
          // B = Next_Switch_Block(S, RAIL_LINK_S, NEXT | SWITCH_CARE, level);
          loggerf(DEBUG, "Ar N(%i)\t%x <p %i:%i n> %x\n", level, B, Bl->module, Bl->id, A);
          np_blocks[pairs].next = A;
          np_blocks[pairs].prev = B;
          pairs++;
        }
        else if(Next(A, PREV, level) == Bl){
          // A = Next_Switch_Block(S, RAIL_LINK_s, NEXT | SWITCH_CARE, level);
          // B = Next_Switch_Block(S, RAIL_LINK_S, NEXT | SWITCH_CARE, level);
          loggerf(DEBUG, "Ar P(%i)\t%x <p %i:%i n> %x\n", level, A, Bl->module, Bl->id, B);
          np_blocks[pairs].next = B;
          np_blocks[pairs].prev = A;
          pairs++;
        }
      }
    }
    else if(A && B && _B && A == _B){
      loggerf(DEBUG, "B Is the same %i", i);
      if(B->dir == Bl->dir ||
        ((B->dir == 0 || B->dir == 2) && Bl->dir == 0b101) ||
        (B->dir == 1 && (Bl->dir == 0b100 || Bl->dir == 0b110))){
        if(Next(B, NEXT, level) == Bl){
          // A = Next_Switch_Block(S, RAIL_LINK_s, NEXT | SWITCH_CARE, level);
          // B = Next_Switch_Block(S, RAIL_LINK_S, NEXT | SWITCH_CARE, level);
          loggerf(DEBUG, "B N(%i)\t%x <p %i:%i n> %x\n", level, B, Bl->module, Bl->id, A);
          np_blocks[pairs].next = A;
          np_blocks[pairs].prev = B;
          pairs++;
        }
        else{
          // A = Next_Switch_Block(S, RAIL_LINK_s, NEXT | SWITCH_CARE, level);
          // B = Next_Switch_Block(S, RAIL_LINK_S, NEXT | SWITCH_CARE, level);
          loggerf(DEBUG, "B P(%i)\t%x <p %i:%i n> %x\n", level, A, Bl->module, Bl->id, B);
          np_blocks[pairs].next = B;
          np_blocks[pairs].prev = A;
          pairs++;
        }
      }
      else{
        if(Next(A, NEXT, level) == Bl){
          // A = Next_Switch_Block(S, RAIL_LINK_s, NEXT | SWITCH_CARE, level);
          // B = Next_Switch_Block(S, RAIL_LINK_S, NEXT | SWITCH_CARE, level);
          loggerf(DEBUG, "Br N(%i)\t%x <p %i:%i n> %x\n", level, A, Bl->module, Bl->id, B);
          np_blocks[pairs].next = B;
          np_blocks[pairs].prev = A;
          pairs++;
        }
        else{
          // A = Next_Switch_Block(S, RAIL_LINK_s, NEXT | SWITCH_CARE, level);
          // B = Next_Switch_Block(S, RAIL_LINK_S, NEXT | SWITCH_CARE, level);
          loggerf(DEBUG, "Br P(%i)\t%x <p %i:%i n> %x\n", level, B, Bl->module, Bl->id, A);
          np_blocks[pairs].next = A;
          np_blocks[pairs].prev = B;
          pairs++;
        }
      }
    }
    else if(level == 1){
      printf("Weird pairs");
      if(np_blocks[pairs].next){
        printf("N%i\t", Next_check_Switch_Path_one_block(Bl, np_blocks[pairs].next, np_blocks[pairs].next->next, SWITCH_CARE));
        printf(" %i\t", Next_check_Switch_Path_one_block(Bl, np_blocks[pairs].next, np_blocks[pairs].next->prev, SWITCH_CARE));
      }
      if(np_blocks[pairs].prev){
        printf("P%i\t", Next_check_Switch_Path_one_block(Bl, np_blocks[pairs].prev, np_blocks[pairs].prev->next, SWITCH_CARE));
        printf(" %i\t", Next_check_Switch_Path_one_block(Bl, np_blocks[pairs].prev, np_blocks[pairs].prev->prev, SWITCH_CARE));
      }
    }
  }

  for(int i = 0; i < Bl->msswitch_len; i++){
    loggerf(ERROR, "Implement msswitch in Next_Special_Block (%i:%i)", Bl->module, Bl->id);
    return 0;
  }

  Block * tmp = 0;
  if(pairs == 1){
    loggerf(DEBUG, "1 pair [%i:%i <p=n> %i:%i]", np_blocks[0].prev->module, np_blocks[0].prev->id, np_blocks[0].next->module, np_blocks[0].next->id);
    if(dir)
      loggerf(DEBUG, "Prev %i", Bl->dir);
    else
      loggerf(DEBUG, "Next %i", Bl->dir);
    // if((dir == 0 && (Bl->dir == 4 || Bl->dir == 6)) ||
    //   (dir == 1 && (Bl->dir == 2 || Bl->dir == 5))){
    //   // If next + reversed direction / flipped normal / flipped switching
    //   // Or prev + normal direction / switching direction (normal) / flipped reversed direction
    //   tmp = np_blocks[0].prev;
    // }
    // else if((dir == 0 && (Bl->dir == 2 || Bl->dir == 5)) ||
    //   (dir == 1 && (Bl->dir == 4 || Bl->dir == 6))){
    //   // If next + normal direction / switching direction (normal) / flipped reversed
    //   // or prev + reversed direction / flipped normal / flipped switching
    //   tmp = np_blocks[0].next;
    // }
    if(dir == 0)
      tmp = np_blocks[0].next;
    else if(dir == 1)
      tmp = np_blocks[0].prev;
  }
  else if(pairs >= 2){
    _Bool same = TRUE;
    loggerf(DEBUG, "2 pair");
    for(int i = 0; i < pairs - 1; i++){
      // loggerf(DEBUG, " prev %2i:%2i   %2i:%2i next <==> prev %2i:%2i   %2i:%2i next\n",np_blocks[i].prev->module,np_blocks[i].prev->id, np_blocks[i].next->module, np_blocks[i].next->id,np_blocks[i+1].prev->module,np_blocks[i+1].prev->id, np_blocks[i+1].next->module, np_blocks[i+1].next->id);
      if(np_blocks[i].next != np_blocks[i+1].next && np_blocks[i].prev != np_blocks[i+1].prev){
        same = FALSE;
      }
    }
    if(same){
      if(dir == NEXT){
        loggerf(DEBUG, "N_E_X_T");
        tmp = np_blocks[0].next;
      }
      else{
        loggerf(DEBUG, "P_R_E_V");
        tmp = np_blocks[0].prev;
      }
    }
    else{
      loggerf(WARNING, "FIX %i pairs", pairs);
      printf("Multiple pairs: ");
      for(int i = 0; i < pairs; i++){
        if(np_blocks[i].next)
          printf("%2i:%2i<>",np_blocks[i].next->module,np_blocks[i].next->id);
        else
          printf("     <>");
        if(np_blocks[i].prev)
          printf("%i:%i\t",np_blocks[i].prev->module,np_blocks[i].prev->id);
        else
          printf("     \t");
      }
      printf("\n");
    }
  }
  else{
    loggerf(DEBUG, "SPECIAL-BLOCK %i:%i: Multiple pairs:", Bl->module, Bl->id);
    for(int i = 0; i < pairs; i++){
      if(np_blocks[i].next && np_blocks[i].prev)
        loggerf(DEBUG, " - %i:%i<>%i:%i",np_blocks[i].next->module,np_blocks[i].next->id,np_blocks[i].prev->module,np_blocks[i].prev->id);
    }
  }

  _free(np_blocks);
  if(tmp){
    return tmp;
  }
  else{
    loggerf(ERROR, "FIX, NO BLOCK FOUND");
    return 0;
  }
}

int Switch_to_rail(Block ** B, void * Sw, char type, uint8_t counter){
  struct rail_link next;

  //if(type == RAIL_LINK_S || type == RAIL_LINK_s){
  //  printf("Sw %i:%i\t%x\n", ((Switch *)Sw)->module, ((Switch *)Sw)->id, type);
  //}

  if(type == RAIL_LINK_S){
    if(( ((Switch *)Sw)->state & 0x7f) == 0)
      next = ((Switch *)Sw)->str;
    else
      next = ((Switch *)Sw)->div;
  }
  else if(type == RAIL_LINK_s){
    next = ((Switch *)Sw)->app;
  }

  if(next.type == RAIL_LINK_S){
    Switch * NSw = (Switch *)next.p;
    if(NSw->Detection && NSw->Detection != *B){
      counter++;
      *B = NSw->Detection;
    }
    return Switch_to_rail(B, (Switch *)next.p, RAIL_LINK_S, counter);
  }
  else if(next.type == RAIL_LINK_s){
    Switch * NSw = (Switch *)next.p;
    if(NSw->Detection && NSw->Detection != *B){
      counter++;
      *B = NSw->Detection;
      //printf("-%i:%i\n", (*B)->module, (*B)->id);
    }
    if((NSw->state & 0x7f) == 0 && NSw->str.p == Sw){
      return Switch_to_rail(B, NSw, RAIL_LINK_s, counter);
    }
    else if((NSw->state & 0x7f) == 1 && NSw->div.p == Sw){
      return Switch_to_rail(B, NSw, RAIL_LINK_s, counter);
    }
    else{
      *B = 0;
      return counter;
    }
  }
  else if(next.type == RAIL_LINK_R){
    Block * tmp_B = (Block *)next.p;
    if(tmp_B != *B){
      counter++;
      *B = tmp_B;
      return counter;
    }
  }
  return 0;
}

struct rail_link * Next_link(Block * B, int flags){
  // dir: 0 next, 1 prev
  int dir = flags & 0x01;

  struct rail_link * next = 0;

  if(!B){
    loggerf(ERROR, "Empty block");
    return next;
  }

  if((dir == NEXT && (B->dir == 1 || B->dir == 4 || B->dir == 6)) ||
     (dir == PREV && (B->dir == 0 || B->dir == 2 || B->dir == 5))) {
    next = &B->prev;
  }
  else{
    next = &B->next;
  }

  if(B->dir == 2){
    dir = !dir;
  }

  if(!next)
    loggerf(ERROR, "Empty next Link");
  return next;
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

void Reserve_To_Next_Switch(Block * B){
  Block * Next_Block = B->Alg.BN->B[0];

  while(1){
    if(!Next_Block){
      break;
    }
    else if(Next_Block->type == SPECIAL){
      putAlgorQueue(Next_Block, 1);
      break;
    }

    if(Next_Block->state >= PROCEED)
      Next_Block->state = RESERVED;
    Next_Block->reverse_state = DANGER;
    Block_reserve(Next_Block);
    // Next_Block->reserved++;
    Next_Block->changed |= State_Changed;

    Next_Block = Next_Block->Alg.BN->B[0];
  }
}

void Block_Reverse(Block * B){
  B->dir ^= 0b100;

  int tmp_state = B->state;
  B->state = B->reverse_state;
  B->reverse_state = tmp_state;

  Algor_Block * tmp = B->Alg.BN;
  B->Alg.BN = B->Alg.BP;
  B->Alg.BP = tmp;

  tmp = B->Alg.BNN;
  B->Alg.BNN = B->Alg.BPP;
  B->Alg.BPP = tmp;

  tmp = B->Alg.BNNN;
  B->Alg.BNNN = B->Alg.BPPP;
  B->Alg.BPPP = tmp;
}

int Block_Reverse_To_Next_Switch(Block * B){
  Block * Next_Block = B->Alg.BN->B[0];

  while(1){
    if(!Next_Block){
      break;
    }
    else if(Next_Block->type == SPECIAL){
      putAlgorQueue(Next_Block, 1);
      break;
    }

    Block_Reverse(Next_Block);

    if(Next_Block->state >= PROCEED)
      Next_Block->state = RESERVED;
    Next_Block->reverse_state = DANGER;
    Block_reserve(Next_Block);
    // Next_Block->reserved++;
    Next_Block->changed |= State_Changed;

    Next_Block = Next_Block->Alg.BN->B[0];
  }
  return 0;
}
