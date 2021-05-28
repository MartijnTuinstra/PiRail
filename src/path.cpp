#include <algorithm>

#include "path.h"
#include "utils/logger.h"

#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/station.h"

#include "algorithm/core.h"

#include "rollingstock/train.h"

std::vector<Path *> pathlist;

Path::Path(Block * B){
  loggerf(DEBUG, "NEW PATH %02i:%02i    %x", B->module, B->id, (unsigned int)this);

  if(B->path)
    loggerf(CRITICAL, "Block has allready a path");

  direction = B->dir & 0b1;

  if(direction == 0){
    next = &B->next;
    prev = &B->prev;
  }
  else{
    next = &B->prev;
    prev = &B->next;
  }

  front = B;
  front_direction = direction;

  end = B;
  end_direction = direction;

  Blocks.clear();
  Blocks.push_back(B);

  B->path = this;
  pathlist.push_back(this);

  updateEntranceExit();
}

Path::~Path(){
  char buffer[400];
  sprint(buffer);

  Blocks.clear();
  trains.clear();
  reservedTrains.clear();

  loggerf(DEBUG, "Path destroyed %x %s", (unsigned int)this, buffer);
}

void Path::updateEntranceExit(){
  // Set Entrance/Exit right
  // if(!direction){
    Entrance = end;
    Exit = front;
  // }
  // else{
    // Entrance = front;
    // Exit = end;
  // }

}

void Path::add(Block * B, bool side){
  loggerf(DEBUG, "Add block to path %02d:%02d", B->module, B->id);

  // If block has allready a path
  //   join if different
  //   stop if same
  if(B->path){
    if(B->path != this){
      join(B->path);
      updateEntranceExit();
    }

    return;
  }

  // Add block to block list
  Blocks.push_back(B);

  // Add block to the right side
  if(side == NEXT){
    front = B;
    front_direction = B->dir;
  }
  else if(side == PREV){
    end = B;
    end_direction = B->dir;
  }

  // Link path from block
  B->path = this;
  updateEntranceExit();
}

void Path::join(Path * P){
  loggerf(DEBUG, "Join path %02d<>%02d", this->direction, P->direction);
  if(this->direction == P->direction){
    if(this->next->p.B == P->end){
      char buffer[200];
      P->sprint(buffer);
      loggerf(DEBUG, buffer);
      this->front = P->front;
      this->front_direction = P->front_direction;
      this->next = P->next;

      for(auto i: P->Blocks){
        if (!i)
          continue;

        this->Blocks.push_back(i);
        i->path = this;
      }

      uint8_t j = 0;
      for(auto i: pathlist){
        if(i == P)
          break;
        j++;
      }

      if (j < pathlist.size()){
        pathlist.erase(pathlist.begin() + j);
        delete P;
      }
      return;
    }

    if(this->prev->p.B == P->front){
      char buffer[200];
      P->sprint(buffer);
      loggerf(DEBUG, buffer);
      this->end = P->end;
      this->end_direction = P->end_direction;
      this->prev = P->prev;

      for(auto i: P->Blocks){
        if (!i)
          continue;

        this->Blocks.push_back(i);
        i->path = this;
      }

      uint8_t j = 0;
      for(auto i: pathlist){
        if(i == P)
          break;
        j++;
      }

      if (j < pathlist.size()){
        pathlist.erase(pathlist.begin() + j);
        delete P;
      }

      return;
    }
  }
  else if(this->direction != P->direction){
    if(this->next->p.B == P->front){
      char buffer[200];
      P->sprint(buffer);
      loggerf(DEBUG, buffer);
      this->front = P->end;
      this->front_direction = P->end_direction ^ 0b100;
      this->next = P->prev;

      for(auto i: P->Blocks){
        this->Blocks.push_back(i);
        i->path = this;
      }

      uint8_t j = 0;
      for(auto i: pathlist){
        if(i == P)
          break;
        j++;
      }

      if (j < pathlist.size()){
        pathlist.erase(pathlist.begin() + j);
        delete P;
      }
      return;
    }

    else if(this->prev->p.B == P->end){
      char buffer[200];
      P->sprint(buffer);
      loggerf(DEBUG, buffer);
      this->end = P->front;
      this->end_direction = P->front_direction ^ 0b100;
      this->prev = P->next;

      for(auto i: P->Blocks){
        this->Blocks.push_back(i);
        i->path = this;
      }

      uint8_t j = 0;
      for(auto i: pathlist){
        if(i == P)
          break;
        j++;
      }

      if (j < pathlist.size()){
        pathlist.erase(pathlist.begin() + j);
        delete P;
      }

      return;
    }
  }
}

void Path::find(){
  loggerf(DEBUG, "Path Find %02d:%02d", this->Blocks[0]->module, this->Blocks[0]->id);
  uint8_t i = 0;
  Block * B = 0;
  uint8_t dir = 0;
  do{
    B = 0;
    if(this->next->type == RAIL_LINK_R){
      if(this->next->p.B->type != NOSTOP)
        B = this->next->p.B;

      if(B && !((B->type == STATION && this->front->type == STATION && this->front->station == B->station) || (B->type != STATION && this->front->type != STATION)))
        B = 0;
    }

    if(!B)
      break;

    loggerf(DEBUG, "Next Block %02d:%02d,  front %02d:%02d", B->module, B->id, this->front->module, this->front->id);

    struct rail_link * link = B->NextLink(NEXT);

    loggerf(DEBUG, "                 -> link %02d:%02d:%02x", link->module, link->id, link->type);

    if(link->p.B == this->front){
      loggerf(DEBUG, "FLIP");
      // dir ^= 0b1;
      B->reverse();
      link = B->NextLink(NEXT);
    }

    loggerf(DEBUG, "                 -> link %02d:%02d:%02x\n", link->module, link->id, link->type);

    this->add(B, NEXT);
    this->next = link;

    i++;
  }
  while (B && i < 10);

  dir = 0;
  i = 0;
  do{
    B = 0;
    if(this->prev->type == RAIL_LINK_R){
      if(this->prev->p.B->type != NOSTOP)
        B = this->prev->p.B;

      if (B && !((B->type == STATION && this->end->type == STATION && this->end->station == B->station) || (B->type != STATION && this->end->type != STATION)))
        B = 0;
    }

    if(!B)
      break;

    struct rail_link * link = B->NextLink(PREV);

    // loggerf(INFO, "                 -> link %02d:%02d:%02x", link->module, link->id, link->type);

    if(link->p.B == this->front){
      // loggerf(WARNING, "FLIP");
      B->reverse();
      link = B->NextLink(PREV);
    }

    // loggerf(INFO, "Prev Block %02d:%02d -> link %02d:%02d:%02x", B->module, B->id, link->module, link->id, link->type);

    this->add(B, PREV);
    this->prev = link;

    i++;
  }
  while (B && i < 10);
}

void Path::sprint(char * string){
  char buffer[200];
  char * bufferptr = &buffer[0];
  for(auto b: this->Blocks){
    bufferptr += sprintf(bufferptr, "%02d:%02d ", b->module, b->id);
  }

  string += sprintf(string, "%2i:%2i ---%02d->> %2i:%2i\n", Entrance->module, Entrance->id, Blocks.size(), Exit->module, Exit->id);

  string += sprintf(string, "%02d:%02d:%02x ", prev->module, prev->id, prev->type);
  string += sprintf(string, "[%02d:%02d {%s} %02d:%02d]", end->module, end->id, buffer, front->module, front->id);
  string += sprintf(string, " %02d:%02d:%02x", next->module, next->id, next->type);
}

void Path::print(){
  char buffer[200];
  sprint(buffer);
  printf("%s\n", buffer);
}

bool Path::reversable(){
  for(Train * T: trains){
    if(T->speed){
      loggerf(INFO, "Path has a moving train");
      return false;
    }
  }

  return true;
}

void Path::reverse(){
  return reverse((Train *)0);
}

void Path::reverse(Train * T){
  loggerf(INFO, "Path::reverse");

  if(!reversable())
    return;

  direction ^= 1;

  for(Block * B: Blocks){
    B->reverse();
  }

  for(Train * t: trains){
    if(T == t)
      continue;

    loggerf(INFO, "Reverse Train");
    t->reverseFromPath(this);
  }

  std::swap(next, prev);
  std::swap(Entrance, Exit);
}

void Path::reserve(Train * T){
  reserved = true;
  reservedTrains.push_back(T);

  for(auto B: Blocks){
    T->reserveBlock(B);
  }

  if(Exit->Alg.next){
    Algorithm::print_block_debug(Exit->Alg.N[0]);
    Algorithm::rail_state(&Exit->Alg.N[0]->Alg, 0);
  }
}

void Path::reserve(Train * T, Block * B){
  reserved = true;
  reservedTrains.push_back(T);

  if(B->Alg.next > 0)
    B = B->Alg.N[0];
  else
    B = 0;

  while(B && B->path == this){
    T->reserveBlock(B);

    if(B->Alg.next > 0)
      B = B->Alg.N[0];
    else
      B = 0;
  }

  if(Exit->Alg.next){
    Algorithm::print_block_debug(Exit->Alg.N[0]);
    Algorithm::rail_state(&Exit->Alg.N[0]->Alg, 0);
  }
}

void Path::dereserve(Train * T){
  reservedTrains.erase(std::remove_if(reservedTrains.begin(),
                                     reservedTrains.end(),
                                     [T](const auto & o) { return (o == T); }),
                                     reservedTrains.end()
                                    );

  for(auto B: Blocks){
    T->dereserveBlock(B);
  }

  if(reservedTrains.size() == 0)
    reserved = false;
}
void Path::trainAtEnd(Train * T){
  reservedTrains.erase(std::remove_if(reservedTrains.begin(),
                                     reservedTrains.end(),
                                     [T](const auto & o) { return (o == T); }),
                                     reservedTrains.end()
                                    );

  if(reservedTrains.size() == 0)
    reserved = false;
}



void Path::reg(Train * T){
  trains.push_back(T);
}
void Path::unreg(Train * T){
  trains.erase(std::remove_if(trains.begin(),
                              trains.end(),
                             [T](const auto & o) { return (o == T); }),
                             trains.end()
                            );
}

void pathlist_find(){
  uint8_t i = 0;
  uint8_t j = pathlist.size();
  do{
    pathlist[i]->find();

    if(j == pathlist.size()){
      i++;
    }
    else{
      j = pathlist.size();
    }
  }
  while(i < pathlist.size());

  i = 0;
  // for(auto k: pathlist){
  //   char buffer[100];
  //   k->sprint(buffer);
  //   printf("Path %2d/%x: %s\n", i++, (unsigned int)k, buffer);
  // }
}
