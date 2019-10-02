#include "rail.h"
#include "system.h"
#include "mem.h"
#include "modules.h"
#include "switch.h"
#include "logger.h"
#include "IO.h"
#include "algorithm.h"

#include <signal.h>

Station ** stations;
int stations_len;

char * rail_states_string[8] = {
  "BLOCKED",
  "DANGER",
  "RESTRICTED",
  "CAUTION",
  "PROCEED",
  "RESERVED",
  "RESERVED_SWITCH",
  "UNKNOWN" 
};


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
  p->reverse_state = PROCEED;

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

int dircmp(Block *A, Block *B){
  if((A->dir == 2 && (B->dir == 1 || B->dir == 0)) || ((A->dir == 1 || A->dir == 0) && B->dir == 2)){
    return 1;
  }else if(A->dir == B->dir){
    return 1;
  }else if(((A->dir == 0 || A->dir == 2) && B->dir == 0b101) || (A->dir == 1 && B->dir == 0b100)){
    return 1;
  }else if(((B->dir == 0 || B->dir == 2) && A->dir == 0b101) || (B->dir == 1 && A->dir == 0b100)){
    return 1;
  }
  else{
    return 0;
  }
}

int dircmp_algor(Algor_Block * A, Algor_Block * B){
  return dircmp(A->p.B, B->p.B);
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
    // loggerf(ERROR, "IMPLEMENT SPECIAL BLOCK");
    // return 0;
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

  if(!next.p){
    if(next.type != RAIL_LINK_E && next.type != RAIL_LINK_C)
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
      if(level <= 0 && ((Switch *)next.p)->Detection != B){
        return ((Switch *)next.p)->Detection;
      }
      else{
        return Next_Switch_Block((Switch *)next.p, next.type, flags, level);
      }
    }
  }
  else if(next.type == RAIL_LINK_M || next.type == RAIL_LINK_m){
    if(Next_check_Switch(B, next, flags)){
      if(level <= 0 && ((MSSwitch *)next.p)->Detection != B){
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
      loggerf(ERROR, "No solution");
      // if(np_blocks[pairs].next){
      //   printf("N%i\t", Next_check_Switch_Path_one_block(Bl, np_blocks[pairs].next, np_blocks[pairs].next->next, SWITCH_CARE));
      //   printf(" %i\t", Next_check_Switch_Path_one_block(Bl, np_blocks[pairs].next, np_blocks[pairs].next->prev, SWITCH_CARE));
      // }
      // if(np_blocks[pairs].prev){
      //   printf("P%i\t", Next_check_Switch_Path_one_block(Bl, np_blocks[pairs].prev, np_blocks[pairs].prev->next, SWITCH_CARE));
      //   printf(" %i\t", Next_check_Switch_Path_one_block(Bl, np_blocks[pairs].prev, np_blocks[pairs].prev->prev, SWITCH_CARE));
      // }
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

Block * Next_Switch_Block(Switch * S, enum link_types type, int flags, int level){
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
    if(S->Detection != next.p){
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
      if(N->Detection && S->Detection != N->Detection){
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
    if(N->Detection && S->Detection != N->Detection && level == 1){
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

Block * Next_MSSwitch_Block(MSSwitch * S, enum link_types type, int flags, int level){
  struct rail_link * next;

  if(flags & SWITCH_CARE){
    loggerf(CRITICAL, "Fix next_msswitch_block switch state care");
  }

  if(type == RAIL_LINK_TT){
    //turntable
    if(S->Detection){
      if(S->Detection->dir & 0x4){ // reversed
        if(flags & PREV)
          next = &S->sideB[S->state & 0x7F];
        else
          next = &S->sideA[S->state & 0x7F];
      }
      else{
        if(flags & PREV)
          next = &S->sideA[S->state & 0x7F];
        else
          next = &S->sideB[S->state & 0x7F];
      }
    }
  }
  else if(type == RAIL_LINK_M){
    next = &S->sideB[S->state & 0x7F];
  }
  else if(type == RAIL_LINK_m){
    next = &S->sideA[S->state & 0x7F];
  }

  if(!next->p){
    if(next->type != RAIL_LINK_E)
      loggerf(ERROR, "NO POINTERS");
    return 0;
  }

  if(next->type == RAIL_LINK_R){
    if(S->Detection != next->p){
      level--;
    }
    if(level <= 0){
      return (Block *)next->p;
    }
    else{
      return _Next((Block *)next->p, flags, level);
    }
  }
  else if(next->type == RAIL_LINK_S || next->type == RAIL_LINK_s){
    if(Next_check_Switch(S, *next, flags)){
      return Next_Switch_Block((Switch *)next->p, next->type, flags, level);
    }
  }
  else if(next->type == RAIL_LINK_M || next->type == RAIL_LINK_m){
    if(Next_check_Switch(S, *next, flags)){
      return Next_MSSwitch_Block((MSSwitch *)next->p, next->type, flags, level);
    }
  }
  else if(next->type == RAIL_LINK_E){
    return 0;
  }

  return 0;
}

void Reserve_To_Next_Switch(Block * B){
  loggerf(WARNING, "Block_Reverse %02i:%02i", B->module, B->id);
  Block * Next_Block = 0;

  if(B->Alg.next > 0 && B->Alg.N[0]->len == 0)
    Next_Block = B->Alg.N[0]->p.B;

  while(1){
    if(!Next_Block){
      break;
    }
    // else if(Next_Block->type == SPECIAL){
    //   putAlgorQueue(Next_Block, 1);
    //   break;
    // }

    loggerf(INFO, "%02i:%02i", Next_Block->module, Next_Block->id);
    Block_reserve(Next_Block);

    if(Next_Block->Alg.next > 0 && Next_Block->Alg.N[0]->len == 0)
      Next_Block = Next_Block->Alg.N[0]->p.B;
    else
      Next_Block = 0;

    usleep(100000);
  }
}

void Block_Reverse(Algor_Blocks * AB){
  if(!AB)
    return;
  loggerf(WARNING, "Block_Reverse %02i:%02i", AB->B->p.SB[0]->module, AB->B->p.SB[0]->id);

  //_ALGOR_BLOCK_APPLY(_ABl, _A, _B, _C) if(_ABl->len == 0){_A}else{_B;for(uint8_t i = 0; i < _ABl->len; i++){_C}}
  int tmp_state;
  _ALGOR_BLOCK_APPLY(AB->B, i,
    AB->B->p.B->dir ^= 0b100; tmp_state = AB->B->p.B->state; AB->B->p.B->state = AB->B->p.B->reverse_state; AB->B->p.B->reverse_state = tmp_state;,
    ,
    AB->B->p.SB[i]->dir ^= 0b100; tmp_state = AB->B->p.SB[i]->state; AB->B->p.SB[i]->state = AB->B->p.SB[i]->reverse_state; AB->B->p.SB[i]->reverse_state = tmp_state;)

  uint8_t len = 0;
  if(AB->next > AB->prev)
    len = AB->next;
  else
    len = AB->prev;

  {
    uint8_t temp = AB->next;
    AB->next = AB->prev;
    AB->prev = temp;
  }

  Algor_Block * tmp;
  for(uint8_t i = 0; i < len; i++){
    tmp = AB->N[i];
    AB->N[i] = AB->P[i];
    AB->P[i] = tmp;
  }

  if(AB->B->len){ // Switchblock
    for(int i = 0; i < AB->prev; i++){
      if(!AB->P[i])
        continue;

      _ALGOR_BLOCK_APPLY(AB->P[i], j,
        AB->P[i]->p.B->algorchanged = 1; AB->P[i]->p.B->IOchanged = 1;,
        ,
        AB->P[i]->p.SB[j]->algorchanged = 1; AB->P[i]->p.SB[j]->IOchanged = 1;)
    }
    for(int i = 0; i < AB->next; i++){
      if(!AB->N[i])
        continue;
      
      _ALGOR_BLOCK_APPLY(AB->N[i], j,
        AB->N[i]->p.B->algorchanged = 1; AB->N[i]->p.B->IOchanged = 1;,
        ,
        AB->N[i]->p.SB[j]->algorchanged = 1; AB->N[i]->p.SB[j]->IOchanged = 1;)
    }
    putList_AlgorQueue(*AB, 1);
  }

  Algor_print_block_debug(AB->B->p.B);
}

void Block_reserve(Block * B){
  if(B->state >= PROCEED)
    B->state = RESERVED;
  B->reverse_state = DANGER;

  BLOCK_RESERVE(B);

  B->statechanged = 1;
  Units[B->module]->block_state_changed = 1;
}

int Block_Reverse_To_Next_Switch(Block * B){
  loggerf(WARNING, "Block_Reverse_To_Next_Switch %02i:%02i", B->module, B->id);
  Block * Next_Block = 0;

  if(B->Alg.next > 0 && B->Alg.N[0]->len == 0)
    Next_Block = B->Alg.N[0]->p.B;

  while(1){
    if(!Next_Block){
      break;
    }

    Block_Reverse(&Next_Block->Alg);
    Block_reserve(Next_Block);

    if(Next_Block->Alg.next > 0 && Next_Block->Alg.N[0]->len == 0)
      Next_Block = Next_Block->Alg.N[0]->p.B;
    else
      Next_Block = 0;
  }
  return 0;
}
