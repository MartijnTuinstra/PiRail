#include <stdio.h>
#include <stdlib.h>
#include "logger.h"
#include "system.h"
#include "mem.h"
#include "IO.h"

void Add_IO_Node(Unit * U, int Node_nr, int IO){
  IO_Node Z;

  Z.id = Node_nr;
  Z.io_ports = IO;
  
  Z.io = _calloc(IO, IO_Port*);

  for(int i = 0; i<IO; i++){
    Z.io[i] = _calloc(1, IO_Port);
  }

  if(U->IO_Nodes <= Node_nr){
    U->Node = _realloc(U->Node, U->IO_Nodes + 1, IO_Node);
  }

  U->Node[Node_nr] = Z;
  loggerf(INFO, "Node %i, IO %i\n", Node_nr, IO);
  return;
}
