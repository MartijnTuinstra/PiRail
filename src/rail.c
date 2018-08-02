#include "rail.h"
#include "system.h"
#include "module.h"
#include "switch.h"
#include "logger.h"
#include "IO.h"

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

void Create_Segment(Node_adr IO_Adr, struct block_connect connect ,char max_speed, char dir,char len){
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

  if(IO_Adr.Node < Units[p->module]->IO_Nodes && Units[p->module]->Node[IO_Adr.Node].io[IO_Adr.io]){
    IO_Port * A = Units[p->module]->Node[IO_Adr.Node].io[IO_Adr.io];

    if(A->type != IO_Undefined){
      loggerf(WARNING, "IO %i:%i already in use", IO_Adr.Node, IO_Adr.io);
    }

    A->type = IO_Input;
    A->state = 0;
    A->id = IO_Adr.io;

    loggerf(DEBUG, "IO %i:%i", IO_Adr.Node, IO_Adr.io);
  }

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
    Z->blocks[find_free_index(Z->blocks, Z->blocks_len)] = blocks[i];
  }
  
  Z->uid = find_free_index(stations, stations_len);

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

Block * Next(Block * B, int flags, int level){
  // Find next (detection) block in direction dir. Could be used recurse for x-levels
  int dir = flags & 0x0F;
  // dir: 0 next, 1 prev
  if(!B){
    loggerf(ERROR, "Empty block");
    return 0;
  }

  level--;

  if(B->type == SPECIAL){
    return Next_Special_Block(B, flags, level);
  }

  struct rail_link next;

  if((dir == 0 && (B->dir == 1 || B->dir == 4 || B->dir == 6)) || 
    (dir == 1 && (B->dir == 0 || B->dir == 2 || B->dir == 5))){
    // If next + reversed direction / flipped normal / flipped switching
    // Or prev + normal direction / switching direction (normal) / flipped reversed direction
    next = B->prev;
  }
  else if((dir == 0 && (B->dir == 0 || B->dir == 2 || B->dir == 5)) || 
    (dir == 1 && (B->dir == 1 || B->dir == 4 || B->dir == 6))){
    // If next + normal direction / switching direction (normal) / flipped reversed
    // or prev + reversed direction / flipped normal / flipped switching
    next = B->next;
  }
  else
    loggerf(WARNING, "No dir found");


  if(B->dir == 2){
    dir = !dir;
  }

  flags = (flags & 0xF0) + (dir & 0x0F);

  // printf("Next     : dir:%i\t%i:%i => %i:%i:%c\t%i\n", dir, B->module, B->id, next.module, next.id, next.type, level);

  if(!next.p && next.type != 'e'){
    loggerf(ERROR, "NO POINTERS %i:%i", B->module, B->id);
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
    if(Next_check_Switch(B, next, flags)){
      if(level <= 0 && !block_cmp( ((Switch *)next.p)->Detection, B)){
        return ((Switch *)next.p)->Detection;
      }
      else{
        return Next_Switch_Block((Switch *)next.p, next.type, flags, level);
      }
    }
  }
  else if(next.type == 'M' || next.type == 'm'){
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
  else if(next.type == 'e' && next.module == 0 && next.id == 0){
    return 0;
  }

  return 0;
}

int Next_check_Switch(void * p, struct rail_link link, int flags){
  if((flags & 0x80) == 0){
    //No SWITCH_CARE
    return 1;
  }
  else if(link.type == 'S'){
    return 1;
  }
  else if(link.type == 's'){
    Switch * N = link.p;
    if(N->state == 0 && N->str.p == p || N->state == 1 && N->div.p == p){
      return 1;
    }
    // else
      // printf("str: %i  %x==%x\tdiv: %i  %x==%x\t",N->state, N->str.p, p, N->state, N->div.p, p);
  }
  else if(link.type == 'M'){
    MSSwitch * N = link.p;
    if(N->sideB[N->state].p == p){
      return 1;
    }
  }
  else if(link.type == 'm'){
    MSSwitch * N = link.p;
    if(N->sideA[N->state].p == p){
      return 1;
    }
  }
  return 0;
}

Block * Next_Switch_Block(Switch * S, char type, int flags, int level){
  struct rail_link next;

  // printf("%i:%i\t",S->module,S->id);

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

  // printf("N%cL%i\t",next.type,level);

  if(!next.p){
    loggerf(ERROR, "NO POINTERS");
    return 0;
  }

  if(next.type == 'R'){
    if(!block_cmp(S->Detection, next.p)){
      level--;
    }
    if(level <= 0){
      // printf("RET BLOCK %i:%i\n",((Block *)next.p)->module, ((Block *)next.p)->id);
      return (Block *)next.p;
    }
    else{
      struct rail_link nextnext = Next_link(next.p, flags);
      // printf("-%x==%x-\t",S,nextnext.p);
      if(nextnext.p == S){
        // printf("FLIP\n");
        if((flags & 0xf) == NEXT){
          flags = (flags & 0xf0) | PREV;
        }
        else
          flags = (flags & 0xf0) | NEXT;
      }
      // printf("NB %i:%i\t",((Block *)next.p)->module,((Block *)next.p)->id);
      return Next((Block *)next.p, flags, level);
    }
  }
  else if(next.type == 'S' || next.type == 's'){
    Switch * N = next.p;
    if(N->Detection && !block_cmp(S->Detection, N->Detection)){
      level--;
      if(level == 0){
        // printf("RET DET\n");
        return N->Detection;
      }
    }
    if(Next_check_Switch(S, next, flags)){
      return Next_Switch_Block(N, next.type, flags, level);
    }
  }
  else if(next.type == 'M' || next.type == 'm'){
    // printf("RET MSSw\n");
    MSSwitch * N = next.p;
    if(N->Detection && !block_cmp(S->Detection, N->Detection) && level == 1){
      return N->Detection;
    }
    else if(Next_check_Switch(S, next, flags)){
      return Next_MSSwitch_Block(N, next.type, flags, level);
    }
  }
  else if(next.type == 'e'){
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

  if(type == 'M'){
    next = S->sideB[S->state];
  }
  else{
    next = S->sideA[S->state];
  }

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
      return Next((Block *)next.p, flags, level);
    }
  }
  else if(next.type == 'S' || next.type == 's'){
    if(Next_check_Switch(S, next, flags)){
      return Next_Switch_Block((Switch *)next.p, next.type, flags, level);
    }
  }
  else if(next.type == 'M' || next.type == 'm'){
    if(Next_check_Switch(S, next, flags)){
      return Next_MSSwitch_Block((MSSwitch *)next.p, next.type, flags, level);
    }
  }
  else if(next.type == 'e'){
    return 0;
  }

  return 0;
}

Block * Next_Special_Block(Block * Bl, int flags, int level){
  struct next_prev_Block {
    Block * prev;
    Block * next;
  };
  int pairs = 0;
  struct next_prev_Block * np_blocks = _calloc(Bl->switch_len + Bl->msswitch_len, struct next_prev_Block);
  level++;

  for(int i = 0; i < Bl->switch_len; i++){
    if(!Bl->Sw[i]){
      continue;
    }
    Switch * S = Bl->Sw[i];
    Block * A = Next_Switch_Block(S, 's', NEXT | SWITCH_CARE, level);
    Block * B = Next_Switch_Block(S, 'S', NEXT | SWITCH_CARE, level);
    Block * _A = 0;
    Block * _B = 0;
    if(A && A->type != SPECIAL){
      if(Next(A, NEXT | SWITCH_CARE, level) == Bl)
        _A = Next(A, NEXT | SWITCH_CARE, 2*level);
      else
        _A = Next(A, PREV | SWITCH_CARE, 2*level);
    }

    if(B && B->type != SPECIAL){
      if(Next(B, NEXT | SWITCH_CARE, level) == Bl)
        _B = Next(B, NEXT | SWITCH_CARE, 2*level);
      else
        _B = Next(B, PREV | SWITCH_CARE, 2*level);
    }
    // printf("%i ", level);
    // if(A)
    //   printf("\t%2i:%2i<>",A->module,A->id);
    // else
    //   printf("\t     <>");
    // if(_A)
    //   printf("%2i:%2i", _A->module,_A->id);
    // else
    //   printf("     ");
    // if(B)
    //   printf("\t%2i:%2i",B->module,B->id);
    // else
    //   printf("\t     ");
    // if(_B)
    //   printf("<>%2i:%2i\n",_B->module,_B->id);
    // else
    //   printf("<>\n");
    

    if(B && _A && B==_A){
      if(A->dir == Bl->dir || A->dir == (Bl->dir | 0x4) || (A->dir | 0x4) == Bl->dir){
        if(Next(A, NEXT | SWITCH_CARE, level) == Bl){
          // printf("\tA\t%i:%i Next %i == %i:%i\n", A->module, A->id, level, Bl->module, Bl->id);
          np_blocks[pairs].next = B;
          np_blocks[pairs].prev = A;
        }
        else{
          // printf("\tA\t%i:%i Prev %i == %i:%i\n", A->module, A->id, level, Bl->module, Bl->id);
          np_blocks[pairs].next = A;
          np_blocks[pairs].prev = B;
        }
        pairs++;
      }
    }
    else if(A && _B && A==_B){
      if(B->dir == Bl->dir || B->dir == Bl->dir ^ 0x4 || B->dir ^ 0x4 == Bl->dir){
        if(Next(B, NEXT | SWITCH_CARE, level) == Bl){
          // printf("\tB\t%i:%i Next %i == %i:%i\n", B->module, B->id, level, Bl->module, Bl->id);
          np_blocks[pairs].next = A;
          np_blocks[pairs].prev = B;
        }
        else{
          // printf("\tB\t%i:%i Prev %i == %i:%i\n", B->module, B->id, level, Bl->module, Bl->id);
          np_blocks[pairs].next = B;
          np_blocks[pairs].prev = A;
        }
        pairs++;
      }
    }
  }

  for(int i = 0; i < Bl->msswitch_len; i++){
    loggerf(ERROR, "Implement msswitch in Next_Special_Block");
  }

  Block * tmp = 0;
  if(pairs == 1){
    // printf("1 pair\n");
    if((flags & 0x0F) == 0)
      tmp = np_blocks[0].next;
    else
      tmp = np_blocks[0].prev;
  }
  else if(pairs >= 2){
    _Bool same = TRUE;
    // printf("2 pairs\n");
    for(int i = 0; i < pairs - 1; i++){
      // printf(" prev %2i:%2i   %2i:%2i next <==> prev %2i:%2i   %2i:%2i next\n",np_blocks[i].prev->module,np_blocks[i].prev->id, np_blocks[i].next->module, np_blocks[i].next->id,np_blocks[i+1].prev->module,np_blocks[i+1].prev->id, np_blocks[i+1].next->module, np_blocks[i+1].next->id);
      if(np_blocks[i].next != np_blocks[i+1].next && np_blocks[i].prev != np_blocks[i+1].prev){
        same = FALSE;
      }
    }
    if(same){
      if((flags & 0x0F) == NEXT){
        // printf("N_E_X_T");
        tmp = np_blocks[0].next;
      }
      else{
        // printf("P_R_E_V");
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
    printf("Multiple pairs: ");
    for(int i = i; i < pairs; i++){
      if(np_blocks[i].next && np_blocks[i].prev)
        printf("%i:%i<>%i:%i\t",np_blocks[i].next->module,np_blocks[i].next->id,np_blocks[i].prev->module,np_blocks[i].prev->id);
    }
    printf("\n");
  }

  _free(np_blocks);
  if(tmp){
    return tmp;
  }
  else{
    loggerf(ERROR, "FIX");
    return Bl;
  }
}

struct rail_link Next_link(Block * B, int flags){
  // dir: 0 next, 1 prev
  int dir = flags & 0x0F;

  struct rail_link next;
  next.p = 0;

  if(!B){
    loggerf(ERROR, "Empty block");
    return next;
  }

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
  return next;
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
