#include "mem.h"
#include "logger.h"
#include "system.h"

#include "sim.h"
#include "rail.h"
#include "switch.h"
#include "train.h"
#include "modules.h"

#include "submodule.h"
#include "algorithm.h"
#include "websocket_msg.h"

pthread_mutex_t mutex_lockA;

#define delayA 5000000
#define delayB 5000000
#define OneSec 1000000

#define TRAIN_A_LEN   5 //cm
#define TRAIN_A_SPEED 5 //cm/s

#define TRAIN_B_LEN   5 //cm
#define TRAIN_B_SPEED 5 //cm/s


void change_Block(Block * B, enum Rail_states state){
  B->IOchanged = 1;
  B->state = state;
  if (state == BLOCKED)
    B->blocked = 1;
  else
    B->blocked = 0;

  putAlgorQueue(B, 1);
  // process(B, 3);
}

void *TRAIN_SIMA(){
  while(SYS->LC.state != Module_Run){
    usleep(10000);
  }
  Block *B = Units[21]->B[1];
  Block *N = Units[21]->B[1];
  Block *N2 = 0;

  B->state = BLOCKED;
  B->blocked = 1;
  B->IOchanged = 1;

  putAlgorQueue(B, 1);

  usleep(100000);

  if(B->train){
    while(!B->train->p){
      usleep(1000);
    }
  }

  SYS->SimA.state = Module_Run;
  WS_stc_SubmoduleState();

  while(SYS->SimA.state & Module_Run){

    N = B->Alg.N[0];
    if(!N){
      loggerf(WARNING, "Sim A reached end of the line");
      return 0;
    }
    loggerf(INFO, "Sim A step %i:%i", N->module, N->id);
    change_Block(N, BLOCKED);

    // IF len(N) < len(TRAIN)
    if(N->length < TRAIN_A_LEN){
      usleep((N->length/TRAIN_A_SPEED) * OneSec);
      N2 = N->Alg.N[0];
      if(!N2){
        loggerf(WARNING, "Sim A reached end of the line");
        return 0;
      }
      loggerf(DEBUG, "Sim A substep %i:%i", N2->module, N2->id);

      change_Block(N2, BLOCKED);
      usleep(((TRAIN_A_LEN - N->length)/TRAIN_A_SPEED) * OneSec);
      change_Block(B, PROCEED);
      if(N2 && N2->length > TRAIN_A_LEN){
        usleep(((N2->length - (TRAIN_A_LEN - N->length))/TRAIN_A_SPEED) * OneSec);
        change_Block(N, PROCEED);
        usleep(((N2->length - TRAIN_A_LEN)/TRAIN_A_SPEED) * OneSec);

        B = N2;
      }
      else{
        loggerf(WARNING, "Two short blocks smaller than train A");
        change_Block(N, PROCEED);
        usleep(OneSec);
        B = N2;
      }
    }
    else{
      usleep((TRAIN_A_LEN/TRAIN_A_SPEED) * OneSec);
      change_Block(B, PROCEED);
      usleep(((N->length - TRAIN_A_LEN)/TRAIN_A_SPEED) * OneSec);

      B = N;
    }
  }

  return 0;
}

void *TRAIN_SIMB(){
  while(SYS->LC.state != Module_Run){
    usleep(10000);
  }
  Block *B = Units[26]->B[2];
  Block *N = Units[26]->B[2];
  Block *N2 = 0;

  // Reserve_To_Next_Switch(B);

  B->state = BLOCKED;
  B->blocked = 1;
  B->IOchanged = 1;

  putAlgorQueue(B, 1);

  usleep(100000);

  SYS->SimB.state = Module_Run;
  WS_stc_SubmoduleState();

  while(SYS->SimB.state & Module_Run){

    N = B->Alg.N[0];
    if(!N){
      loggerf(WARNING, "Sim B reached end of the line");
      return 0;
    }
    loggerf(INFO, "Sim B step %i:%i", N->module, N->id);
    change_Block(N, BLOCKED);

    // IF len(N) < len(TRAIN)
    if(N->length < TRAIN_B_LEN){
      usleep((N->length/TRAIN_B_SPEED) * OneSec);
      N2 = N->Alg.N[0];
      if(!N2){
        loggerf(WARNING, "Sim B reached end of the line");
        return 0;
      }
      loggerf(DEBUG, "Sim B substep %i:%i", N2->module, N2->id);

      change_Block(N2, BLOCKED);
      usleep(((TRAIN_B_LEN - N->length)/TRAIN_B_SPEED) * OneSec);
      change_Block(B, PROCEED);
      if(N2 && N2->length > TRAIN_B_LEN){
        usleep(((N2->length - (TRAIN_B_LEN - N->length))/TRAIN_B_SPEED) * OneSec);
        change_Block(N, PROCEED);
        usleep(((N2->length - TRAIN_B_LEN)/TRAIN_B_SPEED) * OneSec);

        B = N2;
      }
      else{
        loggerf(WARNING, "Two short blocks smaller than train B");
        change_Block(N, PROCEED);
        usleep(OneSec);
        B = N2;
      }
    }
    else{
      usleep((TRAIN_B_LEN/TRAIN_B_SPEED) * OneSec);
      change_Block(B, PROCEED);
      usleep(((N->length - TRAIN_B_LEN)/TRAIN_B_SPEED) * OneSec);

      B = N;
    }
  }

  return 0;
}


int init_connect_Algor(struct ConnectList * List){
  // printf("init_connect_Algor\n");
  int return_value = 0;
  for(int i = 0;i < unit_len;i++){
  if(!Units[i])
    continue;

  for(int j = 0;j < Units[i]->block_len; j++){
    if(!Units[i]->B[j])
      continue;

    if(Units[i]->B[j]->next.type == RAIL_LINK_C || Units[i]->B[j]->prev.type == RAIL_LINK_C){
      printf("found block %i:%i\n",i,j);
      if(List->list_index <= List->length + 1){
        struct rail_link ** temp = _calloc(List->list_index+8, struct rail_link *);
        for(int q = 0;q < List->list_index;q++){
        temp[q] = List->R_L[q];
        }
        _free(List->R_L);
        List->R_L = temp;
        List->list_index += 8;
      }
      // printf("write index: %i\n",List->length);
      List->R_L[List->length] = _calloc(1, struct rail_link);
      List->R_L[List->length]->type = 'R';
      List->R_L[List->length++]->p  = Units[i]->B[j];
    }
  }

  for(int j = 0;j < Units[i]->switch_len; j++){
    if(!Units[i]->Sw[j])
      continue;

    if(Units[i]->Sw[j]->app.type == RAIL_LINK_C || Units[i]->Sw[j]->div.type == RAIL_LINK_C || Units[i]->Sw[j]->str.type == RAIL_LINK_C){
      printf("module %i, switch %i\n",i,j);
      if(List->list_index <= List->length + 1){
        struct rail_link ** temp = _calloc(List->list_index+8, struct rail_link *);
        for(int q = 0;q < List->list_index;q++){
        temp[q] = List->R_L[q];
        }
        _free(List->R_L);
        List->R_L = temp;
        List->list_index += 8;
      }
      List->R_L[List->length] = _calloc(1, struct rail_link);
      List->R_L[List->length]->type = 'S';
      List->R_L[List->length++]->p  = Units[i]->Sw[j];
    }
  }

  return_value += Units[i]->connections_len;
  }
  return return_value;
}

_Bool find_and_connect(uint8_t ModuleA, char anchor_A, uint8_t ModuleB, char anchor_B){
  //Node shouldn't be connected to the same Module
  if(ModuleA == ModuleB){return FALSE;}

  char typeA = 0;
  char typeB = 0;

  _Bool connected = FALSE;

  // printf("find_and_connect: %i:%i\t\t%i:%i\n",ModuleA,anchor_A,ModuleB,anchor_B);

  for(int Rail = 1;Rail<3;Rail++){
    struct rail_link A;A.p = 0;
    struct rail_link B;B.p = 0;

    //Find Anchor A
    // - Find a Block
      for(int k = 0;k<Units[ModuleA]->block_len;k++){
      if(Units[ModuleA]->B[k]){
          if(Units[ModuleA]->B[k]->prev.type == RAIL_LINK_C){
          // printf(" - A block Prev %i:%i",ModuleA,k);
          if(Units[ModuleA]->B[k]->prev.module == anchor_A && Units[ModuleA]->B[k]->prev.id == Rail){
            // printf("++++++\n");
            A.type = RAIL_LINK_R;
            typeA  = 'P';
            A.p = Units[ModuleA]->B[k];
            break;
          }
          // printf("\n");
          }
          else if(Units[ModuleA]->B[k]->next.type == RAIL_LINK_C){
          // printf(" - A block Next %i:%i",ModuleA,k);
          if(Units[ModuleA]->B[k]->next.module == anchor_A && Units[ModuleA]->B[k]->next.id == Rail){
            // printf("++++++\n");
            A.type = RAIL_LINK_R;
            typeA  = 'N';
            A.p = Units[ModuleA]->B[k];
            break;
          }
          // printf("\n");
          }
        }
      }
    // - Find a switch
      if(!A.p){
        for(int k = 0;k<Units[ModuleA]->switch_len;k++){
          if(Units[ModuleA]->Sw[k]){
            if(Units[ModuleA]->Sw[k]->app.type == RAIL_LINK_C){
              // printf(" - A Switch App %i:%i",ModuleA,k);
              if(Units[ModuleA]->Sw[k]->app.module == anchor_A && Units[ModuleA]->Sw[k]->app.id == Rail){
                // printf("++++++\n");
                A.type = RAIL_LINK_S;
                typeA  = 'A';
                A.p = Units[ModuleA]->Sw[k];
                break;
              }
              // printf("\n");
            }
            else if(Units[ModuleA]->Sw[k]->str.type == RAIL_LINK_C){
              // printf(" - A Switch Str %i:%i",ModuleA,k);
              if(Units[ModuleA]->Sw[k]->str.module == anchor_A && Units[ModuleA]->Sw[k]->str.id == Rail){
                // printf("++++++\n");
                A.type = RAIL_LINK_S;
                typeA  = 'S';
                A.p = Units[ModuleA]->Sw[k];
                break;
              }
              // printf("\n");
            }
            else if(Units[ModuleA]->Sw[k]->div.type == RAIL_LINK_C){
              // printf(" - A Switch Div %i:%i",ModuleA,k);
              if(Units[ModuleA]->Sw[k]->div.module == anchor_A && Units[ModuleA]->Sw[k]->div.id == Rail){
                // printf("++++++\n");
                A.type = RAIL_LINK_S;
                typeA  = 'D';
                A.p = Units[ModuleA]->Sw[k];
                break;
              }
              // printf("\n");
            }
          }
        }
      }
    // - Find a msswitch
      if(!A.p){
        printf("Mayby a MSwitch, but not IMPLEMENTED!!\n");
        continue;
      }

    //Find Anchor B
    // - Find a block
      for(int k = 0;k<Units[ModuleB]->block_len;k++){
        if(Units[ModuleB]->B[k]){
          if(Units[ModuleB]->B[k]->next.type == RAIL_LINK_C){
          printf(" - B block Prev %i:%i",ModuleB,k);
          if(Units[ModuleB]->B[k]->next.module == anchor_B && Units[ModuleB]->B[k]->next.id == Rail){
            printf("++++++\n");
            B.type = RAIL_LINK_R;
            typeB  = 'N';
            B.p = Units[ModuleB]->B[k];
            break;
          }
          printf("\n");
          }
          else if(Units[ModuleB]->B[k]->prev.type == RAIL_LINK_C){
          printf(" - B block Prev %i:%i",ModuleB,k);
          if(Units[ModuleB]->B[k]->prev.module == anchor_B && Units[ModuleB]->B[k]->prev.id == Rail){
            printf("++++++\n");
            B.type = RAIL_LINK_R;
            typeB  = 'P';
            B.p = Units[ModuleB]->B[k];
            break;
          }
          printf("\n");
          }
        }
      }
    // - Find a Switch
      if(!B.p){
        for(int k = 0;k<Units[ModuleB]->switch_len;k++){
          if(Units[ModuleB]->Sw[k]){
            if(Units[ModuleB]->Sw[k]->app.type == RAIL_LINK_C){
              printf(" - B switch App %i:%i",ModuleB,k);
              if(Units[ModuleB]->Sw[k]->app.module == anchor_B && Units[ModuleB]->Sw[k]->app.id == Rail){
              printf("++++++\n");
              B.type = RAIL_LINK_S;
              typeB  = 'A';
              B.p = Units[ModuleB]->Sw[k];
              break;
              }
              printf("\n");
            }
            else if(Units[ModuleB]->Sw[k]->str.type == RAIL_LINK_C){
              printf(" - B switch Str %i:%i",ModuleB,k);
              if(Units[ModuleB]->Sw[k]->str.module == anchor_B && Units[ModuleB]->Sw[k]->str.id == Rail){
              printf("++++++\n");
              B.type = RAIL_LINK_S;
              typeB  = 'S';
              B.p = Units[ModuleB]->Sw[k];
              break;
              }
              printf("\n");
            }
            else if(Units[ModuleB]->Sw[k]->div.type == RAIL_LINK_C){
              printf(" - B switch Div %i:%i",ModuleB,k);
              if(Units[ModuleB]->Sw[k]->div.module == anchor_B && Units[ModuleB]->Sw[k]->div.id == Rail){
              printf("++++++\n");
              B.type = RAIL_LINK_S;
              typeB  = 'D';
              B.p = Units[ModuleB]->Sw[k];
              break;
              }
              printf("\n");
            }
          }
        }
      }
    // - Find a MSwitch
      if(!B.p){
        printf("Mayby a MSwitch, but not IMPLEMENTED!!\n");
        continue;
      }

    if(A.type == RAIL_LINK_R && B.type == RAIL_LINK_R){
      printf("Connecting R %i:%i <==> %i:%i R\n",((Block *)A.p)->module,((Block *)A.p)->id,((Block *)B.p)->module,((Block *)B.p)->id);
    }
    else if(A.type == RAIL_LINK_S && B.type == RAIL_LINK_R){
      printf("Connecting S %i:%i <==> %i:%i R\n",((Switch *)A.p)->module,((Switch *)A.p)->id,((Block *)B.p)->module,((Block *)B.p)->id);
    }
    else if(A.type == RAIL_LINK_R && B.type == RAIL_LINK_S){
      printf("Connecting R %i:%i <==> %i:%i S\n",((Block *)A.p)->module,((Block *)A.p)->id,((Switch *)B.p)->module,((Switch *)B.p)->id);
    }

    connected = TRUE;

    //Connect
    if(A.type == RAIL_LINK_R && B.type == RAIL_LINK_R){
      if(typeA == 'P'){
        ((Block *)A.p)->prev.module = ((Block *)B.p)->module;
        ((Block *)A.p)->prev.id    = ((Block *)B.p)->id;
        ((Block *)A.p)->prev.type   = RAIL_LINK_R;
        ((Block *)B.p)->next.module = ((Block *)A.p)->module;
        ((Block *)B.p)->next.id    = ((Block *)A.p)->id;
        ((Block *)B.p)->next.type   = RAIL_LINK_R;
      }
      else{
        ((Block *)A.p)->next.module = ((Block *)B.p)->module;
        ((Block *)A.p)->next.id    = ((Block *)B.p)->id;
        ((Block *)A.p)->next.type   = RAIL_LINK_R;
        ((Block *)B.p)->prev.module = ((Block *)A.p)->module;
        ((Block *)B.p)->prev.id    = ((Block *)A.p)->id;
        ((Block *)B.p)->prev.type   = RAIL_LINK_R;
      }
    }
    else if(A.type == RAIL_LINK_S && B.type == RAIL_LINK_R){
      if(typeB == 'N'){
        ((Block *)B.p)->next.module = ((Switch *)A.p)->module;
        ((Block *)B.p)->next.id    = ((Switch *)A.p)->id;
        ((Block *)B.p)->next.type   = (typeA == 'A') ? RAIL_LINK_S : RAIL_LINK_s;
      }
      else{
        ((Block *)B.p)->prev.module = ((Switch *)A.p)->module;
        ((Block *)B.p)->prev.id    = ((Switch *)A.p)->id;
        ((Block *)B.p)->prev.type   = (typeA == 'A') ? RAIL_LINK_S : RAIL_LINK_s;
      }

      if(typeA == 'A'){
        ((Switch *)A.p)->app.module = ((Block *)B.p)->module;
        ((Switch *)A.p)->app.id    = ((Block *)B.p)->id;
        ((Switch *)A.p)->app.type   = RAIL_LINK_R;
      }
      else if(typeA == 'S'){
        ((Switch *)A.p)->str.module = ((Block *)B.p)->module;
        ((Switch *)A.p)->str.id    = ((Block *)B.p)->id;
        ((Switch *)A.p)->str.type   = RAIL_LINK_R;
      }
      else if(typeA == 'D'){
        ((Switch *)A.p)->div.module = ((Block *)B.p)->module;
        ((Switch *)A.p)->div.id    = ((Block *)B.p)->id;
        ((Switch *)A.p)->div.type   = RAIL_LINK_R;
      }
    }
    else if(A.type == RAIL_LINK_R && B.type == RAIL_LINK_S){
      if(typeA == 'N'){
        ((Block *)A.p)->next.module = ((Switch *)B.p)->module;
        ((Block *)A.p)->next.id    = ((Switch *)B.p)->id;
        ((Block *)A.p)->next.type   = (typeB == 'A') ? RAIL_LINK_S : RAIL_LINK_s;
      }
      else{
        ((Block *)A.p)->prev.module = ((Switch *)B.p)->module;
        ((Block *)A.p)->prev.id    = ((Switch *)B.p)->id;
        ((Block *)A.p)->prev.type   = (typeB == 'A') ? RAIL_LINK_S : RAIL_LINK_s;
      }

      if(typeB == 'A'){
        ((Switch *)B.p)->app.module = ((Block *)A.p)->module;
        ((Switch *)B.p)->app.id    = ((Block *)A.p)->id;
        ((Switch *)B.p)->app.type   = RAIL_LINK_R;
      }
      else if(typeB == 'S'){
        ((Switch *)B.p)->str.module = ((Block *)A.p)->module;
        ((Switch *)B.p)->str.id    = ((Block *)A.p)->id;
        ((Switch *)B.p)->str.type   = RAIL_LINK_R;
      }
      else if(typeB == 'D'){
        ((Switch *)B.p)->div.module = ((Block *)A.p)->module;
        ((Switch *)B.p)->div.id    = ((Block *)A.p)->id;
        ((Switch *)B.p)->div.type   = RAIL_LINK_R;
      }
    }
  }

  if(connected && ModuleA && anchor_A && ModuleB && anchor_B){
    Units[ModuleA]->connection[anchor_A-1] = Units[ModuleB];
    Units[ModuleB]->connection[anchor_B-1] = Units[ModuleA];
  }

  return connected;
}

int connect_Algor(struct ConnectList * List){
  struct rail_link * R = 0;

  int value = 0;

  for(int i = 0;i<List->length;i++){
    if(!List->R_L[i]->p)
      continue;

    if(List->R_L[i]->type == 'R'){
      if(((Block *)List->R_L[i]->p)->next.type != RAIL_LINK_C && ((Block *)List->R_L[i]->p)->prev.type != RAIL_LINK_C) {
        value++;
        continue;
      }
      if(((Block *)List->R_L[i]->p)->blocked){
        printf("Found block %i:%i %i\t",((Block*)List->R_L[i]->p)->module,((Block*)List->R_L[i]->p)->id,((Block*)List->R_L[i]->p)->blocked);
        //Blocked block
        if(!R)
          R = List->R_L[i];
        else
        {
          _Bool connected = FALSE;
          char anchor_A = 0;
          char anchor_B = 0;

          int ModuleA = 0;
          int ModuleB = 0;
          if(R->type == 'R'){
            ModuleA = ((Block *)R->p)->module;
            ModuleB = ((Block *)List->R_L[i]->p)->module;

            if(((Block *)R->p)->next.type == RAIL_LINK_C){
              anchor_A = ((Block *)R->p)->next.module;
              anchor_B = ((Block *)List->R_L[i]->p)->prev.module;
            }
            else if(((Block *)R->p)->prev.type == RAIL_LINK_C){
              anchor_A = ((Block *)R->p)->prev.module;
              anchor_B = ((Block *)List->R_L[i]->p)->next.module;
            }

            connected = find_and_connect(ModuleA,anchor_A,ModuleB,anchor_B);
          }
          else if(R->type == 'S'){
            ModuleA = ((Switch *)R->p)->module;
            ModuleB = ((Block *)List->R_L[i]->p)->module;
            if(((Block *)List->R_L[i]->p)->next.type == RAIL_LINK_C){
              anchor_B = ((Block *)List->R_L[i]->p)->next.module;
            }
            else{
              anchor_B = ((Block *)List->R_L[i]->p)->prev.module;
            }

            if(((Switch *)R->p)->app.type == RAIL_LINK_C){
              anchor_A = ((Switch *)R->p)->app.module;
            }
            else if(((Switch *)R->p)->str.type == RAIL_LINK_C){
              anchor_A = ((Switch *)R->p)->str.module;
            }
            else if(((Switch *)R->p)->div.type == RAIL_LINK_C){
              anchor_A = ((Switch *)R->p)->div.module;
            } //End Switch approach type

            connected = find_and_connect(ModuleA,anchor_A,ModuleB,anchor_B);
          } // End Switch type

          if(connected){
            WS_Partial_Layout(ModuleA,ModuleB);
            connected = FALSE;
          }
        }
      }
    }
    else if(List->R_L[i]->type == 'S' && ((Switch *)List->R_L[i]->p)->Detection->blocked){
      //Blocked switch
      if(!R){
        R = List->R_L[i];
      }
    }
  }

  value = 0;
  int total = 0;

  for(int i = 0;i < unit_len;i++){
    if(!Units[i])
      continue;

    for(int j = 0;j<Units[i]->connections_len;j++){
      total++;
      if(Units[i]->connection[j]){
      value++;
      }
    }
  }

  if(value == total){
    SYS->modules_linked = 1;
  }

  return value;
}


void * rail_link_pointer(struct rail_link link){
  if(link.type == RAIL_LINK_R){
    return Units[link.module]->B[link.id];
  }
  else if(link.type == RAIL_LINK_S || link.type == RAIL_LINK_s){
    return Units[link.module]->Sw[link.id];
  }
  else if(link.type == RAIL_LINK_M || link.type == RAIL_LINK_m){
    return Units[link.module]->MSSw[link.id];
  }
  return 0;
}


void SIM_JoinModules(){
  printf("Ready to join modules\n");

  struct ConnectList List;
  List.length = 0;
  List.list_index = 8;
  List.R_L = _calloc(8, struct rail_link *);


  int i = 0;
  int x = 0;
  int max_j = init_connect_Algor(&List);
  int cur_j = max_j;
  int prev_j = max_j;
  while(SYS->modules_linked == 0){
    cur_j = connect_Algor(&List);
    if(i > 30){
      printf(" (%02i/%02i)\n",cur_j,max_j);
      i = 0;
      x++;
    }
    if(prev_j == cur_j){
      printf(".");
    }else{
      printf("+");

      char data[20];
      data[0] = 0x82;
      data[1] = cur_j;
      data[2] = max_j;
      int k = 3;
      for(int j = 0;j<unit_len;j++){
        if(Units[j]){
          data[k++] = j;
        }
      }
      ws_send_all(data,k,0x10);
    }
    i++;
    usleep(1000);
    prev_j = cur_j;

    if(i == 15){
    usleep(5000);
    if(x == 1){
      Units[20]->B[5]->blocked = 1;
      Units[25]->B[0]->blocked = 1;
      printf("\n1\n");
    }else if(x == 2){
      Units[25]->B[3]->blocked = 1;
      Units[22]->B[0]->blocked = 1;

      Units[20]->B[5]->blocked = 0;
      Units[25]->B[0]->blocked = 0;
      printf("\n2\n");
    }else if(x == 3){
      Units[22]->B[1]->blocked = 1;
      Units[26]->B[0]->blocked = 1;

      Units[25]->B[3]->blocked = 1;
      Units[22]->B[0]->blocked = 1;
      printf("\n3\n");
    }else if(x == 4){
      Units[26]->B[3]->blocked = 1;
      Units[21]->B[0]->blocked = 1;

      Units[22]->B[1]->blocked = 0;
      Units[26]->B[0]->blocked = 0;
      printf("\n4\n");
    }else if(x == 5){
      Units[21]->B[3]->blocked = 1;
      Units[23]->B[0]->blocked = 1;

      Units[26]->B[3]->blocked = 0;
      Units[21]->B[0]->blocked = 0;
      printf("\n5\n");
    }else if(x == 6){
      Units[23]->B[1]->blocked = 1;
      Units[20]->B[0]->blocked = 1;

      Units[21]->B[3]->blocked = 0;
      Units[23]->B[0]->blocked = 0;
      printf("\n6\n");
    }else if(x == 7){
      Units[23]->B[1]->blocked = 0;
      Units[20]->B[0]->blocked = 0;
      printf("\n7\n");
    }else if(x == 6){
      printf("\nend\n");
    }
    else if(x == 10){
      // _SYS_change(STATE_Modules_Coupled,1);
    }
    }
    //IF ALL JOINED
    //BREAK
  }
  
  Units[21]->B[0]->blocked = 0;
  Units[22]->B[1]->blocked = 0;

  for(int i = 0;i<List.length;i++){
    if(List.R_L[i]->type == 'S'){
      if(((Switch *)List.R_L[i]->p)->app.type == RAIL_LINK_C){
        ((Switch *)List.R_L[i]->p)->app.type = 0;
      }else if(((Switch *)List.R_L[i]->p)->str.type == RAIL_LINK_C){
        ((Switch *)List.R_L[i]->p)->str.type = 0;
      }else if(((Switch *)List.R_L[i]->p)->div.type == RAIL_LINK_C){
        ((Switch *)List.R_L[i]->p)->div.type = 0;
      }
    }else if(((Block *)List.R_L[i]->p)){
      if(((Block *)List.R_L[i]->p)->next.type == RAIL_LINK_C){
        ((Block *)List.R_L[i]->p)->next.type = 0;
      }else if(((Block *)List.R_L[i]->p)->prev.type == RAIL_LINK_C){
        ((Block *)List.R_L[i]->p)->prev.type = 0;
      }
    }

    _free(List.R_L[i]);
  }
  _free(List.R_L);

  for(uint8_t i = 0; i < unit_len; i++){
    if(!Units[i])
      continue;

    for(uint8_t j = 0; j < Units[i]->block_len; j++){
      if(Units[i]->B[j]){
        Units[i]->B[j]->blocked = 0;
      }
    }
  }

  // WS_Track_Layout();

}

void SIM_Connect_Rail_links(){
  // add pointer to the rail_link
  for(int m = 0; m<unit_len; m++){
    if(!Units[m]){
      continue;
    }

    printf("LINKING UNIT %i\n", m);

    Unit * tU = Units[m];

    for(int i = 0; i<tU->block_len; i++){
      if(!tU->B[i]){
        continue;
      }

      Block * tB = tU->B[i];

      tB->next.p = rail_link_pointer(tB->next);
      tB->prev.p = rail_link_pointer(tB->prev);
    }

    for(int i = 0; i<tU->switch_len; i++){
      if(!tU->Sw[i]){
        continue;
      }

      Switch * tSw = tU->Sw[i];

      tSw->app.p = rail_link_pointer(tSw->app);
      tSw->str.p = rail_link_pointer(tSw->str);
      tSw->div.p = rail_link_pointer(tSw->div);
    }

    for(int i = 0; i<tU->msswitch_len; i++){
      if(!tU->MSSw[i]){
        continue;
      }

      MSSwitch * tMSSw = tU->MSSw[i];

      for(int s = 0; s < tMSSw->state_len; s++){
        tMSSw->sideA[s].p = rail_link_pointer(tMSSw->sideA[s]);
        tMSSw->sideB[s].p = rail_link_pointer(tMSSw->sideB[s]);
      }
    }
  }
}
