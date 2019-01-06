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
  return;
}

void Init_IO(Unit * U, Node_adr adr, enum IO_type type){
  if((adr.Node < U->IO_Nodes) && 
     (adr.io < U->Node[adr.Node].io_ports) &&
     U->Node[adr.Node].io[adr.io]){

    IO_Port * A = U->Node[adr.Node].io[adr.io];

    if(A->type != IO_Undefined)
      loggerf(WARNING, "IO %i:%i:%i already in use", U->module, adr.Node, adr.io);

    A->type = type;
    A->id = adr.io;
  }
  else{
    loggerf(ERROR, "Init_IO Error");
  }
}

void update_IO(){
  for(int u = 0; u < unit_len; u++){
    if(!Units[u] || Units[u]->io_out_changed == 0)
      continue;

    for(int n = 0; n < Units[u]->IO_Nodes; n++){
      for(int io = 0; io < Units[u]->Node[n].io_ports; io++){
        if(U_IO(u, n, io)->type == IO_Output && U_IO(u, n, io)->w_state != U_IO(u, n, io)->r_state){
          loggerf(WARNING, "Update io %02i:%02i:%02i", u, n, io);
          U_IO(u, n, io)->r_state = U_IO(u, n, io)->w_state;
        }
      }
    }
  }
}