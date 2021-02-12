#include <stdio.h>
#include <stdlib.h>

#include "utils/mem.h"
#include "utils/logger.h"
#include <pthread.h>

unsigned int allocs = 0;

struct allocations * allocations;

pthread_mutex_t mem_lock;

void init_allocs(){
  allocations = (struct allocations *)calloc(2048, sizeof(struct allocations));
  allocs = 2048;
}

void destroy_allocs(){
  free(allocations);
}

void * my_calloc(int elements, int size, const char * file, const int line){
  void * p = calloc(elements, size);
  logger.f(MEMORY, file, line, "calloc \tsize: %i \tpointer: %08x", elements * size, p);

  // pthread_mutex_lock(&mem_lock);
  // for(unsigned int i = 0; i < allocs; i++){
  //   if(!allocations[i].pointer){
  //     allocations[i].pointer = p;
  //     allocations[i].location = (char *)calloc(30, sizeof(char));
  //     sprintf(allocations[i].location, "%s:%i", file, line);
  //     break;
  //   }
  // }
  // pthread_mutex_unlock(&mem_lock);

  return p;
}

void * my_realloc(void * p, int type_size, int elements, const char * file, const int line){
  void * old_p = p;
  p = realloc(p, type_size * elements);
  logger.f(MEMORY, file, line, "realloc \told_pointer: %08x \tnew_size: %i*%i \tnew_pointer: %08x", old_p, type_size, elements, p);

  pthread_mutex_lock(&mem_lock);
  for(unsigned int i = 0; i < allocs; i++){
    if(allocations[i].pointer == old_p){
      allocations[i].pointer = p;
      break;
    }
  }
  pthread_mutex_unlock(&mem_lock);

  return p;
}

void * my_free(void ** p, const char * file, const int line){
  if(!*p)
    return 0;
  logger.f(MEMORY, file, line, "free \tpointer: %08x", p);
  free(*p);

  pthread_mutex_lock(&mem_lock);
  for(unsigned int i = 0; i < allocs; i++){
    if(allocations[i].pointer == *p){
      allocations[i].pointer = 0;
      free(allocations[i].location);
      break;
    }
  }
  pthread_mutex_unlock(&mem_lock);

  *p = 0;

  return 0;
}

void print_allocs(){
  for(unsigned int i = 0; i < allocs; i++){
    if(allocations[i].pointer){
      printf("NON FREED HEAP %8x\t%s\n", (unsigned int)allocations[i].pointer, allocations[i].location);
      free(allocations[i].pointer);
      free(allocations[i].location);
    }
  }

  destroy_allocs();
}
