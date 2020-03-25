#include "logger.h"
#include "com.h"
#include "mem.h"
#include "system.h"

struct systemState * _SYS;

int main(){
   _SYS = _calloc(1, struct systemState);
  init_allocs();

  init_logger("uartlog.txt");
  set_level(DEBUG);

  UART();

  _free(_SYS);
  return 1;
}
