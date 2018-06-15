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

int block_adrcmp(Block *A, Block *B){
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

void Create_Segment(int IO_Adr, struct segment_connect connect ,char max_speed, char dir,char len){
  Block * p = _calloc(1, Block);

  p->ioadr = IO_Adr;

  p->module = connect.module;
  p->id = connect.id;
  p->type = connect.type;

  p->next.type = connect.next_type;
  p->next.module = connect.next_module;
  p->next.id = connect.next_id;

  p->prev.type = connect.prev_type;
  p->prev.module = connect.prev_module;
  p->prev.id = connect.prev_id;

  p->max_speed = max_speed;
  p->dir = dir;
  p->length = len;
}

void Create_Station(){

}

void Connect_Segments(){

}

Block * Next(Block * B, int dir){

}

Block * Prev(Block * B, int dir){

}

struct rail_link Next_link(Block * B){

}

struct rail_link Prev_link(Block * B){

}
