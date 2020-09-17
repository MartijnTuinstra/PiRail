#include "path.h"
#include "logger.h"

#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/station.h"

#include "rollingstock/railtrain.h"

std::vector<Path *> pathlist;

Path::Path(Block * B){
  loggerf(TRACE, "NEW PATH %02i:%02i    %x", B->module, B->id, (unsigned int)this);

  if(B->path)
    loggerf(CRITICAL, "Block has allready a path");

  this->direction = B->dir & 0b1;

  this->next = &B->next;
  this->prev = &B->prev;

  this->front = B;
  this->front_direction = this->direction;

  this->end = B;
  this->end_direction = this->direction;

  this->Blocks.clear();
  this->Blocks.push_back(B);

  B->path = this;
  pathlist.push_back(this);
}

Path::~Path(){
  char buffer[100];
  this->sprint(buffer);
  loggerf(DEBUG, "Path destroyed %s", buffer);
}

void Path::add(Block * B, bool side){
  loggerf(DEBUG, "Add block to path %02d:%02d", B->module, B->id);
  if(side == NEXT){
    if(B->path){
      if(B->path != this){
        this->join(B->path);
        return;
      }
      else{
        return;
      }
    }
    this->Blocks.push_back(B);
    this->front = B;
    this->front_direction = B->dir;
    B->path = this;
  }
  else if(side == PREV){
    if(B->path){
      if(B->path != this){
        this->join(B->path);
        return;
      }
      else{
        return;
      }
    }
    this->Blocks.push_back(B);
    this->end = B;
    this->end_direction = B->dir;
    B->path = this;
  }
}

void Path::join(Path * P){
  loggerf(DEBUG, "Join path %02d<>%02d", this->direction, P->direction);
  if(this->direction == P->direction){
    if(this->next->p.B == P->end){
      char buffer[200];
      P->sprint(buffer);
      loggerf(WARNING, buffer);
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
      loggerf(WARNING, buffer);
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
      loggerf(WARNING, buffer);
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
      loggerf(WARNING, buffer);
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
  loggerf(TRACE, "Path Find %02d:%02d", this->Blocks[0]->module, this->Blocks[0]->id);
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

    // loggerf(INFO, "Next Block %02d:%02d,  front %02d:%02d", B->module, B->id, this->front->module, this->front->id);

    struct rail_link * link = B->NextLink(NEXT ^ dir);

    // loggerf(INFO, "                 -> link %02d:%02d:%02x", link->module, link->id, link->type);

    if(link->p.B == this->front){
      // loggerf(WARNING, "FLIP");
      dir ^= 0b1;
      link = B->NextLink(NEXT ^ dir);
    }

    // loggerf(INFO, "                 -> link %02d:%02d:%02x\n", link->module, link->id, link->type);

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

    struct rail_link * link = B->NextLink(PREV ^ dir);

    // loggerf(INFO, "                 -> link %02d:%02d:%02x", link->module, link->id, link->type);

    if(link->p.B == this->front){
      // loggerf(WARNING, "FLIP");
      dir ^= 0b1;
      link = B->NextLink(PREV ^ dir);
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
  char * bufferptr = buffer;
  for(auto b: this->Blocks){
    bufferptr += sprintf(bufferptr, "%02d:%02d ", b->module, b->id);
  }
  sprintf(string, "---%02d->>\n%02d:%02d:%02x [%02d:%02d {%s} %02d:%02d] %02d:%02d:%02x", this->Blocks.size(), this->prev->module, this->prev->id, this->prev->type,
                                                                                    this->end->module, this->end->id,
                                                                                    buffer,
                                                                                    this->front->module, this->front->id,
                                                                                    this->next->module, this->next->id, this->next->type);
}

void Path::print(){
  char buffer[200];
  this->sprint(buffer);
  printf("%s\n", buffer);
}

void Path::reverse(){
  loggerf(INFO, "Path::reverse");

  for(RailTrain * T: this->trains){
    if(T->speed){
      loggerf(INFO, "Path has a train with speed");
      return;
    }
  }

  this->direction ^= 1;

  for(Block * B: this->Blocks){
    B->reverse();
  }

  for(RailTrain * T: this->trains){
    loggerf(INFO, "Reverse Train");
    T->dir ^= 1;
    // T->setSpeedZ21(0);
  }
}

void Path::reserve(){
  this->reserved = 1;

  for(Block * B: this->Blocks){
    B->reserve();
  }
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
