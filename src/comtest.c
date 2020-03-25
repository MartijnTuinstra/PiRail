#include "logger.h"
#include "com.h"
#include "mem.h"
#include "system.h"

struct s_systemState * SYS;

int main(){
  init_main();
  init_allocs();

  init_logger("uartlog.txt");
  set_level(DEBUG);

  UART();

  _free(SYS);
  return 1;
}
