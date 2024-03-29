#include <algorithm>

#include "path.h"
#include "utils/logger.h"

#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/station.h"

#include "flags.h"

#include "algorithm/core.h"

#include "rollingstock/train.h"

std::vector<Path *> pathlist;

Path::Path(Block * B){
  loggerf(DEBUG, "NEW PATH %02i:%02i    %x", B->module, B->id, (unsigned long)this);

  if(B->path)
    loggerf(WARNING, "Block has allready a path");

  direction = B->dir & FL_DIRECTION_MASK;

  StationPath = (B->station != 0);
  SwitchPath  = (B->switch_len != 0 || B->MSSw != 0);

  if(direction == NEXT){
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

  // TODO cleanup
  // polarity_type = B->polarity_type;

  Blocks.clear();
  Blocks.push_back(B);

  B->path = this;
  pathlist.push_back(this);

  length = B->length;

  updateEntranceExit();
}

Path::~Path(){
  char buffer[400];
  sprint(1, buffer);

  for(auto b: Blocks){
    if(!b)
      continue;

    b->path = nullptr;
    loggerf(INFO, " Removing block %02i:%02i to path %x", b->module, b->id, b->path);
  }

  Blocks.clear();
  trains.clear();
  reservedTrains.clear();

  loggerf(DEBUG, "Path destroyed %x %s", (unsigned long)this, buffer);
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
  loggerf(TRACE, "Add block to path %02d:%02d", B->module, B->id);

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
  length += B->length;

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

void Path::join(Path * P, Block ** side, Block * Pside, bool * sideDirection, bool PsideDirection, RailLink ** link, RailLink * Plink){
  *side = Pside;
  *sideDirection = PsideDirection;
  *link = Plink;

  for(auto b: P->Blocks){
    if (!b)
      continue;

    Blocks.push_back(b);
    b->path = this;
  }

  // All blocks are copied, so list can be cleared
  P->Blocks.clear();

  pathlist.erase(std::remove_if(pathlist.begin(),
                                pathlist.end(),
                                [P](const auto & o) { return (o == P); }),
                 pathlist.end());

  length += P->length;

  delete P;
}

void Path::join(Path * P){
  loggerf(TRACE, "Join path %02d<>%02d", this->direction, P->direction);
  if(this->direction == P->direction){
    if(this->next->p.B == P->end){
      join(P, &front, P->front, &front_direction, P->front_direction, &next, P->next);
      return;
    }

    if(this->prev->p.B == P->front){
      join(P, &end, P->end, &end_direction, P->end_direction, &prev, P->prev);
      return;
    }
  }
  else if(this->direction != P->direction){
    if(this->next->p.B == P->front){
      join(P, &front, P->end, &front_direction, P->end_direction, &next, P->prev);
      return;
    }

    else if(this->prev->p.B == P->end){
      join(P, &end, P->front, &end_direction, P->front_direction, &prev, P->next);
      return;
    }
  }
}

void Path::find(){
  loggerf(DEBUG, "Path Find %02d:%02d / %x", this->Blocks[0]->module, this->Blocks[0]->id, (unsigned long)this);

  if(Blocks[0]->type == NOSTOP)
    return;

  find(&next, &front, NEXT);
  find(&prev, &end,   PREV);
}

void Path::find(RailLink ** linkPtr, Block ** ptr_side, uint8_t SearchDir){
  loggerf(DEBUG, "Path find %c", (SearchDir == NEXT) ? 'N' : 'P');
  Block * B = 0;
  uint8_t i = 0;
  RailLink * link = *linkPtr;
  do{
    Block * side = *ptr_side;

    B = 0;
    if(link->type == RAIL_LINK_R){
      if(link->p.B->type != NOSTOP)
        B = link->p.B;

      if(!B)
        break;

      if(!((B->type == STATION && side->type == STATION && side->station == B->station) || (B->type != STATION && side->type != STATION)))
        break;

      // loggerf(INFO, "Block has a polarity type %i", B->polarity_type);
      // if(B->polarity_type == BLOCK_FL_POLARITY_LINKED_BLOCK){
      //   loggerf(INFO, "Block has a linked block %x == %x", B->polarity_link, Blocks[0]);
      //   if(B->polarity_link != Blocks[0])
      //     break;
      // }
      // else if(polarity_type >= BLOCK_FL_POLARITY_NO_IO && B->polarity_type == polarity_type){
      //   break;
      // }
      // else if(B->polarity_type != polarity_type){
      //   if(!B->path) new Path(B);
      //   break;
      // }
    }

    if(!B)
      break;

    // loggerf(DEBUG, "Next Block %02d:%02d,  side %02d:%02d", B->module, B->id, side->module, side->id);

    // if(!side->cmpPolarity(B)){
    //   // loggerf(DEBUG, "  wrong polarity, breaking path B->%2i:%2i:%i," \
    //                 //  "  side->%2i:%2i:%i", B->next.module, B->next.id, B->next.type,
    //                                         // side->next.module, side->next.id, side->next.type);
    //   if(!B->path)
    //     B->path = new Path(B);
    //   break;
    // }

    link = B->NextLink(SearchDir);

    // loggerf(DEBUG, "                 -> link %02d:%02d:%02x", link->module, link->id, link->type);

    if(link->p.B == side){
      B->reverse();
      link = B->NextLink(SearchDir);
    }

    // loggerf(DEBUG, "                 -> link %02d:%02d:%02x\n", link->module, link->id, link->type);

    add(B, SearchDir);
    *linkPtr = link;

    i++;
  }
  while (B && i < 1000);
}

void Path::setMaxLength(){
  Path * adjacentPath = this;

  maxLength = length;

  if(Blocks[0]->type == NOSTOP)
    return;

  // Find the two surrounding paths and add there length
  if(next->type == RAIL_LINK_R){
    adjacentPath = next->p.B->path;

    // TODO cleanup    
    // if(adjacentPath && adjacentPath->polarity_type != BLOCK_FL_POLARITY_DISABLED && next->p.B->type != NOSTOP)
    //   maxLength += adjacentPath->length;
  }

  if(prev->type == RAIL_LINK_R){
    adjacentPath = prev->p.B->path;
    
    // TODO cleanup   
    // if(adjacentPath && adjacentPath->polarity_type != BLOCK_FL_POLARITY_DISABLED && prev->p.B->type != NOSTOP)
    //   maxLength += adjacentPath->length;
  }
}

void Path::sprint(uint8_t detail, char * string){
  char buffer[200];
  char * bufferptr = &buffer[0];
  for(auto b: this->Blocks){
    bufferptr += sprintf(bufferptr, "%02d:%02d ", b->module, b->id);
  }

  char l[4] = "---";
  char r[4] = "---";
  
  if(direction)
    l[1] = l[0] = '<';
  else
    r[2] = r[1] = '>';

  // TODO cleanup
  // if(polarity)
  //   l[2] = '<';
  // else
  //   r[0] = '>';

  switch(detail){
    case 0:
      string += sprintf(string, "%2i:%2i %s%02ld%s %2i:%2i", Entrance->module, Entrance->id, l, Blocks.size(), r, Exit->module, Exit->id);
      break;
    
    case 1:
      string += sprintf(string, "%2i:%2i %s%02ld/%4dcm/M:%4dcm%s %2i:%2i", Entrance->module, Entrance->id, l, Blocks.size(), length, maxLength, r, Exit->module, Exit->id);
      break;

    case 2:
      string += sprintf(string, "%02d:%02d:%02x %s[%02d:%02d ...%2i... %02d:%02d]%s %02d:%02d:%02x",
                        prev->module, prev->id, prev->type,
                        l, end->module, end->id, Blocks.size(), front->module, front->id, r,
                        next->module, next->id, next->type);
      break;

    case 3:
      string += sprintf(string, "%2i:%2i %s%02ld/%4dcm/M:%4dcm%s %2i:%2i\n", Entrance->module, Entrance->id, l, Blocks.size(), length, maxLength, r, Exit->module, Exit->id);
      string += sprintf(string, "%02d:%02d:%02x %s[%02d:%02d {%s} %02d:%02d]%s %02d:%02d:%02x",
                        prev->module, prev->id, prev->type,
                        l, end->module, end->id, buffer, front->module, front->id, r,
                        next->module, next->id, next->type);
      break;
  }
}

void Path::print(){
  char buffer[200];
  sprint(3, buffer);
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

/*
void Path::flipPolarity(bool _reverse){
  if(_reverse && !reversable())
    return;

  if(!polarityFlippable())
    return;

  polarity ^= 1;

  for(Block * B: Blocks){
    B->flipPolarity();
  }

  if(_reverse)
    reverse();
}
void Path::flipPolarityNow(){
  polarity ^= 1;

  for(Block * B: Blocks){
    B->flipPolarity();
  }
}

bool Path::polarityFlippable(){
  if(polarity_type == BLOCK_FL_POLARITY_DISABLED)
    return 0;

  if(Entrance->blocked){
    Block * B = getBlockAtEdge(prev);

    loggerf(WARNING, "polFlip? Entrance %02i:%02i%cT%i, %02i:%02i%cT%i",
            Entrance->module, Entrance->id, Entrance->blocked ? 'B': ' ', Entrance->train ? Entrance->train->id : -1,
            B->module, B->id, B->blocked ? 'B': ' ', B->train ? B->train->id : -1);

    if(B && B->blocked && Entrance->train == B->train){
      return 0;
    }
  }
  
  if(Exit->blocked){
    Block * B = getBlockAtEdge(next);

    loggerf(WARNING, "polFlip? Exit %02i:%02i%cT%i, %02i:%02i%cT%i",
            Exit->module, Exit->id, Exit->blocked ? 'B': ' ', Exit->train ? Exit->train->id : -1,
            B->module, B->id, B->blocked ? 'B': ' ', B->train ? B->train->id : -1);

    if(B && B->blocked && Exit->train == B->train){
      return 0;
    }
  }
  return 1;
}
*/

void Path::reserve(Train * T){
  reserved = true;
  reservedTrains.push_back(T);

  for(auto B: Blocks){
    T->reserveBlock(B);
  }

  if(Exit->Alg.N->group[3]){
    Algorithm::print_block_debug(Exit->Alg.N->B[0]);
    Algorithm::rail_state(&Exit->Alg.N->B[0]->Alg, 0);
  }
}

void Path::reserve(Train * T, Block * B){
  loggerf(INFO, "Path::Reserve T%i from %02i:%02i", T->id, B->module, B->id);
  reserved = true;
  reservedTrains.push_back(T);

  if(B->Alg.N->group[3] > 0)
    B = B->Alg.N->B[0];
  else
    B = 0;

  while(B && B->path == this){
    loggerf(INFO, "  Path::Reserve %02i:%02i", B->module, B->id);
    T->reserveBlock(B);

    if(B->Alg.N->group[3] > 0)
      B = B->Alg.N->B[0];
    else
      B = 0;
  }

  if(Exit->Alg.N->group[3]){
    Algorithm::print_block_debug(Exit->Alg.N->B[0]);
    Algorithm::rail_state(&Exit->Alg.N->B[0]->Alg, 0);
  }
}

void Path::trainAtEnd(Train * T){
  // Removes train from reserved list and resets the reserved flag if no other train is in the list
  reservedTrains.erase(std::remove_if(reservedTrains.begin(),
                                     reservedTrains.end(),
                                     [T](const auto & o) { return (o == T); }),
                                     reservedTrains.end()
                                    );

  if(reservedTrains.size() == 0)
    reserved = false;
}
void Path::dereserve(Train * T){
  // Dereserves blocks associated with the train
  //  also exectures trainAtEnd

  trainAtEnd(T);

  for(auto B: Blocks){
    T->dereserveBlock(B);
  }
}



void Path::trainEnter(Train * T){
  // Train enters path, add the train to the train list
  loggerf(DEBUG, "Path::trainEnter  %2i:%2i T%2i", Entrance->module, Entrance->id, T->id);
  trains.push_back(T);
}
void Path::trainExit(Train * T){
  // Train leaves path, removes train from train list
  loggerf(DEBUG, "Path::trainExit  %2i:%2i T%2i", Exit->module, Exit->id, T->id);
  trains.erase(std::remove_if(trains.begin(),
                              trains.end(),
                             [T](const auto & o) { return (o == T); }),
                             trains.end()
                            );
}

void Path::analyzeTrains(){
  trains.clear();

  Train * prevTrain = 0;

  for(auto b: Blocks){
    Train * T = b->train;
    if(T && T != prevTrain){
      trains.push_back(T);
      prevTrain = T;
    }
  }
}

Block * Path::getBlockAtEdge(RailLink * edge){
  return edge->getNextBlock();
}

void pathlist_find(){
  if(pathlist.size() == 0){
    loggerf(INFO, "No paths to find!!!");
    return;
  }

  uint8_t i = 0;
  Path * P;
  do{
    P = pathlist[i];
    P->find();

    if(P == pathlist[i])
      i++;
  }
  while(i < pathlist.size());

  i = 0;
  for(auto p: pathlist){
    p->setMaxLength();
    
    char buffer[400];
    p->sprint(2, buffer);
    loggerf(DEBUG, "Path %2d/%8x  %4dcm: %s\n", i++, (unsigned long)p, p->maxLength, buffer);
  }

  i = 0;
  // for(auto k: pathlist){
  //   char buffer[100];
  //   k->sprint(buffer);
  //   printf("Path %2d/%x: %s\n", i++, (unsigned long)k, buffer);
  // }
}
