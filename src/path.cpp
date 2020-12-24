#include <algorithm>

#include "path.h"
#include "utils/logger.h"

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

  direction = B->dir & 0b1;

  next = &B->next;
  prev = &B->prev;

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
  char buffer[100];
  sprint(buffer);
  loggerf(DEBUG, "Path destroyed %s", buffer);
}

void Path::updateEntranceExit(){
  // Set Entrance/Exit right
  if(!direction){
    Entrance = end;
    Exit = front;
  }
  else{
    Entrance = front;
    Exit = end;
  }
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
      loggerf(TRACE, buffer);
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
      loggerf(TRACE, buffer);
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
      loggerf(TRACE, buffer);
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
      loggerf(TRACE, buffer);
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

    loggerf(TRACE, "Next Block %02d:%02d,  front %02d:%02d", B->module, B->id, this->front->module, this->front->id);

    struct rail_link * link = B->NextLink(NEXT);

    loggerf(TRACE, "                 -> link %02d:%02d:%02x", link->module, link->id, link->type);

    if(link->p.B == this->front){
      loggerf(TRACE, "FLIP");
      // dir ^= 0b1;
      B->reverse();
      link = B->NextLink(NEXT);
    }

    loggerf(TRACE, "                 -> link %02d:%02d:%02x\n", link->module, link->id, link->type);

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
  bufferptr = string;

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

int Path::reverse() { return reverse(0); }

int Path::reverse(RailTrain * ReqT){
  /* public
  ** reverse the whole path iff all trains are stopped
  ** also reverses all trains on the path except the Requested Train
  **
  ** arguments
  **  ReqT: the train that has to be exempted
  */
  loggerf(INFO, "Path::reverse");

  for(RailTrain * T: trains){
    if(T->speed){
      loggerf(INFO, "Path has a train with speed");
      return 0;
    }
  }

  direction ^= 1;

  for(Block * B: Blocks){
    B->reverse();
  }

  for(RailTrain * T: trains){
    if(T == ReqT)
      continue;

    loggerf(INFO, "Reverse Train");
    T->reverse(REVERSE_NO_BLOCKS);
  }

  std::swap(next, prev);
  std::swap(Entrance, Exit);

  return 1;
}

void Path::reserve(RailTrain * T){
  /* public
  ** reserve the whole path by a specific Train
  **
  ** arguments:
  **  T: the train that reserves the path
  */
  loggerf(TRACE, "Path %x reserve (%x)", (unsigned int)this, (unsigned int)T);
  reserved++;

  for(Block * B: Blocks){
    B->reserve();
  }

  trains.push_back(T);
  T->reservePath(this);
}

void Path::reserve(RailTrain * T, Block * B){
  /* public
  ** reserve a part of the path by a specific Train
  **   the block marks the beginning
  **
  ** arguments:
  **  T: the train that reserves the path
  **  B: the start block
  */

  if(B == Entrance){
    reserve(T);
    return;
  }

  reserved++;

  while(B && B->path == this){
    B->reserve();
    B = B->Next_Block(NEXT | SWITCH_CARE, 1);
  }

  trains.push_back(T);
  T->reservePath(this);
}

void Path::dereserve(RailTrain * T){
  /* public
  ** dereserve the whole path that is reserved by a specific Train
  **
  ** arguments:
  **  T: the train that dereserves the path
  */
  loggerf(TRACE, "Path::dereserve %x (%i)", (unsigned int)T, T->id);
  reserved--;

  for(Block * B: Blocks){
    B->dereserve();
  }

  trains.erase(std::remove_if(trains.begin(), trains.end(), [T](const auto & o) { return (o == T); }), trains.end());
  T->dereservePath(this);
}

void Path::dereserve(RailTrain * T, Block * B){
  /* public
  ** dereserve a part of the path that is reserved by a specific Train
  **  the block marks the beginning
  **
  ** arguments:
  **  T: the train that dereserves the apth
  **  B: the start block
  */
  loggerf(TRACE, "Path::dereserve %x (%i), %2i:%2i", (unsigned int)T, T->id, B->module, B->id);
  if(B == Entrance){
    reserve(T);
    return;
  }

  reserved--;

  while(B && B->path == this){
    B->dereserve();
    B = B->Next_Block(NEXT | SWITCH_CARE, 1);
  }

  trains.erase(std::remove_if(trains.begin(), trains.end(), [T](const auto & o) { return (o == T); }), trains.end());
  T->dereservePath(this);
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
