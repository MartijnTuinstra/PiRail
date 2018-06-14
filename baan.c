#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <logger.h>
#include <train.h>


#define _calloc(elements, type) my_calloc(elements, sizeof(type), __FILE__, __LINE__)
#define _realloc(p, elements, type) my_realloc(p, sizeof(type) * elements, __FILE__, __LINE__)
#define _free(p) my_free(p, __FILE__, __LINE__)

void * my_calloc(int elements, int size, char * file, int line){
  void * p = calloc(elements, size);
  floggerf(MEMORY, file, line, "calloc\tsize: %i\tpointer: %08x", elements * size, p);
  return p;
}

void * my_realloc(void * p, int size, char * file, int line){
  void * old_p = p;
  p = realloc(p, size);
  floggerf(MEMORY, file, line, "realloc\told_pointer: %08x\tnew_size: %i\tnew_pointer: %08x", old_p, size, p);
  return p;
}

void my_free(void * p, char * file, int line){
  free(p);
  floggerf(MEMORY, file, line, "free\tpointer: %08x", p);
}

void main(){
  init_logger("log.txt");
  set_level(MEMORY);
  uint16_t * p = _calloc(5, uint16_t);
  p = _realloc(p, 10, uint16_t);
  _free(p);
}
